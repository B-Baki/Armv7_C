__asm__ (
".equ pb, 0xC8000000\n\t"
".equ cb, 0xC9000000\n\t"
".equ ps2, 0xff200100\n\t"
"\n\t"
"VGA_draw_point_ASM:\n\t"
"        push {lr}\n\t"
"        ldr a4, =pb\n\t"
"        add a4, a4, a1, lsl #1\n\t"
"        add a4, a4, a2, lsl #10\n\t"
"        strh a3, [a4]\n\t"
"        pop {lr}\n\t"
"        bx lr\n\t"
"\n\t"
"\n\t"
"VGA_clear_pixelbuff_ASM:\n\t"
"        push {v1-v3, lr}\n\t"
"        mov v1, #0\n\t"
"        mov v2, #0\n\t"
"        mov v3, #0\n\t"
"p_clear_outer_loop:\n\t"
"        cmp v2, #240\n\t"
"        beq p_clear_end\n\t"
"p_clear_inner_loop:\n\t"
"        cmp v1, #320\n\t"
"        beq p_clear_inner_loop_end\n\t"
"        mov a1, v1\n\t"
"        mov a2, v2\n\t"
"        mov a3, v3\n\t"
"        bl VGA_draw_point_ASM\n\t"
"        add v1, v1, #1\n\t"
"        b p_clear_inner_loop\n\t"
"p_clear_inner_loop_end:\n\t"
"        add v2, v2, #1\n\t"
"        mov v1, #0\n\t"
"        b p_clear_outer_loop\n\t"
"p_clear_end:\n\t"
"        pop {v1-v3, lr}\n\t"
"        bx lr\n\t"
"\n\t"
"\n\t"
"VGA_write_char_ASM:\n\t"
"        push {lr}\n\t"
"        ldr a4, =cb\n\t"
"        add a4, a4, a1\n\t"
"        add a4, a4, a2, lsl #7\n\t"
"        strb a3, [a4]\n\t"
"        pop {lr}\n\t"
"        bx lr\n\t"
"\n\t"
"\n\t"
"VGA_clear_charbuff_ASM:\n\t"
"        push {v1-v3, lr}\n\t"
"        mov v1, #0\n\t"
"        mov v2, #0\n\t"
"        mov v3, #0\n\t"
"c_clear_outer_loop:\n\t"
"        cmp v2, #60\n\t"
"        beq c_clear_end\n\t"
"c_clear_inner_loop:\n\t"
"        cmp v1, #80\n\t"
"        beq c_clear_inner_loop_end\n\t"
"        mov a1, v1\n\t"
"        mov a2, v2\n\t"
"        mov a3, v3\n\t"
"        bl VGA_write_char_ASM\n\t"
"        add v1, v1, #1\n\t"
"        b c_clear_inner_loop\n\t"
"c_clear_inner_loop_end:\n\t"
"        add v2, v2, #1\n\t"
"        mov v1, #0\n\t"
"        b c_clear_outer_loop\n\t"
"c_clear_end:\n\t"
"        pop {v1-v3, lr}\n\t"
"        bx lr\n\t"
"\n\t"
"\n\t"
"read_PS2_data_ASM:\n\t"
"        push {v1, v2, lr}\n\t"
"        mov v2, a1\n\t"
"        ldr v1, =ps2\n\t"
"        ldr v1, [v1]\n\t"
"        mov a1, v1, lsr #15\n\t"
"        and a1, a1, #1\n\t"
"        tst a1, #1\n\t"
"        beq return_0\n\t"
"        strb v1, [v2]\n\t"
"        mov a1, #1\n\t"
"        pop {v1, v2, lr}\n\t"
"        bx lr\n\t"
"\n\t"
"\n\t"
"return_0:\n\t"
"        mov a1, #0\n\t"
"        pop {v1, v2, lr}\n\t"
"        bx lr\n\t"
"\n\t"
"\n\t"
"VGA_draw_line:\n\t"
"        push {v1-v5, lr}\n\t"
"        mov v1, a1\n\t"
"        mov v2, a2\n\t"
"        mov v3, a3\n\t"
"        mov v4, a4\n\t"
"\n\t"
"        ldr v5, [sp, #24]\n\t"
"\n\t"
"        cmp v1, v3\n\t"
"        beq vertical_loop\n\t"
"\n\t"
"horizontal_loop:\n\t"
"        cmp v1, v3\n\t"
"        beq end_draw_line\n\t"
"        mov a1, v1\n\t"
"        mov a2, v2\n\t"
"        mov a3, v5\n\t"
"        bl VGA_draw_point_ASM\n\t"
"        add v1, v1, #1\n\t"
"        b horizontal_loop\n\t"
"\n\t"
"vertical_loop:\n\t"
"        cmp v2, v4\n\t"
"        beq end_draw_line\n\t"
"        mov a1, v1\n\t"
"        mov a2, v2\n\t"
"        mov a3, v5\n\t"
"        bl VGA_draw_point_ASM\n\t"
"        add v2, v2, #1\n\t"
"        b vertical_loop\n\t"
"\n\t"
"end_draw_line:\n\t"
"        pop {v1-v5, lr}\n\t"
"        bx lr\n\t"
);


