////////////////////////////////////////////////////////////////////////////////
// Module: management
// Authors: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module mgmt #(
  int unsigned GW = 8
)(
  // GPIO mode
  output logic  [GW-1:0] cfg_iom,
  // stream multiplexers
  output logic   [2-1:0] cfg_loop,
  // system bus
  sys_bus_if.s           bus
);

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  bus.err <= 1'b0;
  bus.ack <= bus.wen | bus.ren;
end

localparam int unsigned BAW=4;

// write access
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  // GPIO mode
  cfg_iom <= '0;
  // stream multiplexers
  cfg_loop <= '0;
end else begin
  if (bus.wen) begin
    // GPIO mode
    if (bus.addr[BAW-1:0]=='h00)  cfg_iom <= bus.wdata;
    // stream multiplexers
    if (bus.addr[BAW-1:0]=='h04)  cfg_loop <= bus.wdata;
  end
end

// read access
always_ff @(posedge bus.clk)
casez (bus.addr[BAW-1:0])
  // GPIO mode
  'h00: bus.rdata <= cfg_iom;
  // stream multiplexers
  'h04: bus.rdata <= cfg_loop;
  // default
  default: bus.rdata <= 'x;
endcase

endmodule: mgmt
