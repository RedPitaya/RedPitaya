`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 15.05.2021 13:29:51
// Design Name: 
// Module Name: vga_tb
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


module vga_tb(

    );
    
reg clk = 0;
wire [10:0]hst;
wire [9:0]vst;
wire hsync;
wire vsync;
wire [2:0]rgb;

always
    #10 clk = !clk;
    
VGA vga_0 (
    .clk50(clk),
    .hst(hst),
    .vst(vst),
    .hsync(hsync),
    .vsync(vsync)
);

picture pic_0 (
    .clk50(clk),
    .hst(hst),
    .vst(vst),
    .rgb(rgb)
);

endmodule
