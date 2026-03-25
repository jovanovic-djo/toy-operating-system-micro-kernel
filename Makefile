CC = gcc
ASM = nasm
LD = ld

CFLAGS = -ffreestanding -m32 -c
LDFLAGS = -T linker.ld -m elf_i386

all: build/kernel.bin iso

build/kernel.bin: kernel.o boot.o
	$(LD) $(LDFLAGS) $^ -o $@

kernel.o: kernel.c
	$(CC) $(CFLAGS) $< -o $@

boot.o: boot.s
	$(ASM) -f elf32 $< -o $@

iso:
	mkdir -p iso/boot/grub
	cp build/kernel.bin iso/boot/
	grub-mkrescue -o build/os.iso iso

run:
	qemu-system-x86_64 -cdrom build/os.iso

clean:
	rm -rf build/*.o build/*.bin build/*.iso