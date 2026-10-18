#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TIMER_HZ 100

void timer_init(void);
uint32_t timer_ticks(void);
void timer_sleep(uint32_t ms);

/* Call fn from the timer interrupt on every tick. */
void timer_set_tick_handler(void (*fn)(void));

#endif
