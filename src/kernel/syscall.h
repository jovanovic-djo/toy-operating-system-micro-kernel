#ifndef SYSCALL_H
#define SYSCALL_H

/* Install the int 0x80 gate, callable from user mode. Call after isr_init. */
void syscall_init(void);

#endif
