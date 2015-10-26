////////////////////////////////////////////////////////////////////////////////
// Linear transformation (gain, offset and saturation)
// Author: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// GENERAL DESCRIPTION:
//
// A linear transformation is applied to the signal. Multiplication by gain and
// offset addition. At the end there is a saturation module to meet the output
// data width.
//
// sto = floor (((x * mul) >>> (DWM-1)) + sum)
//
// BLOCK DIAGRAM:
//
//         -----     -------     -----     ------------ 
// sti -->| mul |-->| shift |-->| sum |-->| saturation |--> sto
//         -----     -------     -----     ------------
//           ^                     ^
//           |                     |
//          mul                   sum
//
////////////////////////////////////////////////////////////////////////////////

module linear #(
  int unsigned DWI = 14,   // data width for input
  int unsigned DWO = 14,   // data width for output
  int unsigned DWM = 16,   // data width for multiplier (gain)
  int unsigned DWS = DWO   // data width for summation (offset)
)(
  // system signals
  input  logic                  clk ,  // clock
  input  logic                  rstn,  // reset - active low
  // input stream
  input  logic signed [DWI-1:0] sti_dat,  // data
  input  logic                  sti_vld,  // valid
  output logic                  sti_rdy,  // ready
  // output stream
  output logic signed [DWO-1:0] sto_dat,  // data
  output logic                  sto_vld,  // valid
  input  logic                  sto_rdy,  // ready
  // configuration
  input  logic signed [DWM-1:0] cfg_mul,  // gain
  input  logic signed [DWS-1:0] cfg_sum   // offset
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

logic signed [DWI+DWM  -1:0] mul_dat;
logic signed [DWI+    1-1:0] shf_dat;
logic signed [DWI+   +2-1:0] sum_dat;

logic mul_vld, mul_rdy, mul_trn;
logic shf_vld, shf_rdy, shf_trn;
logic sum_vld, sum_rdy, sum_trn;

////////////////////////////////////////////////////////////////////////////////
// multiplication
////////////////////////////////////////////////////////////////////////////////

assign sti_trn = sti_vld & sti_rdy;

always @(posedge clk)
if (sti_trn)  mul_dat <= sti_dat * cfg_mul;

always @(posedge clk)
if (~rstn)         mul_vld <= 1'b0;
else if (sti_rdy)  mul_vld <= sti_vld;

assign sti_rdy = mul_rdy | ~mul_vld;

////////////////////////////////////////////////////////////////////////////////
// shift
////////////////////////////////////////////////////////////////////////////////

assign shf_dat <= mul_dat >>> (DWM-1);

assign shf_vld = mul_vld;

assign sti_rdy = shf_rdy;

////////////////////////////////////////////////////////////////////////////////
// summation
////////////////////////////////////////////////////////////////////////////////

assign shf_trn = shf_vld & shf_rdy;

always @(posedge clk)
if (shf_trn)  sum_dat <= shf_dat + cfg_sum;

always @(posedge clk)
if (~rstn)         sum_vld <= 1'b0;
else if (shf_rdy)  sum_vld <= sti_vld;

assign shf_rdy = out_rdy | ~sum_vld;

////////////////////////////////////////////////////////////////////////////////
// saturation
////////////////////////////////////////////////////////////////////////////////

assign sum_trn = sum_vld & sum_rdy;

always @(posedge clk)
if (sum_trn)  sto_dat <= ^sum_dat[DWO-1:DWO-2] ? {sum_dat[DWO], {DWO-1{~sum_dat[DWO-1]}}}
                                               :  sum_dat[DWO-1:0];

always @(posedge clk)
if (~rstn)         out_vld <= 1'b0;
else if (sum_rdy)  out_vld <= sti_vld;

assign sum_rdy = out_rdy | ~sum_vld;

endmodule: linear
