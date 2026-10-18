#include <stdint.h>
#include "paging.h"
#include "pmm.h"
#include "kprintf.h"
#include "string.h"
#include "layout.h"

#define ENTRIES      1024
#define TABLE_SPAN   (ENTRIES * PAGE_SIZE) /* 4 MiB covered by one page table */
#define CR0_PAGING   0x80000000
#define ADDR_MASK    0xFFFFF000

/*
 * Page tables and process page directories come from the page allocator and
 * are reached through their physical address, which works because the
 * kernel's RAM is identity-mapped.
 */
static uint32_t kernel_directory[ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static uint32_t* active_directory;

/* The page table covering virt, allocated on first use; NULL if out of memory. */
static uint32_t* page_table(uint32_t* dir, uint32_t virt, uint32_t flags) {
    uint32_t* pde = &dir[virt >> 22];

    if (!(*pde & PAGE_PRESENT)) {
        uint32_t table = pmm_alloc_page();
        if (!table)
            return 0;
        memset((void*)table, 0, PAGE_SIZE);
        *pde = table | PAGE_PRESENT | PAGE_WRITE;
    }
    /* User pages need the user bit on the directory entry as well. */
    *pde |= flags & PAGE_USER;
    return (uint32_t*)(*pde & ADDR_MASK);
}

int paging_map_in(uint32_t* dir, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t* table = page_table(dir, virt, flags);
    if (!table)
        return 0;

    table[(virt >> 12) % ENTRIES] = (phys & ADDR_MASK) | (flags & ~ADDR_MASK) | PAGE_PRESENT;
    if (dir == active_directory)
        __asm__ volatile ("invlpg (%0)" : : "r"(virt) : "memory");
    return 1;
}

int paging_map(uint32_t virt, uint32_t phys, uint32_t flags) {
    return paging_map_in(kernel_directory, virt, phys, flags);
}

uint32_t* paging_create_directory(void) {
    uint32_t* dir = (uint32_t*)pmm_alloc_page();
    if (!dir)
        return 0;

    /* The kernel never maps the user range, so this shares exactly the kernel's mappings. */
    memcpy(dir, kernel_directory, PAGE_SIZE);
    return dir;
}

void paging_destroy_directory(uint32_t* dir) {
    for (uint32_t i = USER_START >> 22; i < USER_END >> 22; i++) {
        if (!(dir[i] & PAGE_PRESENT))
            continue;

        uint32_t* table = (uint32_t*)(dir[i] & ADDR_MASK);
        for (int j = 0; j < ENTRIES; j++)
            if (table[j] & PAGE_PRESENT)
                pmm_free_page(table[j] & ADDR_MASK);
        pmm_free_page((uint32_t)table);
    }
    pmm_free_page((uint32_t)dir);
}

void paging_switch(uint32_t* dir) {
    if (!dir)
        dir = kernel_directory;
    if (dir == active_directory)
        return;

    active_directory = dir;
    __asm__ volatile ("mov %0, %%cr3" : : "r"(dir) : "memory");
}

int paging_user_accessible(uint32_t* dir, uint32_t addr, uint32_t len, int write) {
    uint32_t need = PAGE_PRESENT | PAGE_USER | (write ? PAGE_WRITE : 0);

    if (addr < USER_START || addr > USER_END || len > USER_END - addr)
        return 0;

    for (uint32_t page = addr & ADDR_MASK; page < addr + len; page += PAGE_SIZE) {
        uint32_t pde = dir[page >> 22];
        if ((pde & need) != need)
            return 0;
        uint32_t pte = ((uint32_t*)(pde & ADDR_MASK))[(page >> 12) % ENTRIES];
        if ((pte & need) != need)
            return 0;
    }
    return 1;
}

void paging_init(void) {
    uint32_t end = pmm_memory_end();

    /* Low memory holds VGA and the Multiboot data even if the map reports little RAM. */
    if (end < 0x100000)
        end = 0x100000;

    for (uint32_t addr = PAGE_SIZE; addr < end; addr += PAGE_SIZE) {
        if (!paging_map(addr, addr, PAGE_WRITE)) {
            kprintf("paging: out of memory for page tables, staying unpaged\n");
            return;
        }
    }

    /*
     * Create the heap's page tables now, so later page directories can share
     * them by copying the kernel's directory entries once.
     */
    for (uint32_t addr = HEAP_START; addr < HEAP_END; addr += TABLE_SPAN) {
        if (!page_table(kernel_directory, addr, 0)) {
            kprintf("paging: out of memory for page tables, staying unpaged\n");
            return;
        }
    }

    paging_switch(kernel_directory);

    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0 | CR0_PAGING) : "memory");
}
