////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module scope_filter_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // PWM parameters
  int unsigned DWI = 14,   // data width for input
  int unsigned DWO = 14,   // data width for output
  int unsigned DWC = DWO   // data width for coeficients
);

// system signals
logic                  clk ;  // clock
logic                  rstn;  // reset - active low

// configuration
logic signed [ 18-1:0] cfg_aa;   // config AA coefficient
logic signed [ 25-1:0] cfg_bb;   // config BB coefficient
logic signed [ 25-1:0] cfg_pp;   // config PP coefficient
logic signed [ 25-1:0] cfg_kk;   // config KK coefficient

// stream input/output
axi4_stream_if #(.DT (logic signed [DWI-1:0])) sti (.ACLK (clk), .ARESETn (rstn));
axi4_stream_if #(.DT (logic signed [DWO-1:0])) sto (.ACLK (clk), .ARESETn (rstn));

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  // for now initialize configuration to an idle value
  cfg_aa = 0;
  cfg_bb = 0;
  cfg_pp = 0;
  cfg_kk = 0;
  cfg_aa = 'h7D93;
  cfg_bb = 'h437C7;
  cfg_pp = 'h2666;
  cfg_kk = 'hd9999a;

  // initialization
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  // start
  rstn = 1'b1;
  repeat(4) @(posedge clk);

  // send data into stream
  for (int i=-8; i<8; i++) begin
    str_src.put(i);
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

axi4_stream_src #(
  .DW  (DWI)
) str_src (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // stream
  .str      (sti)
);

scope_filter #(
  .DWI (DWI),
  .DWO (DWO)
) filter (
  // stream input/output
  .sti      (sti),
  .sto      (sto),
  // configuration
  .cfg_aa   (cfg_aa),
  .cfg_bb   (cfg_bb),
  .cfg_kk   (cfg_kk),
  .cfg_pp   (cfg_pp),
  // control
  .ctl_rst  ()
);

red_pitaya_dfilt1 dfilt1 (
   // ADC
   .adc_clk_i  (clk ),
   .adc_rstn_i (rstn),
   .adc_dat_i  (sti.dat),
   .adc_dat_o  (),
   // configuration
   .cfg_aa_i   (cfg_aa),
   .cfg_bb_i   (cfg_bb),
   .cfg_kk_i   (cfg_kk),
   .cfg_pp_i   (cfg_pp)
);

axi4_stream_drn #(
  .DW  (DWI)
) str_drn (
  // system signals
  .clk      (clk ),
  .rstn     (rstn),
  // stream
  .str      (sto)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("scope_filter_tb.vcd");
  $dumpvars(0, scope_filter_tb);
end

endmodule: scope_filter_tb
