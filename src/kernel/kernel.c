#include <stdint.h>
#include "multiboot.h"
#include "kprintf.h"
#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "isr.h"
#include "keyboard.h"
#include "timer.h"
#include "shell.h"

void kernel_main(uint32_t magic, const struct multiboot_info* info) {
    vga_init();
    vga_print("System started\n");
    if (!multiboot_init(magic, info))
        kprintf("Warning: not booted by a Multiboot loader (magic %x)\n", magic);
    else if (info->flags & MULTIBOOT_INFO_MEMORY)
        kprintf("Memory: %u KiB lower, %u KiB upper\n", info->mem_lower, info->mem_upper);

    gdt_init();
    idt_init();
    isr_init();
    keyboard_init();
    timer_init();
    __asm__ volatile ("sti");

    shell_run();
}
