#ifndef ULIB_H
#define ULIB_H

#include <stdint.h>
#include "abi/syscall.h"

/* Minimal runtime for user programs: system call wrappers and printing. */

static inline uint32_t syscall3(uint32_t n, uint32_t a, uint32_t b, uint32_t c) {
    uint32_t ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(n), "b"(a), "c"(b), "d"(c) : "memory");
    return ret;
}

static inline void exit(int code) {
    syscall3(SYS_EXIT, code, 0, 0);
    for (;;)
        ;
}

static inline int write(const char* buf, uint32_t len) {
    return syscall3(SYS_WRITE, (uint32_t)buf, len, 0);
}

static inline void sleep(uint32_t ms) {
    syscall3(SYS_SLEEP, ms, 0, 0);
}

static inline uint32_t getpid(void) {
    return syscall3(SYS_GETPID, 0, 0, 0);
}

uint32_t strlen(const char* s);
void puts(const char* s);         /* no newline added */
void put_uint(uint32_t value);

#endif
