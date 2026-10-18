# Entry point of every user program, linked first at USER_START.
# Calls main() and passes its return value to the exit system call.
.section .text.entry
.global _start
.extern main

_start:
    call main
    mov %eax, %ebx          # exit code
    mov $0, %eax            # SYS_EXIT
    int $0x80
1:
    jmp 1b                  # exit does not return
