#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#define PHYS_BASE 0xc0000000
#define STACK_BOTTOM 0x8048000
#define INVAILD -1
#define VAILD 1
static void syscall_handler (struct intr_frame *);
struct lock file_lock;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  lock_init(&file_lock);
}

int
check_address_vaild (void *address)
{
  //printf("check_address\n");
  
  int check = (address < STACK_BOTTOM || address >= PHYS_BASE || address == 0);
  if (check)
  {
    //printf("check_address-return\n");
    return INVAILD;
  } 

  else
  {
    //printf("check_address-return\n");
    return VAILD;
  } 
}
void
pop_arguments (int *sp, int argc, int *argv)
{
  //printf("pop_arguments\n");
  //int *_sp = sp + 1;
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
  //printf("pop_arguments-end\n");
}
void check_syscall_ID_vaild (int syscall_ID)
{
  if (syscall_ID < 0 || syscall_ID >= 15) syscall_exit (INVAILD);
}
static void
syscall_handler (struct intr_frame *f) 
{
  //printf("syscall_handler\n");
  if (check_address_vaild(f->esp) == INVAILD) syscall_exit(INVAILD);

  int argv[3], syscall_ID;
  syscall_ID = *(int *)f->esp;
  //printf("%d\n", syscall_ID);
  check_syscall_ID_vaild (syscall_ID);
  
  thread_current()->esp = f->esp;
  //printf("%d\n", thread_current()->esp);
  switch (syscall_ID)
  {
    case SYS_HALT:
      //printf("call syscall_halt\n");
      syscall_halt ();
      break;
    case SYS_EXIT:
      //printf("call syscall_exit\n");
      pop_arguments (f->esp, 1, argv);
      syscall_exit (argv[0]);
      break;
    case SYS_EXEC:
      //hex_dump((int*)f->esp, (int*)f->esp, 0x8048000 - *(int*)f->esp, true);
      pop_arguments (f->esp, 1, argv);
      //printf("call syscall_exec\n");
      
      f->eax = syscall_exec (argv[0]);
      break;
    case SYS_WAIT:
      //hex_dump((int*)f->esp, (int*)f->esp, 0x8048000 - *(int*)f->esp, true);
      pop_arguments (f->esp, 1, argv);
      //printf("call syscall_wait\n");
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
      //printf("call syscall_write\n");
      pop_arguments (f->esp, 3, argv);
      //printf("pop\n");
      if (check_address_vaild ((void *)argv[1]) == INVAILD) 
        {
          //printf("syscall_exit call\n");
          syscall_exit (INVAILD);
        }
        
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
  }
  //printf("syscall_end\n");
}
void
syscall_halt (void)
{
  shutdown_power_off ();
}

void
syscall_exit (int status)
{
  //printf("syscall_exit\n");
  struct thread *t = thread_current ();
  t->exit_status = status;
  printf("%s: exit(%d)\n", t->name, status);
  thread_exit ();
  //printf("syscall_exit-end\n");
}

int
syscall_exec (const char* command)
{
  //printf("syscall_exec\n");
  int pid = process_execute (command);
  if (pid == -1) 
  {
    //printf("syscall_exec-end\n");
    return -1;
  }
  //printf("syscall_exec-end\n");
  return pid;
}

int
syscall_wait (int pid)
{
  //printf("syscall_wait\n");
  return process_wait (pid);
}

int
syscall_create (const char *file, unsigned initial_size)
{
  return 0;
}

int syscall_remove (const char *file)
{
  return 0;
}

int syscall_open (const char *file)
{
  return 0;
}
int syscall_filesize (int fd)
{
  return 0;
}
int syscall_read (int fd, void *buffer, unsigned size)
{
  return 0;
}
int syscall_write (int fd, const void *buffer, unsigned size)
{
  int bytes_written = -1;
  if (fd == 1)
  {
    lock_acquire (&file_lock);
    putbuf(buffer, size);
    bytes_written = size;
    lock_release (&file_lock);
  } else if (fd > 2){
    struct file *file = thread_current ()->fd_list[fd];
    if (file == NULL) {
      syscall_exit (INVAILD);
    }

    lock_acquire (&file_lock);
    bytes_written = file_write (file, buffer, size);
    lock_release (&file_lock);
  }

  return bytes_written;
}
void syscall_seek (int fd, unsigned position)
{

}
unsigned syscall_tell (int fd)
{
  return 0;
}
void syscall_close (int fd)
{
  struct file *file = thread_current()->fd_list[fd];
  
  if (file == NULL)
    syscall_exit(INVAILD);

  thread_current()->fd_list[fd] = NULL;
  file_close(file);
}

