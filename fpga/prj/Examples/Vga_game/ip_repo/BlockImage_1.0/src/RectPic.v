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


module RectPic #
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
    input wire [15 : 0] block_sizex,  // The size of the rectangle on the x-axis
    input wire [15 : 0] block_sizey,  // The size of the rectangle on the y-axis
    input wire [2 : 0] draw_color,    // The color with which the object will be drawn
    input wire [2 : 0] rgb_i,         // Sent to rgb_o when there is nothing to draw
    output reg [2 : 0] rgb_o          
    );
        
    always @(posedge clk)
    begin
        if ((hst < SCREEN_WIDTH) &&  (vst < SCREEN_HEIGHT)) // inside the screen
            if ((hst > block_posx) && (hst < (block_posx + block_sizex)))
                if ((vst > block_posy) && (vst < (block_posy + block_sizey)))
                    rgb_o <= draw_color;
                else
                    rgb_o <= rgb_i;
            else
                rgb_o <= rgb_i;
        else  
            rgb_o <= 3'b000;
    end
    
endmodule
