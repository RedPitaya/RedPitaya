/**
 * $Id: red_pitaya_scope_tb.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya oscilloscope testbench.
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
 * Testbench for Red Pitaya oscilloscope module.
 *
 * This testbench generates two signals which are captured into ram. Writing into
 * buffers is done via ARM/trig.
 * Generating also external trigger to test debouncer logic.
 * 
 */

// test plan
// 1. trigger:
// 1.1. software
// 1.2. treshold            p/n
// 1.3. treshold hysteresis p/n
// 1.4. external            p/n
// 1.5. external no repeat  p/n
// 1.6. external from ASG
// 2. filter/decimator configurations
// 2. ...
// 3. accumulation

`timescale 1ns / 1ps

module red_pitaya_scope_tb #(
  // time periods
  realtime  TP_ADC = 8.0ns,  // 125MHz
  realtime  TP_SYS = 9.8ns,  // 102MHz
  // DUT configuration
  int unsigned ADC_DW = 14, // ADC data width
  int unsigned RSZ = 14  // RAM size is 2**RSZ
);

////////////////////////////////////////////////////////////////////////////////
// ADC signal generation
////////////////////////////////////////////////////////////////////////////////

function [ADC_DW-1:0] saw_a (int unsigned cyc);
  saw_a = ADC_DW'(cyc*14);
endfunction: saw_a

function [ADC_DW-1:0] saw_b (int unsigned cyc);
  saw_b = ADC_DW'(cyc*14);
endfunction: saw_b

logic              adc_clk ;
logic              adc_rstn;

logic [ADC_DW-1:0] adc_a    ;
logic [ADC_DW-1:0] adc_b    ;
logic              adc_b_dir;

// ADC clock
initial            adc_clk = 1'b0;
always #(TP_ADC/2) adc_clk = ~adc_clk;

// ADC reset
initial begin
  adc_rstn = 1'b0;
  repeat(4) @(posedge adc_clk);
  adc_rstn = 1'b1;
end

// ADC cycle counter
int unsigned adc_cyc=0;
always_ff @ (posedge adc_clk)
adc_cyc <= adc_cyc+1;


initial begin
  adc_a = 0;
  adc_b = -2**(ADC_DW-1);
  adc_b_dir = 1'b1;
end

always @(posedge adc_clk)
adc_a <= adc_a + 14'h17;

always @(posedge adc_clk) begin
  if      ($signed(adc_b) > $signed( 14'd8000)) adc_b_dir <= 1'b0;
  else if ($signed(adc_b) < $signed(-14'd8000)) adc_b_dir <= 1'b1;
  adc_b <= adc_b_dir ? adc_b + 14'h5 : adc_b - 14'h5;
end

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

logic            trig_ext ;

logic            sys_clk  ;
logic            sys_rstn ;
logic [ 32-1: 0] sys_addr ;
logic [ 32-1: 0] sys_wdata;
logic [  4-1: 0] sys_sel  ;
logic            sys_wen  ;
logic            sys_ren  ;
logic [ 32-1: 0] sys_rdata;
logic            sys_err  ;
logic            sys_ack  ;

logic [ 32-1: 0] rdata;

// system clock & reset
initial begin
  sys_rstn = 1'b0;
  repeat(4) @(posedge sys_clk);
  sys_rstn = 1'b1;
end

initial          sys_clk = 1'b0;
always #(TP_SYS) sys_clk = ~sys_clk;

// State machine programming
initial begin
   // external trigger
   trig_ext = 1'b0;

   wait (sys_rstn && adc_rstn)
   repeat(10) @(posedge sys_clk);

   bus.bus_write(32'h10,32'd10000);  // after trigger delay
   bus.bus_write(32'h14,32'd8    );  // data decimation
   bus.bus_write(32'hC,-32'd7000 );  // trigger treshold
   bus.bus_write(32'h20,32'd20   );  // hysteresis
   bus.bus_write(32'h24,32'd200  );  // hysteresis

   bus.bus_write(32'h0,32'h1     );  // start aquisition
//   repeat(800) @(posedge sys_clk);
//   bus.bus_write(32'h4,32'h1     );  // do trigger
//   repeat(100) @(posedge sys_clk);
//   bus.bus_write(32'h0,32'h2     );  // reset
//   repeat(800) @(posedge sys_clk);
//   bus.bus_write(32'h4,32'h1     );  // do trigger
//
//   repeat(800) @(posedge sys_clk);
//   bus.bus_write(32'h0,32'h1     );  // start aquisition
//   repeat(100000) @(posedge sys_clk);
//   bus.bus_write(32'h4,32'h5     );  // do trigger
//
//   repeat(20000) @(posedge adc_clk);
//   repeat(1) @(posedge sys_clk);
//   bus.bus_read(32'h10000, rdata);  // read value from memory
//   bus.bus_read(32'h10004, rdata);  // read value from memory
//   bus.bus_read(32'h20000, rdata);  // read value from memory
//   bus.bus_read(32'h20004, rdata);  // read value from memory
//
   $finish ();
end

////////////////////////////////////////////////////////////////////////////////
// clock & reset
////////////////////////////////////////////////////////////////////////////////

sys_bus_model bus (
  // system signals
  .sys_clk_i      (sys_clk  ),
  .sys_rstn_i     (sys_rstn ),
  // bus protocol signals
  .sys_addr_o     (sys_addr ),
  .sys_wdata_o    (sys_wdata),
  .sys_sel_o      (sys_sel  ),
  .sys_wen_o      (sys_wen  ),
  .sys_ren_o      (sys_ren  ),
  .sys_rdata_i    (sys_rdata),
  .sys_err_i      (sys_err  ),
  .sys_ack_i      (sys_ack  ) 
);

red_pitaya_scope #(
  .RSZ (RSZ)
) scope (
  // ADC
  .adc_clk_i      (adc_clk  ),  // clock
  .adc_rstn_i     (adc_rstn ),  // reset - active low
  .adc_a_i        (adc_a    ),  // CH 1
  .adc_b_i        (adc_b    ),  // CH 2
  // trigger sources
  .trig_ext_i     (trig_ext ),  // external trigger
  .trig_asg_i     (trig_ext ),  // ASG trigger
   // System bus
  .sys_clk_i      (sys_clk  ),  // clock
  .sys_rstn_i     (sys_rstn ),  // reset - active low
  .sys_addr_i     (sys_addr ),  // address
  .sys_wdata_i    (sys_wdata),  // write data
  .sys_sel_i      (sys_sel  ),  // write byte select
  .sys_wen_i      (sys_wen  ),  // write enable
  .sys_ren_i      (sys_ren  ),  // read enable
  .sys_rdata_o    (sys_rdata),  // read data
  .sys_err_o      (sys_err  ),  // error indicator
  .sys_ack_o      (sys_ack  )   // acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_scope_tb.vcd");
  $dumpvars(0, red_pitaya_scope_tb);
end

endmodule: red_pitaya_scope_tb
