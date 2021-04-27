`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Anton Potocnik
// 
// Create Date: 09.10.2016 00:45:55
// Design Name: 
// Module Name: knight_rider_tb
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


module knight_rider_tb();

reg clk = 0;
wire [7:0] ledout;

knight_rider kr (.clk(clk),
                .led_out(ledout)
                );

initial begin
    forever #1 clk = ~clk;
end

endmodule
