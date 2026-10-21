# MyOS Boot Protocol v1

## Grundsätzliches

- Magic: `MYOS_BOOT_MAGIC` (`"MYOSBOOT"` als `u64`).
- Version: `MYOS_BOOT_PROTOCOL_VERSION = 1`.
- Struktur: `struct myos_boot_info` in `bootloader/common/include/boot_protocol.h`.

## Kernel-Einstieg

- Einstiegspunkt: ELF64 `e_entry`.
- Aufruf: direkte Übergabe an Kernel-Einstieg (`_start` -> `kernel_main`).
- Registervertrag:
  - `RDI`: Zeiger auf `myos_boot_info`.
- Stack: 16-Byte ausgerichtet, durch Loader/Kernstub gesetzt.

## Erwarteter CPU-/Paging-Zustand

- CPU im 64-bit Long Mode.
- Paging aktiv.
- Segmentierung für 64-bit Code/Data gültig.

## Boot-Informationen

- `platform`: BIOS oder UEFI.
- `kernel_base`, `kernel_size`: geladener Kernelbereich.
- `memory_map`, `memory_map_entries`, `memory_map_entry_size`: übergebene Speicherkarte.
- `rsdp`: ACPI RSDP falls bekannt.
- `framebuffer`: optional (UEFI GOP).

## Lebensdauer der Daten

- Loader reserviert Übergabestrukturen so, dass sie beim Kernel-Einstieg gültig sind.
- Nach `ExitBootServices` darf der Kernel keine UEFI Boot Services mehr verwenden.
