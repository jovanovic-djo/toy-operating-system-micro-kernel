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

all: build/kernel.bin iso

build/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

src/arch/boot.o: src/arch/boot.s
	$(AS) $(ASFLAGS) $< -o $@

iso:
	mkdir -p iso/boot/grub
	cp build/kernel.bin iso/boot/
	grub-mkrescue -o build/os.iso iso

run:
	qemu-system-x86_64 -cdrom build/os.iso

clean:
	rm -rf $(OBJS) build/*.o build/*.bin build/*.iso
