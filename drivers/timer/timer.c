#include <stdint.h>
#include "timer.h"
#include "idt.h"
#include "isr.h"
#include "io.h"

#define PIT_CHANNEL0  0x40
#define PIT_COMMAND   0x43
#define PIT_BASE_HZ   1193182
#define PIT_MODE_RATE 0x36 /* channel 0, lobyte/hibyte, mode 3 (square wave) */
#define TIMER_IRQ     0

static volatile uint32_t ticks = 0;

static void timer_handler(struct registers* regs) {
    (void)regs;
    ticks++;
}

void timer_init(void) {
    uint16_t divisor = PIT_BASE_HZ / TIMER_HZ;

    outb(PIT_COMMAND, PIT_MODE_RATE);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, divisor >> 8);

    register_interrupt_handler(IRQ(TIMER_IRQ), timer_handler);
    pic_unmask_irq(TIMER_IRQ);
}

uint32_t timer_ticks(void) {
    return ticks;
}

/* Requires interrupts to be enabled, otherwise the tick count never advances. */
void timer_sleep(uint32_t ms) {
    uint32_t start = ticks;
    uint32_t wait = (ms * TIMER_HZ + 999) / 1000;

    /* Unsigned subtraction keeps working when the counter wraps around. */
    while (ticks - start < wait)
        __asm__ volatile ("hlt");
}
