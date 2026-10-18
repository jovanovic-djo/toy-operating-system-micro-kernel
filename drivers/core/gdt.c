#include <stdint.h>
#include "gdt.h"

#define GDT_ENTRIES 6

/* Access bytes: present, privilege level, segment type. */
#define GDT_CODE_ACCESS      0x9A /* ring 0, executable, readable */
#define GDT_DATA_ACCESS      0x92 /* ring 0, writable */
#define GDT_USER_CODE_ACCESS 0xFA /* ring 3, executable, readable */
#define GDT_USER_DATA_ACCESS 0xF2 /* ring 3, writable */
#define GDT_TSS_ACCESS       0x89 /* ring 0, 32-bit available TSS */
#define GDT_FLAGS_32BIT      0xC  /* 4 KiB granularity, 32-bit */

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

/* Only the fields used for a ring 3 -> ring 0 stack switch; the rest stay zero. */
struct tss {
    uint32_t prev_tss;
    uint32_t esp0;     /* kernel stack loaded on an interrupt from user mode */
    uint32_t ss0;
    uint32_t unused[22];
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdtr;
static struct tss tss;

static void gdt_set_entry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[index].limit_low        = limit & 0xFFFF;
    gdt[index].base_low         = base & 0xFFFF;
    gdt[index].base_mid         = (base >> 16) & 0xFF;
    gdt[index].access           = access;
    gdt[index].limit_high_flags = (flags << 4) | ((limit >> 16) & 0xF);
    gdt[index].base_high        = (base >> 24) & 0xFF;
}

/* Flat segment covering the whole 4 GiB address space. */
static void gdt_set_flat(int index, uint8_t access) {
    gdt_set_entry(index, 0, 0xFFFFF, access, GDT_FLAGS_32BIT);
}

void tss_set_kernel_stack(uint32_t esp0) {
    tss.esp0 = esp0;
}

/*
 * Multiboot leaves a GDT behind but says it may be invalid, so the first
 * segment register reload (or interrupt) could fault. Install our own,
 * with user segments and a TSS for running code in ring 3.
 */
void gdt_init(void) {
    gdt_set_flat(1, GDT_CODE_ACCESS);
    gdt_set_flat(2, GDT_DATA_ACCESS);
    gdt_set_flat(3, GDT_USER_CODE_ACCESS);
    gdt_set_flat(4, GDT_USER_DATA_ACCESS);

    tss.ss0 = GDT_KERNEL_DATA;
    tss.iomap_base = sizeof(tss); /* no I/O permission bitmap: user code gets no port access */
    gdt_set_entry(5, (uint32_t)&tss, sizeof(tss) - 1, GDT_TSS_ACCESS, 0);

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
        "mov %%ax, %%ss\n\t"
        "mov %3, %%ax\n\t"
        "ltr %%ax"
        :
        : "m"(gdtr), "i"(GDT_KERNEL_CODE), "i"(GDT_KERNEL_DATA), "i"(GDT_TSS)
        : "eax", "memory");
}
