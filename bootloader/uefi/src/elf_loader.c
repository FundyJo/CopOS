#include <efi.h>
#include <efilib.h>

#include "elf_loader.h"

#define EI_NIDENT 16
#define PT_LOAD 1
#define EM_X86_64 62
#define ET_EXEC 2
#define ELFMAG0 0x7F
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

struct elf64_ehdr {
    unsigned char e_ident[EI_NIDENT];
    UINT16 e_type;
    UINT16 e_machine;
    UINT32 e_version;
    UINT64 e_entry;
    UINT64 e_phoff;
    UINT64 e_shoff;
    UINT32 e_flags;
    UINT16 e_ehsize;
    UINT16 e_phentsize;
    UINT16 e_phnum;
    UINT16 e_shentsize;
    UINT16 e_shnum;
    UINT16 e_shstrndx;
};

struct elf64_phdr {
    UINT32 p_type;
    UINT32 p_flags;
    UINT64 p_offset;
    UINT64 p_vaddr;
    UINT64 p_paddr;
    UINT64 p_filesz;
    UINT64 p_memsz;
    UINT64 p_align;
};

struct segment_range {
    UINT64 start;
    UINT64 end;
};

static BOOLEAN ranges_overlap(UINT64 a_start, UINT64 a_end, UINT64 b_start, UINT64 b_end) {
    return !(a_end <= b_start || b_end <= a_start);
}

static BOOLEAN valid_kernel_range(UINT64 base, UINT64 size) {
    if (size == 0) {
        return TRUE;
    }
    if (base < 0x00100000ULL) {
        return FALSE;
    }
    if (base + size < base) {
        return FALSE;
    }
    if (base + size > 0x0000FFFFFFFFF000ULL) {
        return FALSE;
    }
    return TRUE;
}

EFI_STATUS myos_uefi_load_kernel(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table,
    VOID *kernel_image,
    UINTN kernel_size,
    struct myos_boot_info *boot_info,
    EFI_PHYSICAL_ADDRESS *entry_out
) {
    (void)image_handle;
    EFI_BOOT_SERVICES *bs = system_table->BootServices;

    if (kernel_size < sizeof(struct elf64_ehdr)) {
        return EFI_LOAD_ERROR;
    }

    struct elf64_ehdr *ehdr = (struct elf64_ehdr *)kernel_image;
    if (ehdr->e_ident[0] != ELFMAG0 || ehdr->e_ident[1] != ELFMAG1 || ehdr->e_ident[2] != ELFMAG2 || ehdr->e_ident[3] != ELFMAG3) {
        return EFI_LOAD_ERROR;
    }
    if (ehdr->e_ident[4] != 2 || ehdr->e_ident[5] != 1) {
        return EFI_UNSUPPORTED;
    }
    if (ehdr->e_type != ET_EXEC || ehdr->e_machine != EM_X86_64) {
        return EFI_UNSUPPORTED;
    }
    if (ehdr->e_phentsize != sizeof(struct elf64_phdr)) {
        return EFI_LOAD_ERROR;
    }
    if (ehdr->e_phoff + ((UINT64)ehdr->e_phnum * sizeof(struct elf64_phdr)) > kernel_size) {
        return EFI_LOAD_ERROR;
    }

    struct elf64_phdr *phdrs = (struct elf64_phdr *)((UINT8 *)kernel_image + ehdr->e_phoff);
    struct segment_range ranges[64];
    UINTN range_count = 0;
    UINT64 low = ~0ULL;
    UINT64 high = 0;

    for (UINT16 i = 0; i < ehdr->e_phnum; ++i) {
        struct elf64_phdr *ph = &phdrs[i];
        if (ph->p_type != PT_LOAD) {
            continue;
        }
        if (range_count >= 64) {
            return EFI_OUT_OF_RESOURCES;
        }
        if (ph->p_filesz > ph->p_memsz) {
            return EFI_LOAD_ERROR;
        }
        if (ph->p_offset + ph->p_filesz > kernel_size) {
            return EFI_LOAD_ERROR;
        }

        UINT64 base = ph->p_paddr ? ph->p_paddr : ph->p_vaddr;
        if (!valid_kernel_range(base, ph->p_memsz)) {
            return EFI_LOAD_ERROR;
        }

        UINT64 seg_end = base + ph->p_memsz;
        for (UINTN r = 0; r < range_count; ++r) {
            if (ranges_overlap(base, seg_end, ranges[r].start, ranges[r].end)) {
                return EFI_LOAD_ERROR;
            }
        }

        ranges[range_count].start = base;
        ranges[range_count].end = seg_end;
        ++range_count;

        EFI_PHYSICAL_ADDRESS target = (EFI_PHYSICAL_ADDRESS)base;
        UINTN pages = EFI_SIZE_TO_PAGES(ph->p_memsz);
        EFI_STATUS st = bs->AllocatePages(AllocateAddress, EfiLoaderData, pages, &target);
        if (EFI_ERROR(st)) {
            return st;
        }

        CopyMem((VOID *)(UINTN)target, (UINT8 *)kernel_image + ph->p_offset, ph->p_filesz);
        if (ph->p_memsz > ph->p_filesz) {
            SetMem((VOID *)(UINTN)(target + ph->p_filesz), ph->p_memsz - ph->p_filesz, 0);
        }

        if (base < low) {
            low = base;
        }
        if (seg_end > high) {
            high = seg_end;
        }
    }

    if (!valid_kernel_range(ehdr->e_entry, 1)) {
        return EFI_LOAD_ERROR;
    }

    boot_info->kernel_base = low;
    boot_info->kernel_size = high - low;
    *entry_out = (EFI_PHYSICAL_ADDRESS)ehdr->e_entry;
    return EFI_SUCCESS;
}
