#include <stdint.h>
#include "idt.h"
#include "io.h"

#define IDT_ENTRIES 256
#define IDT_INTERRUPT_GATE 0x8E /* present, ring 0, 32-bit interrupt gate */

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtr;

/* GRUB's GDT layout is not specified, so use whatever code segment we run in. */
static uint16_t code_selector(void) {
    uint16_t cs;
    __asm__ volatile ("mov %%cs, %0" : "=r"(cs));
    return cs;
}

void idt_set_gate(uint8_t vector, uint32_t handler) {
    idt[vector].offset_low  = handler & 0xFFFF;
    idt[vector].selector    = code_selector();
    idt[vector].zero        = 0;
    idt[vector].type_attr   = IDT_INTERRUPT_GATE;
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
}

/* Move IRQ 0-15 to vectors 0x20-0x2F so they don't collide with CPU exceptions. */
static void pic_remap(void) {
    outb(PIC1_CMD, 0x11);   /* start initialization, expect ICW4 */
    outb(PIC2_CMD, 0x11);

    outb(PIC1_DATA, 0x20);  /* master vector offset */
    outb(PIC2_DATA, 0x28);  /* slave vector offset */

    outb(PIC1_DATA, 0x04);  /* slave attached to IRQ2 */
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);  /* 8086 mode */
    outb(PIC2_DATA, 0x01);

    /* Mask every IRQ; drivers unmask their own line once a handler is installed. */
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
}

void idt_init(void) {
    pic_remap();

    idtr.limit = sizeof(idt) - 1;
    idtr.base  = (uint32_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idtr));
}
