#include <stdint.h>
#include "vga.h"

#define VGA_MEMORY ((volatile uint16_t*)0xB8000)
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_COLOR  0x07 /* light grey on black */

static int row = 0;
static int col = 0;

static uint16_t vga_entry(char c) {
    return (uint16_t)(uint8_t)c | (uint16_t)VGA_COLOR << 8;
}

void vga_init(void) {
    row = 0;
    col = 0;
}

void vga_putc(char c) {
    if (c == '\n') {
        row++;
        col = 0;
        return;
    }

    VGA_MEMORY[row * VGA_WIDTH + col] = vga_entry(c);

    col++;
    if (col >= VGA_WIDTH) {
        col = 0;
        row++;
    }
}

void vga_print(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}
