#include "ulib.h"

uint32_t strlen(const char* s) {
    uint32_t len = 0;
    while (s[len])
        len++;
    return len;
}

void puts(const char* s) {
    write(s, strlen(s));
}

void put_uint(uint32_t value) {
    char buf[10];
    int len = 0;

    do {
        buf[sizeof(buf) - 1 - len++] = '0' + value % 10;
        value /= 10;
    } while (value);
    write(buf + sizeof(buf) - len, len);
}
