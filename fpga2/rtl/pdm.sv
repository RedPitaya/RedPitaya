////////////////////////////////////////////////////////////////////////////////
// Module: PDM (pulse density modulation)
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module pdm #(
  int unsigned  CCW = 8,  // configuration counter width (resolution)
  bit [CCW-1:0] CCE = '1  // 100% value
)(
  // system signals
  input  logic           clk ,  // clock
  input  logic           rstn,  // reset (active low)
  // configuration
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
else        cnt <= str_rdy ? '0 : nxt;

// counter next value
assign nxt = cnt + 'd1;

// counter cycle end
assign str_rdy = nxt == CCE;

// counter
always_ff @(posedge clk)
if (~rstn)  acu <= '0;
else        acu <= ~sub[CCW] ? sub[CCW-1:0] : sum[CCW-1:0];

// summation
assign sum = acu + str_dat;

// subtraction
assign sub = sum - CCE;

// PDM output
assign pdm = ~sub[CCW] | ~|sub[CCW-1:0];

endmodule: pdm
