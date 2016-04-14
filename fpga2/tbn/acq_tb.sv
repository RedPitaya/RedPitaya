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
axi4_stream_if #(.DN (DN), .DT (DT)) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DN (DN), .DT (DT)) sto (.ACLK (clk), .ARESETn (rstn));

int unsigned error = 0;

////////////////////////////////////////////////////////////////////////////////
// clock and time stamp
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial                cts  = 0;
always @ (posedge clk) cts <= cts + 1;

// clocking 
default clocking cb @ (posedge clk);
  // current time stamp
  input  rstn;
  // current time stamp
  input  cts;
  // interrupts
  input  irq_trg;
  input  irq_stp;
  // control
  output ctl_rst;
  // configuration (mode)
  output cfg_con;
  output cfg_aut;
  // configuration/status pre trigger
  output cfg_trg;
  output cfg_pre;
  input  sts_pre;
  // configuration/status post trigger
  output cfg_pst;
  input  sts_pst;
  // control/status/timestamp acquire
  output ctl_acq;
  input  sts_acq;
  input  cts_acq;
  // control/status/timestamp trigger
  output ctl_trg;
  input  sts_trg;
  input  cts_trg;
  // control/status/timestamp stop
  output ctl_stp;
  input  cts_stp;
endclocking: cb

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  // for now initialize configuration to an idle value
  cb.ctl_rst <= 1'b0;
  cb.cfg_trg <= '1;
  cb.cfg_con <= 1'b0;
  cb.cfg_aut <= 1'b0;
  cb.cfg_pre <= 0;
  cb.cfg_pst <= 0;
  cb.ctl_acq <= 1'b0;
  cb.ctl_trg <= 1'b0;
  cb.ctl_stp <= 1'b0;

  // initialization
  rstn <= 1'b0;
  ##4;
  rstn <= 1'b1;
  ##4;

  // tests
  test_block;
  test_pass;
  test_trigger;
  test_trigger (.vld_max (2), .vld_rnd (2));
  test_stop;

  // end simulation
  ##4;
  if (error)  $display("FAILURE");
  else        $display("SUCCESS");
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// tests
////////////////////////////////////////////////////////////////////////////////

task test_block;
  DT dti [];
  DT dto [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) clo;
  // prepare data
  cli = new;
  clo = new;
  dti = cli.range (-8, 8);
  dto = new [0];
  // add packet into queue
  cli.add_pkt (dti);
  clo.add_pkt (dto);
  fork
    str_src.run (cli);
    str_drn.run (clo);
  join
  // check received data
  error += clo.check (dto);
endtask: test_block


task test_pass;
  DT dti [];
  DT dto [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) clo;
  // prepare data
  cli = new;
  clo = new;
  dti = cli.range (-8, 8);
  dto = clo.range (-8, 8);
  // add packet into queue
  cli.add_pkt (dti);
  clo.add_pkt (dto);
  // activate acquire
  acq_pls;
  fork
    str_src.run (cli);
    str_drn.run (clo);
  join
  // check received data
  error += clo.check (dto);
endtask: test_pass


task test_trigger (
  int unsigned vld_max = 0,
  int unsigned vld_rnd = 1,
  int unsigned vld_fix = 0,
  int unsigned rdy_max = 0,
  int unsigned rdy_rnd = 1,
  int unsigned rdy_fix = 0
);
  DT dti [];
  DT dto [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) clo;
  // prepare data
  cli = new;
  clo = new;
  dti = cli.range (-8, 8);
  dto = clo.range (-8, 8);
  // add packet into queue
  cli.add_pkt (dti, .vld_max (vld_max),
                    .vld_rnd (vld_rnd),
                    .vld_fix (vld_fix),
                    .rdy_max (rdy_max),
                    .rdy_rnd (rdy_rnd),
                    .rdy_fix (rdy_fix) );
  clo.add_pkt (dto);
  // activate acquire
  acq_pls;
  fork
    wait ((sti.TDATA == 0) & sti.transf) begin
      trg_pls ('1);
    end
    str_src.run (cli);
    str_drn.run (clo);
  join
  // check received data
  error += clo.check (dto);
endtask: test_trigger


task test_stop (
  int unsigned vld_max = 0,
  int unsigned vld_rnd = 1,
  int unsigned vld_fix = 0,
  int unsigned rdy_max = 0,
  int unsigned rdy_rnd = 1,
  int unsigned rdy_fix = 0
);
  DT dti [];
  DT dto [];
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) cli;
  axi4_stream_pkg::axi4_stream_class #(.DT (DT)) clo;
  // prepare data
  cli = new;
  clo = new;
  dti = cli.range (0, 15);
  dto = clo.range (0, 6);
  // add packet into queue
  cli.add_pkt (dti, .vld_max (vld_max),
                    .vld_rnd (vld_rnd),
                    .vld_fix (vld_fix),
                    .rdy_max (rdy_max),
                    .rdy_rnd (rdy_rnd),
                    .rdy_fix (rdy_fix) );
  clo.add_pkt (dto);
  // activate acquire
  acq_pls;
  fork
    str_src.run (cli);
    str_drn.run (clo);
    wait (cb.sts_pre == 4) begin
      stp_pls;
    end
  join
  // check received data
  error += clo.check (dto);
endtask: test_stop

////////////////////////////////////////////////////////////////////////////////
// helper tasks
////////////////////////////////////////////////////////////////////////////////

// generate reset pulse
task rst_pls ();
  cb.ctl_rst <= 1'b1;
  ##1;
  cb.ctl_rst <= 1'b0;
endtask: rst_pls

// activate acquire
task acq_pls ();
  cb.ctl_acq <= 1'b1;
  ##1;
  cb.ctl_acq <= 1'b0;
endtask: acq_pls

// stop acquire
task stp_pls ();
  cb.ctl_stp <= 1'b1;
  ##1;
  cb.ctl_stp <= 1'b0;
endtask: stp_pls

// generate trigger pulse
task trg_pls (logic [TN-1:0] trg);
  cb.ctl_trg <= trg;
  ##1;
  cb.ctl_trg <= '0;
endtask: trg_pls

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

axi4_stream_src #(.DN (DN), .DT (DT)) str_src (.str (sti));

acq #(
  .DN (DN),
  .TN (TN),
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
