#include <stdint.h>
#include "isr.h"
#include "idt.h"
#include "vga.h"
#include "kprintf.h"

extern uint32_t isr_stub_table[ISR_COUNT];

static interrupt_handler_t handlers[256];
static user_fault_handler_t user_fault_handler;

void isr_init(void) {
    for (int i = 0; i < ISR_COUNT; i++)
        idt_set_gate(i, isr_stub_table[i]);
}

void register_interrupt_handler(uint8_t vector, interrupt_handler_t handler) {
    handlers[vector] = handler;
}

void set_user_fault_handler(user_fault_handler_t handler) {
    user_fault_handler = handler;
}

#define PAGE_FAULT 14

static const char* exception_names[IRQ_BASE] = {
    "Divide Error", "Debug", "Non-Maskable Interrupt", "Breakpoint",
    "Overflow", "Bound Range Exceeded", "Invalid Opcode", "Device Not Available",
    "Double Fault", "Coprocessor Segment Overrun", "Invalid TSS", "Segment Not Present",
    "Stack-Segment Fault", "General Protection Fault", "Page Fault", "Reserved",
    "x87 Floating-Point Exception", "Alignment Check", "Machine Check", "SIMD Floating-Point Exception",
    "Virtualization Exception", "Control Protection Exception", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Hypervisor Injection Exception", "VMM Communication Exception", "Security Exception", "Reserved",
};

static void halt(void) {
    for (;;)
        __asm__ volatile ("cli; hlt");
}

static void print_reg(const char* name, uint32_t value) {
    vga_print(name);
    vga_print("=");
    vga_print_hex(value);
    vga_print("  ");
}

static void exception_panic(struct registers* regs) {
    /* Kernel-mode faults push no SS:ESP, so the interrupted stack starts where user_esp would be. */
    uint32_t fault_esp = FROM_USER(regs) ? regs->user_esp : (uint32_t)&regs->user_esp;

    vga_set_color(VGA_WHITE, VGA_RED);
    vga_clear();

    vga_print("KERNEL PANIC: unhandled CPU exception\n\n");
    vga_print(exception_names[regs->int_no]);
    vga_print(" (vector ");
    vga_print_hex(regs->int_no);
    vga_print(")\nError code: ");
    vga_print_hex(regs->err_code);
    vga_print("\n\n");

    print_reg("EIP", regs->eip);
    print_reg("CS ", regs->cs);
    print_reg("EFLAGS", regs->eflags);
    vga_print("\n");
    print_reg("EAX", regs->eax);
    print_reg("EBX", regs->ebx);
    print_reg("ECX", regs->ecx);
    print_reg("EDX", regs->edx);
    vga_print("\n");
    print_reg("ESI", regs->esi);
    print_reg("EDI", regs->edi);
    print_reg("EBP", regs->ebp);
    print_reg("ESP", fault_esp);
    vga_print("\n");

    if (regs->int_no == PAGE_FAULT) {
        uint32_t cr2;
        __asm__ volatile ("mov %%cr2, %0" : "=r"(cr2));
        print_reg("CR2", cr2);
        vga_print("\n");
    }

    vga_print("\nSystem halted.");
    halt();
}

/* Called from isr_common for every exception and IRQ. */
void isr_dispatch(struct registers* regs) {
    interrupt_handler_t handler = handlers[regs->int_no];

    if (regs->int_no >= IRQ_BASE && regs->int_no < IRQ_BASE + IRQ_COUNT) {
        uint8_t irq = regs->int_no - IRQ_BASE;
        if (pic_is_spurious(irq))
            return;
        /*
         * Acknowledge first: a handler may switch to another thread and not
         * return here for a while, and the PIC holds back lower-priority IRQs
         * until it gets the EOI. Interrupts stay off until iret either way.
         */
        pic_send_eoi(irq);
        if (handler)
            handler(regs);
        return;
    }

    if (handler) {
        handler(regs);
        return;
    }

    if (regs->int_no >= IRQ_BASE) {
        kprintf("unhandled interrupt %u\n", regs->int_no);
        return;
    }

    /* A faulting user program only takes itself down. */
    if (FROM_USER(regs) && user_fault_handler) {
        user_fault_handler(regs, exception_names[regs->int_no]);
        return;
    }

    /* Unhandled CPU exception: returning would re-run the faulting instruction. */
    exception_panic(regs);
}
