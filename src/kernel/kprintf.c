#include <stdarg.h>
#include <stdint.h>
#include "kprintf.h"
#include "vga.h"
#include "irq.h"

static void print_number(uint32_t value, unsigned base, int negative, int width, char pad) {
    static const char digits[] = "0123456789abcdef";
    char buf[11];
    int len = 0;

    do {
        buf[len++] = digits[value % base];
        value /= base;
    } while (value);

    if (negative) {
        if (pad == '0') {
            vga_putc('-');
            width--;
        } else {
            buf[len++] = '-';
        }
    }

    for (int i = len; i < width; i++)
        vga_putc(pad);
    while (len)
        vga_putc(buf[--len]);
}

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    /* Print the whole message at once, so output from different threads does not interleave. */
    uint32_t flags = irq_save();

    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            vga_putc(*fmt);
            continue;
        }

        fmt++;
        char pad = ' ';
        int width = 0;

        if (*fmt == '0') {
            pad = '0';
            fmt++;
        }
        while (*fmt >= '0' && *fmt <= '9')
            width = width * 10 + (*fmt++ - '0');

        switch (*fmt) {
        case 's': {
            const char* s = va_arg(args, const char*);
            vga_print(s ? s : "(null)");
            break;
        }
        case 'c':
            vga_putc((char)va_arg(args, int));
            break;
        case 'd': {
            int n = va_arg(args, int);
            /* Negate as unsigned so INT_MIN does not overflow. */
            uint32_t magnitude = n < 0 ? -(uint32_t)n : (uint32_t)n;
            print_number(magnitude, 10, n < 0, width, pad);
            break;
        }
        case 'u':
            print_number(va_arg(args, unsigned), 10, 0, width, pad);
            break;
        case 'x':
            print_number(va_arg(args, unsigned), 16, 0, width, pad);
            break;
        case 'p':
            vga_print("0x");
            print_number((uint32_t)va_arg(args, void*), 16, 0, 8, '0');
            break;
        case '%':
            vga_putc('%');
            break;
        case '\0':
            fmt--; /* lone '%' at the end; let the loop terminate */
            break;
        default:
            vga_putc('%');
            vga_putc(*fmt);
            break;
        }
    }

    irq_restore(flags);
    va_end(args);
}
