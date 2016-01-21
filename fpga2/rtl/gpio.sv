////////////////////////////////////////////////////////////////////////////////
// Module: GPIO
// Authors: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module gpio #(
  int unsigned DW = 8 // data width
)(
  // expansion connector
  input  logic [DW-1:0] gpio_i,  // input
  output logic [DW-1:0] gpio_o,  // output
  output logic [DW-1:0] gpio_e,  // output enable
  // system bus
  axi4_lite_if.s        bus
);

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  gpio_o <= '0;
  gpio_e <= '0;
end else if (bus.wen) begin
  if (bus.addr[BDW-1:0]=='h00)   gpio_e <= bus.wdata[DW-1:0];
  if (bus.addr[BDW-1:0]=='h04)   gpio_o <= bus.wdata[DW-1:0];
end

always_ff @(posedge bus.clk)
if (!bus.rstn)  bus.err <= 1'b1;
else            bus.err <= 1'b0;

logic sys_en;
assign sys_en = bus.wen | bus.ren;

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  bus.ack <= 1'b0;
end else begin
  bus.ack <= sys_en;
  casez (bus.addr)
    'h00:  bus.rdata <= DW'(gpio_e);
    'h04:  bus.rdata <= DW'(gpio_o);
    'h08:  bus.rdata <= DW'(gpio_i);
    default: bus.rdata <= '0;
  endcase
end

endmodule: gpio
