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

`timescale 1ns / 1ps

module red_pitaya_scope_tb();

reg   [ 14-1: 0] adc_a           ;
reg   [ 14-1: 0] adc_b           ;
reg              adc_b_dir       ;
reg              adc_clk         ;
reg              adc_rstn        ;
reg              trig_ext        ;

reg              sys_clk         ;
reg              sys_rstn        ;
wire  [ 32-1: 0] sys_addr        ;
wire  [ 32-1: 0] sys_wdata       ;
wire  [  4-1: 0] sys_sel         ;
wire             sys_wen         ;
wire             sys_ren         ;
wire  [ 32-1: 0] sys_rdata       ;
wire             sys_err         ;
wire             sys_ack         ;

sys_bus_model i_bus
(
  .sys_clk_i      (  sys_clk      ),
  .sys_rstn_i     (  sys_rstn     ),
  .sys_addr_o     (  sys_addr     ),
  .sys_wdata_o    (  sys_wdata    ),
  .sys_sel_o      (  sys_sel      ),
  .sys_wen_o      (  sys_wen      ),
  .sys_ren_o      (  sys_ren      ),
  .sys_rdata_i    (  sys_rdata    ),
  .sys_err_i      (  sys_err      ),
  .sys_ack_i      (  sys_ack      ) 
);

red_pitaya_scope i_scope
(
  // ADC
  .adc_a_i         (  adc_a         ),  // CH 1
  .adc_b_i         (  adc_b         ),  // CH 2
  .adc_clk_i       (  adc_clk       ),  // clock
  .adc_rstn_i      (  adc_rstn      ),  // reset - active low
  .trig_ext_i      (  trig_ext      ),  // external trigger
  .trig_asg_i      (  trig_ext      ),  // ASG trigger

  // AXI0 master
  .axi0_clk_o      (        ),  // global clock
  .axi0_rstn_o     (        ),  // global reset
  .axi0_waddr_o    (        ),  // system write address
  .axi0_wdata_o    (        ),  // system write data
  .axi0_wsel_o     (        ),  // system write byte select
  .axi0_wvalid_o   (        ),  // system write data valid
  .axi0_wlen_o     (        ),  // system write burst length
  .axi0_wfixed_o   (        ),  // system write burst type (fixed / incremental)
  .axi0_werr_i     (  1'b0  ),  // system write error
  .axi0_wrdy_i     (  1'b1  ),  // system write ready
  .axi0_rstn_i     (  1'b1  ),  // reset from PS

   // AXI1 master
  .axi1_clk_o      (        ),  // global clock
  .axi1_rstn_o     (        ),  // global reset
  .axi1_waddr_o    (        ),  // system write address
  .axi1_wdata_o    (        ),  // system write data
  .axi1_wsel_o     (        ),  // system write byte select
  .axi1_wvalid_o   (        ),  // system write data valid
  .axi1_wlen_o     (        ),  // system write burst length
  .axi1_wfixed_o   (        ),  // system write burst type (fixed / incremental)
  .axi1_werr_i     (  1'b0  ),  // system write error
  .axi1_wrdy_i     (  1'b1  ),  // system write ready
  .axi1_rstn_i     (  1'b1  ),  // reset from PS

   // System bus
  .sys_clk_i       (  sys_clk       ),  // clock
  .sys_rstn_i      (  sys_rstn      ),  // reset - active low
  .sys_addr_i      (  sys_addr      ),  // address
  .sys_wdata_i     (  sys_wdata     ),  // write data
  .sys_sel_i       (  sys_sel       ),  // write byte select
  .sys_wen_i       (  sys_wen       ),  // write enable
  .sys_ren_i       (  sys_ren       ),  // read enable
  .sys_rdata_o     (  sys_rdata     ),  // read data
  .sys_err_o       (  sys_err       ),  // error indicator
  .sys_ack_o       (  sys_ack       )   // acknowledge signal
);

//---------------------------------------------------------------------------------
//
// signal generation

// system clock & reset
initial begin
   sys_clk  <= 1'b0 ;
   sys_rstn <= 1'b0 ;
   repeat(10) @(posedge sys_clk);
      sys_rstn <= 1'b1  ;
end

always begin
   #4.9  sys_clk <= !sys_clk ;
end

// ADC clock & reset
initial begin
   adc_clk  <= 1'b0  ;
   adc_rstn <= 1'b0  ;
   repeat(10) @(posedge adc_clk);
      adc_rstn <= 1'b1  ;
end

always begin
   #4  adc_clk <= !adc_clk ;
end

// ADC simple signal generation
initial begin
   adc_a <= 14'h0    ;
   adc_b <= 14'h2000 ;
   adc_b_dir <= 1'b1 ;
end

always @(posedge adc_clk) begin
   adc_a <= adc_a + 14'h17 ;
end
always @(posedge adc_clk) begin
   if ($signed(adc_b) > $signed(14'd8000)) adc_b_dir <= 1'b0; else if ($signed(adc_b) < $signed(-14'd8000)) adc_b_dir <= 1'b1 ;

   if (adc_b_dir) adc_b <= adc_b + 14'h5 ; else adc_b <= adc_b - 14'h5 ;
end

// External trigger
initial begin
   trig_ext <= 1'b0 ;

   wait (sys_rstn && adc_rstn)
   repeat(1000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;

   repeat(10000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;

   repeat(100000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;

   repeat(100000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;
   repeat(10000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;
   repeat(10000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;

   repeat(100000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;
   repeat(10000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;
   repeat(10000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;
   repeat(10000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;

   repeat(100000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;

   repeat(100000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;

   repeat(100000) @(posedge sys_clk);
   trig_ext <= !trig_ext ;
end

// State machine programming
initial begin
   wait (sys_rstn && adc_rstn)
   repeat(10) @(posedge sys_clk);

   i_bus.bus_write(32'h10,32'd10000);  // after trigger delay
   i_bus.bus_write(32'h14,32'd1);  // data decimation
   i_bus.bus_write(32'hC,-32'd7000);  // trigger treshold
   i_bus.bus_write(32'h20,32'd20);  // hysteresis
   i_bus.bus_write(32'h24,32'd200);  // hysteresis

   i_bus.bus_write(32'h50,32'h300);   // a_axi_start
   i_bus.bus_write(32'h54,32'h50000); // a_axi_stop
   i_bus.bus_write(32'h58,32'd50);    // a_axi_dly
   i_bus.bus_write(32'h5C,32'd1);     // a_axi_en

   i_bus.bus_write(32'h70,32'h500);   // a_axi_start
   i_bus.bus_write(32'h74,32'h5000); // a_axi_stop
   i_bus.bus_write(32'h78,32'd900);    // a_axi_dly
   i_bus.bus_write(32'h7C,32'd1);     // a_axi_en

   i_bus.bus_write(32'h0,32'h1);  // start aquisition
   repeat(800) @(posedge sys_clk);
   i_bus.bus_write(32'h4,32'h1);  // do trigger
   repeat(100) @(posedge sys_clk);
   i_bus.bus_write(32'h0,32'h2);  // reset
   repeat(800) @(posedge sys_clk);
   i_bus.bus_write(32'h4,32'h1);  // do trigger

   repeat(800) @(posedge sys_clk);
   i_bus.bus_write(32'h0,32'h1);  // start aquisition
   repeat(100000) @(posedge sys_clk);
   i_bus.bus_write(32'h4,32'h5);  // do trigger

   repeat(20000) @(posedge adc_clk);
   repeat(1) @(posedge sys_clk);
   i_bus.bus_read(32'h10000);  // read value from memory
   i_bus.bus_read(32'h10004);  // read value from memory
   i_bus.bus_read(32'h20000);  // read value from memory
   i_bus.bus_read(32'h20004);  // read value from memory
end

endmodule
