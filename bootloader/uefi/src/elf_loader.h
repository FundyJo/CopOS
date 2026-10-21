#ifndef MYOS_UEFI_ELF_LOADER_H
#define MYOS_UEFI_ELF_LOADER_H

#include <efi.h>

#include "../../common/include/boot_protocol.h"

EFI_STATUS myos_uefi_load_kernel(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table,
    VOID *kernel_image,
    UINTN kernel_size,
    struct myos_boot_info *boot_info,
    EFI_PHYSICAL_ADDRESS *entry_out
);

#endif
