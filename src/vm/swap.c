#include "vm/swap.h"

bool is_valid_swap_index (int swap_index);

void init_swap_table()
{
    swap_disk = block_get_role(BLOCK_SWAP);
    swap_table = bitmap_create(block_size(swap_disk) / PGSIZE * BLOCK_SECTOR_SIZE);
    bitmap_set_all(swap_table, true);
    lock_init(&swap_lock);
}

void swap_in(struct spt *s, void *vaddr)
{
    lock_acquire(&swap_lock);
    if (!is_valid_swap_index(s->swap_index) || bitmap_test(swap_table, s->swap_index) == true) syscall_exit(-1);

    bitmap_set(swap_table, s->swap_index, true);
    lock_release(&swap_lock);

    for (int i = 0; i < PGSIZE / BLOCK_SECTOR_SIZE; i++)
    {
        //read the slot to address
        block_read(swap_disk, s->swap_index * PGSIZE / BLOCK_SECTOR_SIZE + i, vaddr + i * BLOCK_SECTOR_SIZE);
    }
}

int swap_out(void *vaddr)
{
    lock_acquire(&swap_lock);
    int swap_index = bitmap_scan_and_flip(swap_table, 0, 1, true);
    lock_release(&swap_lock);

    for (int i = 0; i < PGSIZE / BLOCK_SECTOR_SIZE; i++)
    {
        block_write(swap_disk, swap_index * PGSIZE / BLOCK_SECTOR_SIZE + i, vaddr + i * BLOCK_SECTOR_SIZE);
    }
    return swap_index;
}



bool is_valid_swap_index (int swap_index)
{
    if (swap_index >= 0 && swap_index <= bitmap_size(swap_table)) return true;
    else return false;
}