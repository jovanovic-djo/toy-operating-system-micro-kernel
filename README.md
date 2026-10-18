# toy-operating-system-micro-kernel

A small 32-bit x86 hobby kernel, written in C and GNU assembly, booted via Multiboot.

## Features

- Multiboot boot (GRUB or QEMU `-kernel`) with its own 16 KiB stack, GDT and TSS
- Memory management: Multiboot memory map, bitmap page allocator, paging with an
  identity-mapped kernel (page 0 unmapped to catch NULL pointers) and a `kmalloc`/`kfree` heap
- User mode: programs run in ring 3 in their own address space, call the kernel through
  `int 0x80` (`exit`, `write`, `sleep`, `getpid`) and are killed, not the kernel, when they fault
- IDT with a generic ISR/IRQ dispatcher (`register_interrupt_handler`) that ignores spurious IRQs
- Panic screen for unhandled CPU exceptions with a register dump (and CR2 for page faults)
- Kernel threads with a preemptive round-robin scheduler (50 ms time slices on the timer),
  an idle thread, `thread_sleep` and automatic cleanup of exited threads
- PS/2 keyboard driver (IRQ1, US layout, Shift and Caps Lock, buffered input)
- PIT timer at 100 Hz with `timer_sleep()`
- VGA text console with scrolling, backspace, a hardware cursor and `kprintf`
- Interactive shell with `help`, `clear`, `echo`, `uptime`, `mem`, `ps`, `spawn`, `run` and `reboot`

## Requirements

A Linux environment (WSL works) with:

| Purpose | Ubuntu/Debian packages |
|---|---|
| Build | `gcc` `binutils` `make` |
| Run | `qemu-system-x86` |
| Debug | `gdb` |
| Bootable ISO (optional) | `grub-pc-bin` `grub-common` `xorriso` |

```sh
sudo apt install gcc binutils make qemu-system-x86 gdb grub-pc-bin grub-common xorriso
```

## Build and run

| Command | What it does |
|---|---|
| `make` | Build `build/kernel.bin` |
| `make run` | Boot the kernel directly in QEMU |
| `make iso` | Build a GRUB bootable `build/os.iso` |
| `make run-iso` | Boot the ISO in QEMU |
| `make debug` | Start QEMU paused with a GDB server on `localhost:1234` |
| `make clean` | Remove build output |

Use `make run QEMU=...` to pick a different emulator binary or add QEMU options.

## Debugging

**VS Code:** open the folder in a WSL/Linux window with the C/C++ extension installed, set breakpoints, and start **Debug Kernel (QEMU + GDB)**. It runs `make debug` and attaches GDB automatically.

**Command line:** run `make debug` in one terminal, then in another:

```sh
gdb build/kernel.bin -ex "target remote localhost:1234" -ex "break kernel_main" -ex "continue"
```

## Memory layout

| Virtual addresses | Use |
|---|---|
| `0` - `1 GiB` | RAM, identity-mapped for the kernel (RAM above 1 GiB is not used) |
| `1 GiB` - `0xE0000000` | user program image and stack, private to each process |
| `0xE0000000` - `0xF0000000` | kernel heap |

## User programs

Programs live in `user/` and are embedded in the kernel image at build time. In the
shell, `run` lists them and `run hello` starts one:

- `hello` prints a few lines with `sleep` in between and exits
- `fault` shows that user code cannot read or write kernel memory

To add a program, create `user/<name>.c` with a `main` function (see `user/ulib.h` for
the available calls), then add its name to `USER_PROGRAMS` in the Makefile, to
`user/programs.s` and to the table in `src/kernel/programs.c`.

## Project layout

```
src/arch/       boot entry, interrupt stubs, context switch, entry into user mode
src/kernel/     kernel_main, Multiboot info, threads and scheduler, processes, system calls, shell, kprintf
src/lib/        string and memory functions
src/mm/         page allocator, paging, kernel heap
drivers/core/   GDT and TSS, IDT, PIC, ISR/IRQ dispatch, port I/O
drivers/keyboard/
drivers/timer/  PIT
drivers/vga/
user/           user programs and their runtime (crt0, ulib, linker script)
abi/            system call numbers shared by the kernel and user programs
iso/boot/grub/  GRUB config used by `make iso`
linker.ld       kernel memory layout (loaded at 1 MiB)
```

## License

GPL-3.0, see [LICENSE](LICENSE).
