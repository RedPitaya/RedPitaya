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
// local signals
////////////////////////////////////////////////////////////////////////////////

logic signed [23-1:0] fir_dat;
logic                 fir_vld;
logic                 fir_rdy;
logic                 fir_trn;

(* use_dsp48="yes" *) logic signed [23-1:0] iir1_dat;
                      logic                 iir1_vld;
                      logic                 iir1_rdy;
                      logic                 iir1_trn;

logic signed [15-1:0] iir2_dat;
logic                 iir2_vld;
logic                 iir2_rdy;
logic                 iir2_trn;

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

logic sti_trn;

assign sti_trn = sti_vld & sti_rdy;

////////////////////////////////////////////////////////////////////////////////
//
// FIR
//
// Time domain:
//
// y[n] = ( ( 2**18 + bb / 2**10) * x[n-0] +
//          (-2**18             ) * x[n-1] ) / 2**10
//
// Frequency domain:
//
// y[n] = ( (     2**18) *        ) +
//          (bb - 2**18) * z^(-1) ) / 2**10
//
////////////////////////////////////////////////////////////////////////////////

typedef logic signed [   DWI-1:0] fir_buf_t;  // data buffer
typedef logic signed [29    -1:0] fir_cfg_t;  // coeficients
typedef logic signed [25+DWI-1:0] fir_mul_t;  // multiplications

// FIR filter
fir_buf_t [2-1:0] fir_buf;  // data buffer
fir_cfg_t [2-1:0] fir_cfg;  // coeficients
fir_mul_t [2-1:0] fir_mul;  // multiplications

logic                             fir_mul_trn;  //
logic                             fir_mul_vld;  //
logic                             fir_mul_rdy;  //

// FIR data buffer
assign fir_buf [0] = sti_dat;

always_ff @(posedge clk)
if (~rstn)         fir_buf [1] <= '0;
else if (sti_trn)  fir_buf [1] <= fir_buf [0];

// FIR coeficients
assign fir_cfg [0] =  (1 <<< (18+10)) + cfg_bb;
assign fir_cfg [1] = -(1 <<< (18+10))         ;

// multiplications
generate
for (genvar i=0; i<2; i++) begin: for_fir_mul
  always_ff @(posedge clk)
  if (sti_trn) fir_mul[i] <= (fir_buf[i] * fir_cfg[i]) >>> 10;
end: for_fir_mul
endgenerate

// final summation
always_ff @(posedge clk)
if (fir_mul_trn)  fir_dat <= (fir_mul[1] + fir_mul[0]) >>> 10;



// TODO: a generic FOR filter should be used here

// control signals
always_ff @(posedge clk)
if (~rstn) begin
  fir_mul_vld <= 1'b0;
  fir_vld     <= 1'b0;
end else begin
  if (sti_rdy)     fir_mul_vld <= sti_vld;
  if (fir_mul_rdy) fir_vld     <= fir_mul_vld;
end

assign sti_rdy     = fir_mul_rdy | ~fir_mul_vld;

assign fir_mul_rdy = fir_rdy     | ~fir_vld;

assign fir_mul_trn = fir_mul_vld & fir_mul_rdy;

assign fir_trn = fir_vld & fir_rdy;

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

// data processing
always_ff @(posedge clk)
if (~rstn)         iir1_dat <= '0;
else if (fir_trn)  iir1_dat <= ((fir_dat <<< 25) + (iir1_dat * ((1 <<< 25) - cfg_aa))) >>> 25;

// control signals
always_ff @(posedge clk)
if (~rstn)         iir1_vld <= 1'b0;
else if (fir_rdy)  iir1_vld <= fir_vld;

assign fir_rdy = iir1_rdy | ~iir1_vld;

assign iir1_trn = iir1_vld & iir1_rdy;

////////////////////////////////////////////////////////////////////////////////
//
// IIR 2
//
// Time domain:
//
// y[n] = (      x[n-0] / (2**8 ) +
//          pp * y[n-1] / (2**16) )
//
// Frequency domain:
//
// TODO
//
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge clk)
if (~rstn)          iir2_dat <= '0;
else if (iir1_trn)  iir2_dat <= (iir1_dat >>> 8) + ((iir2_dat * cfg_pp) >>> 16);

// control signals
always_ff @(posedge clk)
if (~rstn)          iir2_vld <= 1'b0;
else if (iir1_rdy)  iir2_vld <= iir1_vld;

assign iir1_rdy = iir2_rdy | ~iir2_vld;

assign iir2_trn = iir2_vld & iir2_rdy;

////////////////////////////////////////////////////////////////////////////////
// Scaling and saturation
////////////////////////////////////////////////////////////////////////////////

logic signed [40-1:0] kk_mult;

assign kk_mult = ($signed(iir2_dat) * $signed(cfg_kk)) >>> 24;

// saturation
always_ff @(posedge clk)
if (~rstn)          sto_dat <= '0;
else if (iir2_trn)  sto_dat <= kk_mult[40-1:40-2] ? {kk_mult[40-1], {DWO-1{~kk_mult[40-1]}}} : kk_mult[DWO-1:0];

// control signals
always_ff @(posedge clk)
if (~rstn)          sto_vld <= 1'b0;
else if (iir2_rdy)  sto_vld <= iir2_vld;

assign iir2_rdy = sto_rdy | ~sto_vld;

endmodule: scope_filter
