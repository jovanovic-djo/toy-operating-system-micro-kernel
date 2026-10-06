.section .text
.global irq1_stub
.extern keyboard_handler

# C functions may clobber eax/ecx/edx and expect DF clear,
# so save all general registers and return with iret.
irq1_stub:
    pusha
    cld
    call keyboard_handler
    popa
    iret

# Every vector gets a stub that pushes the same frame: an error code
# (a dummy 0 when the CPU does not push one) and the vector number.
.macro ISR_NOERR num
isr\num:
    push $0
    push $\num
    jmp isr_common
.endm

.macro ISR_ERR num
isr\num:
    push $\num
    jmp isr_common
.endm

# CPU exceptions 0-31. Only the second list gets an error code from the CPU.
.irp num, 0,1,2,3,4,5,6,7,9,15,16,18,19,20,22,23,24,25,26,27,28,31
    ISR_NOERR \num
.endr
.irp num, 8,10,11,12,13,14,17,21,29,30
    ISR_ERR \num
.endr

# Hardware IRQs 0-15, remapped to vectors 32-47.
.irp num, 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47
    ISR_NOERR \num
.endr

.extern isr_dispatch

isr_common:
    pusha
    cld
    push %esp               # struct registers *
    call isr_dispatch
    add $4, %esp
    popa
    add $8, %esp            # drop vector number and error code
    iret

.section .rodata
.global isr_stub_table
isr_stub_table:
.irp num, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47
    .long isr\num
.endr
