#include "minemu/irq.h"
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

#define UART_RX_SIZE 256u

static volatile uint8_t rx_buf[UART_RX_SIZE];
static volatile uint32_t rx_head, rx_tail;

void uart_rx_init(void) {
    MINEMU_UART0->control |= MINEMU_UART_CONTROL_RX_IRQ_ENABLE;
    MINEMU_INTERRUPT->enable |= UINT32_C(1) << MINEMU_IRQ_UART0;
    uart_rx_irq_handler();
}

void uart_rx_irq_handler(void) {
    while (MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) {
        uint8_t c = (uint8_t)MINEMU_UART0->rx_data;
        uint32_t next = (rx_head + 1) & (UART_RX_SIZE - 1);
        if (next != rx_tail) {
            rx_buf[rx_head] = c;
            rx_head = next;
        }
    }
}

/* Called only from the SVC path, where the CPU has already masked IRQs.
 * minemu only checks for a pending IRQ when an emulator batch stops; a user loop that
 * spins on ioctl stops at every SVC entry (I already set), so the UART IRQ can starve.
 * Drain the hardware FIFO here so a read never depends on the IRQ having been taken. */
int uart_getc(void) {
    int result = -1;
    uart_rx_irq_handler();
    if (rx_head != rx_tail) {
        result = rx_buf[rx_tail];
        rx_tail = (rx_tail + 1) & (UART_RX_SIZE - 1);
    }
    return result;
}
