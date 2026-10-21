#include <stdint.h>

extern "C" {
#include "../include/boot_protocol.h"
#include "../include/serial.h"
}

static bool boot_info_valid(const myos_boot_info *info) {
    if (info == nullptr) {
        return false;
    }
    if (info->magic != MYOS_BOOT_MAGIC) {
        return false;
    }
    if (info->version != MYOS_BOOT_PROTOCOL_VERSION) {
        return false;
    }
    if (info->size < sizeof(myos_boot_info)) {
        return false;
    }
    return true;
}

extern "C" void kernel_main(const myos_boot_info *boot_info) {
    serial_init();

    if (!boot_info_valid(boot_info)) {
        serial_write("MyOS: invalid boot info\\r\\n");
    } else {
        serial_write("MyOS: kernel entry reached\\r\\n");
    }

    for (;;) {
        asm volatile("cli; hlt");
    }
}
