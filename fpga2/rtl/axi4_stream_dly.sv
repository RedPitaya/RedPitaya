////////////////////////////////////////////////////////////////////////////////
// AXI4-Stream delay
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module axi4_stream_dly #(
  // delay length
  int unsigned LN = 2,
  // data stream parameters
  int unsigned DN = 1,
  type DT = logic [8-1:0]
)(
  // streams
  axi4_stream_if.d sti,  // input
  axi4_stream_if.s sto   // output
);

axi4_stream_if #(.DN (DN), .DT (DT)) str [LN:0] (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));

axi4_stream_pas (.ena (1'b1), .sti (sti), .sto (str[0]));

generate
for (genvar i=0; i<LN; i++) begin: for_ln

axi4_stream_reg (.sti (str[i]), .sto (str[i+1]));

end: for_ln
endgenerate

axi4_stream_pas (.ena (1'b1), .sti (str[LN]), .sto (sto));

endmodule: axi4_stream_dly
