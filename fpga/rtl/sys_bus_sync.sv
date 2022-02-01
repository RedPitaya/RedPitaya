////////////////////////////////////////////////////////////////////////////////
// Module: System bus sync between 2 clock domains
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module sys_bus_sync #(
  int unsigned AW = 32,
  int unsigned DW = 32  // slave width (address bus width)
)(
  sys_bus_if.s bus_m,          // from master
  sys_bus_if.m bus_s           // to   slaves
);

logic          wen_r  , wen_r2  ;  // write enable
logic          ren_r  , ren_r2  ;  // read enable
logic [AW-1:0] addr_r , addr_r2 ;  // read/write address
logic [DW-1:0] wdata_r, wdata_r2;  // write data
logic [DW-1:0] rdata_r, rdata_r2;  // read data
logic          ack_r  , ack_r2  ;  // acknowledge signal
logic          err_r  , err_r2  ;  // error indicator

always @(posedge bus_s.clk) begin
  wen_r    <= bus_m.wen  ;
  wen_r2   <= wen_r      ;

  ren_r    <= bus_m.ren  ;
  ren_r2   <= ren_r      ;

  addr_r   <= bus_m.addr ;
  addr_r2  <= addr_r     ;

  wdata_r  <= bus_m.wdata;
  wdata_r2 <= wdata_r    ;
end
assign bus_s.wen   = wen_r2  ;
assign bus_s.ren   = ren_r2  ;
assign bus_s.addr  = addr_r2 ;
assign bus_s.wdata = wdata_r2;

always @(posedge bus_m.clk) begin
  ack_r    <= bus_s.ack  ;
  ack_r2   <= ack_r      ;

  err_r    <= bus_s.err  ;
  err_r2   <= err_r      ;

  rdata_r  <= bus_s.rdata;
  rdata_r2 <= rdata_r    ;
end
assign bus_m.ack   = ack_r2  ;
assign bus_m.err   = err_r2  ;
assign bus_m.rdata = rdata_r2;

endmodule: sys_bus_sync
