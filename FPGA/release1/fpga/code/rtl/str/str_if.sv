////////////////////////////////////////////////////////////////////////////////
// AXI4-Stream interface
// (C) 2015 Iztok Jeras
////////////////////////////////////////////////////////////////////////////////

interface str_if #(
  // stream parameter
  int unsigned DN = 1,
  int unsigned DW = 8
)(
  input  logic clk,
  input  logic rst
);

logic                  tready;
logic                  tvalid;
logic                  tlast ;
logic [DN-1:0][DW-1:0] tdata ;
logic [DN-1:0]         tkeep ;

// stream source
modport src (
  // system signals
  input  clk,
  input  rst,
  // stream
  input  tready,
  output tvalid,
  output tlast ,
  output tdata ,
  output tkeep
);

// stream drain
modport drn (
  // system signals
  input  clk,
  input  rst,
  // stream
  output tready,
  input  tvalid,
  input  tlast ,
  input  tdata ,
  input  tkeep
);

endinterface: str_if
