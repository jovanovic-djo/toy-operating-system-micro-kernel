void kernel_main() {
    char* video = (char*) 0xb8000;
    video[0] = 'T';
    video[1] = 0x07;
    print("Toy Kernel");
}