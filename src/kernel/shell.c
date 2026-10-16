#include <stdint.h>
#include "shell.h"
#include "keyboard.h"
#include "kprintf.h"
#include "string.h"
#include "timer.h"
#include "vga.h"
#include "io.h"
#include "multiboot.h"

#define PROMPT      "~: "
#define LINE_MAX    128

#define KBC_STATUS  0x64
#define KBC_COMMAND 0x64
#define KBC_RESET   0xFE /* pulse the CPU reset line */

struct command {
    const char* name;
    const char* help;
    void (*run)(const char* args);
};

static void cmd_help(const char* args);
static void cmd_clear(const char* args);
static void cmd_echo(const char* args);
static void cmd_uptime(const char* args);
static void cmd_mem(const char* args);
static void cmd_reboot(const char* args);

static const struct command commands[] = {
    { "help",   "list available commands", cmd_help },
    { "clear",  "clear the screen",        cmd_clear },
    { "echo",   "print the given text",    cmd_echo },
    { "uptime", "time since boot",         cmd_uptime },
    { "mem",    "show the memory map",     cmd_mem },
    { "reboot", "restart the machine",     cmd_reboot },
};

#define COMMAND_COUNT (sizeof(commands) / sizeof(commands[0]))

static void cmd_help(const char* args) {
    (void)args;
    for (unsigned i = 0; i < COMMAND_COUNT; i++)
        kprintf("  %s - %s\n", commands[i].name, commands[i].help);
}

static void cmd_clear(const char* args) {
    (void)args;
    vga_clear();
}

static void cmd_echo(const char* args) {
    kprintf("%s\n", args);
}

static void cmd_uptime(const char* args) {
    (void)args;
    uint32_t ticks = timer_ticks();
    kprintf("%u.%02u s\n", ticks / TIMER_HZ, ticks % TIMER_HZ * 100 / TIMER_HZ);
}

static void cmd_mem(const char* args) {
    (void)args;
    multiboot_print_memory_map();
}

static void cmd_reboot(const char* args) {
    (void)args;
    /* Wait for the controller's input buffer to empty, then request a reset. */
    while (inb(KBC_STATUS) & 0x02)
        ;
    outb(KBC_COMMAND, KBC_RESET);

    for (;;)
        __asm__ volatile ("cli; hlt");
}

static void read_line(char* line) {
    unsigned len = 0;

    for (;;) {
        char c = keyboard_getchar();

        if (c == '\n') {
            vga_putc('\n');
            line[len] = '\0';
            return;
        }
        if (c == '\b') {
            if (len > 0) { /* never erase the prompt */
                len--;
                vga_putc('\b');
            }
            continue;
        }
        if (c >= ' ' && len < LINE_MAX - 1) {
            line[len++] = c;
            vga_putc(c);
        }
    }
}

static void execute(char* line) {
    while (*line == ' ')
        line++;
    if (!*line)
        return;

    /* Split "name args" at the first space. */
    char* args = line;
    while (*args && *args != ' ')
        args++;
    if (*args) {
        *args++ = '\0';
        while (*args == ' ')
            args++;
    }

    for (unsigned i = 0; i < COMMAND_COUNT; i++) {
        if (strcmp(line, commands[i].name) == 0) {
            commands[i].run(args);
            return;
        }
    }

    kprintf("unknown command: %s (type 'help')\n", line);
}

void shell_run(void) {
    char line[LINE_MAX];

    kprintf("Type 'help' for a list of commands.\n");
    for (;;) {
        vga_print(PROMPT);
        read_line(line);
        execute(line);
    }
}
