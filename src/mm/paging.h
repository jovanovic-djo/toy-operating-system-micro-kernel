#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2

/*
 * Identity-map all RAM except page 0 (so NULL dereferences fault) and turn
 * paging on. Call after pmm_init.
 */
void paging_init(void);

/* Map one 4 KiB page. Returns 0 if a page table could not be allocated. */
int paging_map(uint32_t virt, uint32_t phys, uint32_t flags);

#endif
