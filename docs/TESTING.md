# Testing

## Tatsächlich ausgeführte Prüfungen

- `cargo xtask check`: Toolchain-Prüfung.
- Lokaler Compile/Build je nach verfügbarer Host-Toolchain (siehe Abschlussbericht).

## BIOS Boot-Testplan

1. `cargo xtask run-bios`
2. Prüfen: Stage1 -> Stage2 -> Kernel-Handoff
3. Prüfen: serielle Ausgabe `MyOS: kernel entry reached`

## UEFI Boot-Testplan

1. `cargo xtask run-uefi`
2. Prüfen: `BOOTX64.EFI` startet, lädt `kernel.elf`, `ExitBootServices` erfolgreich
3. Prüfen: serielle Ausgabe `MyOS: kernel entry reached`

## Einschränkungen

- Wenn `qemu-system-x86_64`, `xorriso`, OVMF oder UEFI-Bibliotheken fehlen, sind vollständige Boottests lokal nicht möglich.
- Rufus-/Realhardwaretests müssen separat dokumentiert werden.
