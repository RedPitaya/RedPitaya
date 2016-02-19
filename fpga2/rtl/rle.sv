////////////////////////////////////////////////////////////////////////////////
// Module: RLE (Run Length Encoding) compression
// Author: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// GENERAL DESCRIPTION:
//
// A RLE compression is applied to the signal.
////////////////////////////////////////////////////////////////////////////////

module rle #(
  // stream properties
  int unsigned DN = 1,
  type DTI = logic signed [8-1:0], // data type for input
  type DTO = logic signed [8-1:0], // data type for output
  // counter properties
  int unsigned CW = 8
)(
  // input stream input/output
  axi4_stream_if.d sti,  // input
  axi4_stream_if.s sto,  // output
  // configuration
  input  logic     ctl_rst,  // reset
  input  logic     cfg_ena   // enable
);

////////////////////////////////////////////////////////////////////////////////
// store previous value
////////////////////////////////////////////////////////////////////////////////

DTI old_tdata ;
DTI old_tvalid;

always_ff @(posedge sti.ACLK)
if (sti.transf)  old_tdata <= sti.TDATA;

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)     old_tvalid <= 1'b0;
else if (sti.transf)  old_tvalid <= ~sti.TLAST;

////////////////////////////////////////////////////////////////////////////////
// comparator
////////////////////////////////////////////////////////////////////////////////

logic cmp;  // compare

assign cmp = (old_tdata == sti.TDATA);


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

endmodule: rle
