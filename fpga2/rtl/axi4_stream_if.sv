////////////////////////////////////////////////////////////////////////////////
// AXI4-Stream interface
// Author: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

interface axi4_stream_if #(
  int unsigned DN = 1,
  type DT = logic signed [8-1:0]
)(
  input  logic ACLK   ,  // clock
  input  logic ARESETn   // reset - active low
);

DT    [DN-1:0] TDATA ;  // data
logic [DN-1:0] TKEEP ;  // keep
logic          TLAST ;  // last
logic          TVALID;  // valid
logic          TREADY;  // ready

logic          transf;

assign transf = TVALID & TREADY;

// source
modport s (
  input  ACLK   ,
  input  ARESETn,
  output TDATA ,
  output TKEEP ,
  output TLAST ,
  output TVALID,
  input  TREADY,
  input  transf
);

// drain
modport d (
  input  ACLK   ,
  input  ARESETn,
  input  TDATA ,
  input  TKEEP ,
  input  TLAST ,
  input  TVALID,
  output TREADY,
  input  transf
);

// monitor
modport m (
  input  ACLK   ,
  input  ARESETn,
  input  TDATA ,
  input  TKEEP ,
  input  TLAST ,
  input  TVALID,
  input  TREADY,
  input  transf
);

endinterface: axi4_stream_if
