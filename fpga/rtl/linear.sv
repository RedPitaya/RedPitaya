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
  axi4_stream_if.d              sti,      // input
  axi4_stream_if.s              sto,      // output
  // configuration
  input  logic signed [DWM-1:0] cfg_mul,  // gain
  input  logic signed [DWS-1:0] cfg_sum   // offset
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

axi4_stream_if #(.DN (DN), .DT (logic signed [DWI+DWM  -1:0])) str_mul (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));
axi4_stream_if #(.DN (DN), .DT (logic signed [DWI+    1-1:0])) str_shf (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));
axi4_stream_if #(.DN (DN), .DT (logic signed [DWI+    2-1:0])) str_sum (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

////////////////////////////////////////////////////////////////////////////////
// multiplication
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_mul
  always_ff @(posedge sti.ACLK)
  if (sti.transf) begin
    str_mul.TDATA[i] <= sti.TDATA[i] * cfg_mul;
    str_mul.TKEEP[i] <= sti.TKEEP[i];
  end
end: for_mul
endgenerate

always_ff @(posedge sti.ACLK)
if (sti.transf)  str_mul.TLAST <= sti.TLAST;

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)     str_mul.TVALID <= 1'b0;
else if (sti.TREADY)  str_mul.TVALID <= sti.TVALID;

assign sti.TREADY = str_mul.TREADY | ~str_mul.TVALID;

////////////////////////////////////////////////////////////////////////////////
// shift
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_shf
  assign str_shf.TDATA[i] = str_mul.TDATA[i] >>> (DWM-2);
  assign str_shf.TKEEP[i] = str_mul.TKEEP[i] >>> (DWM-2);
end: for_shf
endgenerate

assign str_shf.TLAST  = str_mul.TLAST;
assign str_shf.TVALID = str_mul.TVALID;

assign str_mul.TREADY = str_shf.TREADY;

////////////////////////////////////////////////////////////////////////////////
// summation
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_sum
  always_ff @(posedge sti.ACLK)
  if (str_shf.transf) begin
    str_sum.TDATA[i] <= str_shf.TDATA[i] + cfg_sum;
    str_sum.TKEEP[i] <= str_shf.TKEEP[i];
  end
end: for_sum
endgenerate

always_ff @(posedge sti.ACLK)
if (str_shf.transf) begin
  str_sum.TLAST <= str_shf.TLAST;
end

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)         str_sum.TVALID <= 1'b0;
else if (str_shf.TREADY)  str_sum.TVALID <= str_shf.TVALID;

assign str_shf.TREADY = sto.TREADY | ~str_sum.TVALID;

////////////////////////////////////////////////////////////////////////////////
// saturation
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_sat
  always_ff @(posedge sti.ACLK)
  if (str_sum.transf) begin
    sto.TDATA[i] <= ^str_sum.TDATA[i][DWO:DWO-1] ? {str_sum.TDATA[i][DWO], {DWO-1{~str_sum.TDATA[i][DWO-1]}}}
                                                 :  str_sum.TDATA[i][DWO-1:0];
    sto.TKEEP[i] <= str_sum.TKEEP[i];
  end
end: for_sat
endgenerate

always_ff @(posedge sti.ACLK)
if (str_sum.transf) begin
  sto.TLAST <= str_sum.TLAST;
end

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)         sto.TVALID <= 1'b0;
else if (str_sum.TREADY)  sto.TVALID <= str_sum.TVALID;

assign str_sum.TREADY = sto.TREADY | ~str_sum.TVALID;

endmodule: linear
