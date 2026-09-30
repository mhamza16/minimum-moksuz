#include "minemu/platform.h"
#include "uart.h"

void uart_putc(char c) {
    while ((MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY) == 0) {
    }
    MINEMU_UART0->tx_data = (uint8_t)c;
}

void uart_puts(const char *s) {
    while (*s != '\0') {
        uart_putc(*s);
        ++s;
    }
}
