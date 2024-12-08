#include "vm/spt.h"

void set_spt_idle(struct hash *spt, void *user_page, void *kernel_page)
{
    struct spt *s;
    s = (struct spt *)malloc(sizeof *s);
    s->user_page = user_page;
    s->kernel_page = kernel_page;
    s->status = PAGE_FRAME;
    hash_insert(spt, &s->hash_elem);
}

void set_spt_with_zero(struct hash *spt, void *user_page) 
{
    struct spt *s;
    s = (struct spt *)malloc(sizeof *s);
    s->user_page = user_page;
    s->kernel_page = NULL;
    s->status = PAGE_ZERO;
    s->file = NULL;
    s->writable = true;
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

void set_spt_for_file(struct hash *spt, void *user_page, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{
    struct spt *s;
    s = (struct spt *) malloc(sizeof *s);
    s->user_page = user_page;
    s->kernel_page = NULL;
    s->status = PAGE_FILE;
    s->file = file;
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

unsigned spt_hash_bytes (const struct hash_elem *elem, void *aux)
{
    struct spt *s = hash_entry(elem, struct spt, hash_elem);
    return hash_bytes(&s->user_page, sizeof(s->kernel_page));
}

bool spt_compare (const struct hash_elem *s_1, const struct hash_elem *s_2, void *aux)
{
    return hash_entry(s_1, struct spt, hash_elem)->user_page < hash_entry(s_2, struct spt, hash_elem)->user_page;
}