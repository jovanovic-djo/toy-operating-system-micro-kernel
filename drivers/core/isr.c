#include <stdint.h>
#include "isr.h"
#include "idt.h"

extern uint32_t isr_stub_table[ISR_COUNT];

static interrupt_handler_t handlers[ISR_COUNT];

void isr_init(void) {
    for (int i = 0; i < ISR_COUNT; i++)
        idt_set_gate(i, isr_stub_table[i]);
}

void register_interrupt_handler(uint8_t vector, interrupt_handler_t handler) {
    if (vector < ISR_COUNT)
        handlers[vector] = handler;
}

static void halt(void) {
    for (;;)
        __asm__ volatile ("cli; hlt");
}

/* Called from isr_common for every exception and IRQ. */
void isr_dispatch(struct registers* regs) {
    interrupt_handler_t handler = handlers[regs->int_no];

    if (regs->int_no >= IRQ_BASE) {
        if (handler)
            handler(regs);
        pic_send_eoi(regs->int_no - IRQ_BASE);
        return;
    }

    if (handler) {
        handler(regs);
        return;
    }

    /* Unhandled CPU exception: returning would re-run the faulting instruction. */
    halt();
}
