#include "vga.h"
#include "idt.h"
#include "keyboard.h"

void kernel_main() {
    vga_init();
    vga_print("System started\n");

    idt_init();
    keyboard_init();

    vga_print("~:\n");

    while (1) {
        __asm__("hlt");
    }
}
