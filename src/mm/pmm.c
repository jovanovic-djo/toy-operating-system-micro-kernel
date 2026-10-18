#include <stdint.h>
#include "pmm.h"
#include "multiboot.h"
#include "kprintf.h"
#include "string.h"

#define PAGE_SHIFT 12
#define MAX_PAGES  (1u << 20) /* 4 GiB / 4 KiB */
#define LOW_MEMORY 0x100000   /* BIOS, VGA and bootloader data live below 1 MiB */

extern char _kernel_start[], _kernel_end[]; /* from linker.ld */

/* One bit per page below 4 GiB; a set bit means used or not RAM. */
static uint32_t bitmap[MAX_PAGES / 32];
static uint32_t free_count, total_count;
static uint64_t memory_end;
static uint32_t search_from; /* bitmap word to start the next search at */

static int page_used(uint32_t page) {
    return bitmap[page / 32] & (1u << (page % 32));
}

/* Free every page that lies entirely inside the region. */
static void free_region(uint64_t base, uint64_t len) {
    /* Shifts, not 64-bit division, which would need libgcc. */
    uint64_t start = (base + PAGE_SIZE - 1) >> PAGE_SHIFT;
    uint64_t end = (base + len) >> PAGE_SHIFT;
    if (end > MAX_PAGES)
        end = MAX_PAGES;
    if (start >= end)
        return;

    for (uint32_t page = start; page < end; page++) {
        if (page_used(page)) {
            bitmap[page / 32] &= ~(1u << (page % 32));
            free_count++;
        }
    }
    if ((end << PAGE_SHIFT) > memory_end)
        memory_end = end << PAGE_SHIFT;
}

/* Mark every page that overlaps [start, end) as used. */
static void reserve_region(uint32_t start, uint32_t end) {
    for (uint32_t page = start / PAGE_SIZE; page < (end + PAGE_SIZE - 1) / PAGE_SIZE; page++) {
        if (!page_used(page)) {
            bitmap[page / 32] |= 1u << (page % 32);
            free_count--;
        }
    }
}

void pmm_init(void) {
    const struct multiboot_info* info = multiboot_get_info();

    memset(bitmap, 0xFF, sizeof(bitmap));

    const struct multiboot_mmap_entry* entry = multiboot_mmap_next(NULL);
    if (entry) {
        for (; entry; entry = multiboot_mmap_next(entry))
            if (entry->type == MULTIBOOT_MEMORY_AVAILABLE)
                free_region(entry->addr, entry->len);
    } else if (info && (info->flags & MULTIBOOT_INFO_MEMORY)) {
        free_region(LOW_MEMORY, (uint64_t)info->mem_upper * 1024);
    }
    total_count = free_count;

    reserve_region(0, LOW_MEMORY);
    reserve_region((uint32_t)_kernel_start, (uint32_t)_kernel_end);
    if (info) {
        /* The bootloader may place these above 1 MiB; keep them for the mem command. */
        reserve_region((uint32_t)info, (uint32_t)info + sizeof(*info));
        if (info->flags & MULTIBOOT_INFO_MMAP)
            reserve_region(info->mmap_addr, info->mmap_addr + info->mmap_length);
    }
}

uint32_t pmm_alloc_page(void) {
    for (uint32_t n = 0; n < MAX_PAGES / 32; n++) {
        uint32_t i = (search_from + n) % (MAX_PAGES / 32);
        if (bitmap[i] == 0xFFFFFFFF)
            continue;

        uint32_t bit = 0;
        while (bitmap[i] & (1u << bit))
            bit++;
        bitmap[i] |= 1u << bit;
        free_count--;
        search_from = i;
        return (i * 32 + bit) * PAGE_SIZE;
    }
    return 0;
}

void pmm_free_page(uint32_t addr) {
    uint32_t page = addr / PAGE_SIZE;

    /* The bitmap cannot tell allocated pages from reserved ones, so reject the known reserved ranges. */
    int reserved = addr < LOW_MEMORY || addr >= memory_end ||
                   (addr >= (uint32_t)_kernel_start && addr < (uint32_t)_kernel_end);

    if (addr % PAGE_SIZE || reserved || !page_used(page)) {
        kprintf("pmm: bad free of %x\n", addr);
        return;
    }
    bitmap[page / 32] &= ~(1u << (page % 32));
    free_count++;
    if (page / 32 < search_from)
        search_from = page / 32;
}

uint32_t pmm_free_pages(void) {
    return free_count;
}

uint32_t pmm_total_pages(void) {
    return total_count;
}

uint32_t pmm_memory_end(void) {
    return memory_end > 0xFFFFF000 ? 0xFFFFF000 : (uint32_t)memory_end;
}
