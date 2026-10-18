#ifndef ISR_H
#define ISR_H

#include <stdint.h>

#define ISR_COUNT 48  /* vectors with a stub: CPU exceptions and IRQs */
#define IRQ_BASE  32
#define IRQ_COUNT 16
#define IRQ(n)    (IRQ_BASE + (n))

/* Stack layout built by isr_common in interrupts.s, lowest address first. */
struct registers {
    uint32_t ds;                                     /* data segment at the time of the interrupt */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* pusha */
    uint32_t int_no, err_code;                       /* pushed by the stub */
    uint32_t eip, cs, eflags;                        /* pushed by the CPU */
    uint32_t user_esp, user_ss;                      /* pushed by the CPU only when coming from user mode */
};

/* True if the interrupt arrived while running user code (ring 3). */
#define FROM_USER(regs) (((regs)->cs & 3) == 3)

typedef void (*interrupt_handler_t)(struct registers* regs);

void isr_init(void);
void register_interrupt_handler(uint8_t vector, interrupt_handler_t handler);

#endif
