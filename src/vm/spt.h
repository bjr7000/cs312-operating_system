#ifndef VM_SPT_H
#define VM_SPT_H

#include <stdbool.h>
#include <hash.h>
#include "filesys/file.h"
#include "filesys/off_t.h"

//spt status
#define PAGE_ZERO 0
#define PAGE_FRAME 1
#define PAGE_FILE 2
#define PAGE_SWAP 3

struct spt
{
    void *user_page;
    void *kernel_page;

    struct hash_elem hash_elem;

    int status;

    struct file *file; 
    off_t ofs; 
    uint32_t read_bytes;
    uint32_t zero_bytes;
    bool writable;
    int swap_index;
};

void init_spt(struct hash *, void *, void *);
void init_spt_with_zero(struct hash *, void *);
void init_spt_for_frame(struct hash *, void *, void *);
void init_spt_for_file(struct hash *, void *, struct file *, off_t, uint32_t, uint32_t, bool);
struct spt *vm_get_spt(struct hash *, void *);
void vm_spt_page_delete(struct hash *, struct spt *);
unsigned spt_hash_bytes (const struct hash_elem *elem, void *aux);
bool spt_compare (const struct hash_elem *s_1, const struct hash_elem *s_2, void *aux);
bool load_page (struct hash *spt, void *user_page);
void page_destroy(struct hash_elem *elem, void *aux);
void destroy_spt(struct hash *spt);
#endif
 