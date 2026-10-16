#include <stdint.h>
#include "gdt.h"

#define GDT_ENTRIES 3

/* Access bytes: present, ring 0, code/data segment. */
#define GDT_CODE_ACCESS 0x9A /* executable, readable */
#define GDT_DATA_ACCESS 0x92 /* writable */
#define GDT_FLAGS_32BIT 0xC  /* 4 KiB granularity, 32-bit */

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  limit_high_flags; /* limit bits 16-19, then flags */
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdtr;

/* Flat segment covering the whole 4 GiB address space. */
static void gdt_set_flat(int index, uint8_t access) {
    gdt[index].limit_low        = 0xFFFF;
    gdt[index].base_low         = 0;
    gdt[index].base_mid         = 0;
    gdt[index].access           = access;
    gdt[index].limit_high_flags = (GDT_FLAGS_32BIT << 4) | 0xF;
    gdt[index].base_high        = 0;
}

/*
 * Multiboot leaves a GDT behind but says it may be invalid, so the first
 * segment register reload (or interrupt) could fault. Install our own.
 */
void gdt_init(void) {
    gdt_set_flat(1, GDT_CODE_ACCESS);
    gdt_set_flat(2, GDT_DATA_ACCESS);

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base  = (uint32_t)&gdt;

    __asm__ volatile (
        "lgdt %0\n\t"
        "ljmp %1, $1f\n"        /* far jump reloads CS */
        "1:\n\t"
        "mov %2, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss"
        :
        : "m"(gdtr), "i"(GDT_KERNEL_CODE), "i"(GDT_KERNEL_DATA)
        : "eax", "memory");
}
