#ifndef MYOS_BOOT_PROTOCOL_H
#define MYOS_BOOT_PROTOCOL_H

#include <stdint.h>

#define MYOS_BOOT_MAGIC 0x4D594F53424F4F54ULL /* MYOSBOOT */
#define MYOS_BOOT_PROTOCOL_VERSION 1

enum myos_boot_platform {
    MYOS_BOOT_PLATFORM_BIOS = 1,
    MYOS_BOOT_PLATFORM_UEFI = 2,
};

enum myos_memory_type {
    MYOS_MEM_USABLE = 1,
    MYOS_MEM_RESERVED = 2,
    MYOS_MEM_ACPI_RECLAIM = 3,
    MYOS_MEM_ACPI_NVS = 4,
    MYOS_MEM_MMIO = 5,
    MYOS_MEM_BOOTLOADER_RECLAIM = 6,
};

struct myos_memory_map_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

struct myos_framebuffer_info {
    uint64_t address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
    uint8_t memory_model;
    uint16_t reserved;
};

struct myos_boot_info {
    uint64_t magic;
    uint32_t version;
    uint32_t size;
    uint32_t platform;
    uint32_t flags;
    uint64_t loader_base;
    uint64_t loader_size;
    uint64_t kernel_base;
    uint64_t kernel_size;
    uint64_t rsdp;
    uint64_t memory_map;
    uint32_t memory_map_entries;
    uint32_t memory_map_entry_size;
    struct myos_framebuffer_info framebuffer;
    uint8_t reserved[64];
};

#endif
