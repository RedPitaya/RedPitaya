interface axi_bus_if #(
  parameter AW = 32,
  parameter DW = 32,
  parameter SW = DW/8,
  parameter IW = 1
)(
  // global signals
  input logic ACLK,
  input logic ARESETn
);

// write address channel
logic [IW-1:0] AWID    ;
logic [AW-1:0] AWADDR  ;
logic    [3:0] AWREGION;
logic    [7:0] AWLEN   ;
logic    [2:0] AWSIZE  ;
logic    [1:0] AWBURST ;
logic          AWLOCK  ;
logic    [3:0] AWCACHE ;
logic    [2:0] AWPROT  ;
logic    [3:0] AWQOS   ;
logic          AWVALID ;
logic          AWREADY ;
// write dta channel
logic [DW-1:0] WDATA   ;
logic [SW-1:0] WSTRB   ;
logic          WLAST   ;
logic          WVALID  ;
logic          WREADY  ;
// write response channel
logic [IW-1:0] BID     ;
logic    [1:0] BRESP   ;
logic          BVALID  ;
logic          BREADY  ;
// read address channel
logic [IW-1:0] ARID    ;
logic [AW-1:0] ARADDR  ;
logic    [3:0] ARREGION;
logic    [7:0] ARLEN   ;
logic    [2:0] ARSIZE  ;
logic    [1:0] ARBURST ;
logic          ARLOCK  ;
logic    [3:0] ARCACHE ;
logic    [2:0] ARPROT  ;
logic    [3:0] ARQOS   ;
logic          ARVALID ;
logic          ARREADY ;
// read data channel
logic [IW-1:0] RID     ;
logic [DW-1:0] RDATA   ;
logic    [1:0] RRESP   ;
logic          RLAST   ;
logic          RVALID  ;
logic          RREADY  ;

modport m (
  output AWID    ,
  output AWADDR  ,
  output AWREGION,
  output AWLEN   ,
  output AWSIZE  ,
  output AWBURST ,
  output AWLOCK  ,
  output AWCACHE ,
  output AWPROT  ,
  output AWQOS   ,
  output AWVALID ,
  input  AWREADY ,
  output WDATA   ,
  output WSTRB   ,
  output WLAST   ,
  output WVALID  ,
  input  WREADY  ,
  input  BID     ,
  input  BRESP   ,
  input  BVALID  ,
  output BREADY  ,
  output ARID    ,
  output ARADDR  ,
  output ARREGION,
  output ARLEN   ,
  output ARSIZE  ,
  output ARBURST ,
  output ARLOCK  ,
  output ARCACHE ,
  output ARPROT  ,
  output ARQOS   ,
  output ARVALID ,
  input  ARREADY ,
  input  RID     ,
  input  RDATA   ,
  input  RRESP   ,
  input  RLAST   ,
  input  RVALID  ,
  output RREADY
);

modport s (
  input  AWID    ,
  input  AWADDR  ,
  input  AWREGION,
  input  AWLEN   ,
  input  AWSIZE  ,
  input  AWBURST ,
  input  AWLOCK  ,
  input  AWCACHE ,
  input  AWPROT  ,
  input  AWQOS   ,
  input  AWVALID ,
  output AWREADY ,
  input  WDATA   ,
  input  WSTRB   ,
  input  WLAST   ,
  input  WVALID  ,
  output WREADY  ,
  output BID     ,
  output BRESP   ,
  output BVALID  ,
  input  BREADY  ,
  input  ARID    ,
  input  ARADDR  ,
  input  ARREGION,
  input  ARLEN   ,
  input  ARSIZE  ,
  input  ARBURST ,
  input  ARLOCK  ,
  input  ARCACHE ,
  input  ARPROT  ,
  input  ARQOS   ,
  input  ARVALID ,
  output ARREADY ,
  output RID     ,
  output RDATA   ,
  output RRESP   ,
  output RLAST   ,
  output RVALID  ,
  input  RREADY
);

endinterface: axi_bus_if
