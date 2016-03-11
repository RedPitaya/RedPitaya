`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company:  private
// Engineer: Ulrich Habel, DF4IAH
//
// Create Date: 13.11.2015 01:42:31
// Design Name: RedPitaya-RadioBox, AXI port checker to LED display interface
// Module Name: axi_pc2leds
// Project Name: RedPitaya-RadioBox
// Target Devices: XC7Z010
// Tool Versions: Vivado 2015.3
// Description: This module interfaces between the AXI Port Checker output and
//              the 8 LEDs on the RedPitaya System. When a failure on the AXI
//              bus is detected, a LED pattern is sequences to display the
//              status bits which are set by the PC.
//
// Dependencies: 
//
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
//
//////////////////////////////////////////////////////////////////////////////////


module axi_pc2leds (
  // System signals
  input              clk                ,
  input              resetn             ,

  input              PC_asserted_i      ,      // AXI Protocol Checker has detected a bus failure
  input    [  96: 0] PC_status_i        ,      // AXI Protocol Checker status vector

  output reg         LED_drive_o        ,      // this module requests the LEDs to show the current data message
  output reg [ 7: 0] LED_data_o
);


reg  [   26: 0] dly_ctr     = 27'b0     ;
reg             dly_done    =  1'b0     ;
reg  [    1: 0] dly_state   =  2'h0     ;

reg  [   96: 0] sr          = 97'b0     ;
reg  [    6: 0] sr_ctr      =  7'b0     ;
reg             sr_tmr_go   =  1'b0     ;
reg  [    0: 0] sr_state    =  1'h0     ;


always @(posedge clk)
begin
   if (!resetn) begin
      dly_ctr   <= 27'b0;
      dly_done  <=  1'b0;
      dly_state <=  1'h0;

   end else begin
      casez (dly_state)
      1'h0: begin
         dly_done <= 1'b0;
         if (sr_tmr_go) begin
            dly_ctr   <= ~(27'b0);
            dly_state <= 1'h1;
         end else begin
            dly_ctr  <= 27'b0;
         end
      end

      1'h1: begin
         if (dly_ctr == 27'b0) begin
            dly_done  <= 1'b1;
            dly_state <= 1'h0;
         end else begin
            dly_ctr = dly_ctr - 1;
         end
      end
      endcase
   end
end


always @(posedge clk)
begin
   if (!resetn) begin
      LED_drive_o   <=  1'b0;
      LED_data_o    <=  8'b0;
      sr            <= 97'b0;
      sr_ctr        <=  7'b0;
      sr_tmr_go     <=  1'b0;
      sr_state      <=  2'h0;

   end else begin
      casez (sr_state)
      2'h0: begin
         if (PC_asserted_i) begin
            LED_drive_o <= 1'b1;
            LED_data_o  <= 8'hff;              // show alert pattern
            sr          <= PC_status_i;
            sr_ctr      <= 7'd0;               // reset display counter
            sr_tmr_go   <= 1'b1;
            sr_state    <= 2'h1;
         end else begin
            sr_tmr_go   <= 1'b0;
         end
      end

      2'h1: begin
         sr_tmr_go <= 1'b0;
         if (dly_done) begin
            sr_state <= 2'h2;
         end
      end

      2'h2: begin
         if (sr[0]) begin
            LED_data_o  <= sr_ctr;
            sr_tmr_go   <= 1'b1;
            sr_state    <= 2'h1;
         end

         if (sr_ctr == 7'd96) begin
            LED_drive_o <= 1'b0;
            sr_tmr_go   <= 1'b0;
            sr_state    <= 2'h0;
         end else begin
            sr     <= { 1'b0, sr[96:1]};       // right shift, LSB first
            sr_ctr <= sr_ctr + 1;
         end
      end

      2'h3: begin
      end
      endcase
   end
end

endmodule
