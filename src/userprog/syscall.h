#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
int check_address_vaild (void *address);
void pop_arguments (int *sp, int argc, int *argv);
void check_syscall_ID_vaild (int syscall_ID);

void syscall_halt (void);
void syscall_exit (int status);
int syscall_exec (const char *cmd_line);
int syscall_wait (int pid);
int syscall_create (const char *file, unsigned initial_size);
int syscall_remove (const char *file);
int syscall_open (const char *file);
int syscall_filesize (int fd);
int syscall_read (int fd, void *buffer, unsigned size);
int syscall_write (int fd, const void *buffer, unsigned size);
void syscall_seek (int fd, unsigned position);
unsigned syscall_tell (int fd);
void syscall_close (int fd);
int syscall_mmap(int fd, void *address);
void syscall_munmap(int mmf_if);


#endif /* userprog/syscall.h */
