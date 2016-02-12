////////////////////////////////////////////////////////////////////////////////
// AXI4-Stream pass through
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module axi4_stream_pas (
  // control
  input  logic     ena,  // enable
  // streams
  axi4_stream_if.d sti,  // input
  axi4_stream_if.s sto   // output
);

assign sti.TREADY = sto.TREADY & ena;

assign sto.TVALID = sti.TVALID & ena;
assign sto.TDATA  = sti.TDATA ;
assign sto.TKEEP  = sti.TKEEP ;
assign sto.TLAST  = sti.TLAST ;

endmodule: axi4_stream_pas
