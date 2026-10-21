OUT ?= out
CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy

UEFI_OUT := $(OUT)/uefi
EFI_BIN := $(UEFI_OUT)/BOOTX64.EFI
CFLAGS := -ffreestanding -fshort-wchar -fno-stack-protector -mno-red-zone -Wall -Wextra -Werror -I../common/include -I/usr/include/efi -I/usr/include/efi/x86_64
LDFLAGS := -nostdlib -znocombreloc -shared -Bsymbolic -L/usr/lib -lefi -lgnuefi -T /usr/lib/elf_x86_64_efi.lds

OBJS := \
	$(UEFI_OUT)/main.o \
	$(UEFI_OUT)/elf_loader.o

all: $(EFI_BIN)

$(UEFI_OUT)/.deps-check: | $(UEFI_OUT)
	@test -f /usr/include/efi/efi.h || { echo "missing gnu-efi headers at /usr/include/efi"; exit 1; }
	@test -f /usr/lib/elf_x86_64_efi.lds || { echo "missing gnu-efi linker script at /usr/lib/elf_x86_64_efi.lds"; exit 1; }
	@test -f /usr/lib/libefi.a || { echo "missing libefi.a at /usr/lib/libefi.a"; exit 1; }
	@test -f /usr/lib/libgnuefi.a || { echo "missing libgnuefi.a at /usr/lib/libgnuefi.a"; exit 1; }
	@touch $@

$(UEFI_OUT):
	mkdir -p $@

$(UEFI_OUT)/main.o: src/main.c src/elf_loader.h $(UEFI_OUT)/.deps-check
	$(CC) $(CFLAGS) -c $< -o $@

$(UEFI_OUT)/elf_loader.o: src/elf_loader.c src/elf_loader.h $(UEFI_OUT)/.deps-check
	$(CC) $(CFLAGS) -c $< -o $@

$(UEFI_OUT)/loader.so: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(EFI_BIN): $(UEFI_OUT)/loader.so
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc --target=efi-app-x86_64 $< $@

clean:
	rm -rf $(UEFI_OUT)

.PHONY: all clean
