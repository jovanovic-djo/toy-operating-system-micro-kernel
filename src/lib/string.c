#include <stddef.h>
#include "string.h"

size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len])
        len++;
    return len;
}

int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char* a, const char* b, size_t n) {
    for (; n; n--, a++, b++) {
        if (*a != *b || !*a)
            return (unsigned char)*a - (unsigned char)*b;
    }
    return 0;
}

void* memset(void* dest, int value, size_t n) {
    unsigned char* d = dest;
    while (n--)
        *d++ = (unsigned char)value;
    return dest;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while (n--)
        *d++ = *s++;
    return dest;
}
