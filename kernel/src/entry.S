.section .text
.global _start
.type _start, @function
_start:
    lea kernel_stack_top(%rip), %rsp
    andq $-16, %rsp
    call kernel_main

1:
    cli
    hlt
    jmp 1b

.section .bss
.align 16
kernel_stack:
    .skip 16384
kernel_stack_top:

.section .note.GNU-stack,"",@progbits
