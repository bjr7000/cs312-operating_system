#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "filesys/off_t.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "vm/spt.h"

#define PHYS_BASE 0xc0000000
#define STACK_BOTTOM 0x8048000
#define INVAILD -1
#define VAILD 1
static void syscall_handler (struct intr_frame *);
struct lock file_lock;

struct file
  {
    struct inode *inode;        /* File's inode. */
    off_t pos;                  /* Current position. */
    bool deny_write;            /* Has file_deny_write() been called? */
  };

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  lock_init(&file_lock);
}

int
check_address_vaild (void *address)
{  
  int check = (address < STACK_BOTTOM || address >= PHYS_BASE || address == 0);
  if (check) return INVAILD; 
  else return VAILD;
}
void
pop_arguments (int *sp, int argc, int *argv)
{

  for (int i = 0; i < argc; ++i)
  {
    if(check_address_vaild (sp + 1 + i) == INVAILD) 
    {
      syscall_exit(INVAILD);
    }
    else {
      argv[i] = *(sp + 1 + i);
    } 
  }  
}
void check_syscall_ID_vaild (int syscall_ID)
{
  if (syscall_ID < 0 || syscall_ID >= 15) syscall_exit (INVAILD);
}
static void
syscall_handler (struct intr_frame *f) 
{
  if (check_address_vaild(f->esp) == INVAILD) syscall_exit(INVAILD);

  int argv[3], syscall_ID;
  syscall_ID = *(int *)f->esp;
  //check_syscall_ID_vaild (syscall_ID);
  
  thread_current()->esp = f->esp;
  switch (syscall_ID)
  {
    case SYS_HALT:
      syscall_halt ();
      break;
    case SYS_EXIT:
      pop_arguments (f->esp, 1, argv);
      syscall_exit (argv[0]);
      break;
    case SYS_EXEC:
      pop_arguments (f->esp, 1, argv);      
      f->eax = syscall_exec (argv[0]);
      break;
    case SYS_WAIT:
      pop_arguments (f->esp, 1, argv);
      f->eax = syscall_wait (argv[0]);
      break;
    case SYS_CREATE:
      pop_arguments (f->esp, 2, argv);      
      f->eax = syscall_create (argv[0], argv[1]);
      break;
    case SYS_REMOVE:
      pop_arguments (f->esp, 1, argv);
      f->eax = syscall_remove (argv[0]);
      break;
    case SYS_OPEN:
      pop_arguments (f->esp, 1, argv);
      f->eax = syscall_open (argv[0]);
      break;
    case SYS_FILESIZE:
      pop_arguments (f->esp, 1, argv);
      f->eax = syscall_filesize (argv[0]);
      break;
    case SYS_READ:
      pop_arguments (f->esp, 3, argv);
      f->eax = syscall_read (argv[0], argv[1], argv[2]);
      break;
    case SYS_WRITE:
      pop_arguments (f->esp, 3, argv);
      if (check_address_vaild ((void *)argv[1]) == INVAILD) syscall_exit (INVAILD);
      f->eax = syscall_write ((int)argv[0], (const void*) argv[1], (unsigned) argv[2]);
      break;
    case SYS_SEEK:
      pop_arguments (f->esp, 2, argv);
      syscall_seek (argv[0], argv[1]);
      break;
    case SYS_TELL:
      pop_arguments (f->esp, 1, argv);
      f->eax = syscall_tell (argv[0]);
      break;
    case SYS_CLOSE:
      pop_arguments (f->esp, 1, argv);
      syscall_close (argv[0]);
      break;
    case SYS_MMAP:
      pop_arguments (f->esp, 2, argv);
      f->eax = syscall_mmap((int) argv[0], (void *) argv[1]);
      break;
    case SYS_MUNMAP:
      pop_arguments (f->esp, &argv[0], 1);
      syscall_munmap((int) argv[0]);
      break;
    default:
      syscall_exit(INVAILD);
      break;
  }
}
void
syscall_halt (void)
{
  shutdown_power_off ();
}

void
syscall_exit (int status)
{
  struct thread *t = thread_current ();
  t->exit_status = status;
  printf("%s: exit(%d)\n", t->name, status);
  thread_exit ();
}

int
syscall_exec (const char* command)
{
  return process_execute(command);
}

int
syscall_wait (int pid)
{
  return process_wait (pid);
}

int
syscall_create (const char *file, unsigned initial_size)
{
  if (file == NULL) syscall_exit(INVAILD);

  return filesys_create(file, initial_size);
}

int syscall_remove (const char *file)
{
  if (file == NULL) syscall_exit(INVAILD);

  return filesys_remove(file);
}

