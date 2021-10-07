`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 26.06.2021 16:34:25
// Design Name: 
// Module Name: Circic_tb
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


module Circic_tb(

    );
    
reg clk = 0;
wire [10:0]hst;
wire [9:0]vst;
wire hsync;
wire vsync;
wire [2:0]rgb;
reg [2:0]rgb_2 = 3'b000;
wire [2:0]common_rgb;

always
    #10 clk = !clk;
        
VGA vga_0 (
    .clk50(clk),
    .hst(hst),
    .vst(vst),
    .hsync(hsync),
    .vsync(vsync)
);


reg [15 : 0] posx = 100;
reg [15 : 0] posy = 100;
reg [2 : 0] draw_color = 1;

CircPic circ_pic (
    .clk50(clk),
    .hst(hst),
    .vst(vst),
    .block_posx(posx),
    .block_posy(0),
    .draw_color(draw_color),
    .rgb_i(common_rgb),
    .rgb_o(rgb)
);

CircPic circ_pic2 (
    .clk50(clk),
    .hst(hst),
    .vst(vst),
    .block_posx(posx),
    .block_posy(posy + 500),
    .draw_color(draw_color),
    .rgb_i(rgb_2),
    .rgb_o(common_rgb)
);
endmodule
