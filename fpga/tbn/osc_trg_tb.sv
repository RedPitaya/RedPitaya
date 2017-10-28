////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module osc_trg_tb #(
  // clock time periods
  realtime  TP = 8.0ns,  // 125MHz
  // stream parameters
  int unsigned DN = 1,
  type DT = logic signed [16-1:0]  // data width
);

typedef DT DTI;
typedef DT DTO;
typedef DT DT_A []; 

// system signals
logic      clk ;  // clock
logic      rstn;  // reset - active low

// control
logic      ctl_rst;  // synchronous reset
// configuration
logic      cfg_edg;  // edge select (0-rising, 1-falling)
DT         cfg_pos;  // positive level
DT         cfg_neg;  // negative level
// output triggers
logic      sts_trg;

// stream input/output
axi4_stream_if #(.DT (DT)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DT (DT)) sto (.ACLK (clk), .ARESETn (rstn));

////////////////////////////////////////////////////////////////////////////////
// clock
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

// clocking 
default clocking cb @ (posedge clk);
  input  rstn;
  // control
  input  ctl_rst;
  // configuration
  input  cfg_edg;
  input  cfg_pos;
  input  cfg_neg;
  // output triggers
  input  sts_trg;
endclocking: cb

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

DT dti [];
DT dto [];
axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;
axi4_stream_pkg::axi4_stream_class #(.DT (DT)) clo;

initial begin
  cli = new;
  clo = new;
  ctl_rst <= 1'b0;
  cfg_edg <= 0;
  cfg_pos <= 0;
  cfg_neg <= 0;
  // initialization
  rstn <= 1'b0;
  ##4;
  rstn <= 1'b1;
  ##4;

  // simple tests
  cfg_pos <= 0;
  cfg_neg <= 0;

  cfg_edg <= 0;
  ##1;
  test(-8,8);

  cfg_edg <= 1;
  ##1;
  test(-8,8);

  cfg_edg <= 1;
  ##1;
  test(8,-8, -1);

  cfg_edg <= 0;
  ##1;
  test(8,-8, -1);

  // hysteresis tests
  cfg_pos <=  4;
  cfg_neg <= -4;

  cfg_edg <= 0;
  ##1;
  test(-8,+8,+1);
  test(+8, 0,-1);
  test( 0,+8,+1);
  test(+8,-8,-1);
  test(-8,+8,+1);

  cfg_edg <= 1;
  ##1;
  test(+8,-8,-1);
  test(-8, 0,+1);
  test( 0,-8,-1);
  test(-8,+8,+1);
  test(+8,-8,-1);

  // end simulation
  ##4;
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// tests
////////////////////////////////////////////////////////////////////////////////

task test (input int from, to, step = 1);
  // prepare data
  dti = cli.range (from, to, step);
  dto = dti;
  // send data into stream
  cli.add_pkt (dti);
  clo.add_pkt (dto);
  fork
    str_src.run (cli);
    str_drn.run (clo);
  join
  // check received data
//  error += clo.check (dto);
endtask: test

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DN (DN), .DT (DT)) str_src (.str (sti));

osc_trg #(
  .DN (DN),
  .DT (DT)
) osc_trg (
  // stream input/output
  .sti      (sti),
  .sto      (sto),
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_edg  (cfg_edg),
  .cfg_pos  (cfg_pos),
  .cfg_neg  (cfg_neg),
  // output triggers
  .sts_trg  (sts_trg)
);

axi4_stream_drn #(.DN (DN), .DT (DT)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("osc_trg_tb.vcd");
  $dumpvars(0, osc_trg_tb);
end

endmodule: osc_trg_tb
