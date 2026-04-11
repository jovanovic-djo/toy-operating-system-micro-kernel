#include "vga.h"
#define VGA_MEMORY ((char*)0xB8000)


static int row = 0;
static int col = 0;

void vga_init() {
    row = 0;
    col = 0;
}

void vga_putc(char c) {
    if (c == '\n') {
        row++;
        col = 0;
        return;
    }

    int index = (row * 80 + col) * 2;
    VGA_MEMORY[index] = c;
    VGA_MEMORY[index + 1] = 0x07;

    col++;
    if (col >= 80) {
        col = 0;
        row++;
    }
}

void vga_print(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}