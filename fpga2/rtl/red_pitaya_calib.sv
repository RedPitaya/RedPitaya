////////////////////////////////////////////////////////////////////////////////
// Module: calibration
// Authors: Matej Oblak, Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module red_pitaya_calib #(
  int unsigned DWM = 16,   // data width for multiplier (gain)
  int unsigned DWS = 14    // data width for summation (offset)
)(
  // system signals
  input  logic           clk ,  // clock
  input  logic           rstn,  // reset - active low
  // ADC calibration
  output logic signed [2-1:0] [DWM-1:0] adc_cfg_mul,  // gain
  output logic signed [2-1:0] [DWS-1:0] adc_cfg_sum,  // offset
  // DAC calibration
  output logic signed [2-1:0] [DWM-1:0] dac_cfg_mul,  // gain
  output logic signed [2-1:0] [DWS-1:0] dac_cfg_sum,  // offset
  // system bus
  input  logic [ 32-1:0] sys_addr   ,  // bus address
  input  logic [ 32-1:0] sys_wdata  ,  // bus write data
  input  logic [  4-1:0] sys_sel    ,  // bus write byte select
  input  logic           sys_wen    ,  // bus write enable
  input  logic           sys_ren    ,  // bus read enable
  output logic [ 32-1:0] sys_rdata  ,  // bus read data
  output logic           sys_err    ,  // bus error indicator
  output logic           sys_ack       // bus acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

always @(posedge clk)
if (!rstn) begin
  // ADC calibration
  adc_cfg_mul[0] <= 1'b1 <<< (DWM-2);
  adc_cfg_sum[0] <= '0;
  adc_cfg_mul[1] <= 1'b1 <<< (DWM-2);
  adc_cfg_sum[1] <= '0;
  // DAC calibration
  dac_cfg_mul[0] <= 1'b1 <<< (DWM-2);
  dac_cfg_sum[0] <= '0;
  dac_cfg_mul[1] <= 1'b1 <<< (DWM-2);
  dac_cfg_sum[1] <= '0;
end else if (sys_wen) begin
  // ADC calibration
  if (sys_addr[19:0]==20'h40)   adc_cfg_mul[0] <= sys_wdata[DWM-1:0];
  if (sys_addr[19:0]==20'h44)   adc_cfg_sum[0] <= sys_wdata[DWS-1:0];
  if (sys_addr[19:0]==20'h48)   adc_cfg_mul[1] <= sys_wdata[DWM-1:0];
  if (sys_addr[19:0]==20'h4C)   adc_cfg_sum[1] <= sys_wdata[DWS-1:0];
  // DAC calibration
  if (sys_addr[19:0]==20'h50)   dac_cfg_mul[0] <= sys_wdata[DWM-1:0];
  if (sys_addr[19:0]==20'h54)   dac_cfg_sum[0] <= sys_wdata[DWS-1:0];
  if (sys_addr[19:0]==20'h58)   dac_cfg_mul[1] <= sys_wdata[DWM-1:0];
  if (sys_addr[19:0]==20'h5C)   dac_cfg_sum[1] <= sys_wdata[DWS-1:0];
end

wire sys_en;
assign sys_en = sys_wen | sys_ren;

always @(posedge clk)
if (!rstn) begin
  sys_err <= 1'b0;
  sys_ack <= 1'b0;
end else begin
  sys_err <= 1'b0;
  sys_ack <= sys_en;
  casez (sys_addr[5-1:0])
    // ADC calibration
    5'b000??: sys_rdata <= adc_cfg_mul[0];
    5'b001??: sys_rdata <= adc_cfg_sum[0];
    5'b010??: sys_rdata <= adc_cfg_mul[1];
    5'b011??: sys_rdata <= adc_cfg_sum[1];
    // DAC calibration
    5'b100??: sys_rdata <= dac_cfg_mul[0];
    5'b101??: sys_rdata <= dac_cfg_sum[0];
    5'b110??: sys_rdata <= dac_cfg_mul[1];
    5'b111??: sys_rdata <= dac_cfg_sum[1];
  endcase
end

endmodule: red_pitaya_calib
