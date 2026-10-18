#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>

enum thread_state {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_SLEEPING,
    THREAD_DEAD,
};

struct thread {
    uint32_t esp;              /* saved stack pointer while not running */
    uint32_t id;
    const char* name;
    enum thread_state state;
    uint32_t wake_tick;        /* when a sleeping thread becomes ready */
    void (*entry)(void* arg);
    void* arg;
    void* stack;               /* kmalloc'd stack, NULL for the boot thread */
    struct thread* next;       /* circular list of all threads */
};

/*
 * Turn the code running kernel_main into thread 0, start the idle thread and
 * preempt threads from the timer. Call after the heap is usable.
 */
void threads_init(void);

/* Start entry(arg) in a new thread. Returns NULL when out of memory. */
struct thread* thread_create(const char* name, void (*entry)(void* arg), void* arg);

/* Let the next ready thread run. */
void thread_yield(void);

/* Block the calling thread for at least ms milliseconds. */
void thread_sleep(uint32_t ms);

/* End the calling thread; its memory is freed later. Returning from the entry function does the same. */
void thread_exit(void) __attribute__((noreturn));

struct thread* thread_current(void);

#endif
