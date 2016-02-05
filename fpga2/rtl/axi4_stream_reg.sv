////////////////////////////////////////////////////////////////////////////////
// AXI4-Stream register
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module axi4_stream_reg (
  // streams
  axi4_stream_if.d sti,  // input
  axi4_stream_if.s sto   // output
);

assign sti.TREADY = sto.TREADY | ~sto.TVALID;

always_ff @(posedge sti.clk)
if (sti.rst) begin
  sto.TVALID <= sti.TVALID;
end

always_ff @(posedge sti.clk)
begin
  sto.TDATA  <= sti.TDATA ;
  sto.TKEEP  <= sti.TKEEP ;
  sto.TLAST  <= sti.TLAST ;
end

endmodule: axi4_stream_reg
