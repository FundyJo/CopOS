# MyOS Architektur

## Module

- `kernel/`: freestanding C++20/C Kernel (ELF64), serieller Output, eigener Einstieg.
- `bootloader/bios/`: eigener Legacy-BIOS Bootpfad.
  - `boot/stage1.S`: 16-bit Bootsektor, lädt Stage 2 per INT 13h Extensions.
  - `stage2/`: Protected-Mode-Loader, ELF-Validierung/-Laden, Long-Mode-Handoff.
- `bootloader/uefi/`: eigener x86-64 UEFI-Loader (`BOOTX64.EFI`).
- `bootloader/common/include/boot_protocol.h`: gemeinsame Boot-ABI.
- `tools/xtask/`: Rust-Orchestrierung für Check/Build/Run/Clean.

## Datenfluss

1. BIOS oder UEFI startet den jeweiligen Loader.
2. Loader validiert und lädt `kernel.elf`.
3. Loader erstellt `myos_boot_info` nach Boot-Protokoll.
4. Loader übergibt in 64-bit Zustand an Kernel-Einstieg.
5. Kernel validiert Boot-Info und schreibt auf COM1.
