////////////////////////////////////////////////////////////////////////////////
// Module: GPIO
// Authors: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module gpio #(
  int unsigned DW = 8 // data width
)(
  // expansion connector
  output logic [DW-1:0] gpio_e,  // output enable
  output logic [DW-1:0] gpio_o,  // output
  input  logic [DW-1:0] gpio_i,  // input
  // system bus
  axi4_lite_if.s        bus
);

localparam int unsigned AW = 4;

localparam AXI_DW =  32        ; // data width (8,16,...,1024)
localparam AXI_AW =  32        ; // address width
localparam AXI_SW = AXI_DW >> 3; // strobe width - 1 bit for every data byte

// RP system read/write channel
logic [ AXI_AW-1: 0] bus_addr ;  //!< system bus read/write address.
logic [ AXI_DW-1: 0] bus_wdata;  //!< system bus write data.
logic [ AXI_SW-1: 0] bus_sel  ;  //!< system bus write byte select.
logic                bus_wen  ;  //!< system bus write enable.
logic                bus_ren  ;  //!< system bus read enable.
logic [ AXI_DW-1: 0] bus_rdata;  //!< system bus read data.
logic                bus_err  ;  //!< system bus error indicator.
logic                bus_ack  ;  //!< system bus acknowledge signal.

axi4_lite_slave axi4_lite_slave (
  .axi_clk_i      (bus.ACLK   ),
  .axi_rstn_i     (bus.ARESETn),
  .axi_awaddr_i   (bus.AWADDR ),
  .axi_awprot_i   (bus.AWPROT ),
  .axi_awvalid_i  (bus.AWVALID),
  .axi_awready_o  (bus.AWREADY),
  .axi_wdata_i    (bus.WDATA  ),
  .axi_wstrb_i    (bus.WSTRB  ),
  .axi_wvalid_i   (bus.WVALID ),
  .axi_wready_o   (bus.WREADY ),
  .axi_bresp_o    (bus.BRESP  ),
  .axi_bvalid_o   (bus.BVALID ),
  .axi_bready_i   (bus.BREADY ),
  .axi_araddr_i   (bus.ARADDR ),
  .axi_arprot_i   (bus.ARPROT ),
  .axi_arvalid_i  (bus.ARVALID),
  .axi_arready_o  (bus.ARREADY),
  .axi_rdata_o    (bus.RDATA  ),
  .axi_rresp_o    (bus.RRESP  ),
  .axi_rvalid_o   (bus.RVALID ),
  .axi_rready_i   (bus.RREADY ),
  // RP system read/write channel
  .sys_addr_o     (bus_addr ),
  .sys_wdata_o    (bus_wdata),
  .sys_sel_o      (bus_sel  ),
  .sys_wen_o      (bus_wen  ),
  .sys_ren_o      (bus_ren  ),
  .sys_rdata_i    (bus_rdata),
  .sys_err_i      (bus_err  ),
  .sys_ack_i      (bus_ack  )
);

localparam int unsigned BDW = 6;

always_ff @(posedge bus.ACLK)
if (!bus.ARESETn) begin
  gpio_o <= '0;
  gpio_e <= '0;
end else if (bus_wen) begin
  if (bus_addr[BDW-1:0]=='h00)   gpio_e <= bus_wdata[DW-1:0];
  if (bus_addr[BDW-1:0]=='h04)   gpio_o <= bus_wdata[DW-1:0];
end

always_ff @(posedge bus.ACLK)
if (!bus.ARESETn)  bus_err <= 1'b1;
else            bus_err <= 1'b0;

logic sys_en;
assign sys_en = bus_wen | bus_ren;

always_ff @(posedge bus.ACLK)
if (!bus.ARESETn) begin
  bus_ack <= 1'b0;
end else begin
  bus_ack <= sys_en;
  casez (bus_addr[BDW-1:0])
    // GPIO
    'h00:  bus_rdata <= {{32-DW{1'b0}}, gpio_e};
    'h04:  bus_rdata <= {{32-DW{1'b0}}, gpio_o};
    'h08:  bus_rdata <= {{32-DW{1'b0}}, gpio_i};
    default: bus_rdata <= '0;
  endcase
end



/*

////////////////////////////////////////////////////////////////////////////////
// write access
////////////////////////////////////////////////////////////////////////////////

logic AW_syn;
logic AW_trn;
logic  W_trn;
logic  B_trn;

assign AW_trn = bus.AWVALID & bus.AWREADY;
assign  W_trn = bus.WVALID  & bus.WREADY ;
assign  B_trn = bus.BVALID  & bus.BREADY ;

// AW - W - synchronization
// TODO: this does not offer full performance
// a different sync scheme is needed
assign AW_syn = bus.AWVALID & bus.WVALID;

// AW - W - ready
// TODO: better performance should be supported
assign bus.AWREADY = bus.BVALID;
assign bus.WREADY  = bus.BVALID;

// W - data
always_ff @(posedge bus.ACLK)
if (!bus.ARESETn) begin
  gpio_o <= '0;
  gpio_e <= '0;
end else if (AW_trn & W_trn) begin
  if (AW'(bus.AWADDR)=='h00)  gpio_e <= DW'(bus.WDATA);
  if (AW'(bus.AWADDR)=='h04)  gpio_o <= DW'(bus.WDATA);
end

// B - response
// TODO: error response in reset state
assign bus.BRESP = 2'b00;

// B - valid
always_ff @(posedge bus.ACLK)
if (~bus.ARESETn)  bus.BVALID <= 1'b0;
else               bus.BVALID <= (AW_syn & ~bus.BVALID) | (bus.BVALID & ~bus.BREADY);

////////////////////////////////////////////////////////////////////////////////
// read access
////////////////////////////////////////////////////////////////////////////////

logic AR_trn;
logic  R_trn;

assign AR_trn = bus.ARVALID & bus.ARREADY;
assign  R_trn = bus.RVALID  & bus.RREADY ;

// AR - ready
// TODO: better performance should be supported
assign bus.ARREADY = bus.RVALID;

// R - data
always_ff @(posedge bus.ACLK)
if (AR_trn) begin
  case (AW'(bus.ARADDR))
       'h00: bus.RDATA <= 32'(gpio_e);
       'h04: bus.RDATA <= 32'(gpio_o);
       'h08: bus.RDATA <= 32'(gpio_i);
    default: bus.RDATA <= '0;
  endcase
end

// R - response
// TODO: error response in reset state
assign bus.RRESP = 2'b00;

// R - valid
always_ff @(posedge bus.ACLK)
if (~bus.ARESETn)  bus.RVALID <= 1'b0;
else               bus.RVALID <= (bus.ARVALID & ~bus.RVALID) | (bus.RVALID & ~bus.RREADY);

*/

endmodule: gpio
