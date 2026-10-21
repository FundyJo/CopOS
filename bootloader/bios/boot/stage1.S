.code16
.section .text
.global _start

.set STAGE2_LOAD_SEG, 0x0000
.set STAGE2_LOAD_OFF, 0x8000

_start:
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw $0x7C00, %sp
    sti

    movb %dl, boot_drive

    movw $dap, %si
    movb $0x42, %ah
    movb boot_drive, %dl
    int $0x13
    jc disk_error

    ljmp $STAGE2_LOAD_SEG, $STAGE2_LOAD_OFF

disk_error:
    movw $msg_disk_error, %si
    call puts
    cli
    hlt

puts:
    lodsb
    testb %al, %al
    jz .done
    movb $0x0E, %ah
    movw $0x0007, %bx
    int $0x10
    jmp puts
.done:
    ret

.align 4
dap:
    .byte 0x10
    .byte 0x00
stage2_sector_count:
    .word STAGE2_SECTORS
    .word STAGE2_LOAD_OFF
    .word STAGE2_LOAD_SEG
stage2_lba:
    .quad 1

boot_drive:
    .byte 0
msg_disk_error:
    .asciz "MyOS BIOS disk read failed"

.org 510
.word 0xAA55
