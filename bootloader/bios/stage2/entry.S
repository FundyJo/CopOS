.code16
.section .text
.global stage2_entry

stage2_entry:
    cli

    lgdt gdt_descriptor

    movl %cr0, %eax
    orl $0x1, %eax
    movl %eax, %cr0

    ljmpl $0x08, $protected_mode_entry

.code32
protected_mode_entry:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss
    movl $0x90000, %esp

    movzbl %dl, %eax
    pushl %eax
    call stage2_main
1:
    cli
    hlt
    jmp 1b

.align 8
gdt:
    .quad 0x0000000000000000
    .quad 0x00CF9A000000FFFF
    .quad 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    .word gdt_end - gdt - 1
    .long gdt

.section .note.GNU-stack,"",@progbits
