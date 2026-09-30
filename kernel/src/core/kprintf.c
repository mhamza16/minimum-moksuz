#include <stdarg.h>
#include <stdint.h>

#include "kprintf.h"
#include "uart.h"

static void print_hex(uint32_t value) {
    int shift;
    int leading = 1;

    for (shift = 28; shift >= 0; shift -= 4) {
        uint32_t digit = (value >> shift) & 0xFu;
        if (digit == 0 && leading && shift != 0) {
            continue;
        }
        leading = 0;
        uart_putc((char)(digit < 10 ? '0' + digit : 'a' + (digit - 10)));
    }
}

static void print_dec(uint32_t value) {
    static const uint32_t powers[10] = {
        1000000000u, 100000000u, 10000000u, 1000000u, 100000u,
        10000u, 1000u, 100u, 10u, 1u,
    };
    unsigned int i;
    int leading = 1;

    for (i = 0; i < 10; ++i) {
        uint32_t power = powers[i];
        uint32_t digit = 0;

        while (value >= power) {
            value -= power;
            ++digit;
        }

        if (digit == 0 && leading && power != 1u) {
            continue;
        }
        leading = 0;
        uart_putc((char)('0' + digit));
    }
}

static void print_signed(int32_t value) {
    if (value < 0) {
        uart_putc('-');
        print_dec(-(uint32_t)value);
    } else {
        print_dec((uint32_t)value);
    }
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (; *fmt != '\0'; ++fmt) {
        if (*fmt != '%') {
            uart_putc(*fmt);
            continue;
        }

        ++fmt;
        switch (*fmt) {
        case 'c':
            uart_putc((char)va_arg(args, int));
            break;
        case 's': {
            const char *s = va_arg(args, const char *);
            uart_puts(s != 0 ? s : "(null)");
            break;
        }
        case 'd':
            print_signed(va_arg(args, int));
            break;
        case 'u':
            print_dec(va_arg(args, unsigned int));
            break;
        case 'x':
            print_hex(va_arg(args, unsigned int));
            break;
        case '%':
            uart_putc('%');
            break;
        case '\0':
            va_end(args);
            return;
        default:
            uart_putc('%');
            uart_putc(*fmt);
            break;
        }
    }

    va_end(args);
}
