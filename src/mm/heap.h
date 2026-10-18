#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

/* 16-byte aligned memory, or NULL when size is 0 or memory runs out. Call after paging_init. */
void* kmalloc(size_t size);
void kfree(void* ptr);

/* Bytes handed out by kmalloc, and bytes of address space mapped for the heap. */
void heap_stats(uint32_t* used, uint32_t* mapped);

#endif
