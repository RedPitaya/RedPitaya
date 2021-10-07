`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 06.06.2021 12:14:15
// Design Name: 
// Module Name: pic_tb
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


module pic_tb(

    );
reg clk = 0;
wire [10:0]hst;
wire [9:0]vst;
wire hsync;
wire vsync;
wire [2:0]rgb;

always
    #10 clk = !clk;
    
endmodule
