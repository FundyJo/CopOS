OUT ?= out
CC ?= gcc
CXX ?= g++
LD ?= ld
AS := gcc

KERNEL_ELF := $(OUT)/kernel.elf
CFLAGS := -ffreestanding -fno-stack-protector -fno-pic -m64 -Wall -Wextra -Werror -Iinclude -I../bootloader/common/include
CXXFLAGS := $(CFLAGS) -std=c++20 -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics
ASFLAGS := -ffreestanding -m64 -c
LDFLAGS := -nostdlib -z max-page-size=0x1000 -T linker.ld

OBJS := \
	$(OUT)/kernel/entry.o \
	$(OUT)/kernel/main.o \
	$(OUT)/kernel/serial.o \
	$(OUT)/kernel/memory.o

all: $(KERNEL_ELF)

$(OUT)/kernel:
	mkdir -p $@

$(OUT)/kernel/entry.o: src/entry.S | $(OUT)/kernel
	$(AS) $(ASFLAGS) $< -o $@

$(OUT)/kernel/main.o: src/main.cpp | $(OUT)/kernel
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OUT)/kernel/serial.o: src/arch/x86_64/serial.c | $(OUT)/kernel
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/kernel/memory.o: src/runtime/memory.c | $(OUT)/kernel
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

clean:
	rm -rf $(OUT)/kernel $(KERNEL_ELF)

.PHONY: all clean
