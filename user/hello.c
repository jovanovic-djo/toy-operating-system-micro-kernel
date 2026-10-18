#include "ulib.h"

int main(void) {
    uint32_t pid = getpid();

    puts("Hello from user mode, pid ");
    put_uint(pid);
    puts("\n");

    for (uint32_t i = 1; i <= 3; i++) {
        sleep(500);
        puts("[hello ");
        put_uint(pid);
        puts("] ");
        put_uint(i);
        puts("/3\n");
    }
    return 0;
}
