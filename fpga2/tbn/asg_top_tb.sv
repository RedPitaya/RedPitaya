////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya arbitrary signal generator testbench.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module asg_top_tb #(
  // time period
  realtime  TP = 8.0ns,  // 125MHz
  // functionality enable
  bit EN_LIN = 0,
  // data parameters
  int unsigned DWO = 14,  // RAM data width
  int unsigned DWM = 16,  // data width for multiplier (gain)
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16   // counter width fraction  (fixed point fraction)
);

localparam type DAT_T = logic signed [DWO-1:0];

////////////////////////////////////////////////////////////////////////////////
// DAC signal generation
////////////////////////////////////////////////////////////////////////////////

// syste signals
logic                  clk ;
logic                  rstn;

// stream
str_bus_if #(.DAT_T (DAT_T)) str (.clk (clk), .rstn (rstn));

// trigger
struct packed {
  logic ext;  // external
  logic out;  // output from generator
  logic swo;  // software
} trg;

// trigger parameters
localparam int unsigned TN = $bits(trg);   // trigger array  width

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

localparam int unsigned ADR_BUF = 1 << (CWM+2);

//int buf_len = 2**CWM;
int buf_len = 8;
real freq  = 10_000; // 10kHz
real phase = 0; // DEG

initial begin
  repeat(10) @(posedge clk);
  // write table
  for (int i=0; i<buf_len; i++) begin
    busm.write(ADR_BUF + (i*4), i);  // write table
  end
  // read table
  rdata_blk = new [80];
  for (int i=0; i<buf_len; i++) begin
    busm.read(ADR_BUF + (i*4), rdata_blk [i]);  // read table
  end
  // configure frequency and phase
  busm.write('h10,  buf_len                    * 2**CWF - 1);  // table size
  busm.write('h14, (buf_len * (phase/360.0)  ) * 2**CWF    );  // offset
//busm.write('h18, (buf_len * (freq*TP/10**6)) * 2**CWF - 1);  // step
  busm.write('h18, 1                           * 2**CWF - 1);  // step
  // configure burst mode
  busm.write('h20, 2'b00);  // burst disable
  // configure amplitude and DC offset
  busm.write('h30, 1 << (DWM-2));  // amplitude
  busm.write('h34, 0);             // DC offset
  // enable SW trigger
  busm.write('h04, 3'b001);
  // start
  busm.write('h00, 2'b10);
  repeat(22) @(posedge clk);

  // stop (reset)
  busm.write('h00, 2'b01);
  repeat(20) @(posedge clk);

  // configure frequency and phase
  busm.write('h14, 0 * 2**CWF    );  // offset
  busm.write('h18, 1 * 2**CWF - 1);  // step
  // configure burst mode
  busm.write('h20, {1'b1, TN'(0)});  // burst enable
  busm.write('h24,  6);  // burst data length
  busm.write('h28, 10);  // burst idle length
  busm.write('h2c,  5);  // burst number of repetitions
  // configure amplitude and DC offset
  busm.write('h30, 1 << (DWM-2));  // amplitude
  busm.write('h34, 0);             // DC offset
  // start
  busm.write('h00, 2'b10);
  repeat(120) @(posedge clk);

  // stop (reset)
  busm.write('h00, 2'b01);
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
  .EN_LIN (0),
  .TN (TN)
) asg_top (
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
