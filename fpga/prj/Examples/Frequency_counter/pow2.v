`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Anton Potocnik
// 
// Create Date: 19.11.2016 13:15:28
// Design Name: 
// Module Name: pow2
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


module pow2 # 
(
	parameter LOG2N_WIDTH = 5,
	parameter N_WIDTH = 32
)
(
    input unsigned [LOG2N_WIDTH-1:0]  log2N,
    output unsigned [N_WIDTH-1:0]	  N
);
	
    assign N = (1<<log2N);
	
endmodule
