.section .text

# void enter_user_mode(uint32_t eip, uint32_t esp) - does not return
#
# Build the frame iret expects for a return to a lower privilege level
# (SS, ESP, EFLAGS, CS, EIP) with the user segments and interrupts
# enabled, and clear the general registers so no kernel values leak.
.global enter_user_mode
enter_user_mode:
    cli
    mov 4(%esp), %ecx           # eip
    mov 8(%esp), %edx           # esp

    mov $0x23, %ax              # GDT_USER_DATA
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push $0x23                  # ss
    push %edx                   # esp
    pushf
    orl $0x200, (%esp)          # IF: interrupts on in user mode
    push $0x1B                  # cs: GDT_USER_CODE
    push %ecx                   # eip

    xor %eax, %eax
    xor %ebx, %ebx
    xor %ecx, %ecx
    xor %edx, %edx
    xor %esi, %esi
    xor %edi, %edi
    xor %ebp, %ebp
    iret
