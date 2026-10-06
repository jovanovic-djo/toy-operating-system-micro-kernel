#include <stdint.h>
#include "keyboard.h"
#include "idt.h"
#include "vga.h"
#include "io.h"

#define KEYBOARD_DATA   0x60
#define KEYBOARD_STATUS 0x64
#define KEYBOARD_IRQ    1
#define IRQ_BASE        0x20

extern void irq1_stub(void);

static const char keymap[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=',
    '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'', '`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,
    '*', 0, ' ',
};

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA);

    if (scancode < 128) {
        char c = keymap[scancode];
        if (c)
            vga_putc(c);
    }

    pic_send_eoi(KEYBOARD_IRQ);
}

void keyboard_init(void) {
    /* Drop any bytes left in the controller buffer by the BIOS/bootloader. */
    while (inb(KEYBOARD_STATUS) & 1)
        inb(KEYBOARD_DATA);

    idt_set_gate(IRQ_BASE + KEYBOARD_IRQ, (uint32_t)irq1_stub);
    pic_unmask_irq(KEYBOARD_IRQ);
}
