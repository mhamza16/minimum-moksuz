#ifndef UART_H
#define UART_H

/* Send one character to UART0, waiting until the transmitter can accept it. */
void uart_putc(char c);

/* Send a NUL-terminated string to UART0, one character at a time. */
void uart_puts(const char *s);

#endif
