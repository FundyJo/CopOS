# MyOS

MyOS ist ein eigenes Bare-Metal-Betriebssystem (x86-64) mit eigenem BIOS- und UEFI-Bootpfad, freestanding C++20-Kernel und Rust-`xtask` Build-Orchestrierung.

## Status

- BIOS: eigener Stage-1-Bootsektor + Stage-2-Loader-Basis vorhanden.
- UEFI: eigener `BOOTX64.EFI`-Loader mit ELF64-Validierung und `ExitBootServices`-Retry vorhanden.
- Kernel: freestanding ELF64-Kernel gibt auf COM1 `MyOS: kernel entry reached` aus und hält an.
- ISO: Hybrid-ISO-Build-Pfad (BIOS El-Torito + UEFI El-Torito/FAT-Image) ist implementiert.
- QEMU/Rufus-Hardwaretests: nur möglich, wenn lokale Abhängigkeiten installiert sind.

## Voraussetzungen

- `gcc`, `g++`, `ld`, `objcopy`, `make`, `cargo`
- Für UEFI-Build: `gnu-efi` Header/Libs (`/usr/include/efi`, `/usr/lib/libefi.a`, `/usr/lib/libgnuefi.a`)
- Für ISO-Build: `xorriso`, `mkfs.fat`, `mtools` (`mcopy`, `mmd`)
- Für Tests: `qemu-system-x86_64`, SeaBIOS (QEMU-intern), OVMF (`OVMF_CODE.fd`, `OVMF_VARS.fd`)

Tool-Verfügbarkeit prüfen:

```bash
cargo xtask check
```

## Build

```bash
cargo xtask build
cargo xtask build-bios
cargo xtask build-uefi
cargo xtask build-iso
```

Artefakte:

- Kernel: `out/kernel.elf`
- BIOS Boot-Image: `out/bios/bios-boot.img`
- UEFI Loader: `out/uefi/BOOTX64.EFI`
- Hybrid-ISO: `out/myos.iso`

## QEMU-Start

BIOS (SeaBIOS):

```bash
cargo xtask run-bios
```

UEFI (OVMF):

```bash
cargo xtask run-uefi
```

Serielle Ausgabe läuft über COM1 (`-serial stdio`).

## Rufus-Hinweise

- Verwende `out/myos.iso`.
- Primär empfohlen: Rufus **DD-Modus** (verändert Hybrid-Struktur am wenigsten).
- ISO-Modus kann je nach Firmware/Board funktionieren, ist aber nicht auf jeder Hardware identisch.
- Secure Boot: aktuell **nicht unterstützt** (unsignierter eigener UEFI-Loader).
- Rufus-/Realhardware-Kompatibilität gilt erst als nachgewiesen, wenn reale Tests dokumentiert wurden.

## Dokumentation

- Architektur: `docs/ARCHITECTURE.md`
- Boot-ABI: `docs/BOOT_PROTOCOL.md`
- Build: `docs/BUILD.md`
- Tests: `docs/TESTING.md`
- Roadmap: `docs/ROADMAP.md`
