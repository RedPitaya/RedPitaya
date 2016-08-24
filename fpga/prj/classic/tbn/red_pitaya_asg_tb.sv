/**
 * $Id: red_pitaya_asg_tb.v 1271 2014-02-25 12:32:34Z matej.oblak $
 *
 * @brief Red Pitaya arbitrary signal generator testbench.
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
 * Testbench for arbitrary signal generator module.
 *
 * This testbench writes values into RAM table and sets desired configuration.
 * 
 */

`timescale 1ns / 1ps

module red_pitaya_asg_tb #(
  // time period
  realtime  TP = 8.0ns,  // 125MHz
  // DUT configuration
  int unsigned DAC_DW = 14, // ADC data width
  int unsigned RSZ = 14  // RAM size is 2**RSZ
);

////////////////////////////////////////////////////////////////////////////////
// DAC signal generation
////////////////////////////////////////////////////////////////////////////////


logic               clk ;
logic               rstn;

logic [DAC_DW-1: 0] dac_a;
logic [DAC_DW-1: 0] dac_b;

logic               trig;

// DAC clock
initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

// DAC reset
initial begin
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
end

// ADC cycle counter
int unsigned dac_cyc=0;
always_ff @ (posedge clk)
dac_cyc <= dac_cyc+1;

always begin
  trig <= 1'b0 ;
  repeat(100000) @(posedge clk);
  trig <= 1'b1 ;
  repeat(1200) @(posedge clk);
  trig <= 1'b0 ;
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

logic        [ 32-1: 0] rdata;
logic signed [ 32-1: 0] rdata_blk [];

//---------------------------------------------------------------------------------
//
// signal generation

reg [9-1: 0] ch0_set;
reg [9-1: 0] ch1_set;

initial begin
  repeat(10) @(posedge clk);

  // CH0 DAC data
  bus.write(32'h10000, 32'd3     );  // write table
  bus.write(32'h10004, 32'd30    );  // write table
  bus.write(32'h10008, 32'd8000  );  // write table
  bus.write(32'h1000C,-32'd4     );  // write table
  bus.write(32'h10010,-32'd40    );  // write table
  bus.write(32'h10014,-32'd8000  );  // write table
  bus.write(32'h10018,-32'd2000  );  // write table
  bus.write(32'h1001c, 32'd250   );  // write table

  // CH0 DAC settings
  bus.write(32'h00004,{2'h0,-14'd500, 2'h0, 14'h2F00}  );  // DC offset, amplitude
  bus.write(32'h00008,{2'h0, 14'd7, 16'hffff}          );  // table size
  bus.write(32'h0000C,{2'h0, 14'h1, 16'h0}             );  // reset offset
  bus.write(32'h00010,{2'h0, 14'h2, 16'h0}             );  // table step
  bus.write(32'h00018,{16'h0, 16'd0}                   );  // number of cycles
  bus.write(32'h0001C,{16'h0, 16'd0}                   );  // number of repetitions
  bus.write(32'h00020,{32'd3}                          );  // number of 1us delay between repetitions

  ch0_set = {1'b0 ,1'b0, 1'b0, 1'b0, 1'b1,    1'b0, 3'h2} ;  // set_rgate, set_zero, set_rst, set_once(NA), set_wrap, 1'b0, trig_src

  // CH1 DAC data
  for (int k=0; k<8000; k++) begin
    bus.write(32'h20000 + (k*4), k);  // write table
  end

  // CH1 DAC settings
  bus.write(32'h00024,{2'h0, 14'd0, 2'h0, 14'h2000}    );  // DC offset, amplitude
  bus.write(32'h00028,{2'h0, 14'd7999, 16'hffff}       );  // table size
  bus.write(32'h0002C,{2'h0, 14'h5, 16'h0}             );  // reset offset
  bus.write(32'h00030,{2'h0, 14'h9, 16'h0}             );  // table step
  bus.write(32'h00038,{16'h0, 16'd0}                   );  // number of cycles
  bus.write(32'h0003C,{16'h0, 16'd5}                   );  // number of repetitions
  bus.write(32'h00040,{32'd10}                         );  // number of 1us delay between repetitions

  ch1_set = {1'b0, 1'b0, 1'b0, 1'b1, 1'b1,    1'b0, 3'h1} ;  // set_rgate, set_zero, set_rst, set_once(NA), set_wrap, 1'b0, trig_src

  bus.write(32'h00000,{8'h0, ch1_set,  8'h0, ch0_set}  ); // write configuration

  repeat(2000) @(posedge clk);

  ch1_set = {1'b0, 1'b0, 1'b1, 1'b1,    1'b0, 3'h1} ;  // set_a_zero, set_a_rst, set_a_once, set_a_wrap, 1'b0, trig_src

  bus.write(32'h00000,{7'h0, ch1_set,  7'h0, ch0_set}  ); // write configuration

  repeat(200) @(posedge clk);

  // CH1 table data readback
  rdata_blk = new [80];
  for (int k=0; k<80; k++) begin
    bus.read(32'h20000 + (k*4), rdata_blk [k]);  // read table
  end

  // CH1 table data readback
  for (int k=0; k<20; k++) begin
    bus.read(32'h00014, rdata);  // read read pointer
    bus.read(32'h00034, rdata);  // read read pointer
    repeat(1737) @(posedge clk);
  end

  repeat(20000) @(posedge clk);

  $finish();
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

red_pitaya_asg asg (
  // DAC
  .dac_clk_i      (clk      ),
  .dac_rstn_i     (rstn     ),
  .dac_a_o        (dac_a    ),  // CH 1
  .dac_b_o        (dac_b    ),  // CH 2
  // trigger
  .trig_a_i       (trig     ),
  .trig_b_i       (trig     ),
  .trig_out_o     (         ),
  // System bus
  .sys_addr       (sys_addr ),
  .sys_wdata      (sys_wdata),
  .sys_sel        (sys_sel  ),
  .sys_wen        (sys_wen  ),
  .sys_ren        (sys_ren  ),
  .sys_rdata      (sys_rdata),
  .sys_err        (sys_err  ),
  .sys_ack        (sys_ack  )
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_asg_tb.vcd");
  $dumpvars(0, red_pitaya_asg_tb);
end

endmodule: red_pitaya_asg_tb
