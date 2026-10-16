#ifndef IDT_H
#define IDT_H

#include <stdint.h>

void idt_init(void);
void idt_set_gate(uint8_t vector, uint32_t handler);

void pic_unmask_irq(uint8_t irq);
void pic_send_eoi(uint8_t irq);
/* Return 1 for a spurious IRQ 7/15, which the caller must then ignore without an EOI. */
int pic_is_spurious(uint8_t irq);

#endif
