////////////////////////////////////////////////////////////////////////////////
// system bus interface
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

interface sys_bus_if #(
  int unsigned DW = 32  ,  // data width (8,16,...,1024)
  int unsigned AW = 32  ,  // address width
  int unsigned SW = DW/8   // select width - 1 bit for every data byte
)(
  input  logic clk ,  // clock
  input  logic rstn   // reset - active low
);

// bus protocol signals
logic          wen  ;  // write enable
logic          ren  ;  // read enable
logic [AW-1:0] addr ;  // read/write address
logic [DW-1:0] wdata;  // write data
logic [DW-1:0] rdata;  // read data
logic          ack  ;  // acknowledge signal
logic          err  ;  // error indicator

// master
modport m (
  input  clk  ,
  input  rstn ,
  output wen  ,
  output ren  ,
  output addr ,
  output wdata,
  input  rdata,
  input  ack  ,
  input  err
);

// slave
modport s (
  input  clk  ,
  input  rstn ,
  input  wen  ,
  input  ren  ,
  input  addr ,
  input  wdata,
  output rdata,
  output ack  ,
  output err
);

endinterface: sys_bus_if
