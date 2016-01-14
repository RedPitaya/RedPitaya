////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module str_dec_tb #(
  // clock time periods
  realtime  TP = 4.0ns,  // 250MHz
  // stream parameters
  type DAT_T = logic [8-1:0], // data type for input
  int unsigned CW = 17  // counter width
);

// system signals
logic clk ;  // clock
logic rstn;  // reset - active low

// control
logic          ctl_rst;  // synchronous reset
// configuration
logic [CW-1:0] cfg_dec;  // decimation factor

// stream input/output
str_bus_if #(.DAT_T (DAT_T)) sti (.clk (clk), .rstn (rstn));
str_bus_if #(.DAT_T (DAT_T)) sto (.clk (clk), .rstn (rstn));

////////////////////////////////////////////////////////////////////////////////
// clock and test sequence
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'h0;
always #(TP/2) clk = ~clk;

initial begin
  // for now initialize configuration to an idle value
  ctl_rst = 1'b0;
  cfg_dec = 0;

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

str_src #(.DAT_T (DAT_T)) str_src (.str (sti));

str_dec #(
  .CW (CW)
) str_dec (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_dec  (cfg_dec),
  // stream input/output
  .sti      (sti    ),
  .sto      (sto    )
);

str_drn #(.DAT_T (DAT_T)) str_drn (.str (sto));

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("str_dec_tb.vcd");
  $dumpvars(0, str_dec_tb);
end

endmodule: str_dec_tb
