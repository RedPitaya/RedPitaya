////////////////////////////////////////////////////////////////////////////////
// Module: Binary AND mask
// Author: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// GENERAL DESCRIPTION:
//
// AND mask.
//
// sto = sti & and
//
// BLOCK DIAGRAM:
//
//         -----    
// sti -->| and |--> sto
//         -----    
//           ^
//           |
//          and
//
////////////////////////////////////////////////////////////////////////////////

module bin_and #(
  int unsigned DN = 1,
  type DT = logic signed [8-1:0]  // data type for input
)(
  // input stream input/output
  axi4_stream_if.d sti,  // input
  axi4_stream_if.s sto,  // output
  // configuration
  input DT     cfg_and   // mask
);

////////////////////////////////////////////////////////////////////////////////
// AND mask
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<DN; i++) begin: for_and
  always_ff @(posedge sti.ACLK)
  if (sti.transf) begin
    sto.TDATA[i] <= sti.TDATA[i] & cfg_and;
    sto.TKEEP[i] <= sti.TKEEP[i];
  end
end: for_and
endgenerate

always_ff @(posedge sti.ACLK)
if (sti.transf)  sto.TLAST <= sti.TLAST;

always_ff @(posedge sti.ACLK)
if (~sti.ARESETn)     sto.TVALID <= 1'b0;
else if (sti.TREADY)  sto.TVALID <= sti.TVALID;

assign sti.TREADY = sto.TREADY | ~sto.TVALID;

endmodule: bin_and
