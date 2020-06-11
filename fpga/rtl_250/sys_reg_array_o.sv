////////////////////////////////////////////////////////////////////////////////
// Module: generic register set array
// Authors: Matej Oblak, Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module sys_reg_array_o #(
  type RT = logic [32-1:0],     // register data type
  int unsigned RN = 4,          // channel number
  int unsigned RL = $clog2(RN)
)(
  // PDM 
  output RT [RN-1:0] val,
  // system bus
  sys_bus_if.s       bus
);

localparam int unsigned DW = $bits(RT);

////////////////////////////////////////////////////////////////////////////////
// system bus connection
////////////////////////////////////////////////////////////////////////////////

// write access
generate
for (genvar i=0; i<RN; i++) begin: for_rn

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  val[i] <= '0;
end else begin
  if (bus.wen) begin
    if (bus.addr[2+:RL]==i)  val[i] <= bus.wdata[DW-1:0];
  end
end

end: for_rn
endgenerate

// control signals
logic sys_en;
assign sys_en = bus.wen | bus.ren;

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  bus.err <= 1'b1;
  bus.ack <= 1'b0;
end else begin
  bus.err <= 1'b0;
  bus.ack <= sys_en;
end

// read access
always_ff @(posedge bus.clk)
if (bus.ren) begin
  bus.rdata <= {{32-DW{1'b0}}, val[bus.addr[2+:RL]]};
end

endmodule: sys_reg_array_o
