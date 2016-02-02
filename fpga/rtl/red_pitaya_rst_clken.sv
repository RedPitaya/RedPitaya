`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 31.01.2016 23:52:05
// Design Name: 
// Module Name: red_pitaya_rst_clken
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


module red_pitaya_rst_clken(
   // clock & reset
   input clk,
   input global_rst_n,

   // sub-module enable
   input enable_i,

   // sub-module reset and clock enable signals
   output reg reset_n_o,
   output reg clk_en_o
);


reg           enable_last                 = 1'b0;
reg  [  1: 0] enable_ctr                  = 2'b0;


always @(posedge clk) begin
if (!global_rst_n) begin
   clk_en_o    <= 1'b0;
   reset_n_o   <= 1'b0;
   enable_last <= 1'b0;
   enable_ctr  <= 2'b0;
   end

else begin
   if (enable_i != enable_last) begin
      enable_ctr <= 2'b11;                                             // load timer on enable bit change
      if (enable_i)                                                    // just enabled
         clk_en_o  <= 1'b1;                                            // firing up
      else                                                             // just disabled
         reset_n_o <= 1'b0;                                            // resetting before sleep
      end
   else if (enable_ctr)                                                // counter runs
      enable_ctr <= enable_ctr - 1;

   if (enable_i == enable_last && !enable_ctr)                         // after the counter has stopped
      if (enable_i)                                                    // when enabling counter elapsed
         reset_n_o <= 1'b1;                                            // release reset
      else                                                             // when disabling counter elapsed
         clk_en_o  <= 1'b0;                                            // going to sleep
   end

   enable_last <= enable_i;
end


endmodule
