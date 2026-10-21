OUT ?= out
CC ?= gcc
LD ?= ld
AS := gcc
OBJCOPY ?= objcopy

KERNEL_ELF ?= $(OUT)/kernel.elf
BIOS_OUT := $(OUT)/bios
STAGE1_BIN := $(BIOS_OUT)/stage1.bin
STAGE2_ELF := $(BIOS_OUT)/stage2.elf
STAGE2_BIN := $(BIOS_OUT)/stage2.bin
KERNEL_BLOB_OBJ := $(BIOS_OUT)/kernel_blob.o
KERNEL_BLOB_BIN := $(BIOS_OUT)/kernel.elf
BIOS_IMG := $(BIOS_OUT)/bios-boot.img

CFLAGS := -m32 -ffreestanding -fno-stack-protector -fno-pic -Wall -Wextra -Werror -I../common/include
ASFLAGS := -m32 -ffreestanding -c
LDFLAGS := -m elf_i386 -nostdlib -T linker.ld

STAGE2_OBJS := \
	$(BIOS_OUT)/entry.o \
	$(BIOS_OUT)/main.o \
	$(BIOS_OUT)/long_mode.o \
	$(KERNEL_BLOB_OBJ)

all: $(BIOS_IMG)

$(BIOS_OUT):
	mkdir -p $@

$(KERNEL_BLOB_BIN): $(KERNEL_ELF) | $(BIOS_OUT)
	cp $< $@

$(KERNEL_BLOB_OBJ): $(KERNEL_BLOB_BIN) | $(BIOS_OUT)
	cd $(BIOS_OUT) && $(OBJCOPY) -I binary -O elf32-i386 -B i386 kernel.elf kernel_blob.o

$(BIOS_OUT)/entry.o: stage2/entry.S | $(BIOS_OUT)
	$(AS) $(ASFLAGS) $< -o $@

$(BIOS_OUT)/main.o: stage2/main.c | $(BIOS_OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIOS_OUT)/long_mode.o: stage2/long_mode.S | $(BIOS_OUT)
	$(AS) $(ASFLAGS) $< -o $@

$(STAGE2_ELF): $(STAGE2_OBJS)
	$(LD) $(LDFLAGS) -o $@ $(STAGE2_OBJS)

$(STAGE2_BIN): $(STAGE2_ELF)
	$(OBJCOPY) -O binary $< $@

$(STAGE1_BIN): boot/stage1.S $(STAGE2_BIN) | $(BIOS_OUT)
	sectors=$$(( ( $$(stat -c%s $(STAGE2_BIN)) + 511 ) / 512 )); \
	if [ $$sectors -gt 65535 ]; then echo "stage2 too large"; exit 1; fi; \
	$(AS) -m16 -x assembler-with-cpp -DSTAGE2_SECTORS=$$sectors -c $< -o $(BIOS_OUT)/stage1.o; \
	$(LD) -m elf_i386 -Ttext 0x7C00 --oformat binary -nostdlib -o $@ $(BIOS_OUT)/stage1.o

$(BIOS_IMG): $(STAGE1_BIN) $(STAGE2_BIN)
	cat $(STAGE1_BIN) $(STAGE2_BIN) > $@

clean:
	rm -rf $(BIOS_OUT)

.PHONY: all clean
