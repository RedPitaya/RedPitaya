////////////////////////////////////////////////////////////////////////////////
// Module: System bus interconnect
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module sys_bus_interconnect #(
  int unsigned SN = 16, // slave number
  int unsigned SW = 20  // slave width (address bus width)
)(
  sys_bus_if.s bus_m,          // from master
  sys_bus_if.m bus_s [SN-1:0]  // to   slaves
);

// slave number logarithm
localparam int unsigned SL = $clog2(SN);

logic [SL-1:0]         bus_s_a    ;
logic [SN-1:0]         bus_s_cs   ;
logic [SN-1:0][32-1:0] bus_s_rdata;
logic [SN-1:0]         bus_s_err  ;
logic [SN-1:0]         bus_s_ack  ;

assign bus_s_a  = bus_m.addr[SW+:SL];
assign bus_s_cs = SN'(1) << bus_s_a;

generate
for (genvar i=0; i<SN; i++) begin: for_bus

assign bus_s[i].addr  =               bus_m.addr ;
assign bus_s[i].wdata =               bus_m.wdata;
assign bus_s[i].wen   = bus_s_cs[i] & bus_m.wen  ;
assign bus_s[i].ren   = bus_s_cs[i] & bus_m.ren  ;

assign bus_s_rdata[i] = bus_s[i].rdata;
assign bus_s_err  [i] = bus_s[i].err  ;
assign bus_s_ack  [i] = bus_s[i].ack  ;

end: for_bus
endgenerate

assign bus_m.rdata = bus_s_rdata[bus_s_a];
assign bus_m.err   = bus_s_err  [bus_s_a];
assign bus_m.ack   = bus_s_ack  [bus_s_a];

endmodule: sys_bus_interconnect
