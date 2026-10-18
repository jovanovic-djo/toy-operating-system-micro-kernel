#ifndef PMM_H
#define PMM_H

#include <stdint.h>

#define PAGE_SIZE 4096

/* Build the free page bitmap from the Multiboot memory map. Call after multiboot_init. */
void pmm_init(void);

/* Physical address of a free 4 KiB page, or 0 when memory is exhausted. */
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t addr);

uint32_t pmm_free_pages(void);
uint32_t pmm_total_pages(void); /* usable RAM below KERNEL_MEMORY_END, in pages */
uint32_t pmm_memory_end(void);  /* end of the highest usable page */

#endif
