# Build

## Kommandos

```bash
cargo xtask check
cargo xtask build
cargo xtask build-bios
cargo xtask build-uefi
cargo xtask build-iso
cargo xtask run-bios
cargo xtask run-uefi
cargo xtask clean
```

## Build-Pipeline

- `build-kernel`: kompiliert `out/kernel.elf`.
- `build-bios`: baut Stage 1/2 und `out/bios/bios-boot.img`.
- `build-uefi`: baut `out/uefi/BOOTX64.EFI`.
- `build-iso`: erstellt FAT-EFI-Image und Hybrid-ISO (`out/myos.iso`).

## Hinweise

- Fehlende externe Tools werden in `check` klar gemeldet.
- Das Build installiert keine Systempakete automatisch.
