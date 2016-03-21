////////////////////////////////////////////////////////////////////////////////
// Module: Acquire
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module asg_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // trigger
  int unsigned TN = 1,
  // data parameters
  int unsigned DWO = 14,  // RAM data width
  int unsigned DWM = 16,  // data width for multiplier (gain)
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16,  // counter width fraction  (fixed point fraction)
  // burst counter parameters
  int unsigned CWL = 32,  // counter width length
  int unsigned CWN = 16   // counter width number
);

localparam int unsigned DN = 1;
localparam type DT = logic signed [DWO-1:0];

// system signals
logic          clk ;  // clock
logic          rstn;  // reset - active low

// control
logic               ctl_rst;  // set FSM to reset
// trigger
logic      [TN-1:0] trg_i  ;  // input
logic               trg_o  ;  // output event
// interrupts
logic               irq_trg;  // trigger
logic               irq_stp;  // stop
// configuration
logic      [TN-1:0] cfg_trg;  // trigger mask
logic [CWM+CWF-1:0] cfg_siz;  // data tablesize
logic [CWM+CWF-1:0] cfg_stp;  // pointer step    size
logic [CWM+CWF-1:0] cfg_off;  // pointer initial offset (used to define phase)
// configuration (burst mode)
logic               cfg_ben;  // burst enable
logic               cfg_inf;  // infinite
logic     [CWM-1:0] cfg_bdl;  // data length
logic     [CWL-1:0] cfg_bln;  // period length (data+idle)
logic     [CWN-1:0] cfg_bnm;  // number of repetitions
// status
logic     [CWL-1:0] sts_bln;  // burst length counter
logic     [CWN-1:0] sts_bnm;  // burst number counter
logic               sts_run;  // running status

DT dat_table [];

// stream input/output
axi4_stream_if #(.DT (DT)) str (.ACLK (clk), .ARESETn (rstn));

// error counter
int unsigned error;

////////////////////////////////////////////////////////////////////////////////
// clock and time stamp
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  // for now initialize configuration to an idle value
  ctl_rst = 1'b0;
  // control
  trg_i   = '0;
  // configuration
  cfg_trg = '1;
  cfg_siz = (1<<CWM+CWF)-1;
  cfg_stp = (1<<CWF    )-1;
  cfg_off = (0<<CWF    );
  // configuration (burst mode)
  cfg_ben = 1'b0;
  cfg_inf = 0;
  cfg_bdl = 0;
  cfg_bln = 0;
  cfg_bnm = 0;

  // reset sequence
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
  repeat(4) @(posedge clk);

  // writing to buffer
  dat_table = new [1<<CWM];
  for (int i=0; i<dat_table.size(); i++) begin
    dat_table[i] = i;
  end
  buf_write(dat_table);

  // running a set of tests
  test_burst_num (.bdl (1), .bln (1), .dat_table (dat_table));
  test_burst_num (.bdl (1), .bln (8), .dat_table (dat_table));
  test_burst_num (.bdl (7), .bln (8), .dat_table (dat_table));
  test_burst_num (.bdl (8), .bln (8), .dat_table (dat_table));
  test_burst_inf (.bdl (1), .bln (1), .dat_table (dat_table));
  test_burst_inf (.bdl (1), .bln (8), .dat_table (dat_table));
  test_burst_inf (.bdl (7), .bln (8), .dat_table (dat_table));
  test_burst_inf (.bdl (8), .bln (8), .dat_table (dat_table));

  // status report
  if (error)  $display ("FAILURE");
  else        $display ("SUCCESS");

  // end simulation
  repeat(4) @(posedge clk);
  $finish();
end

// write buffer
task automatic buf_write (
  ref DT dat []
);
  for (int i=0; i<dat.size(); i++) begin
    busm.write(i, dat[i]);  // write table
  end
endtask: buf_write

////////////////////////////////////////////////////////////////////////////////
// helper tasks
////////////////////////////////////////////////////////////////////////////////

// generate trigger pulse
task rst_pls ();
  ctl_rst = 1'b1;
  @(posedge clk);
  ctl_rst = 1'b0;
endtask: rst_pls

// generate trigger pulse
task trg_pls (logic [TN-1:0] trg);
  trg_i = trg;
  @(posedge clk);
  trg_i = '0;
endtask: trg_pls

////////////////////////////////////////////////////////////////////////////////
// tests
////////////////////////////////////////////////////////////////////////////////

