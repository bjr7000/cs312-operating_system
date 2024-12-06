#ifndef VM_SPT_H
#define VM_SPT_H

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