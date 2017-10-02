////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya trigger counter testbench.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module ctrg_tb #(
  // time period
  realtime  TP = 8.0ns,  // 125MHz
  // types
  type U16 = logic [16-1:0],
  type S16 = logic signed [16-1:0],
  type S14 = logic signed [14-1:0],
  // data parameters
  type DT = S14,
  type DTM = S16,
  type DTS = S14,
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16   // counter width fraction  (fixed point fraction)
);

////////////////////////////////////////////////////////////////////////////////
// signal generation
////////////////////////////////////////////////////////////////////////////////

// system signals
logic clk ;
logic rstn;

// interrupt
logic irq;

// events input/output
evn_pkg::evn_t evn;
// triggers input/output
logic trg;

////////////////////////////////////////////////////////////////////////////////
// clock
////////////////////////////////////////////////////////////////////////////////

initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

// clocking 
default clocking cb @ (posedge clk);
  input  rstn;
  input  trg;
endclocking: cb

// DAC reset
initial begin
  rstn <= 1'b0;
  ##4;
  rstn <= 1'b1;
end

// ADC cycle counter
int unsigned dac_cyc=0;
always_ff @ (posedge clk)
dac_cyc <= dac_cyc+1;

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

localparam int CTL_RST = 4'b0001;
localparam int CTL_STR = 4'b0010;
localparam int CTL_STP = 4'b0100;
localparam int CTL_TRG = 4'b1000;

initial begin
  trg <= 0;
  ##10;
  // events
  busm.write('h04, 0);      // software event select
  busm.write('h08, 2'b01);  // hardware trigger mask
  // start/trigger
  busm.write('h00, CTL_STR);
  busm.write('h00, CTL_TRG);
  ##8;
  for (int unsigned i=0; i<8; i++) begin
    trg <= 1'b1;
    ##1;
    trg <= 1'b0;
    ##4;
  end
  ##8;
  // stop
  busm.write('h00, CTL_STP);
  ##20;
  // reset
  busm.write('h00, CTL_RST);
  ##20;

  // end simulation
  ##20;
  $stop();
  //$finish();
end

////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

sys_bus_if bus (.clk (clk), .rstn (rstn));

sys_bus_model busm (.bus (bus));

ctrg #(
  .EN  (1),
  .TN  ($bits(trg))
) ctrg (
  // events input/output
  .evi      (evn),
  .evo      (evn),
  // triggers input/output
  .trg      (trg),
  .tro      (),
  // system bus
  .bus      (bus)
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("ctrg_tb.vcd");
  $dumpvars(0, ctrg_tb);
end

endmodule: ctrg_tb
