#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char* s);
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t n);

/* GCC may emit calls to these even in freestanding code. */
void* memset(void* dest, int value, size_t n);
void* memcpy(void* dest, const void* src, size_t n);

#endif
