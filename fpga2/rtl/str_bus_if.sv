////////////////////////////////////////////////////////////////////////////////
// streaming bus interface
// Author: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

interface str_bus_if #(
  int unsigned DN = 1,
  type DAT_T = logic signed [8-1:0]
)(
  input  logic clk ,  // clock
  input  logic rstn   // reset - active low
);

// ports
DAT_T [DN-1:0] dat;  // data
logic [DN-1:0] kep;  // keep
logic          lst;  // last
logic          vld;  // valid
logic          rdy;  // ready
// local signal
logic          trn;  // transfer

assign trn = vld & rdy;

// source
modport s (
  // system
  input  clk ,
  input  rstn,
  // ports
  output dat,
  output kep,
  output lst,
  output vld,
  input  rdy,
  // local
  input  trn
);

// drain
modport d (
  // system
  input  clk ,
  input  rstn,
  // ports
  input  dat,
  input  kep,
  input  lst,
  input  vld,
  output rdy,
  // local
  input  trn
);

// monitor
modport m (
  // system
  input  clk ,
  input  rstn,
  // ports
  input  dat,
  input  kep,
  input  lst,
  input  vld,
  input  rdy,
  // local
  input  trn
);

endinterface: str_bus_if
