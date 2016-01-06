////////////////////////////////////////////////////////////////////////////////
// Module: calibration
// Authors: Matej Oblak, Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module red_pitaya_calib #(
  int unsigned DWM = 16,   // data width for multiplier (gain)
  int unsigned DWS = 14    // data width for summation (offset)
)(
  // ADC calibration
  output logic signed [2-1:0] [DWM-1:0] adc_cfg_mul,  // gain
  output logic signed [2-1:0] [DWS-1:0] adc_cfg_sum,  // offset
  // DAC calibration
  output logic signed [2-1:0] [DWM-1:0] dac_cfg_mul,  // gain
  output logic signed [2-1:0] [DWS-1:0] dac_cfg_sum,  // offset
  // system bus
  sys_bus_if.s                          bus
);

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

always @(posedge bus.clk)
if (!bus.rstn) begin
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
end else if (bus.wen) begin
  // ADC calibration
  if (bus.addr[19:0]==20'h40)   adc_cfg_mul[0] <= bus.wdata[DWM-1:0];
  if (bus.addr[19:0]==20'h44)   adc_cfg_sum[0] <= bus.wdata[DWS-1:0];
  if (bus.addr[19:0]==20'h48)   adc_cfg_mul[1] <= bus.wdata[DWM-1:0];
  if (bus.addr[19:0]==20'h4C)   adc_cfg_sum[1] <= bus.wdata[DWS-1:0];
  // DAC calibration
  if (bus.addr[19:0]==20'h50)   dac_cfg_mul[0] <= bus.wdata[DWM-1:0];
  if (bus.addr[19:0]==20'h54)   dac_cfg_sum[0] <= bus.wdata[DWS-1:0];
  if (bus.addr[19:0]==20'h58)   dac_cfg_mul[1] <= bus.wdata[DWM-1:0];
  if (bus.addr[19:0]==20'h5C)   dac_cfg_sum[1] <= bus.wdata[DWS-1:0];
end

always @(posedge bus.clk)
if (!bus.rstn)  bus.err <= 1'b1;
else            bus.err <= 1'b0;

wire sys_en;
assign sys_en = bus.wen | bus.ren;

always @(posedge bus.clk)
if (!bus.rstn) begin
  bus.ack <= 1'b0;
end else begin
  bus.ack <= sys_en;
  casez (bus.addr[5-1:0])
    // ADC calibration
    5'b000??: bus.rdata <= adc_cfg_mul[0];
    5'b001??: bus.rdata <= adc_cfg_sum[0];
    5'b010??: bus.rdata <= adc_cfg_mul[1];
    5'b011??: bus.rdata <= adc_cfg_sum[1];
    // DAC calibration
    5'b100??: bus.rdata <= dac_cfg_mul[0];
    5'b101??: bus.rdata <= dac_cfg_sum[0];
    5'b110??: bus.rdata <= dac_cfg_mul[1];
    5'b111??: bus.rdata <= dac_cfg_sum[1];
  endcase
end

endmodule: red_pitaya_calib
