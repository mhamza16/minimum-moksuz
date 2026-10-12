#include <stdbool.h>

#include "ioctl.h"
#include "minemu/platform.h"
#include "minemu/user_abi.h"
#include "uart.h"
#include "user.h"

/* Every user pointer goes through user_range_ok() BEFORE any side effect.
 * Errors return MINEMU_IOCTL_ERROR. Ops/fds come from minemu/user_abi.h, not the PDF table. */

static int32_t uart_write(volatile struct minemu_uart_regs *uart, uint32_t src, uint32_t n) {
    if (n > MINEMU_IOCTL_MAX_TRANSFER || !user_range_ok(src, n, false)) {
        return MINEMU_IOCTL_ERROR;
    }
    const uint8_t *bytes = (const uint8_t *)(uintptr_t)src;
    for (uint32_t i = 0; i < n; ++i) {
        while ((uart->status & MINEMU_UART_STATUS_TX_READY) == 0) {
        }
        uart->tx_data = bytes[i];
    }
    return (int32_t)n;
}

static int32_t uart_read(uint32_t dst, uint32_t n) {
    if (n > MINEMU_IOCTL_MAX_TRANSFER || !user_range_ok(dst, n, true)) {
        return MINEMU_IOCTL_ERROR;
    }
    uint8_t *bytes = (uint8_t *)(uintptr_t)dst;
    uint32_t count = 0;
    int c;
    while (count < n && (c = uart_getc()) >= 0) {
        bytes[count++] = (uint8_t)c;
    }
    return (int32_t)count;
}

static int32_t uart_status(volatile struct minemu_uart_regs *uart) {
    uint32_t status = uart->status;
    int32_t result = 0;
    if ((status & MINEMU_UART_STATUS_RX_READY) != 0) {
        result |= MINEMU_IOCTL_UART_RX_READY;
    }
    if ((status & MINEMU_UART_STATUS_TX_READY) != 0) {
        result |= MINEMU_IOCTL_UART_TX_READY;
    }
    return result;
}

int32_t sys_ioctl(int32_t fd, uint32_t op, uint32_t arg0, uint32_t arg1) {
    volatile struct minemu_uart_regs *uart = fd == MINEMU_FD_UART_DIAG ? MINEMU_UART1 : MINEMU_UART0;
    bool uart_fd = fd == MINEMU_FD_UART_IN || fd == MINEMU_FD_UART_OUT || fd == MINEMU_FD_UART_DIAG;
    switch (op) {
    case MINEMU_IOCTL_UART_READ:
        return fd == MINEMU_FD_UART_IN ? uart_read(arg0, arg1) : MINEMU_IOCTL_ERROR;
    case MINEMU_IOCTL_UART_WRITE:
        return fd == MINEMU_FD_UART_OUT || fd == MINEMU_FD_UART_DIAG ? uart_write(uart, arg0, arg1)
                                                                     : MINEMU_IOCTL_ERROR;
    case MINEMU_IOCTL_UART_STATUS:
        return uart_fd ? uart_status(uart) : MINEMU_IOCTL_ERROR;
    case MINEMU_IOCTL_RNG_STATE:
    case MINEMU_IOCTL_RNG_NEXT:
        if (fd != MINEMU_FD_RNG || (arg0 & 3U) != 0 || !user_range_ok(arg0, sizeof(uint32_t), true)) {
            return MINEMU_IOCTL_ERROR;
        }
        *(uint32_t *)(uintptr_t)arg0 = op == MINEMU_IOCTL_RNG_NEXT ? MINEMU_RNG->data : MINEMU_RNG->state;
        return 0;
    case MINEMU_IOCTL_RNG_SEED:
        if (fd != MINEMU_FD_RNG) {
            return MINEMU_IOCTL_ERROR;
        }
        MINEMU_RNG->seed = arg0;
        return 0;
    case MINEMU_IOCTL_TRACE_EVENT:
        if (fd != MINEMU_FD_TRACE) {
            return MINEMU_IOCTL_ERROR;
        }
        MINEMU_TRACE->event = arg0;
        return 0;
    default:
        return MINEMU_IOCTL_ERROR;
    }
}