extern void VGA_draw_line(int x1, int y1, int x2, int y2, short color);

extern int read_PS2_data_ASM(char *data);

int cell_size = 20;

void GoL_draw_grid(){
	int cols = 16;
	int rows = 12;
	
	for(int col = 0; col < 320; col++){
		VGA_draw_line(col, 0, col, 240, 0xCE59);
	}
	
	for(int col = 0; col <= cols; col++){
		int x = col * cell_size;
		VGA_draw_line(x, 0, x, rows * cell_size, 0);
	}
		
	for(int row = 0; row <= rows; row++){
		int y = row * cell_size;
		VGA_draw_line(0, y, cols * cell_size, y, 0);
	}
}

void GoL_fill_gridxy(int x, int y, short c){
	//int cols = 16;
	//int rows = 12;
	
	for (int row = 1; row < cell_size; row++){
		VGA_draw_line(x * cell_size + 1, y * cell_size + row, x * cell_size + cell_size,  y * cell_size + row, c);
	}
}

void draw_cursor(int x, int y, int prev_x, int prev_y){
	//int cols = 16;
	//int rows = 12;
	for (int row = 1; row < cell_size; row++){
		VGA_draw_line(prev_x * cell_size + 1, prev_y * cell_size + row, prev_x * cell_size + cell_size,  prev_y * cell_size + row, 0xCE59);
	}
	VGA_draw_line(x * cell_size + 10, y * cell_size, x * cell_size + 10,  y * cell_size + cell_size, 0xF800);
	VGA_draw_line(x * cell_size, y * cell_size + 9, x * cell_size + cell_size,  y * cell_size + 10, 0xF800);
}

void GoL_draw_board(int board[12][16]){
	for (int y = 0; y < 12; y++){
        for (int x = 0; x < 16; x++){
            if (board[y][x] == 1){
				GoL_fill_gridxy(x, y, 0);
			}else{
				GoL_fill_gridxy(x, y, 0xCE59);
			}
        }
    }
}

void update_state(int board[12][16])
{
    int new_board[12][16];
    
    for(int y = 0; y < 12; y++) {
        for(int x = 0; x < 16; x++) {
            new_board[y][x] = 0;
        }
    }

    for(int y = 0; y < 12; y++){
        for(int x = 0; x < 16; x++){ 
            int neighbors = 0;
			
            if(y > 0 && x > 0 && board[y-1][x-1] == 1) neighbors++;
            if(y > 0 && board[y-1][x] == 1) neighbors++;
            if(y > 0 && x < 15 && board[y-1][x+1] == 1) neighbors++;
            if(x > 0 && board[y][x-1] == 1) neighbors++;
            if(x < 15 && board[y][x+1] == 1) neighbors++;
            if(y < 11 && x > 0 && board[y+1][x-1] == 1) neighbors++;
            if(y < 11 && board[y+1][x] == 1) neighbors++;
            if(y < 11 && x < 15 && board[y+1][x+1] == 1) neighbors++;

            if(board[y][x] == 1){
                if(neighbors == 2 || neighbors == 3){
                    new_board[y][x] = 1;
                }else{
                    new_board[y][x] = 0;
                }
            }else{
                if(neighbors == 3) {
                    new_board[y][x] = 1;
                }else{
                    new_board[y][x] = 0;
                }
            }
        }
    }

    for(int y = 0; y < 12; y++){
        for(int x = 0; x < 16; x++){
            board[y][x] = new_board[y][x];
        }
    }
}

