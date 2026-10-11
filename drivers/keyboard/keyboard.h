#ifndef KEYBOARD_H
#define KEYBOARD_H

void keyboard_init(void);

/* Blocks until a key is pressed. Requires interrupts to be enabled. */
char keyboard_getchar(void);

#endif
