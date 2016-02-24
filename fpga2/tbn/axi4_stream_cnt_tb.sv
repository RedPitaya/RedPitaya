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
  type DAT_T = logic [8-1:0],  // data type
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
axi4_stream_if #(.DAT_T (DAT_T)) str (.ACLK (clk), .ARESETn (rstn));

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  // for now initialize configuration to an idle value
  ctl_rst = 1'b0;

  // initialization
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  repeat(4) @(posedge clk);

  // send data into stream
  for (int unsigned i=0; i<16; i++) begin
    str_src.put(i, '1, i==(16-1));
  end
  repeat(16) @(posedge clk);
  repeat(4) @(posedge clk);

  // end simulation
  repeat(4) @(posedge clk);
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DAT_T (DAT_T)) str_src (.str (str));
axi4_stream_drn #(.DAT_T (DAT_T)) str_drn (.str (str));

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
