#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <bitmap.h>
#include "devices/block.h"
#include "threads/vaddr.h"
#include "vm/spt.h"
#include "threads/synch.h"

static struct bitmap *swap_table;
static struct block *swap_disk;
static struct lock swap_lock;

void init_swap_table();
void swap_in(struct spt *, void *);
int swap_out(void *);
#endif
