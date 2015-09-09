////////////////////////////////////////////////////////////////////////////////
// Module: PDM (pulse density modulation)
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module pdm #(
  int unsigned CW = 8,  // counter width (resolution)
  int unsigned OW = 1   // output  width
)(
  // system signals
  input  logic                  clk ,  // clock
  input  logic                  rstn,  // reset (active low)
  // configuration
  input  logic                  ena,   // enable
  input  logic         [CW-1:0] rng,   // range
  // input stream
  input  logic [OW-1:0][CW-1:0] str_dat,  // data
  input  logic                  str_vld,  // valid
  output logic                  str_rdy,  // ready
  // PDM output
  output logic [OW-1:0]         pdm
);

// local signals
logic         [CW-1:0] cnt;  // counter current value
logic         [CW-1:0] nxt;  // counter next value
logic [OW-1:0][CW-1:0] acu;  // accumulator
logic [OW-1:0][CW  :0] sum;  // summation
logic [OW-1:0][CW  :0] sub;  // subtraction

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

generate
for (genvar i=0; i<OW; i++) begin: out

// accumulator
always_ff @(posedge clk)
if (~rstn)  acu[i] <= '0;
else begin
  if (ena)  acu[i] <= ~sub[i][CW] ? sub[i][CW-1:0] : sum[i][CW-1:0];
  else      acu[i] <= '0;
end

// summation
assign sum[i] = acu[i] + str_dat[i];

// subtraction
assign sub[i] = sum[i] - rng;

// PDM output
assign pdm[i] = ena & (~sub[i][CW] | ~|sub[i][CW-1:0]);

end: out
endgenerate

endmodule: pdm
