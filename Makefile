CC = gcc
AS = as
LD = ld

INCLUDES = -Idrivers/core -Idrivers/keyboard -Idrivers/vga
CFLAGS = -m32 -ffreestanding -fno-builtin -nostdlib \
         -fno-pie -fno-stack-protector -Wall -Wextra -c $(INCLUDES)
ASFLAGS = --32 --noexecstack
LDFLAGS = -T linker.ld -m elf_i386

C_SRCS = src/kernel/kernel.c \
         drivers/core/idt.c \
         drivers/core/isr.c \
         drivers/keyboard/keyboard.c \
         drivers/vga/vga.c
ASM_SRCS = src/arch/boot.s \
           src/arch/interrupts.s
OBJS = $(ASM_SRCS:.s=.o) $(C_SRCS:.c=.o)

.PHONY: all iso run clean

all: iso

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

run: build/os.iso
	qemu-system-x86_64 -cdrom build/os.iso

clean:
	rm -rf $(OBJS) build/*.o build/*.bin build/*.iso
