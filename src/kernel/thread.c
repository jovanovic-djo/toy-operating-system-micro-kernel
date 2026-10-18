#include <stddef.h>
#include <stdint.h>
#include "thread.h"
#include "heap.h"
#include "irq.h"
#include "timer.h"
#include "kprintf.h"
#include "string.h"
#include "gdt.h"

#define STACK_SIZE 16384
#define SLICE_MS   50 /* time a thread runs before the timer switches away */

void context_switch(uint32_t* old_esp, uint32_t new_esp); /* switch.s */

static struct thread boot_thread;
static struct thread* idle_thread;
static struct thread* current;
static uint32_t next_id;
static uint32_t slice_ticks;

struct thread* thread_current(void) {
    return current;
}

/* The next ready thread after the current one; the idle thread only when nothing else can run. */
static struct thread* pick_next(void) {
    for (struct thread* t = current->next; t != current; t = t->next)
        if (t->state == THREAD_READY && t != idle_thread)
            return t;

    if (current->state == THREAD_RUNNING && current != idle_thread)
        return current;
    return idle_thread;
}

/* Free threads that have exited, except the current one whose stack we are still on. */
static void reap(void) {
    struct thread* t = current;

    while (t->next != current) {
        struct thread* victim = t->next;
        if (victim->state != THREAD_DEAD) {
            t = victim;
            continue;
        }
        t->next = victim->next;
        if (victim != &boot_thread) {
            kfree(victim->stack);
            kfree(victim);
        }
    }
}

/* Switch to the next thread. Interrupts must be off. */
static void schedule(void) {
    reap();

    struct thread* prev = current;
    struct thread* next = pick_next();

    slice_ticks = 0;
    if (next == prev)
        return;

    if (prev->state == THREAD_RUNNING)
        prev->state = THREAD_READY;
    next->state = THREAD_RUNNING;
    current = next;
    /* Interrupts from user mode must land on the new thread's own kernel stack. */
    if (next->stack)
        tss_set_kernel_stack((uint32_t)next->stack + STACK_SIZE);
    context_switch(&prev->esp, next->esp);
}

/* First code a new thread runs, entered through context_switch's ret. */
static void thread_start(void) {
    __asm__ volatile ("sti"); /* we arrive from schedule() with interrupts off */
    current->entry(current->arg);
    thread_exit();
}

static void idle(void* arg) {
    (void)arg;
    for (;;)
        __asm__ volatile ("hlt");
}

/* Make sleeping threads whose time has come ready; returns 1 if any woke up. */
static int wake_sleepers(void) {
    uint32_t now = timer_ticks();
    int woke = 0;
    struct thread* t = current;

    do {
        /* Signed difference keeps working when the tick counter wraps around. */
        if (t->state == THREAD_SLEEPING && (int32_t)(now - t->wake_tick) >= 0) {
            t->state = THREAD_READY;
            woke = 1;
        }
        t = t->next;
    } while (t != current);
    return woke;
}

/* Runs in the timer interrupt, so interrupts are already off. */
static void thread_tick(void) {
    int woke = wake_sleepers();

    /* Leave idle as soon as there is work; otherwise switch when the time slice is used up. */
    if ((woke && current == idle_thread) || ++slice_ticks >= SLICE_MS * TIMER_HZ / 1000)
        schedule();
}

void threads_init(void) {
    boot_thread.id = next_id++;
    boot_thread.name = "kernel";
    boot_thread.state = THREAD_RUNNING;
    boot_thread.next = &boot_thread;
    current = &boot_thread;

    idle_thread = thread_create("idle", idle, NULL);
    timer_set_tick_handler(thread_tick);
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

void threads_print(void) {
    static const char* states[] = { "ready", "running", "sleeping", "dead" };

    uint32_t flags = irq_save();
    kprintf("  id  state     name\n");
    struct thread* t = current;
    do {
        kprintf("  %2u  %s", t->id, states[t->state]);
        for (int pad = 10 - (int)strlen(states[t->state]); pad > 0; pad--)
            kprintf(" ");
        kprintf("%s\n", t->name);
        t = t->next;
    } while (t != current);
    irq_restore(flags);
}

void thread_sleep(uint32_t ms) {
    uint32_t flags = irq_save();
    current->wake_tick = timer_ticks() + (ms * TIMER_HZ + 999) / 1000;
    current->state = THREAD_SLEEPING;
    schedule();
    irq_restore(flags);
}

/* The scheduler frees the thread once it has switched to another one. */
void thread_exit(void) {
    irq_save();
    current->state = THREAD_DEAD;
    schedule();
    for (;;)
        __asm__ volatile ("hlt"); /* never reached: a dead thread is not scheduled again */
}
