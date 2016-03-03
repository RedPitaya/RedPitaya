////////////////////////////////////////////////////////////////////////////////
// Module: Acquire
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module acq_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // parameters
  int unsigned TN = 1,   // trigger number
  int unsigned TW = 64,  // time width
  int unsigned CW = 32,  // counter width
  // data bus type
  int unsigned DN = 1,
  type DT = logic signed [14-1:0]
);

typedef DT DT_A [];

// system signals
logic          clk ;  // clock
logic          rstn;  // reset - active low

// current time stamp
logic [TW-1:0] cts;
// interrupts
logic          irq_trg;  // trigger
logic          irq_stp;  // stop
// control
logic          ctl_rst;
// configuration (mode)
logic          cfg_con;  // continuous
logic          cfg_aut;  // automatic
// configuration/status pre trigger
logic [TN-1:0] cfg_trg;
logic [CW-1:0] cfg_pre;
logic [CW-1:0] sts_pre;
// configuration/status post trigger
logic [CW-1:0] cfg_pst;
logic [CW-1:0] sts_pst;
// control/status/timestamp acquire
logic          ctl_acq;  // acquire start
logic          sts_acq;
logic [TW-1:0] cts_acq;
// control/status/timestamp trigger
logic [TN-1:0] ctl_trg;
logic          sts_trg;
logic [TW-1:0] cts_trg;
// control/status/timestamp stop
logic          ctl_stp;  // acquire stop
logic [TW-1:0] cts_stp;

// stream input/output
axi4_stream_if #(.DN (DN), .DAT_T (DT)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DN (DN), .DAT_T (DT)) sto (.ACLK (clk), .ARESETn (rstn));

int unsigned error = 0;

////////////////////////////////////////////////////////////////////////////////
// clock and time stamp
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial                cts  = 0;
always @ (posedge clk) cts <= cts + 1;

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  DT dti [];
  DT dto [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) clo;

  // for now initialize configuration to an idle value
  ctl_rst = 1'b0;
  cfg_trg = '1;
  cfg_con = 1'b0;
  cfg_aut = 1'b0;
  cfg_pre = 0;
  cfg_pst = 0;
  ctl_acq = 1'b0;
  ctl_trg = 1'b0;
  ctl_stp = 1'b0;

  // initialization
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  repeat(4) @(posedge clk);

  // send data into stream
  cli = new;
  clo = new;
  dti = cli.range (-8, 8);
//  dto = TODO;
  // send data into stream
  cli.set_packet (dti);
  clo.set_packet (dto);
  fork
    str_src.run (cli);
    str_drn.run (clo);
  join
  // check received data
  error += clo.check (dto);

  // activate acquire
  acq_pls;

  // send data into stream
  cli = new;
  clo = new;
  dti = cli.range (-8, 8);
  dto = clo.range (-8, 8);;
  // send data into stream
  cli.set_packet (dti);
  clo.set_packet (dto);
  fork
    str_src.run (cli);
    str_drn.run (clo);
  join
  // check received data
  error += clo.check (dto);

//  // send array
//  size = 4;
//  sti_dat = new [size];
//  sto_dat = new [size];
//  for (int i=0; i<=size; i++) begin
//    sti_dat[i] = '{i, '1, i==(size-1)};
//  end
//  acq_pls();
//  src_ary (sti_dat);
//  repeat(size+2) @(posedge clk);
//  repeat(size+2) @(posedge clk);
//  drn_ary (sto_dat);
//  $display (sti_dat);
//  $display (sto_dat);

  // end simulation
  repeat(4) @(posedge clk);
  if (error)  $display("FAILURE");
  else        $display("SUCCESS");
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// helper tasks
////////////////////////////////////////////////////////////////////////////////

// generate reset pulse
task rst_pls ();
  ctl_rst = 1'b1;
  @(posedge clk);
  ctl_rst = 1'b0;
endtask: rst_pls

// activate acquire
task acq_pls ();
  ctl_acq = 1'b1;
  repeat(1) @(posedge clk);
  ctl_acq = 1'b0;
endtask: acq_pls

// generate trigger pulse
task trg_pls (logic [TN-1:0] trg);
  ctl_trg = trg;
  @(posedge clk);
  ctl_trg = '0;
endtask: trg_pls

////////////////////////////////////////////////////////////////////////////////
// tests
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DN (DN), .DT (DT)) str_src (.str (sti));

acq #(
  .TW (TW),
  .CW (CW)
) acq (
  // stream input/output
  .sti      (sti),
  .sto      (sto),
  // current time stamp
  .cts      (cts),
  // interrupts
  .irq_trg  (irq_trg),
  .irq_stp  (irq_stp),
  // control
  .ctl_rst  (ctl_rst),
  // configuration (mode)
  .cfg_trg  (cfg_trg),
  .cfg_con  (cfg_con),
  .cfg_aut  (cfg_aut),
  // configuration/status pre trigger
  .cfg_pre  (cfg_pre),
  .sts_pre  (sts_pre),
  // configuration/status post trigger
  .cfg_pst  (cfg_pst),
  .sts_pst  (sts_pst),
  // control/status/timestamp acquire
  .ctl_acq  (ctl_acq),  // acquire start
  .sts_acq  (sts_acq),
  .cts_acq  (cts_acq),
  // control/status/timestamp trigger
  .ctl_trg  (ctl_trg),
  .sts_trg  (sts_trg),
  .cts_trg  (cts_trg),
  // control/status/timestamp stop
  .ctl_stp  (ctl_stp),  // acquire stop
  .cts_stp  (cts_stp)
);

axi4_stream_drn #(.DN (DN), .DT (DT)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("acq_tb.vcd");
  $dumpvars(0, acq_tb);
end

endmodule: acq_tb
