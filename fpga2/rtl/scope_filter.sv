////////////////////////////////////////////////////////////////////////////////
// Red Pitaya equalization filter
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// GENERAL DESCRIPTION:
// Filter to equalize input analog chain. 
////////////////////////////////////////////////////////////////////////////////

module scope_filter #(
  int unsigned DWI = 14,
  int unsigned DWO = 14
)(
  // system signals
  input  logic            clk ,  // clock
  input  logic            rstn,  // reset - active low
  // input stream
  input  logic [DWI-1: 0] adc_dat_i       ,  //!< ADC data
  // output stream
  output logic [DWO-1: 0] adc_dat_o       ,  //!< ADC data
  // configuration
  input  logic [ 18-1: 0] cfg_aa,  // config AA coefficient
  input  logic [ 25-1: 0] cfg_bb,  // config BB coefficient
  input  logic [ 25-1: 0] cfg_kk,  // config KK coefficient
  input  logic [ 25-1: 0] cfg_pp   // config PP coefficient
);

////////////////////////////////////////////////////////////////////////////////
// FIR
////////////////////////////////////////////////////////////////////////////////

logic [ 39-1: 0] bb_mult   ;
logic [ 33-1: 0] r2_sum    ;
logic [ 33-1: 0] r1_reg    ;
logic [ 23-1: 0] r2_reg    ;
logic [ 32-1: 0] r01_reg   ;
logic [ 28-1: 0] r02_reg   ;

assign bb_mult = $signed(adc_dat_i) * $signed(cfg_bb);
assign r2_sum  = $signed(r01_reg) + $signed(r1_reg);

always @(posedge clk)
if (rstn == 1'b0) begin
  r1_reg  <= 33'h0 ;
  r2_reg  <= 23'h0 ;
  r01_reg <= 32'h0 ;
  r02_reg <= 28'h0 ;
end else begin
  r1_reg  <= $signed(r02_reg) - $signed(r01_reg) ;
  r2_reg  <= r2_sum[33-1:10];
  r01_reg <= {adc_dat_i,18'h0};
  r02_reg <= bb_mult[39-2:10];
end

////////////////////////////////////////////////////////////////////////////////
// IIR 1
////////////////////////////////////////////////////////////////////////////////

logic [ 41-1: 0] aa_mult   ;
logic [ 49-1: 0] r3_sum    ; //24 + 25
(* use_dsp48="yes" *) logic [ 23-1: 0] r3_reg    ;

assign aa_mult = $signed(r3_reg) * $signed(cfg_aa);
assign r3_sum  = $signed({r2_reg,25'h0}) + $signed({r3_reg,25'h0}) - $signed(aa_mult[41-1:0]);

always @(posedge clk)
if (rstn == 1'b0) begin
  r3_reg  <= 23'h0 ;
end else begin
  r3_reg  <= r3_sum[49-2:25] ;
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

always @(posedge clk)
if (rstn == 1'b0) begin
  r3_shr <= 15'h0 ;
  r4_reg <= 15'h0 ;
end else begin
  r3_shr <= r3_reg[23-1:8] ;
  r4_reg <= r4_sum[16-2:0] ;
end

////////////////////////////////////////////////////////////////////////////////
// Scaling
////////////////////////////////////////////////////////////////////////////////

logic [ 40-1: 0] kk_mult   ;
logic [ 15-1: 0] r4_reg_r  ;
logic [ 15-1: 0] r4_reg_rr ;
logic [ 14-1: 0] r5_reg    ;

assign kk_mult = $signed(r4_reg_rr) * $signed(cfg_kk);

always @(posedge clk)
if (rstn == 1'b0) begin
  r4_reg_r  <= 15'h0 ;
  r4_reg_rr <= 15'h0 ;
  r5_reg    <= 14'h0 ;
end else begin
  r4_reg_r  <= r4_reg   ;
  r4_reg_rr <= r4_reg_r ;
  // saturation
  if ($signed(kk_mult[40-2:24]) > $signed(14'h1FFF))
     r5_reg <= 14'h1FFF ;
  else if ($signed(kk_mult[40-2:24]) < $signed(14'h2000))
     r5_reg <= 14'h2000 ;
  else
     r5_reg <= kk_mult[24+14-1:24];
end

assign adc_dat_o = r5_reg ;

endmodule: scope_filter
