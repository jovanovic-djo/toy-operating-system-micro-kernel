// #include <stdint.h>

unsigned char scancode;

void keyboard_handler() {
    scancode = inb(0x60);

    char* video = (char*) 0xb8000;
    video[0] = '0' + (scancode % 10);
}