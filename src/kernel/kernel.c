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
#include "pmm.h"
#include "paging.h"
#include "thread.h"
#include "syscall.h"
#include "process.h"

void kernel_main(uint32_t magic, const struct multiboot_info* info) {
    vga_init();
    vga_print("System started\n");
    if (!multiboot_init(magic, info))
        kprintf("Warning: not booted by a Multiboot loader (magic %x)\n", magic);
    pmm_init();
    paging_init();
    threads_init();
    kprintf("Memory: %u KiB usable, %u KiB free\n",
            pmm_total_pages() * (PAGE_SIZE / 1024), pmm_free_pages() * (PAGE_SIZE / 1024));

    gdt_init();
    idt_init();
    isr_init();
    syscall_init();
    process_init();
    keyboard_init();
    timer_init();
    __asm__ volatile ("sti");

    shell_run();
}
