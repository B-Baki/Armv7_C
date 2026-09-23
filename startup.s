@ Freestanding entry point. GCC calls the aeabi helpers for the board
@ initializer in main; they are not the lab's VGA or keyboard code.
.syntax unified
.arm
.cpu cortex-a9

.global _start
.global memcpy
.global memset
.global memmove
.global __aeabi_memcpy
.global __aeabi_memcpy4
.global __aeabi_memcpy8
.global __aeabi_memset
.global __aeabi_memclr
.global __aeabi_memclr4
.global __aeabi_memclr8

_start:
    ldr sp, =__stack_top
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
bss_loop:
    cmp r0, r1
    bhs bss_done
    str r2, [r0], #4
    b bss_loop
bss_done:
    bl main
hang:
    b hang

/* void *memcpy(void *dst, const void *src, size_t n) */
memcpy:
__aeabi_memcpy:
__aeabi_memcpy4:
__aeabi_memcpy8:
    mov r3, r0
    cmp r2, #0
    beq memcpy_done
memcpy_loop:
    ldrb r12, [r1], #1
    strb r12, [r3], #1
    subs r2, r2, #1
    bne memcpy_loop
memcpy_done:
    bx lr

/* void *memset(void *dst, int c, size_t n) */
memset:
    mov r3, r0
    cmp r2, #0
    beq memset_done
memset_loop:
    strb r1, [r3], #1
    subs r2, r2, #1
    bne memset_loop
memset_done:
    bx lr

/* void __aeabi_memset(void *dst, size_t n, int c) */
__aeabi_memset:
    mov r3, r1
    mov r1, r2
    mov r2, r3
    b memset

/* void __aeabi_memclr(void *dst, size_t n) */
__aeabi_memclr:
__aeabi_memclr4:
__aeabi_memclr8:
    mov r2, r1
    mov r1, #0
    b memset

/* void *memmove(void *dst, const void *src, size_t n) */
memmove:
    cmp r0, r1
    bls memcpy
    add r3, r0, r2
    add r1, r1, r2
    cmp r2, #0
    beq memmove_done
memmove_loop:
    ldrb r12, [r1, #-1]!
    strb r12, [r3, #-1]!
    subs r2, r2, #1
    bne memmove_loop
memmove_done:
    bx lr
