.syntax unified
.cpu cortex-a9
.arm

/* Reference: examples/user-mode-hello/kernel/user.S
 * r0 = user entry VA, r1 = user stack top. Never returns. */

.global minemu_enter_user
.type minemu_enter_user, %function
.extern minemu_fail_stop

minemu_enter_user:
    cpsid if
    mov r2, r0
    sub sp, sp, #8
    str r1, [sp]
    mov r3, #0
    str r3, [sp, #4]
    mov r3, sp
    ldmia r3, {sp, lr}^
    add sp, sp, #8
    mov r3, #0x10
    msr spsr_cxsf, r3
    mov lr, r2
    mov r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    movs pc, lr