// test bursts with a specified length
task automatic test_burst_num (
  int unsigned bdl = 8,
  int unsigned bln = 8,
  ref DT dat_table []
);
  $display ("Note: burst number test");
  cfg_ben = 1'b1;  // enable burst mode
  cfg_inf = 1'b0;  // disable infinite bursts
  cfg_bdl = bdl-1;
  cfg_bln = bln-1;
  for (int unsigned bnm=1; bnm<=4; bnm++) begin
    int unsigned len;
    str_drn.clr();
    cfg_bnm = bnm-1;
    len = bln*bnm;
    $display ("Note: burst number = %d", bnm);
    trg_pls (1'b1);
    repeat (len+8) @(posedge clk);
    // check stream length
    if (str_drn.buf_siz != len) begin
      error++;
      $display ("Error: data stream size %d is not correct, should be %d", str_drn.buf_siz, len);
    end
    // check stream data
    for (int unsigned i=0; i<str_drn.buf_siz; i++) begin
      int unsigned idx;
      DT    [DN-1:0] dat, ref_dat;
      logic [DN-1:0] kep, ref_kep;
      logic          lst, ref_lst;
      int unsigned   tmg;
      idx = (i%bln) > cfg_bdl ? cfg_bdl : (i%bln);
      ref_dat = dat_table [idx];
      ref_kep = '1; // TODO
      ref_lst = i==(str_drn.buf_siz-1);
      str_drn.get (dat, kep, lst, tmg);
      if (dat != ref_dat) begin  error++;  $display ("Error: data i=%d missmatch drn=%04h, ref=%04h", i, dat, ref_dat);  end
      if (kep != ref_kep) begin  error++;  $display ("Error: keep i=%d missmatch drn=%04h, ref=%04h", i, kep, ref_kep);  end
      if (lst != ref_lst) begin  error++;  $display ("Error: last i=%d missmatch drn=%04h, ref=%04h", i, lst, ref_lst);  end
    end
  end
endtask: test_burst_num

// test burst of infinite length
task automatic test_burst_inf (
  int unsigned bdl = 8,
  int unsigned bln = 8,
  ref DT dat_table []
);
  $display ("Note: infinite burst test");
  cfg_ben = 1'b1;  // enable burst mode
  cfg_inf = 1'b1;  // disable infinite bursts
  cfg_bdl = bdl-1;
  cfg_bln = bln-1;
  for (int unsigned bnm=1; bnm<=4; bnm++) begin
    int unsigned len;
    str_drn.clr();
    cfg_bnm = bnm-1;
    len = bln*bnm;
    $display ("Note: burst length = %d", bnm);
    trg_pls (1'b1);
    repeat (len+32) @(posedge clk);
    rst_pls ();
    repeat (4) @(posedge clk);
    // check stream length
    if (str_drn.buf_siz <= len) begin
      error++;
      $display ("Error: data stream size is too short");
    end
    // check stream data
    for (int unsigned i=0; i<str_drn.buf_siz; i++) begin
      int unsigned idx;
      DT    [DN-1:0] dat, ref_dat;
      logic [DN-1:0] kep, ref_kep;
      logic          lst, ref_lst;
      int unsigned   tmg;
      idx = (i%bln) > cfg_bdl ? cfg_bdl : (i%bln);
      ref_dat = dat_table [idx];
      ref_kep = '1;
      ref_lst = 1'b0;
      str_drn.get (dat, kep, lst, tmg);
      if (dat != ref_dat) begin  error++;  $display ("Error: data i=%d missmatch drn=%04h, ref=%04h", i, dat, ref_dat);  end
      if (kep != ref_kep) begin  error++;  $display ("Error: keep i=%d missmatch drn=%04h, ref=%04h", i, kep, ref_kep);  end
      if (lst != ref_lst) begin  error++;  $display ("Error: last i=%d missmatch drn=%b, ref=%b"    , i, lst, ref_lst);  end
    end
  end
endtask: test_burst_inf

////////////////////////////////////////////////////////////////////////////////
// module instance
////////////////////////////////////////////////////////////////////////////////

sys_bus_if bus (.clk (clk), .rstn (rstn));

sys_bus_model busm (.bus (bus));

asg #(
  .DN (DN),
  .DT (DT),
  // buffer parameters
  .CWM (CWM),
  .CWF (CWF),
  // burst counters
  .CWL (CWL),
  .CWN (CWN)
) asg (
  // stream output
  .sto      (str),
  // control
  .ctl_rst  (ctl_rst),
  // trigger
  .trg_i    (trg_i),
  .trg_o    (trg_o),
  // interrupts
  .irq_trg  (irq_trg),
  .irq_stp  (irq_stp),
  // configuration
  .cfg_trg  (cfg_trg),
  .cfg_siz  (cfg_siz),
  .cfg_stp  (cfg_stp),
  .cfg_off  (cfg_off),
  // configuration (burst mode)
  .cfg_ben  (cfg_ben),
  .cfg_inf  (cfg_inf),
  .cfg_bdl  (cfg_bdl),
  .cfg_bln  (cfg_bln),
  .cfg_bnm  (cfg_bnm),
  // status
  .sts_bln  (sts_bln),
  .sts_bnm  (sts_bnm),
  .sts_run  (sts_run),
  // CPU buffer access
  .bus      (bus)
);

axi4_stream_drn #(
  .DN (DN),
  .DT (DT)
) str_drn (
  .str      (str)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("asg_tb.vcd");
  $dumpvars(0, asg_tb);
end

endmodule: asg_tb
