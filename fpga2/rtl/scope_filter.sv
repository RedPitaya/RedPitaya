////////////////////////////////////////////////////////////////////////////////
// Red Pitaya equalization filter
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// GENERAL DESCRIPTION:
// Filter to equalize input analog chain. 
//
//  s14  -----  s23  -------  s15  -------  s15  -------  s40  -----  s14
// ---->| FIR |---->| IIR 1 |---->| IIR 2 |---->| SCALE |---->| SAT |---->
//       -----       -------       -------       -------       -----
//         ^            ^             ^             ^
//         | s25        | s18         | s25         | s25
//         |            |             |             |
//         BB           AA            PP            KK
//
////////////////////////////////////////////////////////////////////////////////

module scope_filter #(
  int unsigned DWI = 14,
  int unsigned DWO = 14
)(
  // system signals
  input  logic                  clk ,     // clock
  input  logic                  rstn,     // reset - active low
  // input stream
  input  logic signed [DWI-1:0] sti_dat,  // data
  input  logic                  sti_vld,  // valid
  output logic                  sti_rdy,  // ready
  // output stream
  output logic signed [DWO-1:0] sto_dat,  // ADC data
  output logic                  sto_vld,  // valid
  input  logic                  sto_rdy,  // ready
  // configuration
  input  logic signed [ 18-1:0] cfg_aa,   // config AA coefficient
  input  logic signed [ 25-1:0] cfg_bb,   // config BB coefficient
  input  logic signed [ 25-1:0] cfg_kk,   // config KK coefficient
  input  logic signed [ 25-1:0] cfg_pp,   // config PP coefficient
  // control
  input  logic                  ctl_rst   // synchronous reset
);

////////////////////////////////////////////////////////////////////////////////
//
// FIR
//
// Time domain:
//
// y[n] = ( (     2**18) * x[n-0] +
//          (bb - 2**18) * x[n-1] ) / 2**10
//
// Frequency domain:
//
// y[n] = ( (     2**18) *        ) +
//          (bb - 2**18) * z^(-1) ) / 2**10
//
////////////////////////////////////////////////////////////////////////////////

// FIR filter
logic signed [2-1:0] [   DWI-1:0] fir_buf;  // data buffer
logic signed [2-1:0] [25    -1:0] fir_cfg;  // coeficients
logic signed [2-1:0] [25+DWI-1:0] fir_mul;  // multiplications
logic signed         [23    -1:0] fir_sum;  // summation

// FIR data buffer
assign fir_buf [0] = sti_dat;

always_ff @(posedge clk)
fir_buf [1] <= fir_buf [0];

// FIR coeficients
assign fir_cfg [0] =          (1 <<< 18);
assign fir_cfg [1] = cfg_bb - (1 <<< 18);

// multiplications
generate
for (genvar i=0; i<2; i++) begin: for_fir_mul
  always_ff @(posedge clk)
  fir_mul[i] <= fir_buf[i] * fir_cfg[i];
end: for_fir_mul
endgenerate

// final summation
always_ff @(posedge clk)
fir_sum <= (fir_mul[1] + fir_mul[1]) >>> 10;

////////////////////////////////////////////////////////////////////////////////
//
// IIR 1
//
// Time domain:
//
// y[n] = ( (2**25     ) * x[n-0] +
//          (2**25 - aa) * y[n-1] ) / 2**25
//
// Frequency domain:
//
// TODO
//
////////////////////////////////////////////////////////////////////////////////

(* use_dsp48="yes" *) logic signed [23-1:0] iir1_dat;

always_ff @(posedge clk)
if (~rstn)  iir1_dat <= '0;
else        iir1_dat <= ((fir_sum <<< 25) + (iir1_dat * ((1 <<< 25) - cfg_aa))) >>> 25;

////////////////////////////////////////////////////////////////////////////////
//
// IIR 2
//
// Time domain:
//
// y[n] = ( (2**25     ) * x[n-0] +
//          (2**25 - aa) * y[n-1] ) / 2**25
//
// Frequency domain:
//
// TODO
//
////////////////////////////////////////////////////////////////////////////////

logic signed [15-1:0] iir2_dat;

always_ff @(posedge clk)
if (~rstn)  iir2_dat <= '0;
else        iir2_dat <= (iir1_dat >>> 8) + ((iir2_dat * cfg_pp) >>> 16);

////////////////////////////////////////////////////////////////////////////////
// Scaling and saturation
////////////////////////////////////////////////////////////////////////////////

logic signed [40-1:0] kk_mult;

assign kk_mult = ($signed(iir2_dat) * $signed(cfg_kk)) >>> 24;

// saturation
always_ff @(posedge clk)
if (~rstn)  sto_dat <= '0;
else        sto_dat <= kk_mult[40-1:40-2] ? {kk_mult[40-1], {DWO-1{~kk_mult[40-1]}}} : kk_mult[DWO-1:0];

assign sto_vld = 1'b1;

endmodule: scope_filter
