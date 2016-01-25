////////////////////////////////////////////////////////////////////////////////
// Module: IO and stream multiplexer control
// Authors: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module muxctl #(
  int unsigned DW = 8 // data width
)(
  // expansion connector
  output logic [01-1:0] digital_loop,
  output logic [16-1:0] gpio_mux,
  // system bus
  sys_bus_if.s          bus
);

// bus decoder width
localparam int unsigned BDW = 4;

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  digital_loop <= '0;
  gpio_mux     <= '0;
end else if (bus.wen) begin
  if (bus.addr[BDW-1:0]=='h00)   digital_loop <= bus.wdata[DW-1:0];
  if (bus.addr[BDW-1:0]=='h04)   gpio_mux     <= bus.wdata[DW-1:0];
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
    'h00:  bus.rdata <= 32'(digital_loop);
    'h04:  bus.rdata <= 32'(gpio_mux    );
    default: bus.rdata <= '0;
  endcase
end

endmodule: muxctl
