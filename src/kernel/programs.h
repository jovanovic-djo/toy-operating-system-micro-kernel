#ifndef PROGRAMS_H
#define PROGRAMS_H

#include <stdint.h>

/* A user program image built from user/ and embedded in the kernel. */
struct program {
    const char* name;
    const uint8_t* start;
    const uint8_t* end;
};

extern const struct program programs[];
extern const unsigned program_count;

/* The program with this name, or NULL. */
const struct program* program_find(const char* name);

#endif
