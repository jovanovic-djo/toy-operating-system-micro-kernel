#include "vga.h"
#include "idt.h"
#include "isr.h"
#include "keyboard.h"
#include "timer.h"

void kernel_main(void) {
    vga_init();
    vga_print("System started\n");

    idt_init();
    isr_init();
    keyboard_init();
    timer_init();
    __asm__ volatile ("sti");

    vga_print("~:\n");

    while (1) {
        __asm__("hlt");
    }
}
