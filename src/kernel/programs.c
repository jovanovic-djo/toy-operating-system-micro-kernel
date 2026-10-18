#include <stddef.h>
#include "programs.h"
#include "string.h"

/* Symbols defined in user/programs.s. */
#define PROGRAM(name) \
    extern const uint8_t user_##name##_start[], user_##name##_end[];
PROGRAM(hello)
#undef PROGRAM

#define PROGRAM(name) { #name, user_##name##_start, user_##name##_end }
const struct program programs[] = {
    PROGRAM(hello),
};
#undef PROGRAM

const unsigned program_count = sizeof(programs) / sizeof(programs[0]);

const struct program* program_find(const char* name) {
    for (unsigned i = 0; i < program_count; i++)
        if (strcmp(programs[i].name, name) == 0)
            return &programs[i];
    return NULL;
}
