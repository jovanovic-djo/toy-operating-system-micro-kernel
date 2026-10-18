#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PAGE_PRESENT 0x1
#define PAGE_WRITE   0x2
#define PAGE_USER    0x4

/*
 * Identity-map the kernel's RAM except page 0 (so NULL dereferences fault),
 * create the heap's page tables and turn paging on. Call after pmm_init.
 */
void paging_init(void);

/* Map one 4 KiB page in the kernel's page directory. Returns 0 if a page table could not be allocated. */
int paging_map(uint32_t virt, uint32_t phys, uint32_t flags);

/*
 * Address spaces for processes: a new directory shares all kernel mappings,
 * and the user range (USER_START - USER_END) is private to it.
 */
uint32_t* paging_create_directory(void);              /* NULL when out of memory */
void paging_destroy_directory(uint32_t* dir);         /* frees its user pages too; must not be active */
int paging_map_in(uint32_t* dir, uint32_t virt, uint32_t phys, uint32_t flags);
void paging_switch(uint32_t* dir);                    /* NULL means the kernel's directory */

/* True if [addr, addr + len) is user memory mapped in dir (and writable, if write is set). */
int paging_user_accessible(uint32_t* dir, uint32_t addr, uint32_t len, int write);

#endif