int syscall_open (const char *file)
{

  if (file == NULL) syscall_exit(INVAILD);
  lock_acquire (&file_lock);
  struct file* _file = filesys_open(file);
  int fd = -1;
  if(_file != NULL)
  {
    for(fd = 3; thread_current()->fd_list[fd] != NULL; fd++) {}
    if (!strcmp(thread_current()->name, file)) file_deny_write(_file);

    thread_current()->fd_list[fd] = _file;
  }
  lock_release (&file_lock);
  return fd;
}

int syscall_filesize (int fd)
{
  if (thread_current()->fd_list[fd] == NULL) syscall_exit(INVAILD);

  return file_length(thread_current()->fd_list[fd]);
}

int syscall_read (int fd, void *buffer, unsigned size)
{
  if (check_address_vaild(buffer) == INVAILD) syscall_exit(INVAILD);

  int bytes_read;
  if (fd == 0)
  {
      lock_acquire(&file_lock);
      bytes_read = 0;
      while (bytes_read < size && ((char*)buffer)[bytes_read] != '\0') bytes_read++;
      lock_release(&file_lock);
  }
  else if (fd > 2)
  {
      struct file* file = thread_current()->fd_list[fd];
      if (file == NULL) syscall_exit(INVAILD);

      lock_acquire(&file_lock);
      bytes_read = file_read(file, buffer, size);
      lock_release(&file_lock);
  }
  else syscall_exit(INVAILD);
  return bytes_read;
}

int syscall_write (int fd, const void *buffer, unsigned size)
{
  if (check_address_vaild(buffer) == INVAILD) syscall_exit(INVAILD);

  int bytes_written;
  if (fd == 1)
  {
    lock_acquire (&file_lock);
    putbuf(buffer, size);
    bytes_written = size;
    lock_release (&file_lock);
  } else if (fd > 2)
  {
    struct file *file = thread_current ()->fd_list[fd];
    if (file == NULL) syscall_exit (INVAILD);
    if (file->deny_write) file_deny_write(file);

    lock_acquire (&file_lock); 
    bytes_written = file_write (file, buffer, size);
    lock_release (&file_lock);
  }
  else syscall_exit(INVAILD);

  return bytes_written;
}

void syscall_seek (int fd, unsigned position)
{
  if (thread_current()->fd_list[fd] == NULL) syscall_exit(INVAILD);

  file_seek(thread_current()->fd_list[fd], position);
}

unsigned syscall_tell (int fd)
{
  if (thread_current()->fd_list[fd] == NULL) syscall_exit(INVAILD);

  return file_tell(thread_current()->fd_list[fd]);
}

void syscall_close (int fd)
{
  struct file *file = thread_current()->fd_list[fd];
  
  if (file == NULL)
    syscall_exit(INVAILD);

  thread_current()->fd_list[fd] = NULL;
  file_close(file);
}

int syscall_mmap (int fd, void *address)
{
  struct file *file = thread_current()->fd_list[fd];
  if (file == NULL || address == NULL || (int) address % PGSIZE != 0) syscall_exit(INVAILD);

  lock_acquire(&file_lock);

  struct file *file_reopened = file_reopen(file);
  if(file_reopened == NULL) 
  {
    lock_release(&file_lock);
    syscall_exit(INVAILD);
  }
  struct mmf *mmf = init_mmf(thread_current()->mmf_id++, file_reopened, address);
  if (mmf == NULL)
  {
    lock_release(&file_lock);
    syscall_exit(INVAILD);
  }

  lock_release(&file_lock);

  return mmf->mmf_id;
}

void syscall_munmap (int mmf_id)
{
  struct mmf *mmf = vm_get_mmf(mmf_id);
  if (mmf == NULL) return;

  lock_acquire(&file_lock);

  for (off_t offset = 0; offset < file_length (mmf->file); offset += PGSIZE)
  {
    struct spt *s = vm_get_spt (&thread_current()->spt, mmf->user_page);
    //check file is dirtygit

    //if file is dirty, write it. else just delete

    if(pagedir_is_dirty (thread_current()->pagedir, mmf->user_page))
    {
      file_write_at (s->file, pagedir_get_page(thread_current()->pagedir, mmf->user_page), s->read_bytes, s->ofs);
    }

    vm_spt_page_delete (&thread_current()->spt, s);
    mmf->user_page += PGSIZE;
  }

  for (struct list_elem *elem = list_begin(&thread_current()->mmf_list); elem != list_end(&thread_current()->mmf_list); elem = list_next(elem))
  {
    if(list_entry(elem, struct mmf, mmf_elem)->mmf_id == mmf_id) 
    {
      list_remove(elem);
      break;
    }
  }

  lock_release(&file_lock);

}
