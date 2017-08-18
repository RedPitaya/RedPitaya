////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module la_trg_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // stream parameters
  int unsigned DN = 2,
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

// trigger output
logic [DN-1:0] trg_out;

// stream input/output
axi4_stream_if #(.DN (DN), .DT (DT)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DN (DN), .DT (DT)) sto (.ACLK (clk), .ARESETn (rstn));

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  DT dat [];
  axi4_stream_pkg::axi4_stream_class #(.DN (DN), .DT (DT)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DN (DN), .DT (DT)) clo;
  cli = new;
  clo = new;

  // for now initialize configuration to an idle value
  cfg_cmp_msk = '1;
  cfg_cmp_val = 'h08;
  cfg_edg_pos = 'h00;
  cfg_edg_neg = 'h01;

  // initialization
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  repeat(4) @(posedge clk);

  // create data array
  dat = cli.range (0, 16);
  $display ("dat [%d] = %p", dat.size(), dat);
  // send data into stream
  cli.add_pkt (dat);
  clo.add_pkt (dat);
  $display ("cli.que = %p", cli.que);
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

axi4_stream_src #(.DN (DN), .DT (DT)) str_src (.str (sti));
axi4_stream_drn #(.DN (DN), .DT (DT)) str_drn (.str (sto));

la_trg #(
  .DN (DN),
  .DT (DT)
) la_trg (
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
  $dumpfile("la_trg_tb.vcd");
  $dumpvars(0, la_trg_tb);
end

endmodule: la_trg_tb
