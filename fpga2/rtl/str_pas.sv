////////////////////////////////////////////////////////////////////////////////
// Stream pass through
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module str_pas (
  // control
  input  logic ena,  // enable
  // streams
  str_bus_if.d sti,  // input
  str_bus_if.s sto   // output
);

assign sti.rdy = sto.rdy & ena;

assign sto.vld = sti.vld & ena;
assign sto.dat = sti.dat;
assign sto.kep = sti.kep;
assign sto.lst = sti.lst;

endmodule: str_pas
