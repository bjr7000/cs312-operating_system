#include "vm/frame.h"
#include "threads/synch.h"

static struct list frame_table;
static struct list frame_lock;
void alloc_frame(void *user_page, void *kernel_page)
{
    struct frame *f;
    f = (struct frame*)malloc(sizeof *f);
    f->kernel_page = kernel_page;
    f->user_page = user_page;
    f->t = thread_current();
    list_push_back(&frame_table, &f->list_elem);
}
void free_frame(struct frame *f)
{
    list_remove(&f->list_elem);
    palloc_free_page(f->kernel_page);
    pagedir_clear_page(f->t->pagedir, f->user_page);
    free(f);
}
void vm_evict_frame()
{

}

void frame_init()
{
    list_init (&frame_table);
    lock_init (&frame_lock);
}

void *vm_get_frame(enum palloc_flags flags, void *user_page)
{
    void *kernel_page;
    lock_acquire(&frame_lock);
    kernel_page = palloc_get_page(flags);
    if(kernel_page == NULL)
    {
        vm_evict_frame();
        kernel_page = palloc_get_page(flags);
    }
    alloc_frame(user_page, kernel_page);
    lock_release(&frame_lock);
    return kernel_page;
}


void *vm_free_frame (void *kernel_page)
{
    struct frame *f;
    lock_acquire (&frame_lock);
    
    for (list_elem elem = list_begin(&frame_table); elem != list_end(&frame_table); elem = list_next(elem))
    {
        void *_f = list_entry(elem, struct frame, list_elem);
        if (_f->kernel_page == kernel_page) 
        {
            f = _f;
            break;
        }
    }

    if(f == NULL) syscall_exit(-1);

    free_frame(f);

    lock_release(&frame_lock);
}



