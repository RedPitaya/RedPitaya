////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module la_trigger_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // PWM parameters
  type DAT_T = logic [8-1:0]  // data type
);

// system signals
logic clk ;  // clock
logic rstn;  // reset - active low

// configuration
DAT_T cfg_old_val;  // old     value
DAT_T cfg_old_msk;  // old     mask 
DAT_T cfg_cur_val;  // current value
DAT_T cfg_cur_msk;  // current mask 

// stream input/output
str_bus_if #(.DAT_T (DAT_T)) str (.clk (clk), .rstn (rstn));

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  // for now initialize configuration to an idle value
  cfg_old_val = '0;
  cfg_old_msk = '0;
  cfg_cur_val = 'h0a;
  cfg_cur_msk = 'hff;

  // initialization
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  repeat(4) @(posedge clk);

  // send data into stream
  for (int unsigned i=0; i<16; i++) begin
    str_src.put(i, 1'b0);
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

str_src #(.DAT_T (DAT_T)) str_src (.str (str));
str_drn #(.DAT_T (DAT_T)) str_drn (.str (str));

la_trigger #(
  .DAT_T (DAT_T)
) la_trigger (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_old_val (cfg_old_val),
  .cfg_old_msk (cfg_old_msk),
  .cfg_cur_val (cfg_cur_val),
  .cfg_cur_msk (cfg_cur_msk),
  // output triggers
  .sts_trg  (trg_out),
  // stream monitor
  .str      (str)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("la_trigger_tb.vcd");
  $dumpvars(0, la_trigger_tb);
end

endmodule: la_trigger_tb
