/**
 * $Id: red_pitaya_ams_tb.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya analog mixed signal testbench.
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
 * Testbench for analog mixed signal module .
 *
 * This bench tests functionality of XADC, values which are used as analog 
 * inputs are defined in xadc_sim_values.txt.
 * 
 */

`timescale 1ns / 1ps

module red_pitaya_ams_tb #(
  // time periods
  realtime  TP = 8.0ns  // 125MHz
);

glbl glbl();

////////////////////////////////////////////////////////////////////////////////
// signal generation
////////////////////////////////////////////////////////////////////////////////

logic            clk ;
logic            rstn;

// ADC clock
initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

// ADC reset
initial begin
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
end

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

logic [ 32-1: 0] sys_addr ;
logic [ 32-1: 0] sys_wdata;
logic [  4-1: 0] sys_sel  ;
logic            sys_wen  ;
logic            sys_ren  ;
logic [ 32-1: 0] sys_rdata;
logic            sys_err  ;
logic            sys_ack  ;

logic [ 32-1: 0] rdata;

initial begin
   wait (rstn)
   repeat(10) @(posedge clk);

   bus.write(32'h20,32'h10000);  // test write

   repeat(20000) @(posedge clk);

   bus.read(32'h30, rdata);
   bus.read(32'h34, rdata);
   bus.read(32'h38, rdata);
   bus.read(32'h20, rdata);
   bus.read(32'h24, rdata);
end

////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

sys_bus_model bus (
  // system signals
  .clk          (clk      ),
  .rstn         (rstn     ),
  // bus protocol signals
  .sys_addr     (sys_addr ),
  .sys_wdata    (sys_wdata),
  .sys_sel      (sys_sel  ),
  .sys_wen      (sys_wen  ),
  .sys_ren      (sys_ren  ),
  .sys_rdata    (sys_rdata),
  .sys_err      (sys_err  ),
  .sys_ack      (sys_ack  ) 
);

red_pitaya_ams ams (
  .clk_i        (clk ),
  .rstn_i       (rstn),

  .vinp_i       (   ),  // voltages p
  .vinn_i       (   ),  // voltages n

  .dac_a_o      (   ),
  .dac_b_o      (   ),
  .dac_c_o      (   ),
  .dac_d_o      (   ),
   // System bus
  .sys_addr     (sys_addr ),
  .sys_wdata    (sys_wdata),
  .sys_sel      (sys_sel  ),
  .sys_wen      (sys_wen  ),
  .sys_ren      (sys_ren  ),
  .sys_rdata    (sys_rdata),
  .sys_err      (sys_err  ),
  .sys_ack      (sys_ack  )
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_ams_tb.vcd");
  $dumpvars(0, red_pitaya_ams_tb);
end

endmodule: red_pitaya_ams_tb
