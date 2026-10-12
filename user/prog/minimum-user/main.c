#include <stdint.h>

#include "minemu/user_abi.h"

/* Reference: tests/hw2/fixtures/hello/main.c */

void minemu_user_main(void) {
    static char msg[] = "hello world\n";
    (void)ioctl(MINEMU_FD_UART_OUT, MINEMU_IOCTL_UART_WRITE, (void *)msg, sizeof(msg) - 1U);
    for (;;) {
        __asm__ volatile("nop");
    }
}
