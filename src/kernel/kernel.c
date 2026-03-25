void kernel_main() {
    char* video = (char*) 0xb8000;
    video[0] = 'H';
    video[1] = 0x07;
}