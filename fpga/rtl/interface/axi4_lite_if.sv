interface axi4_lite_if #(
  int unsigned AW = 32,    // address width
  int unsigned DW = 32,    // data width
  int unsigned SW = DW/8   // select width
)(
  // global signals
  input logic ACLK,
  input logic ARESETn
);

// write address channel
logic [AW-1:0] AWADDR  ;
logic  [3-1:0] AWPROT  ;
logic          AWVALID ;
logic          AWREADY ;
// write dta channel
logic [DW-1:0] WDATA   ;
logic [SW-1:0] WSTRB   ;
logic          WVALID  ;
logic          WREADY  ;
// write response channel
logic  [2-1:0] BRESP   ;
logic          BVALID  ;
logic          BREADY  ;
// read address channel
logic [AW-1:0] ARADDR  ;
logic  [3-1:0] ARPROT  ;
logic          ARVALID ;
logic          ARREADY ;
// read data channel
logic [DW-1:0] RDATA   ;
logic  [2-1:0] RRESP   ;
logic          RVALID  ;
logic          RREADY  ;

// local signals for transfer conditions
logic AWtransfer;
logic  Wtransfer;
logic  Btransfer;
logic ARtransfer;
logic  Rtransfer;

// transfer conditions
assign AWtransfer = AWVALID & AWREADY;
assign  Wtransfer =  WVALID &  WREADY;
assign  Btransfer =  BVALID &  BREADY;
assign ARtransfer = ARVALID & ARREADY;
assign  Rtransfer =  RVALID &  RREADY;

// master port
modport m (
  input  ACLK    ,
  input  ARESETn ,
  output AWADDR  ,
  output AWPROT  ,
  output AWVALID ,
  input  AWREADY ,
  output WDATA   ,
  output WSTRB   ,
  output WVALID  ,
  input  WREADY  ,
  input  BRESP   ,
  input  BVALID  ,
  output BREADY  ,
  output ARADDR  ,
  output ARPROT  ,
  output ARVALID ,
  input  ARREADY ,
  input  RDATA   ,
  input  RRESP   ,
  input  RVALID  ,
  output RREADY
);

// slave port
modport s (
  input  ACLK    ,
  input  ARESETn ,
  input  AWADDR  ,
  input  AWPROT  ,
  input  AWVALID ,
  output AWREADY ,
  input  WDATA   ,
  input  WSTRB   ,
  input  WVALID  ,
  output WREADY  ,
  output BRESP   ,
  output BVALID  ,
  input  BREADY  ,
  input  ARADDR  ,
  input  ARPROT  ,
  input  ARVALID ,
  output ARREADY ,
  output RDATA   ,
  output RRESP   ,
  output RVALID  ,
  input  RREADY
);

endinterface: axi4_lite_if
