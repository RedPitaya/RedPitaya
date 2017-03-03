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
  type DTI = logic signed [16-1:0],
  type DTO = logic signed [16-1:0]
)(
  // configuration
  input  logic signed [ 18-1:0] cfg_aa,   // config AA coefficient
  input  logic signed [ 25-1:0] cfg_bb,   // config BB coefficient
  input  logic signed [ 25-1:0] cfg_kk,   // config KK coefficient
  input  logic signed [ 25-1:0] cfg_pp,   // config PP coefficient
  // control
  input  logic                  ctl_rst,  // synchronous reset
  // streams
  axi4_stream_if.d              sti,      // input
  axi4_stream_if.s              sto       // output
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

axi4_stream_if #(.DN (DN), .DT (logic signed [23-1:0])) fir0 (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));
axi4_stream_if #(.DN (DN), .DT (logic signed [23-1:0])) iir1 (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));
axi4_stream_if #(.DN (DN), .DT (logic signed [15-1:0])) iir2 (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

////////////////////////////////////////////////////////////////////////////////
//
// FIR
//
// Time domain:
//
// y[n] = ( (             2**8) * x[n-0] ) +
//          (bb / 2**20 - 2**8) * x[n-1] )
//
// Frequency domain:
//
// y[n] = ( (           - 2**8) *        ) +
//          (bb / 2**20 - 2**8) * z^(-1) )
//
////////////////////////////////////////////////////////////////////////////////

typedef logic signed [   $bits(DTI)-1:0] fir_buf_t;  // data buffer
typedef logic signed [30           -1:0] fir_cfg_t;  // coeficients
typedef logic signed [25+$bits(DTI)-1:0] fir_mul_t;  // multiplications

axi4_stream_if #(.DN (DN), .DT (fir_mul_t [2-1:0])) firm (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

// FIR filter
fir_buf_t [2-1:0] fir_buf;  // data buffer
fir_cfg_t [2-1:0] fir_cfg;  // coeficients

// FIR data buffer
assign fir_buf [0] = sti.TDATA;

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)     fir_buf [1] <= '0;
else if (sti.transf)  fir_buf [1] <= fir_buf [0];

// FIR coeficients
assign fir_cfg [0] =  (1 <<< (18+10))         ;
assign fir_cfg [1] = -(1 <<< (18+10)) + cfg_bb;

// multiplications
generate
for (genvar i=0; i<2; i++) begin: for_fir_mul
  always_ff @(posedge sti.ACLK)
  if (sti.transf) firm.TDATA[0][i] <= (fir_buf[i] * fir_cfg[i]) >>> 10;
end: for_fir_mul
endgenerate

// final summation
always_ff @(posedge sti.ACLK)
if (firm.transf)  fir0.TDATA <= (firm.TDATA[0][1] + firm.TDATA[0][0]) >>> 10;

// control signals
always_ff @(posedge sti.ACLK)
if (~sti.ARESETn) begin
  firm.TVALID <= 1'b0;
  fir0.TVALID <= 1'b0;
end else begin
  if (sti.TREADY)  firm.TVALID <= sti.TVALID;
  if (firm.TREADY) fir0.TVALID <= firm.TVALID;
end

assign sti.TREADY  = firm.TREADY | ~firm.TVALID;
assign firm.TREADY = fir0.TREADY | ~fir0.TVALID;

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
always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)      iir1.TDATA <= '0;
else if (fir0.transf)  iir1.TDATA <= ((fir0.TDATA <<< 25) + (iir1.TDATA * ((1 <<< 25) - cfg_aa))) >>> 25;

// control signals
always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)      iir1.TVALID <= 1'b0;
else if (fir0.TREADY)  iir1.TVALID <= fir0.TVALID;

assign fir0.TREADY = iir1.TREADY | ~iir1.TVALID;

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

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)      iir2.TDATA <= '0;
else if (iir1.transf)  iir2.TDATA <= (iir1.TDATA >>> 8) + ((iir2.TDATA * cfg_pp) >>> 16);

// control signals
always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)      iir2.TVALID <= 1'b0;
else if (iir1.TREADY)  iir2.TVALID <= iir1.TVALID;

assign iir1.TREADY = iir2.TREADY | ~iir2.TVALID;

////////////////////////////////////////////////////////////////////////////////
// Scaling and saturation
////////////////////////////////////////////////////////////////////////////////

logic signed [40-1:0] kk_mult;

assign kk_mult = ($signed(iir2.TDATA) * $signed(cfg_kk)) >>> 24;

// saturation
always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)      sto.TDATA <= '0;
else if (iir2.transf)  sto.TDATA <= kk_mult[40-1:40-2] ? {kk_mult[40-1], {$bits(DTO)-1{~kk_mult[40-1]}}} : kk_mult[$bits(DTO)-1:0];

assign sto.TLAST = 1'b0;
assign sto.TKEEP = '1;

// control signals
always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)      sto.TVALID <= 1'b0;
else if (iir2.TREADY)  sto.TVALID <= iir2.TVALID;

assign iir2.TREADY = sto.TREADY | ~sto.TVALID;

endmodule: scope_filter
