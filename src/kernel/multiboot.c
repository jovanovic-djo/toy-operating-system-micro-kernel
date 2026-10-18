#include <stddef.h>
#include <stdint.h>
#include "multiboot.h"
#include "kprintf.h"

static const struct multiboot_info* boot_info;

int multiboot_init(uint32_t magic, const struct multiboot_info* info) {
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
        return 0;
    boot_info = info;
    return 1;
}

const struct multiboot_info* multiboot_get_info(void) {
    return boot_info;
}

const struct multiboot_mmap_entry* multiboot_mmap_next(const struct multiboot_mmap_entry* entry) {
    if (!boot_info || !(boot_info->flags & MULTIBOOT_INFO_MMAP))
        return NULL;

    uint32_t start = boot_info->mmap_addr;
    uint32_t end = start + boot_info->mmap_length;
    uint32_t next = entry ? (uint32_t)entry + entry->size + sizeof(entry->size) : start;

    if (next + sizeof(struct multiboot_mmap_entry) > end)
        return NULL;
    return (const struct multiboot_mmap_entry*)next;
}

static const char* region_type(uint32_t type) {
    switch (type) {
    case 1:  return "usable";
    case 3:  return "ACPI reclaimable";
    case 4:  return "ACPI NVS";
    case 5:  return "bad memory";
    default: return "reserved";
    }
}

void multiboot_print_memory_map(void) {
    const struct multiboot_mmap_entry* entry = multiboot_mmap_next(NULL);

    if (!entry) {
        kprintf("no memory map from the bootloader\n");
        return;
    }

    kprintf("  base       end      type\n");
    for (; entry; entry = multiboot_mmap_next(entry)) {
        if (entry->addr >> 32) {
            kprintf("  (above 4 GiB)         %s\n", region_type(entry->type));
            continue;
        }
        uint64_t end = entry->addr + entry->len;
        uint32_t last = end > 0xFFFFFFFFull ? 0xFFFFFFFF : (uint32_t)end - 1;
        kprintf("  %08x - %08x %s\n", (uint32_t)entry->addr, last, region_type(entry->type));
    }
}
