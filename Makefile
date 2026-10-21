OUT := $(abspath out)
ISO_ROOT := $(OUT)/iso-root
EFI_IMG := $(OUT)/efi.img
ISO_PATH := $(OUT)/myos.iso

KERNEL_ELF := $(OUT)/kernel.elf
BIOS_IMG := $(OUT)/bios/bios-boot.img
UEFI_EFI := $(OUT)/uefi/BOOTX64.EFI

QEMU ?= qemu-system-x86_64
OVMF_CODE ?= /usr/share/OVMF/OVMF_CODE.fd
OVMF_VARS ?= /usr/share/OVMF/OVMF_VARS.fd

.PHONY: check build build-kernel build-bios build-uefi build-iso run-bios run-uefi clean

check:
	@echo "== MyOS toolchain check =="
	@command -v gcc >/dev/null || { echo "missing: gcc"; exit 1; }
	@command -v g++ >/dev/null || { echo "missing: g++"; exit 1; }
	@command -v ld >/dev/null || { echo "missing: ld"; exit 1; }
	@command -v objcopy >/dev/null || { echo "missing: objcopy"; exit 1; }
	@command -v make >/dev/null || { echo "missing: make"; exit 1; }
	@command -v cargo >/dev/null || { echo "missing: cargo"; exit 1; }
	@command -v $(QEMU) >/dev/null || echo "warning: missing qemu-system-x86_64"
	@command -v xorriso >/dev/null || echo "warning: missing xorriso"
	@command -v mkfs.fat >/dev/null || echo "warning: missing mkfs.fat"
	@command -v mcopy >/dev/null || echo "warning: missing mtools (mcopy)"
	@[ -f "$(OVMF_CODE)" ] || echo "warning: missing OVMF code image at $(OVMF_CODE)"
	@[ -f "$(OVMF_VARS)" ] || echo "warning: missing OVMF vars image at $(OVMF_VARS)"

build: build-iso

build-kernel:
	$(MAKE) -C kernel OUT=$(OUT)

build-bios: build-kernel
	$(MAKE) -C bootloader/bios OUT=$(OUT) KERNEL_ELF=$(KERNEL_ELF)

build-uefi: build-kernel
	$(MAKE) -C bootloader/uefi OUT=$(OUT)

$(EFI_IMG): $(UEFI_EFI) $(KERNEL_ELF)
	@command -v mkfs.fat >/dev/null || { echo "mkfs.fat is required for EFI image"; exit 1; }
	@command -v mcopy >/dev/null || { echo "mcopy is required for EFI image"; exit 1; }
	mkdir -p $(OUT)
	rm -f $@
	dd if=/dev/zero of=$@ bs=1M count=16 status=none
	mkfs.fat $@ >/dev/null
	mmd -i $@ ::/EFI ::/EFI/BOOT
	mcopy -i $@ $(UEFI_EFI) ::/EFI/BOOT/BOOTX64.EFI
	mcopy -i $@ $(KERNEL_ELF) ::/kernel.elf

build-iso: build-bios build-uefi $(EFI_IMG)
	@command -v xorriso >/dev/null || { echo "xorriso is required for ISO generation"; exit 1; }
	mkdir -p $(OUT)/iso-stage/EFI $(OUT)/iso-stage/EFI/BOOT
	cp $(EFI_IMG) $(OUT)/iso-stage/EFI/efiboot.img
	cp $(BIOS_IMG) $(OUT)/iso-stage/boot.img
	cp $(KERNEL_ELF) $(OUT)/iso-stage/kernel.elf
	cp $(UEFI_EFI) $(OUT)/iso-stage/EFI/BOOT/BOOTX64.EFI

	rm -f $(ISO_PATH)
	MBR_OPT=""; \
	if [ -f /usr/lib/ISOLINUX/isohdpfx.bin ]; then MBR_OPT="-isohybrid-mbr /usr/lib/ISOLINUX/isohdpfx.bin"; fi; \
	xorriso -as mkisofs \
		-R -J \
		-V MYOS \
		-o $(ISO_PATH) \
		-c boot/boot.cat \
		-b boot.img \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		-eltorito-alt-boot \
		-e EFI/efiboot.img \
		-no-emul-boot \
		-isohybrid-gpt-basdat \
		$$MBR_OPT \
		$(OUT)/iso-stage

run-bios: build-iso
	@command -v $(QEMU) >/dev/null || { echo "qemu-system-x86_64 is required"; exit 1; }
	$(QEMU) -cdrom $(ISO_PATH) -serial stdio -no-reboot -no-shutdown

run-uefi: build-iso
	@command -v $(QEMU) >/dev/null || { echo "qemu-system-x86_64 is required"; exit 1; }
	@[ -f "$(OVMF_CODE)" ] || { echo "missing OVMF firmware: $(OVMF_CODE)"; exit 1; }
	@[ -f "$(OVMF_VARS)" ] || { echo "missing OVMF variables: $(OVMF_VARS)"; exit 1; }
	cp $(OVMF_VARS) $(OUT)/OVMF_VARS.fd
	$(QEMU) \
		-drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
		-drive if=pflash,format=raw,file=$(OUT)/OVMF_VARS.fd \
		-cdrom $(ISO_PATH) \
		-serial stdio -no-reboot -no-shutdown

clean:
	rm -rf $(OUT)
	$(MAKE) -C kernel clean OUT=$(OUT)
	$(MAKE) -C bootloader/bios clean OUT=$(OUT)
	$(MAKE) -C bootloader/uefi clean OUT=$(OUT)
