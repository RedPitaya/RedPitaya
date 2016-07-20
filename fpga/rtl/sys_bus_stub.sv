////////////////////////////////////////////////////////////////////////////////
// Module: System bus stub
// Author: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module sys_bus_stub (
  sys_bus_if.s bus
);

assign bus.ack = 1'b1;
assign bus.err = 1'b1;
assign bus.rdata = 'x;

endmodule: sys_bus_stub
