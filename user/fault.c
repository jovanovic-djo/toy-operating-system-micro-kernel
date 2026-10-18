#include "ulib.h"

/* Shows that user programs cannot touch kernel memory. */
int main(void) {
    char* kernel = (char*)0x100000; /* kernel code, mapped but supervisor-only */

    puts("write() with a kernel pointer returns ");
    if (write(kernel, 16) == -1)
        puts("-1\n");
    else
        puts("success (bad!)\n");

    puts("writing to kernel memory directly...\n");
    kernel[0] = 0;

    puts("still running (bad!)\n");
    return 1;
}
