.syntax unified
.cpu cortex-a9
.arm

/* IRQ vector target (vectors.S branches here). Reference: examples/irq-context-switch/irq.S
 * Trap frame offsets (minemu/trap.h): r0-r12 @0, return_lr @52, spsr @56,
 * exception_id @60, fault_pc @64, dfsr @68, dfar @72, user_sp @76, user_lr @80, reserved @84.
 * Frame size = 88 bytes. */

.global minemu_irq_trampoline
.type minemu_irq_trampoline, %function
.extern minemu_irq_dispatch

minemu_irq_trampoline:
    /* 1. allocate one trap frame on the IRQ stack */
    sub sp, sp, #88
    /* 2. save r0-r12 first (everything below scratches r1), then lr and spsr */
    stmia sp, {r0-r12}
    str lr, [sp, #52]
    mrs r1, spsr
    str r1, [sp, #56]
    /* 3. claim the active source id (MINEMU_INTERRUPT_BASE + offsetof(claim)) */
    ldr r1, =0x10000000
    ldr r1, [r1, #8]
    str r1, [sp, #60]
    /* 4. interrupted instruction = IRQ lr - 4 */
    sub r1, lr, #4
    str r1, [sp, #64]
    /* 5. no fault registers for an IRQ; save the interrupted mode's banked sp/lr */
    mov r1, #0
    str r1, [sp, #68]
    str r1, [sp, #72]
    str r1, [sp, #84]
    add r1, sp, #76
    stmia r1, {sp, lr}^
    /* 6. frame = minemu_irq_dispatch(frame) */
    mov r0, sp
    bl minemu_irq_dispatch
    /* 7. restore from the returned frame: lr, spsr, banked sp/lr, then r0-r12 last */
    mov sp, r0
    ldr lr, [sp, #52]
    ldr r1, [sp, #56]
    msr spsr_cxsf, r1
    add r1, sp, #76
    ldmia r1, {sp, lr}^
    ldmia sp, {r0-r12}
    add sp, sp, #88
    /* 8. return to the interrupted instruction; also copies SPSR back into CPSR */
    subs pc, lr, #4
