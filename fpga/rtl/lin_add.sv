////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation, offset (adder+saturation)
// Author: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// GENERAL DESCRIPTION:
//
// An offset is added to the signal. Saturation is applied to meet the output
// data width. Saturation can also be disabled.
//
// sto = floor (x + sum)
//
// BLOCK DIAGRAM:
//
//         -----     ------------ 
// sti -->| sum |-->| saturation |--> sto
//         -----     ------------
//           ^
//           |
//          sum
//
////////////////////////////////////////////////////////////////////////////////

module lin_add #(
  int unsigned DN = 1,
  type DTI = logic signed [8-1:0], // data type for input
  type DTO = logic signed [8-1:0], // data type for output
  type DTS = DTI                   // data type for output
)(
  // input stream input/output
  axi4_stream_if.d sti,  // input
  axi4_stream_if.s sto,  // output
  // configuration
  input DTS    cfg_sum   // offset
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned DWI = $bits(DTI);
localparam int unsigned DWO = $bits(DTO);
localparam int unsigned DWS = $bits(DTS);

axi4_stream_if #(.DN (DN), .DT (logic signed [DWO+1-1:0])) str (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

////////////////////////////////////////////////////////////////////////////////
// summation
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_sum
  always_ff @(posedge sti.ACLK)
  if (sti.transf) begin
    str.TDATA[i] <= sti.TDATA[i] + cfg_sum;
    str.TKEEP[i] <= sti.TKEEP[i];
  end
end: for_sum
endgenerate

always_ff @(posedge sti.ACLK)
if (sti.transf) begin
  str.TLAST <= sti.TLAST;
end

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)     str.TVALID <= 1'b0;
else if (sti.TREADY)  str.TVALID <= sti.TVALID;

assign sti.TREADY = sto.TREADY | ~str.TVALID;

////////////////////////////////////////////////////////////////////////////////
// saturation
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_sat
  always_ff @(posedge sti.ACLK)
  if (str.transf) begin
    sto.TDATA[i] <= ^str.TDATA[i][DWO:DWO-1] ? {str.TDATA[i][DWO], {DWO-1{~str.TDATA[i][DWO]}}}
                                             :  str.TDATA[i][DWO-1:0];
    sto.TKEEP[i] <=  str.TKEEP[i];
  end
end: for_sat
endgenerate

always_ff @(posedge sti.ACLK)
if (str.transf) begin
  sto.TLAST <= str.TLAST;
end

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)     sto.TVALID <= 1'b0;
else if (str.TREADY)  sto.TVALID <= str.TVALID;

assign str.TREADY = sto.TREADY | ~str.TVALID;

endmodule: lin_add
