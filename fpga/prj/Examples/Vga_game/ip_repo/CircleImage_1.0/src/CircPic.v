`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 06.06.2021 13:15:51
// Design Name: 
// Module Name: RectPic
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module CircPic #
    (
        parameter integer SCREEN_HEIGHT	= 600,
        parameter integer SCREEN_WIDTH	= 800
    )
    (
    input wire clk,
    input wire [10 : 0] hst,
    input wire [9 : 0] vst,
    input wire [15 : 0] block_posx,  // The position of the rectangle on the x-axis
    input wire [15 : 0] block_posy,  // The position of the rectangle on the y-axis
    input wire [2 : 0] draw_color,
    input wire [2 : 0] rgb_i,
    output reg [2 : 0] rgb_o
    );
        
    reg [31 : 0] mem [31 : 0];
    
    initial begin
    	mem[0]  = 32'b00000000000011111111000000000000;
			mem[1]  = 32'b00000000011111111111111000000000;
			mem[2]  = 32'b00000001111111111111111110000000;
			mem[3]  = 32'b00000011111111111111111111000000;
			mem[4]  = 32'b00000111111111111111111111100000;
			mem[5]  = 32'b00001111111111111111111111110000;
			mem[6]  = 32'b00011111111111111111111111111000;
			mem[7]  = 32'b00111111111111111111111111111100;
			mem[8]  = 32'b00111111111111111111111111111100;
			mem[9]  = 32'b01111111111111111111111111111110;
			mem[10] = 32'b01111111111111111111111111111110;
			mem[11] = 32'b01111111111111111111111111111110;
			mem[12] = 32'b11111111111111111111111111111111;
			mem[13] = 32'b11111111111111111111111111111111;
			mem[14] = 32'b11111111111111111111111111111111;
			mem[15] = 32'b11111111111111111111111111111111;
			mem[16] = 32'b11111111111111111111111111111111;
			mem[17] = 32'b11111111111111111111111111111111;
			mem[18] = 32'b11111111111111111111111111111111;
			mem[19] = 32'b11111111111111111111111111111111;
			mem[20] = 32'b01111111111111111111111111111110;
			mem[21] = 32'b01111111111111111111111111111110;
			mem[22] = 32'b01111111111111111111111111111110;
			mem[23] = 32'b00111111111111111111111111111100;
			mem[24] = 32'b00111111111111111111111111111100;
			mem[25] = 32'b00011111111111111111111111111000;
			mem[26] = 32'b00001111111111111111111111110000;
			mem[27] = 32'b00000111111111111111111111100000;
			mem[28] = 32'b00000011111111111111111111000000;
			mem[29] = 32'b00000001111111111111111110000000;
			mem[30] = 32'b00000000011111111111111000000000;
			mem[31] = 32'b00000000000011111111000000000000;
    end
        
    always @(posedge clk)
    begin
        if ((hst < SCREEN_WIDTH) &&  (vst < SCREEN_HEIGHT)) // inside the screen
            if ((hst >= block_posx) && (hst < (block_posx + 32)))
                if ((vst >= block_posy) && (vst < (block_posy + 32)))
										if (mem[vst - block_posy][hst - block_posx])
												rgb_o <= draw_color;
										else
												rgb_o <= rgb_i;
                else
                    rgb_o <= rgb_i;
            else
                rgb_o <= rgb_i;
        else  
            rgb_o <= 3'b000;
    end
    
endmodule
