#include "msh.h"
#include "uart.h"

/* Max typed bytes per line, not counting '\n' (decision B1). Buffer holds +1 for the NUL. */
#define MSH_LINE_MAX 20

/* Block until a byte arrives, then return it (0-255). */
static int msh_getc(void) {
    int c;
    while ((c = uart_getc()) < 0) {
    }
    return c;
}

/* Read one line into line[] (NUL-terminated). Returns its length, or -1 if it was overlong. */
static int msh_readline(char line[MSH_LINE_MAX + 1]) {
    int len = 0;
    int overflow = 0;

    for (;;) {
        int c = msh_getc();
        if (c == '\n') {
            if (overflow) {
                return -1;
            }
            line[len] = '\0';
            return len;
        } else if (c == 0x08 || c == 0x7f) {
            if (!overflow && len > 0) {
                len--;
            }
        } else if (!overflow) {
            if (len < MSH_LINE_MAX) {
                line[len++] = (char)c;
            } else {
                overflow = 1;
            }
        }
    }
}

/* Run one complete line. */
static void msh_execute(const char *line) {
    while (*line == ' ') {
        line++;
    }
    if (*line == '\0') {
        return;
    }
    if (line[0] == 'e' && line[1] == 'c' && line[2] == 'h' && line[3] == 'o' &&
        (line[4] == ' ' || line[4] == '\0')) {
        const char *p = line + 4;
        int first = 1;
        for (;;) {
            while (*p == ' ') {
                p++;
            }
            if (*p == '\0') {
                break;
            }
            if (!first) {
                uart_putc(' ');
            }
            first = 0;
            while (*p != ' ' && *p != '\0') {
                uart_putc(*p++);
            }
        }
        uart_putc('\n');
        return;
    }
    uart_puts("command not found: ");
    while (*line != ' ' && *line != '\0') {
        uart_putc(*line++);
    }
    uart_putc('\n');
}

void msh_run_line(void) {
    char line[MSH_LINE_MAX + 1];

    uart_puts("msh> ");
    if (msh_readline(line) >= 0) {
        msh_execute(line);
    }
}
