////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya arbitrary signal generator testbench.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module asg_top_tb #(
  // time period
  realtime  TP = 8.0ns,  // 125MHz
  // data parameters
  int unsigned DWO = 14,  // RAM data width
  int unsigned DWM = 16,  // data width for multiplier (gain)
  int unsigned DWS = DWO, // data width for summation (offset)
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16   // counter width fraction  (fixed point fraction)
);

////////////////////////////////////////////////////////////////////////////////
// DAC signal generation
////////////////////////////////////////////////////////////////////////////////

// syste signals
logic                  clk ;
logic                  rstn;
// stream
str_bus_if #(.DAT_T (logic signed [DWO-1:0])) str (.clk (clk), .rstn (rstn));

// trigger
struct packed {
  logic ext;  // external
  logic out;  // output from generator
  logic swo;  // software
} trg;

// trigger parameters
localparam int unsigned TWA = $bits(trg);   // trigger array  width
localparam int unsigned TWS = $clog2(TWA);  // trigger select width

// DAC clock
initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

// DAC reset
initial begin
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
end

// ADC cycle counter
int unsigned dac_cyc=0;
always_ff @ (posedge clk)
dac_cyc <= dac_cyc+1;

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

logic        [ 32-1: 0] rdata;
logic signed [ 32-1: 0] rdata_blk [];

////////////////////////////////////////////////////////////////////////////////
// signal generation
////////////////////////////////////////////////////////////////////////////////

localparam ADR_BUF = 1 << (CWM+2);

//int buf_len = 2**CWM;
int buf_len = 8;
real freq  = 10_000; // 10kHz
real phase = 0; // DEG

initial begin
  repeat(10) @(posedge clk);
  // write table and table size
  for (int i=0; i<buf_len; i++) begin
    busm.write(ADR_BUF + (i*4), i);  // write table
  end
  // CH1 table data readback
  rdata_blk = new [80];
  for (int i=0; i<buf_len; i++) begin
    busm.read(ADR_BUF + (i*4), rdata_blk [i]);  // read table
  end
  // configure frequency and phase
  busm.write(32'h08,  buf_len                    * 2**CWF - 1);  // table size
  busm.write(32'h0C, (buf_len * (phase/360.0)  ) * 2**CWF    );  // offset
//busm.write(32'h10, (buf_len * (freq*TP/10**6)) * 2**CWF    );  // step
  busm.write(32'h10, 1                           * 2**CWF    );  // step
  // configure burst mode
  busm.write(32'h04, {1'b0, TWS'(0)});  // burst disable
  // configure amplitude and DC offset
  busm.write(32'h24, 1 << (DWM-2));  // amplitude
  busm.write(32'h28, 0);             // DC offset
  // start
  busm.write(32'h00, 2'b10);
  repeat(22) @(posedge clk);

  // stop (reset)
  busm.write(32'h00, 2'b01);
  repeat(20) @(posedge clk);

  // configure frequency and phase
  busm.write(32'h0c, 0 * 2**CWF);  // offset
  busm.write(32'h10, 1 * 2**CWF);  // step
  // configure burst mode
  busm.write(32'h04, {1'b1, TWS'(0)});  // burst enable
  busm.write(32'h18, 6);  // number of cycles
  busm.write(32'h1c, 10);  // number of delay periods between repetitions
  busm.write(32'h20, 5);  // number of repetitions
  // configure amplitude and DC offset
  busm.write(32'h24, 1 << (DWM-2));  // amplitude
  busm.write(32'h28, 0);             // DC offset
  // start
  busm.write(32'h00, 2'b10);
  repeat(120) @(posedge clk);

  // stop (reset)
  busm.write(32'h00, 2'b01);
  repeat(20) @(posedge clk);

  // end simulation
  repeat(20) @(posedge clk);
  $stop();
  //$finish();
end

////////////////////////////////////////////////////////////////////////////////
// triggers
////////////////////////////////////////////////////////////////////////////////

assign trg.ext = 1'b0;

////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

sys_bus_if    bus  (.clk (clk), .rstn (rstn));

sys_bus_model busm (.bus (bus));

asg_top #(
  .TWA (TWA)
) asg_top (
  // system signals
  .clk       (clk ),
  .rstn      (rstn),
  // stream output
  .sto       (str),
  // triggers
  .trg_ext   (trg),
  .trg_swo   (trg.swo),
  .trg_out   (trg.out),
  // System bus
  .bus       (bus)
);

// stream drain
assign str.rdy = 1'b1;

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("asg_top_tb.vcd");
  $dumpvars(0, asg_top_tb);
end

endmodule: asg_top_tb
