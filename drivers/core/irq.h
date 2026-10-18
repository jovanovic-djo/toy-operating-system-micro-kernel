#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>

#define EFLAGS_IF 0x200

/*
 * Disable interrupts and return the previous EFLAGS, so a critical section
 * can be nested or entered with interrupts already off:
 *
 *     uint32_t flags = irq_save();
 *     ...
 *     irq_restore(flags);
 */
static inline uint32_t irq_save(void) {
    uint32_t flags;
    __asm__ volatile ("pushf; pop %0; cli" : "=r"(flags) : : "memory");
    return flags;
}

static inline void irq_restore(uint32_t flags) {
    if (flags & EFLAGS_IF)
        __asm__ volatile ("sti" : : : "memory");
}

#endif
