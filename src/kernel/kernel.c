#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "keyboard.h"
#include "timer.h"
#include "shell.h"

void kernel_main(void) {
    vga_init();
    vga_print("System started\n");

    gdt_init();
    idt_init();
    isr_init();
    keyboard_init();
    timer_init();
    __asm__ volatile ("sti");

    shell_run();
}
