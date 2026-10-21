# Testing

## Tatsächlich ausgeführte Prüfungen

Ausgeführt am 2026-10-21 im bereitgestellten CI-Workspace:

1. `cargo xtask check`
2. `cargo xtask build-bios`
3. `cargo xtask build-uefi`
4. `cargo xtask build`

Ergebnis:

- `check`: erfolgreich; meldet fehlende optionale Tools in der Umgebung (`qemu-system-x86_64`, `xorriso`, `mcopy`, OVMF-Dateien).
- `build-bios`: erfolgreich; erzeugt `out/kernel.elf` und `out/bios/bios-boot.img`.
- `build-uefi`: nicht erfolgreich wegen fehlender `gnu-efi`-Header/Libs (`/usr/include/efi`, `/usr/lib/elf_x86_64_efi.lds`, `libefi.a`, `libgnuefi.a`).
- `build`: bricht wie erwartet am UEFI-Schritt wegen derselben fehlenden Abhängigkeiten ab.

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
