`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 07.01.2017 17:22:31
// Design Name: 
// Module Name: freq_count_sim
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

module freq_count_sim();

reg clk = 0;
reg [31:0] Ncycles = 2;
reg [31:0] data_in = 0;  
wire [31:0] out;
wire [31:0] data_out;
wire valid_out;

frequency_counter fc (.S_AXIS_IN_tdata(data_in),
                      .S_AXIS_IN_tvalid(1'b1),
                      .clk(clk),
                      .rst(1'b1),
                      .Ncycles(Ncycles),
                      .M_AXIS_OUT_tdata(data_out),
                      .M_AXIS_OUT_tvalid(valid_out),
                      .counter_output(out)
                     );

initial begin
    clk = 0;
    forever #1 clk = ~clk;
end



//declare the sine ROM - 30 registers each 8 bit wide.  
reg signed [31:0] sine [0:29];
//Internal signals  
integer i; 

//Initialize the sine rom with samples. 
initial begin
    i = 0;
    sine[0] = 0;
    sine[1] = 16;
    sine[2] = 31;
    sine[3] = 45;
    sine[4] = 58;
    sine[5] = 67;
    sine[6] = 74;
    sine[7] = 77;
    sine[8] = 77;
    sine[9] = 74;
    sine[10] = 67;
    sine[11] = 58;
    sine[12] = 45;
    sine[13] = 31;
    sine[14] = 16;
    sine[15] = 0;
    sine[16] = -16;
    sine[17] = -31;
    sine[18] = -45;
    sine[19] = -58;
    sine[20] = -67;
    sine[21] = -74;
    sine[22] = -77;
    sine[23] = -77;
    sine[24] = -74;
    sine[25] = -67;
    sine[26] = -58;
    sine[27] = -45;
    sine[28] = -31;
    sine[29] = -16;
end

//At every positive edge of the clock, output a sine wave sample.
always@ (posedge(clk))
begin
    data_in = 10*sine[i];
    i = i + 1;
    if(i == 30)
        i = 0;
end



endmodule
