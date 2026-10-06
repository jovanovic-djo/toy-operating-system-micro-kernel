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
