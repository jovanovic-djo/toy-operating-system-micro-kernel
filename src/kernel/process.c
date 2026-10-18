#include <stddef.h>
#include <stdint.h>
#include "process.h"
#include "paging.h"
#include "pmm.h"
#include "layout.h"
#include "irq.h"
#include "string.h"

#define USER_STACK_TOP  USER_END
#define USER_STACK_SIZE (16 * 1024)

void enter_user_mode(uint32_t eip, uint32_t esp) __attribute__((noreturn)); /* usermode.s */

/* Map fresh zeroed pages at [virt, virt + size) and copy `len` bytes of data to the start. */
static int map_user_range(uint32_t* dir, uint32_t virt, uint32_t size, const uint8_t* data, uint32_t len) {
    for (uint32_t off = 0; off < size; off += PAGE_SIZE) {
        uint32_t frame = pmm_alloc_page();
        if (!frame)
            return 0;

        /* Frames are below 1 GiB and identity-mapped, so they can be filled directly. */
        memset((void*)frame, 0, PAGE_SIZE);
        if (off < len)
            memcpy((void*)frame, data + off, len - off < PAGE_SIZE ? len - off : PAGE_SIZE);

        if (!paging_map_in(dir, virt + off, frame, PAGE_USER | PAGE_WRITE)) {
            pmm_free_page(frame);
            return 0;
        }
    }
    return 1;
}

static void user_entry(void* arg) {
    (void)arg;
    enter_user_mode(USER_START, USER_STACK_TOP);
}

struct thread* process_spawn(const struct program* program) {
    uint32_t len = program->end - program->start;

    uint32_t* dir = paging_create_directory();
    if (!dir)
        return NULL;

    if (!map_user_range(dir, USER_START, len, program->start, len) ||
        !map_user_range(dir, USER_STACK_TOP - USER_STACK_SIZE, USER_STACK_SIZE, NULL, 0)) {
        paging_destroy_directory(dir); /* also frees the pages mapped so far */
        return NULL;
    }

    /* Attach the address space before the scheduler can run the new thread. */
    uint32_t flags = irq_save();
    struct thread* t = thread_create(program->name, user_entry, NULL);
    if (t)
        t->page_dir = dir;
    irq_restore(flags);

    if (!t)
        paging_destroy_directory(dir);
    return t;
}
