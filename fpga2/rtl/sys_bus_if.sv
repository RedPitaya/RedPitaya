////////////////////////////////////////////////////////////////////////////////
// Red Pitaya system bus interface
// Author Matej Oblak
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
logic [SW-1:0] sel  ;  // write byte select
logic [AW-1:0] addr ;  // read/write address
logic [DW-1:0] wdata;  // write data
logic [DW-1:0] rdata;  // read data
logic          ack  ;  // acknowledge signal
logic          err  ;  // error indicator

modport m (
  input  clk  ,
  input  rstn ,
  output wen  ,
  output ren  ,
  output sel  ,
  output addr ,
  output wdata,
  input  rdata,
  input  ack  ,
  input  err
);

modport s (
  input  clk  ,
  input  rstn ,
  input  wen  ,
  input  ren  ,
  input  sel  ,
  input  addr ,
  input  wdata,
  output rdata,
  output ack  ,
  output err
);

endinterface: sys_bus_if
