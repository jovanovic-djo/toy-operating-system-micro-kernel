#ifndef GDT_H
#define GDT_H

/* Segment selectors (index * 8, ring 0). */
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10

void gdt_init(void);

#endif
