/**
 * $Id: red_pitaya_ams.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya analog mixed signal.
 *
 * @Author Matej Oblak
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */

/**
 * GENERAL DESCRIPTION:
 *
 * Module using XADC and software interface for PWM DAC.
 *
 *
 *                    /------\
 *   SUPPLY V. -----> |      |
 *   TEMPERATURE ---> | XADC | ------
 *   EXTERNAL V. ---> |      |       |
 *                    \------/       |
 *                                   |
 *                                   ˇ
 *                               /------\
 *   PWD DAC <------------------ | REGS | <------> SW
 *                               \------/
 *
 *
 * Reading system and external voltages is done with XADC, running in sequencer
 * mode. It measures supply voltages, temperature and voltages on external
 * connector. Measured values are then exposed to SW.
 *
 * Beside that SW can sets registes which controls logic for PWM DAC (analog module).
 * 
 */

module red_pitaya_ams (
   // ADC
   input                 clk_i           ,  // clock
   input                 rstn_i          ,  // reset - active low
   // PWM DAC
   output reg [ 24-1: 0] dac_a_o         ,  // values used for
   output reg [ 24-1: 0] dac_b_o         ,  // conversion into PWM signal
   output reg [ 24-1: 0] dac_c_o         ,  // 
   output reg [ 24-1: 0] dac_d_o         ,  // 
   // system bus
   input      [ 32-1: 0] sys_addr        ,  // bus address
   input      [ 32-1: 0] sys_wdata       ,  // bus write data
   input      [  4-1: 0] sys_sel         ,  // bus write byte select
   input                 sys_wen         ,  // bus write enable
   input                 sys_ren         ,  // bus read enable
   output reg [ 32-1: 0] sys_rdata       ,  // bus read data
   output reg            sys_err         ,  // bus error indicator
   output reg            sys_ack            // bus acknowledge signal
);

//---------------------------------------------------------------------------------
//
//  System bus connection

always @(posedge clk_i)
if (rstn_i == 1'b0) begin
   dac_a_o     <= 24'h0F_0000 ;
   dac_b_o     <= 24'h4E_0000 ;
   dac_c_o     <= 24'h75_0000 ;
   dac_d_o     <= 24'h9C_0000 ;
end else begin
   if (sys_wen) begin
      if (sys_addr[19:0]==16'h20)   dac_a_o <= sys_wdata[24-1: 0] ;
      if (sys_addr[19:0]==16'h24)   dac_b_o <= sys_wdata[24-1: 0] ;
      if (sys_addr[19:0]==16'h28)   dac_c_o <= sys_wdata[24-1: 0] ;
      if (sys_addr[19:0]==16'h2C)   dac_d_o <= sys_wdata[24-1: 0] ;
   end
end

wire sys_en;
assign sys_en = sys_wen | sys_ren;

always @(posedge clk_i)
if (rstn_i == 1'b0) begin
   sys_err <= 1'b0 ;
   sys_ack <= 1'b0 ;
end else begin
   sys_err <= 1'b0 ;
   casez (sys_addr[19:0])
     20'h00020 : begin sys_ack <= sys_en;         sys_rdata <= {{32-24{1'b0}}, dac_a_o}          ; end
     20'h00024 : begin sys_ack <= sys_en;         sys_rdata <= {{32-24{1'b0}}, dac_b_o}          ; end
     20'h00028 : begin sys_ack <= sys_en;         sys_rdata <= {{32-24{1'b0}}, dac_c_o}          ; end
     20'h0002C : begin sys_ack <= sys_en;         sys_rdata <= {{32-24{1'b0}}, dac_d_o}          ; end
       default : begin sys_ack <= sys_en;         sys_rdata <=   32'h0                           ; end
   endcase
end

endmodule
