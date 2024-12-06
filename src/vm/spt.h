#ifndef VM_SPT_H
#define VM_SPT_H

#include <stdbool.h>
#include <stdio.h>
#include <hash.h>
#include "filesys/hile.h"
#include "filesys/off_t.h"

//spt status
#define PAGE_ZERO 0
#define PAGE_FRAME 1
#define PAGE_FILE 2

struct spt 
{
    void *upage;
    void *kpage;

    struct hash_elem hash_elem;

    int status;

    struct file *file; 
    off_t ofs; 
    uint32_t read_bytes;
    uint32_t zero_bytes;
    int swap_id;
}

void init_spt(struct hash *, void *, void *);
void init_spt_with_zero(struct hash *, void *);
void init_spt_for_frame(struct hash *, void *, void *);
void init_spt_for_file(struct hash *, void *, struct file *, off_t, uint32_t, uint32_t, bool);
struct spt *vm_get_spt(struct hash *, void *)
void vm_spt_page_delete(struct hash *, struct spt *);

#endif
 