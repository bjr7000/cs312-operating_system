#include "vm/spt.h"
#include "vm/frame.h"
#include "threads/thread.h"
#include <string.h>
#include "threads/vaddr.h"
#include "vm/swap.h"
extern struct lock file_lock;

void set_spt_with_zero(struct hash *spt, void *user_page) 
{
    struct spt *s;
    s = (struct spt *)malloc(sizeof *s);
    s->user_page = user_page;
    s->kernel_page = NULL;
    s->status = PAGE_ZERO;
    s->file = NULL;
    s->writable = true;
    hash_insert(spt, &s->hash_elem);
}

void set_spt_for_frame(struct hash *spt, void *user_page, void *kernel_page)
{
    struct spt *s;
    s = (struct spt *) malloc(sizeof *s);
    s->user_page = user_page;
    s->kernel_page = kernel_page;
    s->status = PAGE_FRAME;
    s->file = NULL;
    s->writable = true;
    hash_insert(spt, &s->hash_elem);
}

void set_spt_for_file(struct hash *spt, void *user_page, struct file *_file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{
    struct spt *s;
    s = (struct spt *) malloc(sizeof *s);
    s->user_page = user_page;
    s->kernel_page = NULL;
    s->status = PAGE_FILE;
    s->file = _file;
    s->ofs = ofs;
    s->read_bytes = read_bytes;
    s->zero_bytes = zero_bytes;
    s->writable = writable;
    hash_insert(spt, &s->hash_elem);
}

struct spt *vm_get_spt(struct hash *spt, void *user_page)
{
    struct spt s;
    s.user_page = user_page;
    struct hash_elem *elem = hash_find(spt, &s.hash_elem);
    if (elem == NULL) return NULL;
    else return hash_entry(elem, struct spt, hash_elem);
}

void vm_spt_page_delete(struct hash *spt, struct spt *s)
{
    hash_delete(spt, &s->hash_elem);
    free(s);
}

bool load_page (struct hash *spt, void *user_page)
{
    //printf("load_page\n");
    struct spt *s = vm_get_spt(spt, user_page);
    if (s == NULL)
    {   
        syscall_exit(-1); // Not found in the supplemental page table.
    }
    void *kernel_page = vm_get_frame(PAL_USER, user_page);
    if (kernel_page == NULL)
    {
        syscall_exit(-1); // Failed to allocate a physical frame.
    }


    // Ensure file_lock
    bool lock_acquired = false;
    switch (s->status) {
        case PAGE_ZERO:
            // Fill the page in zero
            memset(kernel_page, 0, PGSIZE);
            break;

        case PAGE_SWAP:
            // Load the page from the swap disk.
            swap_in(s, kernel_page);
            break;

        case PAGE_FILE:
            
            if (!lock_held_by_current_thread(&file_lock)) 
            {
                lock_acquire(&file_lock);
                lock_acquired = true;
            }

            // Read file content into the allocated page.
            if (file_read_at(s->file, kernel_page, s->read_bytes, s->ofs) != s->read_bytes) 
            {
                vm_free_frame(kernel_page);
                if (lock_acquired) lock_release(&file_lock);
                syscall_exit(-1); // Failed to read
            }

            // Fill zero to rest of the page.
            memset(kernel_page + s->read_bytes, 0, s->zero_bytes);

            if (lock_acquired)
                lock_release(&file_lock);
            break;

        default:
            vm_free_frame(kernel_page);
            syscall_exit(-1);
    }

    // Map the user page to the newly allocated kernel page.
    uint32_t *pagedir = thread_current()->pagedir;
    if (!pagedir_set_page(pagedir, user_page, kernel_page, s->writable)) {
        vm_free_frame(kernel_page);
        syscall_exit(-1); // Failed to set up the page in the page directory.
    }

    // Update supplemental page table entry.
    s->kernel_page = kernel_page;
    s->status = PAGE_FRAME;
    return true;
}
void destroy_spt(struct hash *spt)
{
    hash_destroy(spt, page_destroy);
}
void page_destroy(struct hash_elem *elem, void *aux)
{
    struct spt *s;
    s = hash_entry(elem, struct spt, hash_elem);
    free(s);
}
unsigned spt_hash_bytes (const struct hash_elem *elem, void *aux)
{
    struct spt *s = hash_entry(elem, struct spt, hash_elem);
    return hash_bytes(&s->user_page, sizeof(s->kernel_page));
}

bool spt_compare (const struct hash_elem *s_1, const struct hash_elem *s_2, void *aux)
{
    return hash_entry(s_1, struct spt, hash_elem)->user_page < hash_entry(s_2, struct spt, hash_elem)->user_page;
}