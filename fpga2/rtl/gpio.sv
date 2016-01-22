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

localparam int unsigned AW = 3;

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
assign bus.AWREADY = AW_syn & ~bus.BVALID;
assign bus.WREADY  = AW_syn & ~bus.BVALID;

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
else               bus.BVALID <= AW_trn | (bus.BVALID & ~B_trn);

////////////////////////////////////////////////////////////////////////////////
// read access
////////////////////////////////////////////////////////////////////////////////

logic AR_trn;
logic  R_trn;

assign AR_trn = bus.ARVALID & bus.ARREADY;
assign  R_trn = bus.RVALID  & bus.RREADY ;

// AR - ready
// TODO: better performance should be supported
assign bus.ARREADY = ~bus.RVALID;

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
else               bus.RVALID <= AR_trn | (bus.RVALID & ~R_trn);

endmodule: gpio
