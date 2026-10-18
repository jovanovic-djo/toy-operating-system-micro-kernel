#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/* Segment selectors (index * 8). User selectors include RPL 3. */
#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE   (0x18 | 3)
#define GDT_USER_DATA   (0x20 | 3)
#define GDT_TSS         0x28

void gdt_init(void);

/* Stack the CPU switches to when an interrupt arrives from user mode. */
void tss_set_kernel_stack(uint32_t esp0);

#endif
