#include <efi.h>
#include <efilib.h>

#include "elf_loader.h"

#define KERNEL_PATH L"\\kernel.elf"

static EFI_STATUS read_file(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *st, CHAR16 *path, VOID **out_buf, UINTN *out_size) {
    EFI_STATUS status;
    EFI_LOADED_IMAGE *loaded = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fs = NULL;
    EFI_FILE_HANDLE root = NULL;
    EFI_FILE_HANDLE file = NULL;
    EFI_FILE_INFO *info = NULL;
    UINTN info_size = 0;

    status = st->BootServices->HandleProtocol(image_handle, &gEfiLoadedImageProtocolGuid, (VOID **)&loaded);
    if (EFI_ERROR(status)) {
        return status;
    }

    status = st->BootServices->HandleProtocol(loaded->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&fs);
    if (EFI_ERROR(status)) {
        return status;
    }

    status = fs->OpenVolume(fs, &root);
    if (EFI_ERROR(status)) {
        return status;
    }

    status = root->Open(root, &file, path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(status)) {
        root->Close(root);
        return status;
    }

    status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, NULL);
    if (status != EFI_BUFFER_TOO_SMALL) {
        file->Close(file);
        root->Close(root);
        return status;
    }

    status = st->BootServices->AllocatePool(EfiLoaderData, info_size, (VOID **)&info);
    if (EFI_ERROR(status)) {
        file->Close(file);
        root->Close(root);
        return status;
    }

    status = file->GetInfo(file, &gEfiFileInfoGuid, &info_size, info);
    if (EFI_ERROR(status)) {
        st->BootServices->FreePool(info);
        file->Close(file);
        root->Close(root);
        return status;
    }

    *out_size = (UINTN)info->FileSize;
    status = st->BootServices->AllocatePool(EfiLoaderData, *out_size, out_buf);
    if (EFI_ERROR(status)) {
        st->BootServices->FreePool(info);
        file->Close(file);
        root->Close(root);
        return status;
    }

    status = file->Read(file, out_size, *out_buf);

    st->BootServices->FreePool(info);
    file->Close(file);
    root->Close(root);
    return status;
}

static UINT64 find_rsdp(EFI_SYSTEM_TABLE *st) {
    EFI_GUID acpi20 = ACPI_20_TABLE_GUID;
    EFI_GUID acpi10 = ACPI_TABLE_GUID;

    for (UINTN i = 0; i < st->NumberOfTableEntries; ++i) {
        EFI_CONFIGURATION_TABLE *tbl = &st->ConfigurationTable[i];
        if (CompareGuid(&tbl->VendorGuid, &acpi20) || CompareGuid(&tbl->VendorGuid, &acpi10)) {
            return (UINT64)(UINTN)tbl->VendorTable;
        }
    }

    return 0;
}

static void fill_framebuffer(EFI_SYSTEM_TABLE *st, struct myos_boot_info *boot_info) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    EFI_STATUS status = st->BootServices->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID **)&gop);
    if (EFI_ERROR(status)) {
        return;
    }

    boot_info->framebuffer.address = gop->Mode->FrameBufferBase;
    boot_info->framebuffer.width = gop->Mode->Info->HorizontalResolution;
    boot_info->framebuffer.height = gop->Mode->Info->VerticalResolution;
    boot_info->framebuffer.pitch = gop->Mode->Info->PixelsPerScanLine * 4;
    boot_info->framebuffer.bpp = 32;
    boot_info->framebuffer.memory_model = 1;
}

EFI_STATUS efi_main(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *system_table) {
    InitializeLib(image_handle, system_table);

    Print(L"MyOS UEFI loader start\\r\\n");

    VOID *kernel_buf = NULL;
    UINTN kernel_size = 0;
    EFI_STATUS status = read_file(image_handle, system_table, KERNEL_PATH, &kernel_buf, &kernel_size);
    if (EFI_ERROR(status)) {
        Print(L"MyOS UEFI: failed to open kernel.elf: %r\\r\\n", status);
        return status;
    }

    struct myos_boot_info *boot_info = NULL;
    status = system_table->BootServices->AllocatePool(EfiLoaderData, sizeof(*boot_info), (VOID **)&boot_info);
    if (EFI_ERROR(status)) {
        Print(L"MyOS UEFI: failed to alloc boot info: %r\\r\\n", status);
        return status;
    }

    SetMem(boot_info, sizeof(*boot_info), 0);
    boot_info->magic = MYOS_BOOT_MAGIC;
    boot_info->version = MYOS_BOOT_PROTOCOL_VERSION;
    boot_info->size = sizeof(*boot_info);
    boot_info->platform = MYOS_BOOT_PLATFORM_UEFI;
    boot_info->loader_base = (UINT64)(UINTN)image_handle;
    boot_info->loader_size = 0;
    boot_info->rsdp = find_rsdp(system_table);
    fill_framebuffer(system_table, boot_info);

    EFI_PHYSICAL_ADDRESS entry = 0;
    status = myos_uefi_load_kernel(image_handle, system_table, kernel_buf, kernel_size, boot_info, &entry);
    if (EFI_ERROR(status)) {
        Print(L"MyOS UEFI: kernel validation/load failed: %r\\r\\n", status);
        return status;
    }

    UINTN memory_map_size = 0;
    EFI_MEMORY_DESCRIPTOR *memory_map = NULL;
    UINTN map_key = 0;
    UINTN desc_size = 0;
    UINT32 desc_version = 0;

    status = system_table->BootServices->GetMemoryMap(&memory_map_size, memory_map, &map_key, &desc_size, &desc_version);
    if (status != EFI_BUFFER_TOO_SMALL) {
        Print(L"MyOS UEFI: GetMemoryMap probe failed: %r\\r\\n", status);
        return status;
    }

    memory_map_size += desc_size * 16;
    status = system_table->BootServices->AllocatePool(EfiLoaderData, memory_map_size, (VOID **)&memory_map);
    if (EFI_ERROR(status)) {
        Print(L"MyOS UEFI: memory map alloc failed: %r\\r\\n", status);
        return status;
    }

    EFI_STATUS exit_status;
    for (;;) {
        UINTN current_size = memory_map_size;
        exit_status = system_table->BootServices->GetMemoryMap(&current_size, memory_map, &map_key, &desc_size, &desc_version);
        if (EFI_ERROR(exit_status)) {
            Print(L"MyOS UEFI: GetMemoryMap failed: %r\\r\\n", exit_status);
            return exit_status;
        }

        boot_info->memory_map = (UINT64)(UINTN)memory_map;
        boot_info->memory_map_entries = (UINT32)(current_size / desc_size);
        boot_info->memory_map_entry_size = (UINT32)desc_size;

        exit_status = system_table->BootServices->ExitBootServices(image_handle, map_key);
        if (exit_status == EFI_SUCCESS) {
            break;
        }
        if (exit_status != EFI_INVALID_PARAMETER) {
            Print(L"MyOS UEFI: ExitBootServices failed: %r\\r\\n", exit_status);
            return exit_status;
        }
    }

    void (*kernel_entry)(const struct myos_boot_info *) = (void (*)(const struct myos_boot_info *))(UINTN)entry;
    kernel_entry(boot_info);

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
