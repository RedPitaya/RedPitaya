////////////////////////////////////////////////////////////////////////////////
// Module: analog mixed signals
// Authors: Matej Oblak, Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

module red_pitaya_ams #(
  int unsigned DWC = 24,  // data width for counter
  int unsigned CHN =  4,  // channel number
  int unsigned CHL = $clog2(CHN)
)(
  // PDM 
  output logic [CHN-1:0] [DWC-1:0] pdm_cfg,
  // system bus
  sys_bus_if.s                     bus
);

////////////////////////////////////////////////////////////////////////////////
// system bus connection
////////////////////////////////////////////////////////////////////////////////

// write access
generate
for (genvar i=0; i<CHN; i++) begin: for_chn

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  pdm_cfg[i] <= '0;
end else begin
  if (bus.wen) begin
    if (bus.addr[CHL-1:0]==i)  pdm_cfg[i] <= bus.wdata[DWC-1:0];
  end
end

end: for_chn
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
  bus.rdata <= {{32-DWC{1'b0}}, pdm_cfg[bus.addr[CHL-1:0]]};
end

endmodule: red_pitaya_ams
