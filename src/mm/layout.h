#ifndef LAYOUT_H
#define LAYOUT_H

/*
 * Virtual address space, shared by every page directory:
 *
 *   0          - 1 GiB       RAM, identity-mapped for the kernel (page 0 unmapped)
 *   1 GiB      - 0xE0000000  user programs, private to each process
 *   0xE0000000 - 0xF0000000  kernel heap
 *
 * RAM above KERNEL_MEMORY_END is not used.
 */
#define KERNEL_MEMORY_END 0x40000000
#define USER_START        0x40000000
#define USER_END          0xE0000000
#define HEAP_START        0xE0000000
#define HEAP_END          0xF0000000

#endif
