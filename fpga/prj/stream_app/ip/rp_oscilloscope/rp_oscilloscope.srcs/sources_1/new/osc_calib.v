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

localparam CALC_MAX  = (2**(AXIS_DATA_BITS-1))-1;
localparam CALC_MIN  = -(2**(AXIS_DATA_BITS-1));

wire signed [AXIS_DATA_BITS-1:0]  adc_data;
wire signed [15:0]                offset;
wire signed [15:0]                gain;   
reg  signed [CALC1_BITS-1:0]      offset_calc;
reg  signed [AXIS_DATA_BITS-1:0]  offset_calc_limit;
reg  signed [AXIS_DATA_BITS-1:0]  gain_calc;
wire        [AXIS_DATA_BITS-1:0]  gain_uns;
reg                               gain_sign;
reg  signed [2*CALC2_BITS-1:0]    gain_calc_tmp;

reg  signed [AXIS_DATA_BITS-1:0]  gain_calc_limit;
reg                               s_axis_tvalid_p1;
reg                               s_axis_tvalid_p2;

assign adc_data = s_axis_tdata;
assign offset   = cfg_calib_offset;
assign gain     = cfg_calib_gain;
assign s_axis_tready  = 1;
   
////////////////////////////////////////////////////////////
// Name : Offset Calculation
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  offset_calc <= $signed(adc_data) + $signed(offset);  
end

always @(*)
begin
  if (offset_calc > CALC_MAX) begin
    offset_calc_limit = CALC_MAX;
  end else begin
    if (offset_calc < CALC_MIN) begin
      offset_calc_limit = CALC_MIN;
    end else begin
      offset_calc_limit = offset_calc;
    end  
  end
end

////////////////////////////////////////////////////////////
// Name : Gain Calculation
// 
////////////////////////////////////////////////////////////

assign gain_uns = offset_calc_limit[AXIS_DATA_BITS-1] ? -offset_calc_limit : offset_calc_limit;
wire pos_gain = gain_calc_tmp[(2*CALC2_BITS-14-1):((2*CALC2_BITS)-AXIS_DATA_BITS-14)];
wire neg_gain = -gain_calc_tmp[(2*CALC2_BITS-14-1):((2*CALC2_BITS)-AXIS_DATA_BITS-14)];

always @(posedge clk)
begin
  gain_sign     <= offset_calc_limit[AXIS_DATA_BITS-1];
  gain_calc_tmp <= {gain_uns,{15{1'b0}}} * {{15{1'b0}},gain};  
end

always @(*)
begin
  if (gain_sign)
    gain_calc = -gain_calc_tmp[(2*CALC2_BITS-14-1):((2*CALC2_BITS)-AXIS_DATA_BITS-14)];
  else
    gain_calc =  gain_calc_tmp[(2*CALC2_BITS-14-1):((2*CALC2_BITS)-AXIS_DATA_BITS-14)];

  if   (( gain_calc_tmp[(2*CALC2_BITS-1):((2*CALC2_BITS)-30)] > CALC_MAX)   && ~gain_sign) begin
      gain_calc_limit = CALC_MAX;
  end else  begin
    if ((-gain_calc_tmp[(2*CALC2_BITS-1):((2*CALC2_BITS)-30)]-1 < CALC_MIN) &&  gain_sign) begin
      gain_calc_limit = CALC_MIN;
    end else begin
      gain_calc_limit = gain_calc;
    end  
  end
  //gain_calc_limit = gain_calc_tmp[(2*CALC2_BITS-14-1):((2*CALC2_BITS)-AXIS_DATA_BITS-14)];
end

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
    
endmodule