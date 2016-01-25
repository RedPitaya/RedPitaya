////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation (gain, offset and saturation)
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
  int unsigned DN = 1,
  type DTI = logic signed [8-1:0], // data type for input
  type DTO = logic signed [8-1:0], // data type for output
  int unsigned DWI = $bits(DTI),   // data width for input
  int unsigned DWO = $bits(DTO),   // data width for output
  int unsigned DWM = 16,   // data width for multiplier (gain)
  int unsigned DWS = DWO   // data width for summation (offset)
)(
  // input stream input/output
  str_bus_if.d                  sti,      // input
  str_bus_if.s                  sto,      // output
  // configuration
  input  logic signed [DWM-1:0] cfg_mul,  // gain
  input  logic signed [DWS-1:0] cfg_sum   // offset
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

str_bus_if #(.DN (DN), .DAT_T (logic signed [DWI+DWM  -1:0])) str_mul (.clk (sti.clk), .rstn (sti.rstn));
str_bus_if #(.DN (DN), .DAT_T (logic signed [DWI+    1-1:0])) str_shf (.clk (sti.clk), .rstn (sti.rstn));
str_bus_if #(.DN (DN), .DAT_T (logic signed [DWI+    2-1:0])) str_sum (.clk (sti.clk), .rstn (sti.rstn));

logic str_mul_trn;
logic str_shf_trn;
logic str_sum_trn;

////////////////////////////////////////////////////////////////////////////////
// multiplication
////////////////////////////////////////////////////////////////////////////////

assign sti_trn = sti.vld & sti.rdy;

generate
for (genvar i=0; i<DN; i++) begin: for_mul
  always_ff @(posedge sti.clk)
  if (sti_trn) begin
    str_mul.dat[i] <= sti.dat[i] * cfg_mul;
    str_mul.kep[i] <= sti.kep[i];
  end
end: for_mul
endgenerate

always_ff @(posedge sti.clk)
if (sti_trn)  str_mul.lst <= sti.lst;

always_ff @(posedge sti.clk)
if (~sti.rstn)     str_mul.vld <= 1'b0;
else if (sti.rdy)  str_mul.vld <= sti.vld;

assign sti.rdy = str_mul.rdy | ~str_mul.vld;

////////////////////////////////////////////////////////////////////////////////
// shift
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_shf
  assign str_shf.dat[i] = str_mul.dat[i] >>> (DWM-2);
  assign str_shf.kep[i] = str_mul.kep[i] >>> (DWM-2);
end: for_shf
endgenerate

assign str_shf.lst = str_mul.lst;

assign str_shf.vld = str_mul.vld;

assign str_mul.rdy = str_shf.rdy;

////////////////////////////////////////////////////////////////////////////////
// summation
////////////////////////////////////////////////////////////////////////////////

assign shf_trn = str_shf.vld & str_shf.rdy;

generate
for (genvar i=0; i<DN; i++) begin: for_sum
  always_ff @(posedge sti.clk)
  if (shf_trn) begin
    str_sum.dat[i] <= str_shf.dat[i] + cfg_sum;
    str_sum.kep[i] <= str_shf.kep[i];
  end
end: for_sum
endgenerate

always_ff @(posedge sti.clk)
if (shf_trn) begin
  str_sum.lst <= str_shf.lst;
end

always_ff @(posedge sti.clk)
if (~sti.rstn)         str_sum.vld <= 1'b0;
else if (str_shf.rdy)  str_sum.vld <= str_shf.vld;

assign str_shf.rdy = sto.rdy | ~str_sum.vld;

////////////////////////////////////////////////////////////////////////////////
// saturation
////////////////////////////////////////////////////////////////////////////////

assign sum_trn = str_sum.vld & str_sum.rdy;

generate
for (genvar i=0; i<DN; i++) begin: for_sat
  always_ff @(posedge sti.clk)
  if (sum_trn) begin
    sto.dat[i] <= ^str_sum.dat[i][DWO:DWO-1] ? {str_sum.dat[i][DWO], {DWO-1{~str_sum.dat[i][DWO-1]}}}
                                             :  str_sum.dat[i][DWO-1:0];
    sto.kep[i] <= str_sum.kep[i];
  end
end: for_sat
endgenerate

always_ff @(posedge sti.clk)
if (sum_trn) begin
  sto.lst <= str_sum.lst;
end

always_ff @(posedge sti.clk)
if (~sti.rstn)         sto.vld <= 1'b0;
else if (str_sum.rdy)  sto.vld <= str_sum.vld;

assign str_sum.rdy = sto.rdy | ~str_sum.vld;

endmodule: linear
