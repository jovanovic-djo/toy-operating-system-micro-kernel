# User program images embedded in the kernel (see src/kernel/programs.c).
.section .rodata

.macro PROGRAM name
.global user_\name\()_start, user_\name\()_end
.balign 16
user_\name\()_start:
    .incbin "user/\name\().bin"
user_\name\()_end:
.endm

PROGRAM hello
PROGRAM fault
