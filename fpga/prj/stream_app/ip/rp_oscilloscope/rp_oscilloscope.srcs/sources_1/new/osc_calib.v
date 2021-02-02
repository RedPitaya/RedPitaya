`timescale 1ns / 1ps

module osc_calib
  #(parameter AXIS_DATA_BITS = 16)(
  input  wire                       clk,
  // Slave AXI-S
  input  wire [AXIS_DATA_BITS-1:0]  s_axis_tdata,
  input  wire                       s_axis_tvalid,
  output wire                       s_axis_tready,
  // Master AXI-S
  output reg  [AXIS_DATA_BITS-1:0]  m_axis_tdata,
  output reg                        m_axis_tvalid,
  input  wire                       m_axis_tready,
  // Config
  input  wire [15:0]                cfg_calib_offset, 
  input  wire [15:0]                cfg_calib_gain  
);

localparam CALC1_BITS = AXIS_DATA_BITS+1;
localparam CALC2_BITS = (2*AXIS_DATA_BITS-2); //1 bit sign, 1 bit before dec point
localparam CALC3_BITS = 2*CALC2_BITS+2;
localparam CALC_MAX   = (2**(AXIS_DATA_BITS-1))-1;
localparam CALC_MIN   = -(2**(AXIS_DATA_BITS-1));

localparam C_START    = 16;
localparam C_END      = C_START+16;

reg signed [AXIS_DATA_BITS-1:0]  adc_data, adc_data_reg;
reg signed [15:0]                offset, offset_reg;
reg signed [15:0]                gain, gain_reg;     

reg  signed [    CALC1_BITS-1:0]  offset_calc;
wire                              offs_max, offs_min;
wire signed [AXIS_DATA_BITS-1:0]  offset_calc_limit;

reg  signed [    CALC3_BITS-1:0]  gain_calc;
reg  signed [    CALC3_BITS-1:0]  gain_calc_r;
wire                              gain_max, gain_min;
wire signed [AXIS_DATA_BITS-1:0]  gain_calc_limit;

reg                               s_axis_tvalid_p1;
reg                               s_axis_tvalid_p2;


////////////////////////////////////////////////////////////
// Input data registration
// 
////////////////////////////////////////////////////////////
always @(posedge clk)
begin
  adc_data_reg <= s_axis_tdata;
  adc_data <= adc_data_reg;

  offset_reg <= cfg_calib_offset;
  offset <= offset_reg;

  gain_reg <= cfg_calib_gain;
  gain <= gain_reg;
end
/*
assign adc_data = s_axis_tdata;
assign offset   = cfg_calib_offset;
assign gain     = cfg_calib_gain;
*/
////////////////////////////////////////////////////////////
// Name : Gain Calculation
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  gain_calc_r <= $signed({offset_calc_limit,{15{1'b0}}}) * $signed({{15{1'b0}},gain});
  gain_calc   <= gain_calc_r; // output of multiplier needs to be registered to avoid timing issues
end

assign gain_max = (gain_calc[46:45] == 2'b01);
assign gain_min = (gain_calc[46:45] == 2'b10);

assign gain_calc_limit = gain_max ? CALC_MAX : (gain_min ? CALC_MIN : gain_calc[(CALC3_BITS-C_START-1):((CALC3_BITS)-C_END)]);

////////////////////////////////////////////////////////////
// Name : Offset Calculation
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  offset_calc <= $signed(adc_data) + $signed(offset);  
end

assign offs_max = (offset_calc[16:15] == 2'b01);
assign offs_min = (offset_calc[16:15] == 2'b10);

assign offset_calc_limit = offs_max ? CALC_MAX : (offs_min ? CALC_MIN : offset_calc);

////////////////////////////////////////////////////////////
// Name : Master AXI-S TDATA
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  m_axis_tdata <= gain_calc_limit[AXIS_DATA_BITS-1:0];  
end

////////////////////////////////////////////////////////////
// Name : Master AXI-S TVALID
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  s_axis_tvalid_p1  <= s_axis_tvalid;
  s_axis_tvalid_p2  <= s_axis_tvalid_p1;
  m_axis_tvalid     <= s_axis_tvalid_p2;  
end

assign s_axis_tready  = 1;
   
endmodule