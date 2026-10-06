CC = gcc
AS = as
LD = ld

INCLUDES = -Idrivers/core -Idrivers/keyborad -Idrivers/vga
CFLAGS = -ffreestanding -m32 -c $(INCLUDES)
ASFLAGS = --32
LDFLAGS = -T linker.ld -m elf_i386

C_SRCS = src/kernel/kernel.c \
         drivers/core/idt.c \
         drivers/keyborad/keyboard.c \
         drivers/vga/vga.c
OBJS = src/arch/boot.o $(C_SRCS:.c=.o)

.PHONY: all iso run clean

all: iso

build/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

src/arch/boot.o: src/arch/boot.s
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
