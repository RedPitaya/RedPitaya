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
  input  logic signed [ 25-1:0] cfg_pp    // config PP coefficient
);

////////////////////////////////////////////////////////////////////////////////
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

logic signed [ 39-1: 0] bb_mult;
logic signed [ 33-1: 0] r2_sum ;
logic signed [ 33-1: 0] r1_reg ;
logic signed [ 23-1: 0] r2_reg ;
logic signed [ 32-1: 0] r01_reg;
logic signed [ 28-1: 0] r02_reg;

assign bb_mult = sti_vld * cfg_bb;
assign r2_sum  = r01_reg + r1_reg;

always_ff @(posedge clk)
if (~rstn) begin
  r1_reg  <= '0;
  r2_reg  <= '0;
  r01_reg <= '0;
  r02_reg <= '0;
end else begin
  r1_reg  <= r02_reg - r01_reg;
  r2_reg  <= r2_sum[33-1:10];
  r01_reg <= {sti_vld,18'h0};
  r02_reg <= bb_mult[39-2:10];
end

////////////////////////////////////////////////////////////////////////////////
// IIR 1
////////////////////////////////////////////////////////////////////////////////

logic [ 41-1: 0] aa_mult;
logic [ 49-1: 0] r3_sum ; //24 + 25
(* use_dsp48="yes" *) logic [ 23-1: 0] r3_reg;

assign aa_mult = $signed(r3_reg) * $signed(cfg_aa);
assign r3_sum  = $signed({r2_reg,25'h0}) + $signed({r3_reg,25'h0}) - $signed(aa_mult[41-1:0]);

always_ff @(posedge clk)
if (~rstn) begin
  r3_reg  <= 23'h0;
end else begin
  r3_reg  <= r3_sum[49-2:25];
end

////////////////////////////////////////////////////////////////////////////////
// IIR 2
////////////////////////////////////////////////////////////////////////////////

logic [ 40-1: 0] pp_mult;
logic [ 16-1: 0] r4_sum ;
logic [ 15-1: 0] r4_reg ;
logic [ 15-1: 0] r3_shr ;

assign pp_mult = $signed(r4_reg) * $signed(cfg_pp);
assign r4_sum  = $signed(r3_shr) + $signed(pp_mult[40-2:16]);

always_ff @(posedge clk)
if (~rstn) begin
  r3_shr <= 15'h0;
  r4_reg <= 15'h0;
end else begin
  r3_shr <= r3_reg[23-1:8];
  r4_reg <= r4_sum[16-2:0];
end

////////////////////////////////////////////////////////////////////////////////
// Scaling and saturation
////////////////////////////////////////////////////////////////////////////////

logic [ 40-1: 0] kk_mult  ;
logic [ 15-1: 0] r4_reg_r ;
logic [ 15-1: 0] r4_reg_rr;
logic [ 14-1: 0] r5_reg   ;

assign kk_mult = $signed(r4_reg_rr) * $signed(cfg_kk);

always_ff @(posedge clk)
if (~rstn) begin
  r4_reg_r  <= 15'h0;
  r4_reg_rr <= 15'h0;
  r5_reg    <= 14'h0;
end else begin
  r4_reg_r  <= r4_reg  ;
  r4_reg_rr <= r4_reg_r;
  // saturation
  if ($signed(kk_mult[40-2:24]) > $signed(14'h1FFF))
     r5_reg <= 14'h1FFF;
  else if ($signed(kk_mult[40-2:24]) < $signed(14'h2000))
     r5_reg <= 14'h2000;
  else
     r5_reg <= kk_mult[24+14-1:24];
end

assign sto_vld = r5_reg;

endmodule: scope_filter
