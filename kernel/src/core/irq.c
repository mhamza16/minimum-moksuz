#include "minemu/irq.h"
#include "minemu/platform.h"
#include "uart.h"

static void (*const irq_handlers[MINEMU_IRQ_BLOCK + 1])(void) = {
    [MINEMU_IRQ_UART0] = uart_rx_irq_handler,
};

struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame) {
    uint32_t source = (uint32_t)frame->exception_id;

    if (source > MINEMU_IRQ_BLOCK) {  
        return frame;
    }
    if (irq_handlers[source]) {
        irq_handlers[source]();
    }
    MINEMU_INTERRUPT->eoi = source;
    return frame;
}
