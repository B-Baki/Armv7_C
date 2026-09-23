.equ pb, 0xC8000000
.equ cb, 0xC9000000
.equ ps2, 0xff200100

VGA_draw_point_ASM:
        push {lr}
        ldr a4, =pb
        add a4, a4, a1, lsl #1
        add a4, a4, a2, lsl #10
        strh a3, [a4]
        pop {lr}
        bx lr


VGA_clear_pixelbuff_ASM:
        push {v1-v3, lr}
        mov v1, #0
        mov v2, #0
        mov v3, #0
p_clear_outer_loop:
        cmp v2, #240
        beq p_clear_end
p_clear_inner_loop:
        cmp v1, #320
        beq p_clear_inner_loop_end
        mov a1, v1
        mov a2, v2
        mov a3, v3
        bl VGA_draw_point_ASM
        add v1, v1, #1
        b p_clear_inner_loop
p_clear_inner_loop_end:
        add v2, v2, #1
        mov v1, #0
        b p_clear_outer_loop
p_clear_end:
        pop {v1-v3, lr}
        bx lr


VGA_write_char_ASM:
        push {lr}
        ldr a4, =cb
        add a4, a4, a1
        add a4, a4, a2, lsl #7
        strb a3, [a4]
        pop {lr}
        bx lr


VGA_clear_charbuff_ASM:
        push {v1-v3, lr}
        mov v1, #0
        mov v2, #0
        mov v3, #0
c_clear_outer_loop:
        cmp v2, #60
        beq c_clear_end
c_clear_inner_loop:
        cmp v1, #80
        beq c_clear_inner_loop_end
        mov a1, v1
        mov a2, v2
        mov a3, v3
        bl VGA_write_char_ASM
        add v1, v1, #1
        b c_clear_inner_loop
c_clear_inner_loop_end:
        add v2, v2, #1
        mov v1, #0
        b c_clear_outer_loop
c_clear_end:
        pop {v1-v3, lr}
        bx lr


read_PS2_data_ASM:
        push {v1, v2, lr}
        mov v2, a1
        ldr v1, =ps2
        ldr v1, [v1]
        mov a1, v1, lsr #15
        and a1, a1, #1
        tst a1, #1
        beq return_0
        strb v1, [v2]
        mov a1, #1
        pop {v1, v2, lr}
        bx lr


return_0:
        mov a1, #0
        pop {v1, v2, lr}
        bx lr


VGA_draw_line:
        push {v1-v5, lr}
        mov v1, a1
        mov v2, a2
        mov v3, a3
        mov v4, a4

        ldr v5, [sp, #24]

        cmp v1, v3
        beq vertical_loop

horizontal_loop:
        cmp v1, v3
        beq end_draw_line
        mov a1, v1
        mov a2, v2
        mov a3, v5
        bl VGA_draw_point_ASM
        add v1, v1, #1
        b horizontal_loop

vertical_loop:
        cmp v2, v4
        beq end_draw_line
        mov a1, v1
        mov a2, v2
        mov a3, v5
        bl VGA_draw_point_ASM
        add v2, v2, #1
        b vertical_loop

end_draw_line:
        pop {v1-v5, lr}
        bx lr
