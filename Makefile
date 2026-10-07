CC = gcc
AS = as
LD = ld

INCLUDES = -Idrivers/core -Idrivers/keyboard -Idrivers/vga
CFLAGS = -m32 -ffreestanding -fno-builtin -nostdlib \
         -fno-pie -fno-stack-protector -Wall -Wextra -g -c $(INCLUDES)
ASFLAGS = --32 --noexecstack -g
LDFLAGS = -T linker.ld -m elf_i386

C_SRCS = src/kernel/kernel.c \
         drivers/core/idt.c \
         drivers/core/isr.c \
         drivers/keyboard/keyboard.c \
         drivers/vga/vga.c
ASM_SRCS = src/arch/boot.s \
           src/arch/interrupts.s
OBJS = $(ASM_SRCS:.s=.o) $(C_SRCS:.c=.o)

QEMU = qemu-system-i386

.PHONY: all iso run run-iso debug clean

all: build/kernel.bin

build:
	mkdir -p $@

build/kernel.bin: $(OBJS) | build
	$(LD) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

iso: build/os.iso

build/os.iso: build/kernel.bin iso/boot/grub/grub.cfg
	mkdir -p iso/boot/grub
	cp build/kernel.bin iso/boot/
	grub-mkrescue -o $@ iso

# QEMU loads Multiboot kernels directly, so GRUB is only needed for the ISO.
run: build/kernel.bin
	$(QEMU) -kernel $<

run-iso: build/os.iso
	$(QEMU) -cdrom $<

# Start paused with a GDB server on localhost:1234 (see .vscode/launch.json).
debug: build/kernel.bin
	$(QEMU) -kernel $< -s -S

clean:
	rm -rf $(OBJS) build/*.o build/*.bin build/*.iso
