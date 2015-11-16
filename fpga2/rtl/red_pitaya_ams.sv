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
  // ADC
  input  logic          clk ,  // clock
  input  logic          rstn,  // reset - active low
  // PDM 
  output logic [CHN-1:0] [DWC-1:0] pdm_cfg,
  // system bus
  input  logic [32-1:0] sys_addr ,  // bus address
  input  logic [32-1:0] sys_wdata,  // bus write data
  input  logic [ 4-1:0] sys_sel  ,  // bus write byte select
  input  logic          sys_wen  ,  // bus write enable
  input  logic          sys_ren  ,  // bus read enable
  output logic [32-1:0] sys_rdata,  // bus read data
  output logic          sys_err  ,  // bus error indicator
  output logic          sys_ack     // bus acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
// system bus connection
////////////////////////////////////////////////////////////////////////////////

// write access
generate
for (genvar i=0; i<CHN; i++) begin: for_chn

always_ff @(posedge clk)
if (!rstn) begin
  pdm_cfg[i] <= '0;
end else begin
  if (sys_wen) begin
    if (sys_addr[CHL-1:0]==i)  pdm_cfg[i] <= sys_wdata[DWC-1:0];
  end
end

end: for_chn
endgenerate

// control signals
logic sys_en;
assign sys_en = sys_wen | sys_ren;

always_ff @(posedge clk)
if (!rstn) begin
  sys_err <= 1'b1;
  sys_ack <= 1'b0;
end else begin
  sys_err <= 1'b0;
  sys_ack <= sys_en;
end

// read access
always_ff @(posedge clk)
if (sys_ren) begin
  sys_rdata <= {{32-DWC{1'b0}}, pdm_cfg[sys_addr[CHL-1:0]]};
end

endmodule: red_pitaya_ams
