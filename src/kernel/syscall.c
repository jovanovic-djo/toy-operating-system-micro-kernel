#include <stdint.h>
#include "syscall.h"
#include "abi/syscall.h"
#include "idt.h"
#include "isr.h"
#include "thread.h"
#include "paging.h"
#include "kprintf.h"
#include "vga.h"

#define WRITE_MAX 4096 /* longest buffer one write call accepts */

extern char isr128[]; /* interrupts.s */

/* Pointers from user mode must be checked against the caller's own page tables. */
static int user_buffer_ok(struct registers* regs, uint32_t addr, uint32_t len) {
    if (!FROM_USER(regs))
        return 1; /* kernel callers are trusted */
    return paging_user_accessible(thread_current()->page_dir, addr, len, 0);
}

static uint32_t sys_write(struct registers* regs, const char* buf, uint32_t len) {
    if (len > WRITE_MAX || !user_buffer_ok(regs, (uint32_t)buf, len))
        return (uint32_t)-1;
    for (uint32_t i = 0; i < len; i++)
        vga_putc(buf[i]);
    return len;
}

static void sys_exit(int code) {
    kprintf("[thread %u exited with code %d]\n", thread_current()->id, code);
    thread_exit();
}

static void syscall_handler(struct registers* regs) {
    switch (regs->eax) {
    case SYS_EXIT:
        sys_exit((int)regs->ebx);
        break;
    case SYS_WRITE:
        regs->eax = sys_write(regs, (const char*)regs->ebx, regs->ecx);
        break;
    case SYS_SLEEP:
        thread_sleep(regs->ebx);
        regs->eax = 0;
        break;
    case SYS_GETPID:
        regs->eax = thread_current()->id;
        break;
    default:
        regs->eax = (uint32_t)-1;
        break;
    }
}

void syscall_init(void) {
    idt_set_user_gate(SYSCALL_VECTOR, (uint32_t)isr128);
    register_interrupt_handler(SYSCALL_VECTOR, syscall_handler);
}
