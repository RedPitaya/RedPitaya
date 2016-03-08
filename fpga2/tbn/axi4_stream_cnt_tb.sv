////////////////////////////////////////////////////////////////////////////////
// Module: AXI4-Stream data counter
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module axi4_stream_cnt_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // stream parameters
  int unsigned DN = 1,
  type DT = logic [8-1:0],  // data type
  // counter width
  int unsigned CW = 8
);

// system signals
logic clk ;  // clock
logic rstn;  // reset - active low

logic ctl_rst;

// configuration
logic [CW-1:0] sts_cur;  // current     counter status
logic [CW-1:0] sts_lst;  // last packet counter status

// stream input/output
axi4_stream_if #(.DT (DT)) str (.ACLK (clk), .ARESETn (rstn));

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  DT dat [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) clo;

  // for now initialize configuration to an idle value
  ctl_rst = 1'b0;

  // initialization
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  repeat(4) @(posedge clk);

  // send data into stream
  cli = new;
  clo = new;
  dat = cli.range (0, 16);
  $display ("dat [%d] = %p", dat.size(), dat);
  // send data into stream
  cli.add_pkt (dat);
  clo.add_pkt (dat);
  fork
    str_src.run (cli);
    str_drn.run (clo);
  join

  // end simulation
  repeat(4) @(posedge clk);
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DT (DT)) str_src (.str (str));
axi4_stream_drn #(.DT (DT)) str_drn (.str (str));

axi4_stream_cnt #(
  .DN (DN),
  .CW (CW)
) axi4_stream_cnt (
  // control
  .ctl_rst  (ctl_rst),
  // status
  .sts_cur  (sts_cur),
  .sts_lst  (sts_lst),
  // stream monitor
  .str      (str)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("axi4_stream_cnt_tb.vcd");
  $dumpvars(0, axi4_stream_cnt_tb);
end

endmodule: axi4_stream_cnt_tb
