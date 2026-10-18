#ifndef PROCESS_H
#define PROCESS_H

#include "programs.h"
#include "thread.h"

/*
 * Start a program in user mode: a new address space with the program image
 * at USER_START and a stack just below USER_END, run by a new thread.
 * Returns NULL when out of memory.
 */
struct thread* process_spawn(const struct program* program);

/* End user programs that raise a CPU exception instead of panicking. Call after isr_init. */
void process_init(void);

#endif
