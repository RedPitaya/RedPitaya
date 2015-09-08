////////////////////////////////////////////////////////////////////////////////
// Module: PDM (pulse density modulation)
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module pdm #(
  int unsigned  CCW = 8  // configuration counter width (resolution)
)(
  // system signals
  input  logic           clk ,  // clock
  input  logic           rstn,  // reset (active low)
  // configuration
  input  logic           ena,   // enable
  input  logic [CCW-1:0] rng,   // range
  // input stream
  input  logic [CCW-1:0] str_dat,  // data
  input  logic           str_vld,  // valid
  output logic           str_rdy,  // ready
  // PDM output
  output logic           pdm
);

// local signals
logic [CCW-1:0] cnt;  // counter current value
logic [CCW-1:0] nxt;  // counter next value
logic [CCW-1:0] acu;  // accumulator
logic [CCW  :0] sum;  // summation
logic [CCW  :0] sub;  // subtraction

// counter current value
always_ff @(posedge clk)
if (~rstn)  cnt <= '0;
else begin
  if (ena)  cnt <= str_rdy ? '0 : nxt;
  else      cnt <= '0;
end

// counter next value
assign nxt = cnt + 'd1;

// counter cycle end
assign str_rdy = nxt == rng;

// counter
always_ff @(posedge clk)
if (~rstn)  acu <= '0;
else begin
  if (ena)  acu <= ~sub[CCW] ? sub[CCW-1:0] : sum[CCW-1:0];
  else      acu <= '0;
end

// summation
assign sum = acu + str_dat;

// subtraction
assign sub = sum - rng;

// PDM output
assign pdm = ena & (~sub[CCW] | ~|sub[CCW-1:0]);

endmodule: pdm
