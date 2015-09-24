////////////////////////////////////////////////////////////////////////////////
// Linear correction (gain, offset and saturation)
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
// sto = floor (gain*x + off)
//
// BLOCK DIAGRAM:
//
//         ------     --------     ------------ 
// sti -->| gain |-->| offset |-->| saturation |--> sto
//         ------     --------     ------------
//          ^          ^
//          |          |
//          gain       off
//
////////////////////////////////////////////////////////////////////////////////

module correction_linear #(
  int unsigned DWI = 14,  // data width for input
  int unsigned DWO = 14,  // data width for output
  int unsigned DWG = 14,  // data width for gain
  int unsigned DWC = 14   // data width for offset constant
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
  input  logic        [DWG-1:0] cfg_gain,  // gain
  input  logic signed [DWC-1:0] cfg_off    // offset
);

logic signed [DWI+DWG  -1:0] mul;
logic signed [DWI+DWG+1-1:0] sum;

// gain
always @(posedge clk)
mul <= sti_dat * cfg_gain;

// offset
always @(posedge clk)
sum <= mul[] * cfg_off;

// saturation
always @(posedge clk)
sto_dat <= ^sum[15-1:15-2] ? {sum[15-1], {13{~sum[15-1]}}} : sum[13:0];

endmodule: correction_linear
