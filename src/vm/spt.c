#include "vm/spt.h"

init_spt(struct hash *spt, void *user_page, void *kernel_page)
{

}

init_spt_with_zero(struct hash *spt, void *user_page) 
{

}

init_spt_for_frame(struct hash *spt, void *user_page, void *kernel_page)
{

}

init_spt_for_file(struct hash *spt, void *user_page, struct file *file, off_t ofs, uint32_t read_bytes, uint32_t zero_bytes, bool writable) 
{

}

vm_get_spt(struct hash *spt, void *user_page)
{

}

vm_page_delete(struct hash *spt, struct spt *s)
[

]