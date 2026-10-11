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

#endif
