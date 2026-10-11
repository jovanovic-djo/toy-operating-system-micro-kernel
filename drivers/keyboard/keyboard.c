#include <stdint.h>
#include "keyboard.h"
#include "idt.h"
#include "isr.h"
#include "vga.h"
#include "io.h"

#define KEYBOARD_DATA   0x60
#define KEYBOARD_STATUS 0x64
#define KEYBOARD_IRQ    1

#define SC_LEFT_SHIFT   0x2A
#define SC_RIGHT_SHIFT  0x36
#define SC_CAPS_LOCK    0x3A
#define SC_RELEASE      0x80

static const char keymap[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=',
    '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'', '`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',0,
    '*', 0, ' ',
};

static const char keymap_shift[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+',
    '\b','\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,'A','S','D','F','G','H','J','K','L',':','"', '~',
    0,'|','Z','X','C','V','B','N','M','<','>','?',0,
    '*', 0, ' ',
};

static int shift_held = 0;
static int caps_lock = 0;

static char translate(uint8_t scancode) {
    char c = keymap[scancode];
    int shifted = shift_held;

    /* Caps Lock only affects letters, and Shift inverts it. */
    if (c >= 'a' && c <= 'z' && caps_lock)
        shifted = !shifted;

    return shifted ? keymap_shift[scancode] : c;
}

static void keyboard_handler(struct registers* regs) {
    (void)regs;
    uint8_t scancode = inb(KEYBOARD_DATA);

    switch (scancode) {
    case SC_LEFT_SHIFT:
    case SC_RIGHT_SHIFT:
        shift_held = 1;
        return;
    case SC_LEFT_SHIFT | SC_RELEASE:
    case SC_RIGHT_SHIFT | SC_RELEASE:
        shift_held = 0;
        return;
    case SC_CAPS_LOCK:
        caps_lock = !caps_lock;
        return;
    }

    if (scancode < SC_RELEASE) {
        char c = translate(scancode);
        if (c)
            vga_putc(c);
    }
}

void keyboard_init(void) {
    /* Drop any bytes left in the controller buffer by the BIOS/bootloader. */
    while (inb(KEYBOARD_STATUS) & 1)
        inb(KEYBOARD_DATA);

    register_interrupt_handler(IRQ(KEYBOARD_IRQ), keyboard_handler);
    pic_unmask_irq(KEYBOARD_IRQ);
}
