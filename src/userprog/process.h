#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "vm/frame.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

int args_parse(char *command_line, char **argv);
void args_push (int argc, char **argv, void **esp);
#endif /* userprog/process.h */
