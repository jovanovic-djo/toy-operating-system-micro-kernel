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
 * Page tables come from the page allocator and are reached through their
 * physical address, which works because all RAM is identity-mapped.
 */
static uint32_t page_directory[ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static int paging_enabled;

/* The page table covering virt, allocated on first use; NULL if out of memory. */
static uint32_t* page_table(uint32_t virt) {
    uint32_t* pde = &page_directory[virt >> 22];

    if (!(*pde & PAGE_PRESENT)) {
        uint32_t table = pmm_alloc_page();
        if (!table)
            return 0;
        memset((void*)table, 0, PAGE_SIZE);
        *pde = table | PAGE_PRESENT | PAGE_WRITE;
    }
    return (uint32_t*)(*pde & ADDR_MASK);
}

static void flush_tlb(uint32_t virt) {
    if (paging_enabled)
        __asm__ volatile ("invlpg (%0)" : : "r"(virt) : "memory");
}

int paging_map(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t* table = page_table(virt);
    if (!table)
        return 0;

    table[(virt >> 12) % ENTRIES] = (phys & ADDR_MASK) | (flags & ~ADDR_MASK) | PAGE_PRESENT;
    flush_tlb(virt);
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
        if (!page_table(addr)) {
            kprintf("paging: out of memory for page tables, staying unpaged\n");
            return;
        }
    }

    __asm__ volatile ("mov %0, %%cr3" : : "r"(page_directory) : "memory");

    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0 | CR0_PAGING) : "memory");
    paging_enabled = 1;
}
