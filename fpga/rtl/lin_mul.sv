////////////////////////////////////////////////////////////////////////////////
// Module: Linear transformation, gain (multiplication)
// Author: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// GENERAL DESCRIPTION:
//
// Multiplication by gain factor is applied to the input.
//
// sto = (x * mul) >>> (DWM-1)
//
// BLOCK DIAGRAM:
//
//         -----     ------- 
// sti -->| mul |-->| shift |--> sto
//         -----     ------- 
//           ^
//           |
//          mul
//
////////////////////////////////////////////////////////////////////////////////

module lin_mul #(
  int unsigned DN = 1,
  type DTI = logic signed [8-1:0], // data type for input
  type DTO = logic signed [8-1:0], // data type for output
  type DTM = logic signed [8-1:0]  // data type for multiplicand
)(
  // input stream input/output
  axi4_stream_if.d sti,  // input
  axi4_stream_if.s sto,  // output
  // configuration
  input DTM    cfg_mul   // gain
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned DWI = $bits(DTI);
localparam int unsigned DWO = $bits(DTO);
localparam int unsigned DWM = $bits(DTM);

axi4_stream_if #(.DN (DN), .DT (logic signed [DWI+DWM-1:0])) str (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));
axi4_stream_if #(.DN (DN), .DT (logic signed [DWI+DWM-1:0])) stc (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

////////////////////////////////////////////////////////////////////////////////
// multiplication
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_mul
  always_ff @(posedge sti.ACLK)
  if (sti.transf) begin
    str.TDATA[i] <= sti.TDATA[i] * cfg_mul;
    str.TKEEP[i] <= sti.TKEEP[i];
  end
end: for_mul
endgenerate

always_ff @(posedge sti.ACLK)
if (sti.transf)  str.TLAST <= sti.TLAST;

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)     str.TVALID <= 1'b0;
else if (sti.TREADY)  str.TVALID <= sti.TVALID;

assign sti.TREADY = str.TREADY | ~str.TVALID;

////////////////////////////////////////////////////////////////////////////////
// shift (this is a compbinatorial stage)
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_shf
  assign stc.TDATA[i] = str.TDATA[i] >>> (DWM-2);
  assign stc.TKEEP[i] = str.TKEEP[i];
  
  assign stc.TLAST  = str.TLAST;
  assign stc.TVALID = str.TVALID;

end: for_shf
endgenerate

////////////////////////////////////////////////////////////////////////////////
// shift (this is a compbinatorial stage)
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_sat
  always_ff @(posedge sti.ACLK)
  if (str.transf) begin
    sto.TDATA[i] <= ^stc.TDATA[i][DWO:DWO-1] ? {stc.TDATA[i][DWO], {DWO-1{~stc.TDATA[i][DWO]}}}
                                             :  stc.TDATA[i][DWO-1:0];
    sto.TKEEP[i] <=  stc.TKEEP[i];
  end
end: for_sat
endgenerate


assign sto.TLAST  = stc.TLAST;
assign sto.TVALID = stc.TVALID;

assign str.TREADY = sto.TREADY;

endmodule: lin_mul
