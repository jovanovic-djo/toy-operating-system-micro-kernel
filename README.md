# toy-operating-system-micro-kernel

A small 32-bit x86 hobby kernel, written in C and GNU assembly, booted via Multiboot.

## Features

- Multiboot boot (GRUB or QEMU `-kernel`) with its own 16 KiB stack
- IDT with a generic ISR/IRQ dispatcher (`register_interrupt_handler`)
- Panic screen for unhandled CPU exceptions with a register dump
- PS/2 keyboard driver (IRQ1, US layout, lowercase only)
- VGA text console with scrolling, backspace and a hardware cursor

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

## Project layout

```
src/arch/       boot entry (boot.s) and interrupt stubs (interrupts.s)
src/kernel/     kernel_main
drivers/core/   IDT, PIC, ISR/IRQ dispatch, port I/O
drivers/keyboard/
drivers/vga/
iso/boot/grub/  GRUB config used by `make iso`
linker.ld       kernel memory layout (loaded at 1 MiB)
```

## License

GPL-3.0, see [LICENSE](LICENSE).
