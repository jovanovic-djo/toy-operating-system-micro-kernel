.set MB_MAGIC,   0x1BADB002
.set MB_MEMINFO, 1 << 1          # ask for memory information
.set MB_FLAGS,   MB_MEMINFO

.section .multiboot
.align 4
.long MB_MAGIC
.long MB_FLAGS
.long -(MB_MAGIC + MB_FLAGS)

.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

.section .text
.global _start
.extern kernel_main

_start:
    cli
    mov $stack_top, %esp
    # kernel_main(magic, info): the bootloader left them in EAX and EBX.
    push %ebx
    push %eax
    call kernel_main
.hang:
    hlt
    jmp .hang
