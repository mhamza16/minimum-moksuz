.syntax unified
.cpu cortex-a9
.arm

/* SVC vector target (vectors.S branches here). Reference: examples/user-mode-hello/kernel/svc.S
 * Same trap frame as irq_entry.S (minemu/trap.h): r0-r12 @0, return_lr @52, spsr @56,
 * exception_id @60, fault_pc @64, dfsr @68, dfar @72, user_sp @76, user_lr @80, reserved @84.
 * Frame size = 88 bytes. On entry we are in SVC mode: lr = LR_svc, spsr = SPSR_svc. */

.global minemu_svc_trampoline
.type minemu_svc_trampoline, %function
.extern minemu_svc_dispatch
.extern minemu_fail_stop

minemu_svc_trampoline:
    sub sp, sp, #88
    stmia sp, {r0-r12}
    str lr, [sp, #52]
    mrs r1, spsr
    str r1, [sp, #56]
    ldr r1, =-3
    str r1, [sp, #60]
    sub r1, lr, #4
    str r1, [sp, #64]
    mov r1, #0
    str r1, [sp, #68]
    str r1, [sp, #72]
    str r1, [sp, #84]
    add r1, sp, #76
    stmia r1, {sp, lr}^
    mov r0, sp
    bl minemu_svc_dispatch
    mov sp, r0
    ldr lr, [sp, #52]
    ldr r1, [sp, #56]
    msr spsr_cxsf, r1
    add r1, sp, #76
    ldmia r1, {sp, lr}^
    ldmia sp, {r0-r12}
    add sp, sp, #88
    movs pc, lr
