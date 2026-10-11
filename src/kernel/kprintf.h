#ifndef KPRINTF_H
#define KPRINTF_H

/* Supports %s %c %d %u %x %p %% with an optional '0' flag and width, e.g. %08x. */
void kprintf(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

#endif
