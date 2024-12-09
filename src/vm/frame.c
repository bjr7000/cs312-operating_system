#include "vm/frame.h"
#include "threads/synch.h"
#include "vm/spt.h"
#include "vm/swap.h"

static struct list frame_table;
static struct list frame_lock;
static struct frame *cur;

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
    ASSERT(lock_held_by_current_thread(&frame_lock));
    struct frame *f = cur;
    struct spt *s;
    size_t iterations = 0;
    // BEGIN: Find page to evict 
    while (iterations < list_size(&frame_table)) {
        // Clear the accessed bit for the current frame
        if (f != NULL) pagedir_set_accessed(f->t->pagedir, f->user_page, false);
        
         // Found a frame to evict
        // Move to the next frame
        f = (cur == NULL || list_next(&cur->list_elem) == list_end(&frame_table)) ? 
            list_entry(list_begin(&frame_table), struct frame, list_elem) :
            list_next(f);

        // Check if the current frame can be evicted
        if (!pagedir_is_accessed(f->t->pagedir, f->user_page)) break;
        iterations++;
    }
    
    // If no frame is selected
    ASSERT(f != NULL);
    
    // Update current checking frame 
    cur = f;
    // END: Find page to evict 

    // Evict the selected page
    s = vm_get_spt(&thread_current()->spt, f->user_page);
    ASSERT(s != NULL);

    // Update the page's status to PAGE_SWAP and swap it out
    s->status = PAGE_SWAP;
    s->swap_index = swap_out(f->kernel_page);
    lock_release(&frame_lock);
    vm_free_frame(f->kernel_page);
    lock_acquire(&frame_lock);
}

void frame_init()
{
    list_init (&frame_table);
    lock_init (&frame_lock);
    cur = NULL;
}

void *vm_get_frame(enum palloc_flags flags, void *user_page)
{
    void *kernel_page;
    if(!lock_held_by_current_thread (&frame_lock))
        lock_acquire(&frame_lock);
    kernel_page = palloc_get_page(flags);
    if(kernel_page == NULL)
    {
        vm_evict_frame();
        kernel_page = palloc_get_page(flags);
    }
    if (kernel_page == NULL) return NULL;

    alloc_frame(user_page, kernel_page);
    lock_release(&frame_lock);
    return kernel_page;
}


void vm_free_frame (void *kernel_page)
{
    struct frame *f = NULL;
    lock_acquire (&frame_lock);
    
    for (struct list_elem *elem = list_begin(&frame_table); elem != list_end(&frame_table); elem = list_next(elem))
    {
        struct frame *_f = list_entry(elem, struct frame, list_elem);
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



