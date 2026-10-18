.section .text

# void context_switch(uint32_t* old_esp, uint32_t new_esp)
#
# Save the callee-saved registers on the current stack, store the stack
# pointer in *old_esp, then switch to new_esp and restore the registers the
# other thread saved the same way. The C compiler already saves everything
# else around the call, so this is all a thread switch needs.
.global context_switch
context_switch:
    mov 4(%esp), %eax
    mov 8(%esp), %edx

    push %ebp
    push %ebx
    push %esi
    push %edi
    mov %esp, (%eax)

    mov %edx, %esp
    pop %edi
    pop %esi
    pop %ebx
    pop %ebp
    ret
