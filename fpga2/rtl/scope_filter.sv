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
  int unsigned DN = 1, // TODO: functionality not yet enabled
  int unsigned DWI = 14,
  int unsigned DWO = 14
)(
  // configuration
  input  logic signed [ 18-1:0] cfg_aa,   // config AA coefficient
  input  logic signed [ 25-1:0] cfg_bb,   // config BB coefficient
  input  logic signed [ 25-1:0] cfg_kk,   // config KK coefficient
  input  logic signed [ 25-1:0] cfg_pp,   // config PP coefficient
  // control
  input  logic                  ctl_rst,  // synchronous reset
  // streams
  str_bus_if.d                  sti,      // input
  str_bus_if.s                  sto       // output
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

str_bus_if #(.DN (DN), .DAT_T (logic signed [23-1:0])) fir  (.clk (sti.clk), .rstn (sti.rstn));
str_bus_if #(.DN (DN), .DAT_T (logic signed [23-1:0])) iir1 (.clk (sti.clk), .rstn (sti.rstn));
str_bus_if #(.DN (DN), .DAT_T (logic signed [15-1:0])) iir2 (.clk (sti.clk), .rstn (sti.rstn));

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

str_bus_if #(.DN (DN), .DAT_T (fir_mul_t [2-1:0])) firm (.clk (sti.clk), .rstn (sti.rstn));

// FIR filter
fir_buf_t [2-1:0] fir_buf;  // data buffer
fir_cfg_t [2-1:0] fir_cfg;  // coeficients

// FIR data buffer
assign fir_buf [0] = sti.dat;

always_ff @(posedge sti.clk)
if (~sti.rstn)     fir_buf [1] <= '0;
else if (sti.trn)  fir_buf [1] <= fir_buf [0];

// FIR coeficients
assign fir_cfg [0] =  (1 <<< (18+10)) + cfg_bb;
assign fir_cfg [1] = -(1 <<< (18+10))         ;

// multiplications
generate
for (genvar i=0; i<2; i++) begin: for_fir_mul
  always_ff @(posedge sti.clk)
  if (sti.trn) firm.dat[0][i] <= (fir_buf[i] * fir_cfg[i]) >>> 10;
end: for_fir_mul
endgenerate

// final summation
always_ff @(posedge sti.clk)
if (firm.trn)  fir.dat <= (firm.dat[0][1] + firm.dat[0][0]) >>> 10;



// TODO: a generic FOR filter should be used here

// control signals
always_ff @(posedge sti.clk)
if (~sti.rstn) begin
  firm.vld <= 1'b0;
  fir.vld     <= 1'b0;
end else begin
  if (sti.rdy)  firm.vld <= sti.vld;
  if (firm.rdy) fir.vld  <= firm.vld;
end

assign sti.rdy     = firm.rdy | ~firm.vld;

assign firm.rdy = fir.rdy     | ~fir.vld;

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
always_ff @(posedge sti.clk)
if (~sti.rstn)     iir1.dat <= '0;
else if (fir.trn)  iir1.dat <= ((fir.dat <<< 25) + (iir1.dat * ((1 <<< 25) - cfg_aa))) >>> 25;

// control signals
always_ff @(posedge sti.clk)
if (~sti.rstn)     iir1.vld <= 1'b0;
else if (fir.rdy)  iir1.vld <= fir.vld;

assign fir.rdy = iir1.rdy | ~iir1.vld;

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

always_ff @(posedge sti.clk)
if (~sti.rstn)      iir2.dat <= '0;
else if (iir1.trn)  iir2.dat <= (iir1.dat >>> 8) + ((iir2.dat * cfg_pp) >>> 16);

// control signals
always_ff @(posedge sti.clk)
if (~sti.rstn)      iir2.vld <= 1'b0;
else if (iir1.rdy)  iir2.vld <= iir1.vld;

assign iir1.rdy = iir2.rdy | ~iir2.vld;

////////////////////////////////////////////////////////////////////////////////
// Scaling and saturation
////////////////////////////////////////////////////////////////////////////////

logic signed [40-1:0] kk_mult;

assign kk_mult = ($signed(iir2.dat) * $signed(cfg_kk)) >>> 24;

// saturation
always_ff @(posedge sti.clk)
if (~sti.rstn)      sto.dat <= '0;
else if (iir2.trn)  sto.dat <= kk_mult[40-1:40-2] ? {kk_mult[40-1], {DWO-1{~kk_mult[40-1]}}} : kk_mult[DWO-1:0];

assign sto.lst = 1'b0;

// control signals
always_ff @(posedge sti.clk)
if (~sti.rstn)      sto.vld <= 1'b0;
else if (iir2.rdy)  sto.vld <= iir2.vld;

assign iir2.rdy = sto.rdy | ~sto.vld;

endmodule: scope_filter
