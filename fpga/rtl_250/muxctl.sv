////////////////////////////////////////////////////////////////////////////////
// Module: IO and stream multiplexer control
// Authors: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module muxctl #(
  int unsigned DW = 8 // data width
)(
  output logic  [2-1:0] mux_loop,
  output logic  [2-1:0] mux_gen ,
  output logic  [1-1:0] mux_lg  ,
  // system bus
  sys_bus_if.s          bus
);

// bus decoder width
localparam int unsigned BDW = 4;

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  mux_loop <= '0;
  mux_gen  <= '0;
  mux_lg   <= '0;
end else if (bus.wen) begin
  if (bus.addr[BDW-1:0]=='h00)   mux_loop <= bus.wdata;
  if (bus.addr[BDW-1:0]=='h04)   mux_gen  <= bus.wdata;
  if (bus.addr[BDW-1:0]=='h08)   mux_lg   <= bus.wdata;
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
    'h00:  bus.rdata <= 32'(mux_loop);
    'h04:  bus.rdata <= 32'(mux_gen );
    'h08:  bus.rdata <= 32'(mux_lg  );
    default: bus.rdata <= '0;
  endcase
end

endmodule: muxctl
