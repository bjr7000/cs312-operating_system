#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "threads/malloc.h"

struct frame
{
    void *kernel_page;
    void *user_page;
    struct thread *t;
    struct list_elem list_elem;
};

void frame_init();
void *vm_get_frame(enum palloc_flags, void *);
void vm_free_frame (void *);

#endif
