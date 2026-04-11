#include "keyboard.h"
#include "vga.h"

char keymap[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=',
    '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'', '`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,
};

void keyboard_handler() {
    unsigned char scancode = inb(0x60);

    if (scancode < 128) {
        char c = keymap[scancode];
        if (c)
            vga_putc(c);
    }
}

void keyboard_init() {
}
