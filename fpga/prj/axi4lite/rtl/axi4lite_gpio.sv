`timescale 1 ns / 1 ps

module axi4lite_gpio #(
  int unsigned DW = 32,  // Width of AXI data    bus
  int unsigned AW = 4    // Width of AXI address bus
)(
  // Users to add ports here
  output logic [DW-1:0] gpio_o,
  output logic [DW-1:0] gpio_t,
  input  logic [DW-1:0] gpio_i,
  // interrupt
  output logic          irq,
  // User ports ends
  axi4_lite_if.s        bus
);

localparam int unsigned ADDR_LSB = $clog2(DW/8);

// AXI4LITE signals
logic [AW-1:ADDR_LSB] axi_awaddr;
logic [AW-1:ADDR_LSB] axi_araddr;

// Example-specific design signals
// local parameter for addressing 32 bit / 64 bit DW
//----------------------------------------------
//-- Signals for user logic register space example
//------------------------------------------------

logic slv_reg_rden;
logic slv_reg_wren;

logic AWtransfer;
logic  Wtransfer;
logic  Btransfer;
logic ARtransfer;
logic  Rtransfer;

assign AWtransfer = bus.AWVALID & bus.AWREADY;
assign  Wtransfer =  bus.WVALID &  bus.WREADY;
assign  Btransfer =  bus.BVALID &  bus.BREADY;
assign ARtransfer = bus.ARVALID & bus.ARREADY;
assign  Rtransfer =  bus.RVALID &  bus.RREADY;

// Implement AWREADY generation
// AWREADY is asserted for one ACLK clock cycle when both
// AWVALID and WVALID are asserted. AWREADY is
// de-asserted when reset is low.

// slave is ready to accept write address when 
// there is a valid write address and write data
// on the write address and data bus. This design 
// expects no outstanding transactions.
// TODO: implement pipelining
always_ff @(posedge bus.ACLK)
if (~bus.ARESETn)  bus.AWREADY <= 1'b0;
else               bus.AWREADY <= ~bus.AWREADY & bus.AWVALID & bus.WVALID;

// Implement axi_awaddr latching
// This process is used to latch the address when both 
// AWVALID and WVALID are valid. 

// Write Address latching 
// TODO: remove reset
// TODO: combine control signal
always_ff @(posedge bus.ACLK)
if (~bus.AWREADY & bus.AWVALID & bus.WVALID)  axi_awaddr <= bus.AWADDR [AW-1:ADDR_LSB];

// Implement WREADY generation
// WREADY is asserted for one ACLK clock cycle when both
// AWVALID and WVALID are asserted. WREADY is 
// de-asserted when reset is low. 

// slave is ready to accept write data when 
// there is a valid write address and write data
// on the write address and data bus. This design 
// expects no outstanding transactions. 
always_ff @(posedge bus.ACLK)
if (~bus.ARESETn)  bus.WREADY <= 1'b0;
else               bus.WREADY <= ~bus.WREADY & bus.WVALID & bus.AWVALID;

// Implement memory mapped register select and write logic generation
// The write data is accepted and written to memory mapped registers when
// AWREADY, WVALID, WREADY and WVALID are asserted. Write strobes are used to
// select byte enables of slave registers while writing.
// These registers are cleared when reset (active low) is applied.
// Slave register write enable is asserted when valid address and data are available
// and the slave is ready to accept the write address and write data.

//assign slv_reg_wren = bus.Wtransfer & bus.AWtransfer;
assign slv_reg_wren = Wtransfer & AWtransfer;

always_ff @(posedge bus.ACLK)
if (bus.ARESETn == 1'b0) begin
  gpio_o <= '0;
  gpio_t <= '0;
  irq <= '0;
end else if (slv_reg_wren) begin
  if (axi_awaddr == 2'h0) begin
    for (int unsigned i=0; i<(DW/8); i++) begin
      if (bus.WSTRB[i])  gpio_o[(i*8)+:8] <= bus.WDATA[(i*8)+:8];
    end
  end
  if (axi_awaddr == 2'h1) begin
    for (int unsigned i=0; i<(DW/8); i++) begin
      if (bus.WSTRB[i])  gpio_t[(i*8)+:8] <= bus.WDATA[(i*8)+:8];
    end
  end
  if (axi_awaddr == 2'h3) begin
    irq <= bus.WDATA[0];
  end
end

// Implement write response logic generation
// The write response and response valid signals are asserted by the slave 
// when WREADY, WVALID, WREADY and WVALID are asserted.  
// This marks the acceptance of address and indicates the status of 
// write transaction.

always_ff @(posedge bus.ACLK)
//if (bus.AWtransfer & ~bus.BVALID & bus.Wtransfer) begin
if (AWtransfer & ~bus.BVALID & Wtransfer) begin
  // indicates a valid write response is available
  bus.BRESP  <= 2'b0; // 'OKAY' response 
  // work error responses in future
end

always_ff @(posedge bus.ACLK)
if (bus.ARESETn == 1'b0)  bus.BVALID  <= 0;
else begin    
  //if (bus.AWtransfer & ~bus.BVALID & bus.Wtransfer) begin
  if (AWtransfer & ~bus.BVALID & Wtransfer) begin
    // indicates a valid write response is available
    bus.BVALID <= 1'b1;
  end else if (bus.BREADY & bus.BVALID) begin
    //check if bready is asserted while bvalid is high) 
    //(there is a possibility that bready is always asserted high)   
    bus.BVALID <= 1'b0; 
  end
end

// Implement ARREADY generation
// ARREADY is asserted for one ACLK clock cycle when
// ARVALID is asserted. AWREADY is 
// de-asserted when reset (active low) is asserted. 
// The read address is also latched when ARVALID is 
// asserted. axi_araddr is reset to zero on reset assertion.

// indicates that the slave has acceped the valid read address
always_ff @(posedge bus.ACLK)
if (bus.ARESETn == 1'b0)  bus.ARREADY <= 1'b0;
else                      bus.ARREADY <= ~bus.ARREADY & bus.ARVALID;

// Read address latching
always_ff @(posedge bus.ACLK)
if (~bus.ARREADY & bus.ARVALID)  axi_araddr <= bus.ARADDR [AW-1:ADDR_LSB];

// Implement axi_arvalid generation
// RVALID is asserted for one ACLK clock cycle when both 
// ARVALID and ARREADY are asserted. The slave registers 
// data are available on the RDATA bus at this instance. The 
// assertion of RVALID marks the validity of read data on the 
// bus and RRESP indicates the status of read transaction.RVALID 
// is deasserted on reset (active low). RRESP and RDATA are 
// cleared to zero on reset (active low).  
always_ff @(posedge bus.ACLK)
if (slv_reg_rden) begin
  // Valid read data is available at the read data bus
  bus.RRESP  <= 2'b0; // 'OKAY' response
end

always_ff @(posedge bus.ACLK)
if (bus.ARESETn == 1'b0) begin
  bus.RVALID <= 0;
end else begin    
  if (slv_reg_rden) begin
    // Valid read data is available at the read data bus
    bus.RVALID <= 1'b1;
  //end else if (bus.Rtransfer) begin
  end else if (Rtransfer) begin
    // Read data is accepted by the master
    bus.RVALID <= 1'b0;
  end
end

// Implement memory mapped register select and read logic generation
// Slave register read enable is asserted when valid address is available
// and the slave is ready to accept the read address.
//assign slv_reg_rden = bus.ARtransfer & ~bus.RVALID;
assign slv_reg_rden = ARtransfer & ~bus.RVALID;

// Address decoding for reading registers
// Output register or memory read data
// When there is a valid read address (ARVALID) with 
// acceptance of read address by the slave (ARREADY), 
always_ff @(posedge bus.ACLK)
if (slv_reg_rden) begin
  case (axi_araddr)
    2'h0: bus.RDATA <= gpio_o;
    2'h1: bus.RDATA <= gpio_t;
    2'h2: bus.RDATA <= gpio_i;
    2'h3: bus.RDATA <= irq;
    // NOTE: a default is not really needed, values at address 2'h3 are undefined
    // default: bus.RDATA <= '0;
  endcase
end

endmodule: axi4lite_gpio
