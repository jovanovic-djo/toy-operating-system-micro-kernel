#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

/* Value a Multiboot-compliant bootloader leaves in EAX. */
#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

/* multiboot_info.flags: which fields below are valid. */
#define MULTIBOOT_INFO_MEMORY  (1 << 0) /* mem_lower, mem_upper */
#define MULTIBOOT_INFO_CMDLINE (1 << 2)
#define MULTIBOOT_INFO_MMAP    (1 << 6) /* mmap_length, mmap_addr */

/* Boot information passed in EBX (Multiboot 0.6.96, section 3.3). */
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;   /* KiB below 1 MiB */
    uint32_t mem_upper;   /* KiB above 1 MiB, up to the first hole */
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} __attribute__((packed));

#define MULTIBOOT_MEMORY_AVAILABLE 1

/* One memory map entry. `size` does not count itself, so the next entry starts at size + 4. */
struct multiboot_mmap_entry {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed));

/* Remember the boot information; returns 0 if we were not booted by a Multiboot loader. */
int multiboot_init(uint32_t magic, const struct multiboot_info* info);

/* The boot information, or NULL if multiboot_init failed. */
const struct multiboot_info* multiboot_get_info(void);

/* Walk the memory map: pass NULL for the first entry. Returns NULL at the end. */
const struct multiboot_mmap_entry* multiboot_mmap_next(const struct multiboot_mmap_entry* entry);

void multiboot_print_memory_map(void);

#endif