int main(){
	
	int GoLBoard[12][16] = {
//x  0,1,2,3,4,5,6,7,8,9,a,b,c,d,e,f      y
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 0
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 1
    {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0}, // 2
    {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0}, // 3
    {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0}, // 4
    {0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0}, // 5
    {0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0}, // 6
    {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0}, // 7
    {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0}, // 8
    {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0}, // 9
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // a
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}  // b (12th row)
};

	GoL_draw_grid();
	GoL_draw_board(GoLBoard);
	
	
	int cursor_x = 0;
	int cursor_y = 0;
	int prev_x = cursor_x;
	int prev_y = cursor_y;
	
	short cursor_needs_update = 1;
	
	unsigned char scancode;
	int break_flag = 0;
	
	while(1){
		if(read_PS2_data_ASM((char*) &scancode)){
			
			if(break_flag){
				break_flag = 0;
				continue;
			}
			
			switch(scancode){
				case 0xF0:
					break_flag = 1;
					continue;
				case 0x1C: //A
					if(cursor_x > 0){
						prev_x = cursor_x;
						cursor_x--;
						cursor_needs_update = 1;
					}
					break;
				case 0x23: //D
					if(cursor_x < 15){
						prev_x = cursor_x;
						cursor_x++;
						cursor_needs_update = 1;
					}
					break;
				case 0x1B: //S
					if(cursor_y < 11){
						prev_y = cursor_y;
						cursor_y++;
						cursor_needs_update = 1;
					}
					break;
				case 0x1D: //W
					if(cursor_y > 0){
						prev_y = cursor_y;
						cursor_y--;
						cursor_needs_update = 1;
					}
					break;
				case 0x29: //SPACE
					if(GoLBoard[cursor_y][cursor_x]){
						GoLBoard[cursor_y][cursor_x] = 0;
						GoL_fill_gridxy(cursor_x, cursor_y, 0xCE59);
						cursor_needs_update = 1;
					}else{
						GoLBoard[cursor_y][cursor_x] = 1;
						GoL_fill_gridxy(cursor_x, cursor_y, 0);
						cursor_needs_update = 1;
					}
					break;
				case 0x31: //N
					update_state(GoLBoard);
					GoL_draw_board(GoLBoard);
					cursor_needs_update = 1;
					break;
			}
		}
		
		if(cursor_needs_update){
			if(prev_x != cursor_x || prev_y != cursor_y || cursor_needs_update == 1){
				for (int row = 1; row < cell_size; row++) {
					if(GoLBoard[prev_y][prev_x] == 0){
                    	VGA_draw_line(prev_x * cell_size + 1, prev_y * cell_size + row, prev_x * cell_size + cell_size, prev_y * cell_size + row, 0xCE59);
					}else{
						VGA_draw_line(prev_x * cell_size + 1, prev_y * cell_size + row, prev_x * cell_size + cell_size, prev_y * cell_size + row, 0);
					}
				}
			}
			
			VGA_draw_line(cursor_x * cell_size + 10, cursor_y * cell_size + 1, cursor_x * cell_size + 10, cursor_y * cell_size + cell_size, 0xF800);
            VGA_draw_line(cursor_x * cell_size + 1, cursor_y * cell_size + 9, cursor_x * cell_size + cell_size, cursor_y * cell_size + 9, 0xF800);
            prev_x = cursor_x;
            prev_y = cursor_y;
            cursor_needs_update = 0;
        }
	}
}