////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module la_trigger_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // stream parameters
  int unsigned DN = 1,
  type DT = logic [8-1:0]  // data type
);

// system signals
logic clk ;  // clock
logic rstn;  // reset - active low

// configuration
DT    cfg_cmp_msk;  // comparator mask
DT    cfg_cmp_val;  // comparator value
DT    cfg_edg_pos;  // edge positive
DT    cfg_edg_neg;  // edge negative

// stream input/output
axi4_stream_if #(.DT (DT)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DT (DT)) sto (.ACLK (clk), .ARESETn (rstn));
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
  cfg_cmp_msk = '0;
  cfg_cmp_val = '0;
  cfg_edg_pos = 'h00;
  cfg_edg_neg = 'h00;

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

axi4_stream_src #(.DT (DT)) str_src (.str (sti));
axi4_stream_drn #(.DT (DT)) str_drn (.str (sto));

la_trigger #(
  .DT (DT)
) la_trigger (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_cmp_msk (cfg_cmp_msk),
  .cfg_cmp_val (cfg_cmp_val),
  .cfg_edg_pos (cfg_edg_pos),
  .cfg_edg_neg (cfg_edg_neg),
  // output triggers
  .sts_trg  (trg_out),
  // stream monitor
  .sti      (sti),
  .sto      (sto)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("la_trigger_tb.vcd");
  $dumpvars(0, la_trigger_tb);
end

endmodule: la_trigger_tb
