/**
 * @brief Red Pitaya PWM module
 *
 * @Author Matej Oblak
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */

module red_pitaya_pdm #(
  int unsigned DWC = 8,  // counter width (resolution)
  int unsigned CHN = 4   // output  width
)(
  // system signals
  input  logic                      clk ,  // clock
  input  logic                      rstn,  // reset
  // configuration
  input  logic [CHN-1:0] [DWC-1:0]  cfg ,  // 
  input  logic                      ena ,
  input  logic           [DWC-1:0]  rng ,
  // PWM outputs
  output logic [CHN-1:0]            pdm    // PWM output - driving RC
);
generate
for (genvar i=0; i<CHN; i++) begin: for_chn

logic [DWC-1:0] dat;

// input data copy
always_ff @(posedge clk)
if (~rstn)            dat <= '0;
else begin
  if (ena)  dat <= cfg[i];
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
endmodule: red_pitaya_pdm
