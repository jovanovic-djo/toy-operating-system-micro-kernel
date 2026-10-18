#include <stddef.h>
#include <stdint.h>
#include "heap.h"
#include "paging.h"
#include "pmm.h"
#include "kprintf.h"

/* Virtual range for the heap, above the identity-mapped RAM. */
#define HEAP_START 0xE0000000
#define HEAP_LIMIT 0xF0000000

#define ALIGN        16
#define BLOCK_MAGIC  0x4B4D454D /* "MEMK" */

/* Every allocation is preceded by a header; blocks tile the heap in address order. */
struct block {
    uint32_t magic;
    uint32_t size; /* payload bytes, a multiple of ALIGN */
    uint32_t free;
    struct block* next;
};

#define HEADER sizeof(struct block)

static struct block* first;
static uint32_t heap_end = HEAP_START; /* end of the mapped part */

static struct block* last_block(void) {
    struct block* b = first;
    while (b && b->next)
        b = b->next;
    return b;
}

/* Map more pages so that a block of `size` bytes fits at the end of the heap. */
static struct block* grow(uint32_t size) {
    struct block* last = last_block();
    uint32_t start = heap_end;
    uint32_t needed = (last && last->free) ? size - last->size : size + HEADER;

    if (pmm_memory_end() > HEAP_START) {
        kprintf("heap: RAM overlaps the heap address range\n");
        return NULL;
    }

    /* Fail early rather than tie up all free RAM in a heap that still cannot fit the request. */
    uint32_t pages = (needed + PAGE_SIZE - 1) / PAGE_SIZE;
    if (pages > (HEAP_LIMIT - heap_end) / PAGE_SIZE || pages + pages / 1024 + 1 > pmm_free_pages())
        return NULL;

    while (heap_end - start < needed) {
        uint32_t page = heap_end < HEAP_LIMIT ? pmm_alloc_page() : 0;
        if (!page)
            break;
        if (!paging_map(heap_end, page, PAGE_WRITE)) {
            pmm_free_page(page);
            break;
        }
        heap_end += PAGE_SIZE;
    }

    /* Keep whatever was mapped, even if it is not enough for this request. */
    uint32_t added = heap_end - start;
    if (last && last->free) {
        last->size += added;
    } else if (added >= HEADER + ALIGN) {
        struct block* b = (struct block*)start;
        b->magic = BLOCK_MAGIC;
        b->size = added - HEADER;
        b->free = 1;
        b->next = NULL;
        if (last)
            last->next = b;
        else
            first = b;
        last = b;
    }

    return last && last->free && last->size >= size ? last : NULL;
}

/* Cut `b` down to `size` bytes if the rest is big enough to be a block of its own. */
static void split(struct block* b, uint32_t size) {
    if (b->size < size + HEADER + ALIGN)
        return;

    struct block* rest = (struct block*)((uint8_t*)(b + 1) + size);
    rest->magic = BLOCK_MAGIC;
    rest->size = b->size - size - HEADER;
    rest->free = 1;
    rest->next = b->next;
    b->size = size;
    b->next = rest;
}

void* kmalloc(size_t size) {
    if (size == 0 || size > HEAP_LIMIT - HEAP_START)
        return NULL;
    size = (size + ALIGN - 1) & ~(ALIGN - 1);

    struct block* b = first;
    while (b && !(b->free && b->size >= size))
        b = b->next;
    if (!b)
        b = grow(size);
    if (!b)
        return NULL;

    split(b, size);
    b->free = 0;
    return b + 1;
}

void kfree(void* ptr) {
    if (!ptr)
        return;

    struct block* b = (struct block*)ptr - 1;
    if ((uint32_t)b < HEAP_START || (uint32_t)ptr >= heap_end || (uint32_t)ptr % ALIGN ||
        b->magic != BLOCK_MAGIC || b->free) {
        kprintf("kfree: bad pointer %p\n", ptr);
        return;
    }
    b->free = 1;

    /* Merge neighbouring free blocks so the space can be reused for larger requests. */
    for (b = first; b; b = b->next) {
        while (b->free && b->next && b->next->free) {
            b->next->magic = 0;
            b->size += HEADER + b->next->size;
            b->next = b->next->next;
        }
    }
}

void heap_stats(uint32_t* used, uint32_t* mapped) {
    *used = 0;
    for (struct block* b = first; b; b = b->next)
        if (!b->free)
            *used += b->size;
    *mapped = heap_end - HEAP_START;
}
