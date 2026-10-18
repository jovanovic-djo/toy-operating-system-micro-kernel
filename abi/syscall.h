#ifndef ABI_SYSCALL_H
#define ABI_SYSCALL_H

/*
 * System call interface shared by the kernel and user programs.
 *
 * int $0x80 with the call number in EAX and arguments in EBX, ECX, EDX.
 * The result comes back in EAX; -1 means the call failed.
 */
#define SYSCALL_VECTOR 0x80

#define SYS_EXIT   0 /* exit(int code), does not return */
#define SYS_WRITE  1 /* write(const char* buf, uint32_t len) -> bytes written */
#define SYS_SLEEP  2 /* sleep(uint32_t ms) -> 0 */
#define SYS_GETPID 3 /* getpid() -> thread id */

#endif
