#ifndef UART_H
#define UART_H

/* Send one character to UART0, waiting until the transmitter can accept it. */
void uart_putc(char c);

/* Send a NUL-terminated string to UART0, one character at a time. */
void uart_puts(const char *s);

/* Enable UART0 RX interrupts (device control + interrupt controller). Call before cpsie i. */
void uart_rx_init(void);

/* UART0 RX IRQ handler: drains rx_data into the RX ring buffer. Called from minemu_irq_dispatch. */
void uart_rx_irq_handler(void);

/* Non-blocking: next received byte (0-255), or -1 if the RX buffer is empty. */
int uart_getc(void);

#endif
