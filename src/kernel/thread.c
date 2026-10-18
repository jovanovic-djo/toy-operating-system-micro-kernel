#include <stddef.h>
#include <stdint.h>
#include "thread.h"
#include "heap.h"
#include "irq.h"
#include "kprintf.h"

#define STACK_SIZE 16384

void context_switch(uint32_t* old_esp, uint32_t new_esp); /* switch.s */

static struct thread boot_thread;
static struct thread* current;
static uint32_t next_id;

struct thread* thread_current(void) {
    return current;
}

/* Switch to the next ready thread after the current one. Interrupts must be off. */
static void schedule(void) {
    struct thread* prev = current;
    struct thread* next = prev->next;

    while (next != prev && next->state != THREAD_READY)
        next = next->next;

    if (next == prev) {
        if (prev->state == THREAD_RUNNING)
            return; /* nothing else to run */
        kprintf("thread: no thread left to run\n");
        for (;;)
            __asm__ volatile ("cli; hlt");
    }

    if (prev->state == THREAD_RUNNING)
        prev->state = THREAD_READY;
    next->state = THREAD_RUNNING;
    current = next;
    context_switch(&prev->esp, next->esp);
}

/* First code a new thread runs, entered through context_switch's ret. */
static void thread_start(void) {
    __asm__ volatile ("sti"); /* we arrive from schedule() with interrupts off */
    current->entry(current->arg);
    thread_exit();
}

void threads_init(void) {
    boot_thread.id = next_id++;
    boot_thread.name = "kernel";
    boot_thread.state = THREAD_RUNNING;
    boot_thread.next = &boot_thread;
    current = &boot_thread;
}

struct thread* thread_create(const char* name, void (*entry)(void* arg), void* arg) {
    struct thread* t = kmalloc(sizeof(*t));
    void* stack = kmalloc(STACK_SIZE);
    if (!t || !stack) {
        kfree(t);
        kfree(stack);
        return NULL;
    }

    /* Build the frame context_switch pops: edi, esi, ebx, ebp, then the return address. */
    uint32_t* sp = (uint32_t*)((uint8_t*)stack + STACK_SIZE);
    *--sp = 0;                        /* fake return address for thread_start */
    *--sp = (uint32_t)thread_start;
    for (int i = 0; i < 4; i++)
        *--sp = 0;

    t->esp = (uint32_t)sp;
    t->name = name;
    t->state = THREAD_READY;
    t->entry = entry;
    t->arg = arg;
    t->stack = stack;

    uint32_t flags = irq_save();
    t->id = next_id++;
    t->next = current->next;
    current->next = t;
    irq_restore(flags);
    return t;
}

void thread_yield(void) {
    uint32_t flags = irq_save();
    schedule();
    irq_restore(flags);
}

/* The thread's memory is not reclaimed yet; it stays in the list as dead. */
void thread_exit(void) {
    irq_save();
    current->state = THREAD_DEAD;
    schedule();
    for (;;)
        __asm__ volatile ("hlt"); /* never reached: a dead thread is not scheduled again */
}
