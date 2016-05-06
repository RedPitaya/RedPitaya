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
  // interrupt
  output logic          irq,
  // system bus
  sys_bus_if.s          bus
);

// bus decoder width
localparam int unsigned BDW = 4;

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
  casez (bus.addr[BDW-1:0])
    // GPIO
    'h00:  bus.rdata <= {{32-DW{1'b0}}, gpio_e};
    'h04:  bus.rdata <= {{32-DW{1'b0}}, gpio_o};
    'h08:  bus.rdata <= {{32-DW{1'b0}}, gpio_i};
    default: bus.rdata <= '0;
  endcase
end

// TODO
assign irq = 1'b0;

endmodule: gpio
