////////////////////////////////////////////////////////////////////////////////
// AXI4-Stream interface
// (C) 2015 Iztok Jeras
////////////////////////////////////////////////////////////////////////////////

interface str_if #(
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

// Clock edge alignment
sequence sync_posedge;
   @(posedge clk) 1;
endsequence

clocking cb @(posedge clk);
  input  tready;
  output tvalid;
  output tdata ;
  output tlast ;
  output tkeep ;
endclocking

endinterface: str_if
