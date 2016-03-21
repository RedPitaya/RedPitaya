////////////////////////////////////////////////////////////////////////////////
// Module: PDM (pulse density modulation)
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module pdm #(
  int unsigned DWC = 8,  // counter width (resolution)
  int unsigned CHN = 1   // output  width
)(
  // system signals
  input  logic                    clk ,  // clock
  input  logic                    rstn,  // reset (active low)
  input  logic                    cke ,  // clock enable (synchronous)
  // configuration
  input  logic                    ena,   // enable
  input  logic          [DWC-1:0] rng,   // range
  // stream input
  input  logic [CHN-1:0][DWC-1:0] str_dat,  // data
  input  logic                    str_vld,  // valid (it is ignored for now
  output logic                    str_rdy,  // ready
  // PDM output
  output logic [CHN-1:0]          pdm
);

// local signals
logic [DWC-1:0] cnt;  // counter current value
logic [DWC-1:0] nxt;  // counter next value

// counter current value
always_ff @(posedge clk)
if (~rstn)  cnt <= '0;
else begin
  if (ena)  cnt <= str_rdy ? '0 : nxt;
  else      cnt <= '0;
end

// counter next value
assign nxt = cnt + cke;

// counter cycle end
assign str_rdy = nxt == rng;

generate
for (genvar i=0; i<CHN; i++) begin: for_chn

logic [DWC-1:0] dat;

// stream input data copy
always_ff @(posedge clk)
if (~rstn)            dat <= '0;
else begin
  if (ena & str_rdy)  dat <= str_dat[i];
end

logic [DWC-1:0] acu;  // accumulator
logic [DWC  :0] sum;  // summation
logic [DWC  :0] sub;  // subtraction

// accumulator
always_ff @(posedge clk)
if (~rstn)  acu <= '0;
else begin
  if (ena)  acu <= ~sub[DWC] ? sub[DWC-1:0] : sum[DWC-1:0];
  else      acu <= '0;
end

// summation
assign sum = acu + dat;

// subtraction
assign sub = sum - rng;

// PDM output
always_ff @(posedge clk)
if (~rstn)  pdm[i] <= 1'b0;
else        pdm[i] <= ena & (~sub[DWC] | ~|sub[DWC-1:0]);

end: for_chn
endgenerate

endmodule: pdm
