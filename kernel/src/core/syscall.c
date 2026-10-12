#include "ioctl.h"
#include "minemu/syscall.h"
#include "minemu/trap.h"
#include "minemu/user_abi.h"

/* Reference: examples/user-mode-hello/kernel/main.c (minemu_svc_dispatch). */

#define CPSR_MODE_MASK UINT32_C(0x1f)
#define CPSR_MODE_USR UINT32_C(0x10)
#define CPSR_THUMB UINT32_C(0x20)
#define A32_SVC_ZERO UINT32_C(0xef000000)

struct minemu_trap_frame *minemu_svc_dispatch(struct minemu_trap_frame *frame) {
    if (frame->exception_id != MINEMU_EXCEPTION_SVC ||
        (frame->spsr & CPSR_MODE_MASK) != CPSR_MODE_USR || (frame->spsr & CPSR_THUMB) != 0 ||
        *(const uint32_t *)(uintptr_t)frame->fault_pc != A32_SVC_ZERO) {
        frame->r[0] = (uint32_t)MINEMU_IOCTL_ERROR;
        return frame;
    }

    switch (frame->r[7]) {
    case MINEMU_SYSCALL_IOCTL:
        frame->r[0] = (uint32_t)sys_ioctl((int32_t)frame->r[0], frame->r[1], frame->r[2], frame->r[3]);
        break;
    default:
        frame->r[0] = (uint32_t)MINEMU_IOCTL_ERROR;
        break;
    }
    return frame;
}
