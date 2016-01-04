////////////////////////////////////////////////////////////////////////////////
// streaming bus interface
// Author: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

interface str_bus_if #(
  parameter type DAT_T = logic signed [8-1:0]
)(
  input  logic clk ,  // clock
  input  logic rstn   // reset - active low
);

DAT_T dat;  // data
logic lst;  // last
logic vld;  // valid
logic rdy;  // ready

// source
modport s (
  input  clk ,
  input  rstn,
  output dat ,
  output lst ,
  output vld ,
  input  rdy
);

// drain
modport d (
  input  clk ,
  input  rstn,
  input  dat ,
  input  lst ,
  input  vld ,
  output rdy
);

// monitor
modport m (
  input  clk ,
  input  rstn,
  input  dat ,
  input  lst ,
  input  vld ,
  input  rdy
);

endinterface: str_bus_if
