`timescale 1ns / 1ps

module osc_trigger
  #(parameter AXIS_DATA_BITS  = 16,
    parameter TRIG_LEVEL_BITS = 16)(
  input  wire                       clk,
  input  wire                       rst_n,
  // Control
  input  wire                       ctl_rst,
  // Config
  input  wire [TRIG_LEVEL_BITS-1:0] cfg_trig_low_level,
  input  wire [TRIG_LEVEL_BITS-1:0] cfg_trig_high_level,  
  input  wire                       cfg_trig_edge,
  // Trigger
  output wire                       trig,
  // Slave AXI-S 
  input  wire [AXIS_DATA_BITS-1:0]  s_axis_tdata,
  input  wire                       s_axis_tvalid,
  output wire                       s_axis_tready,
  // Master AXI-S
  output reg  [AXIS_DATA_BITS-1:0]  m_axis_tdata,
  output reg                        m_axis_tvalid,
  input  wire                       m_axis_tready
);

////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////// 

reg                               trig_detect;
reg                               trig_detect_p1;
wire                              trig_rising_edge;
wire                              trig_falling_edge;
wire signed [AXIS_DATA_BITS-1:0]  adc_data;
wire signed [TRIG_LEVEL_BITS-1:0] trig_high_level;
wire signed [TRIG_LEVEL_BITS-1:0] trig_low_level;
reg         [AXIS_DATA_BITS-1:0]  axis_tdata_p1;
reg                               axis_tvalid_p1;
reg                               axis_tlast_p1;

assign adc_data           = s_axis_tdata;
assign trig_high_level    = cfg_trig_high_level;
assign trig_low_level     = cfg_trig_low_level;

assign s_axis_tready      = 1;

assign trig_rising_edge   = trig_detect & ~trig_detect_p1 & ~cfg_trig_edge;
assign trig_falling_edge  = ~trig_detect & trig_detect_p1 & cfg_trig_edge;
assign trig               = trig_rising_edge | trig_falling_edge;

always @(posedge clk)
begin
  if (cfg_trig_edge == 0) begin
    if ((adc_data >= trig_high_level) && ((s_axis_tvalid == 1) && (s_axis_tready == 1))) begin
      trig_detect <= 1; 
    end else begin
      trig_detect <= 0; 
    end
  end else begin
    if ((adc_data <= trig_low_level) && ((s_axis_tvalid == 1) && (s_axis_tready == 1))) begin
      trig_detect <= 1; 
    end else begin
      trig_detect <= 0; 
    end  
  end
end

always @(posedge clk)
begin
  trig_detect_p1 <= trig_detect;
end

////////////////////////////////////////////////////////////
// Name : AXI-S Pipeline
// Delays the AXI-S bus by two clocks to account for the 
// delay through the IP block.
//////////////////////////////////////////////////////////// 

always @(posedge clk)
begin
  axis_tdata_p1   <= s_axis_tdata;
  m_axis_tdata    <= axis_tdata_p1;
  
  axis_tvalid_p1  <= s_axis_tvalid;
  m_axis_tvalid   <= axis_tvalid_p1;
end

endmodule