#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../../common/include/boot_protocol.h"

#define EI_NIDENT 16
#define PT_LOAD 1
#define EM_X86_64 62
#define ET_EXEC 2
#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'
#define PAGE_2M 0x200000ULL

struct elf64_ehdr {
    unsigned char e_ident[EI_NIDENT];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};

struct elf64_phdr {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};

extern const uint8_t _binary_kernel_elf_start[];
extern const uint8_t _binary_kernel_elf_end[];
extern void bios_enter_long_mode(uint64_t entry, const struct myos_boot_info *boot_info);

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void serial_init(void) {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

static void serial_putc(char c) {
    while ((inb(0x3F8 + 5) & 0x20) == 0) {
    }
    outb(0x3F8, (uint8_t)c);
}

static void serial_puts(const char *s) {
    while (*s != '\0') {
        if (*s == '\n') {
            serial_putc('\r');
        }
        serial_putc(*s++);
    }
}

static void *memcpy_local(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
    return dst;
}

static void *memset_local(void *dst, int val, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    for (size_t i = 0; i < n; ++i) {
        d[i] = (uint8_t)val;
    }
    return dst;
}

static bool check_range(uint64_t base, uint64_t size) {
    if (size == 0) {
        return true;
    }
    if (base < 0x00100000ULL) {
        return false;
    }
    if (base + size < base) {
        return false;
    }
    if (base + size > 0x3FFFFFFFULL) {
        return false;
    }
    return true;
}

static bool load_kernel(const struct elf64_ehdr *ehdr, uint64_t *entry_out, uint64_t *kernel_base, uint64_t *kernel_end) {
    if (ehdr->e_ident[0] != ELFMAG0 || ehdr->e_ident[1] != ELFMAG1 || ehdr->e_ident[2] != ELFMAG2 || ehdr->e_ident[3] != ELFMAG3) {
        serial_puts("MyOS BIOS: invalid ELF magic\\n");
        return false;
    }
    if (ehdr->e_ident[4] != 2 || ehdr->e_ident[5] != 1) {
        serial_puts("MyOS BIOS: unsupported ELF class/data\\n");
        return false;
    }
    if (ehdr->e_type != ET_EXEC || ehdr->e_machine != EM_X86_64) {
        serial_puts("MyOS BIOS: wrong ELF type/machine\\n");
        return false;
    }
    if (ehdr->e_phentsize != sizeof(struct elf64_phdr)) {
        serial_puts("MyOS BIOS: bad phdr size\\n");
        return false;
    }

    const uint8_t *img = (const uint8_t *)ehdr;
    const struct elf64_phdr *phdrs = (const struct elf64_phdr *)(img + ehdr->e_phoff);
    uint64_t low = UINT64_MAX;
    uint64_t high = 0;

    for (uint16_t i = 0; i < ehdr->e_phnum; ++i) {
        const struct elf64_phdr *ph = &phdrs[i];
        if (ph->p_type != PT_LOAD) {
            continue;
        }
        if (ph->p_filesz > ph->p_memsz) {
            serial_puts("MyOS BIOS: p_filesz > p_memsz\\n");
            return false;
        }

        uint64_t dst = ph->p_paddr ? ph->p_paddr : ph->p_vaddr;
        if (!check_range(dst, ph->p_memsz)) {
            serial_puts("MyOS BIOS: segment out of range\\n");
            return false;
        }

        if (dst < low) {
            low = dst;
        }
        if (dst + ph->p_memsz > high) {
            high = dst + ph->p_memsz;
        }

        memcpy_local((void *)(uintptr_t)dst, img + ph->p_offset, (size_t)ph->p_filesz);
        if (ph->p_memsz > ph->p_filesz) {
            memset_local((void *)(uintptr_t)(dst + ph->p_filesz), 0, (size_t)(ph->p_memsz - ph->p_filesz));
        }
    }

    if (!check_range(ehdr->e_entry, 1)) {
        serial_puts("MyOS BIOS: entry out of range\\n");
        return false;
    }

    *entry_out = ehdr->e_entry;
    *kernel_base = low;
    *kernel_end = high;
    return true;
}

static struct myos_boot_info g_boot_info;
static struct myos_memory_map_entry g_memory_map[1];

void stage2_main(uint32_t bios_drive) {
    (void)bios_drive;
    serial_init();
    serial_puts("MyOS BIOS: stage2 entered\\n");

    const struct elf64_ehdr *ehdr = (const struct elf64_ehdr *)_binary_kernel_elf_start;
    uint64_t entry = 0;
    uint64_t kernel_base = 0;
    uint64_t kernel_end = 0;

    if (!load_kernel(ehdr, &entry, &kernel_base, &kernel_end)) {
        serial_puts("MyOS BIOS: kernel load failed\\n");
        for (;;) {
            __asm__ volatile("cli; hlt");
        }
    }

    g_memory_map[0].base = 0x00100000ULL;
    g_memory_map[0].length = 0x3FF00000ULL;
    g_memory_map[0].type = MYOS_MEM_USABLE;
    g_memory_map[0].reserved = 0;

    memset_local(&g_boot_info, 0, sizeof(g_boot_info));
    g_boot_info.magic = MYOS_BOOT_MAGIC;
    g_boot_info.version = MYOS_BOOT_PROTOCOL_VERSION;
    g_boot_info.size = sizeof(g_boot_info);
    g_boot_info.platform = MYOS_BOOT_PLATFORM_BIOS;
    g_boot_info.loader_base = 0x7C00;
    g_boot_info.loader_size = (uint64_t)(_binary_kernel_elf_start - (const uint8_t *)0x7C00);
    g_boot_info.kernel_base = kernel_base;
    g_boot_info.kernel_size = kernel_end - kernel_base;
    g_boot_info.memory_map = (uint64_t)(uintptr_t)&g_memory_map[0];
    g_boot_info.memory_map_entries = 1;
    g_boot_info.memory_map_entry_size = sizeof(struct myos_memory_map_entry);

    serial_puts("MyOS BIOS: switching to long mode\\n");
    bios_enter_long_mode(entry, &g_boot_info);

    serial_puts("MyOS BIOS: long mode handoff failed\\n");
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
