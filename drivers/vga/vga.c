#include <stdint.h>
#include "vga.h"
#include "io.h"

#define VGA_MEMORY ((volatile uint16_t*)0xB8000)
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_COLOR  0x07 /* light grey on black */

#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
#define VGA_CURSOR_HIGH 0x0E
#define VGA_CURSOR_LOW  0x0F

static int row = 0;
static int col = 0;

static uint16_t vga_entry(char c) {
    return (uint16_t)(uint8_t)c | (uint16_t)VGA_COLOR << 8;
}

static void vga_update_cursor(void) {
    uint16_t pos = row * VGA_WIDTH + col;

    outb(VGA_CRTC_INDEX, VGA_CURSOR_LOW);
    outb(VGA_CRTC_DATA, pos & 0xFF);
    outb(VGA_CRTC_INDEX, VGA_CURSOR_HIGH);
    outb(VGA_CRTC_DATA, (pos >> 8) & 0xFF);
}

void vga_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        VGA_MEMORY[i] = vga_entry(' ');

    row = 0;
    col = 0;
    vga_update_cursor();
}

void vga_init(void) {
    vga_clear();
}

static void vga_scroll(void) {
    for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++)
        VGA_MEMORY[i] = VGA_MEMORY[i + VGA_WIDTH];

    for (int i = 0; i < VGA_WIDTH; i++)
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + i] = vga_entry(' ');

    row = VGA_HEIGHT - 1;
}

static void vga_put_char(char c) {
    if (c == '\n') {
        row++;
        col = 0;
        if (row >= VGA_HEIGHT)
            vga_scroll();
        return;
    }

    if (c == '\b') {
        if (col > 0) {
            col--;
        } else if (row > 0) {
            row--;
            col = VGA_WIDTH - 1;
        }
        VGA_MEMORY[row * VGA_WIDTH + col] = vga_entry(' ');
        return;
    }

    VGA_MEMORY[row * VGA_WIDTH + col] = vga_entry(c);

    col++;
    if (col >= VGA_WIDTH) {
        col = 0;
        row++;
        if (row >= VGA_HEIGHT)
            vga_scroll();
    }
}

void vga_putc(char c) {
    vga_put_char(c);
    vga_update_cursor();
}

void vga_print(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}
