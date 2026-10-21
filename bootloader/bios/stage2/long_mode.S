.code32
.section .text
.global bios_enter_long_mode
.type bios_enter_long_mode, @function

bios_enter_long_mode:
    pushl %ebp
    movl %esp, %ebp

    /* args: [ebp+8]=entry low32 (assumes <4G), [ebp+12]=entry high32, [ebp+16]=boot_info ptr */
    movl 8(%ebp), %eax
    movl 12(%ebp), %edx
    testl %edx, %edx
    jnz .fail

    movl %eax, kernel_entry32
    movl 16(%ebp), %eax
    movl %eax, boot_info_ptr32

    movl $pml4_table, %eax
    movl %eax, %cr3

    movl %cr4, %eax
    orl $0x20, %eax
    movl %eax, %cr4

    movl $0xC0000080, %ecx
    rdmsr
    orl $0x100, %eax
    wrmsr

    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0

    lgdt gdt64_descriptor
    ljmpl $0x08, $long_mode_entry

.fail:
    popl %ebp
    ret

.code64
long_mode_entry:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw %ax, %fs
    movw %ax, %gs

    movq $0x200000, %rsp
    movl boot_info_ptr32(%rip), %edi
    movl kernel_entry32(%rip), %eax
    jmp *%rax

.align 8
gdt64:
    .quad 0x0000000000000000
    .quad 0x00AF9A000000FFFF
    .quad 0x00AF92000000FFFF
gdt64_end:

gdt64_descriptor:
    .word gdt64_end - gdt64 - 1
    .long gdt64

.align 4096
pml4_table:
    .long pdpt_table + 0x003
    .long 0
    .fill 511, 8, 0

.align 4096
pdpt_table:
    .long pd_table + 0x003
    .long 0
    .fill 511, 8, 0

.align 4096
pd_table:
    .set i, 0
    .rept 512
    .long (i * 0x200000) + 0x083
    .long 0
    .set i, i + 1
    .endr

.align 8
kernel_entry32:
    .long 0
boot_info_ptr32:
    .long 0

.section .note.GNU-stack,"",@progbits
