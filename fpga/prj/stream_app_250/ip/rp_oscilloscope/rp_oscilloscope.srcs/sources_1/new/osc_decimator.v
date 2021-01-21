`timescale 1ns / 1ps

module osc_decimator
  #(parameter AXIS_DATA_BITS  = 16,
    parameter CNT_BITS        = 17,
    parameter SHIFT_BITS      = 4)(
  input  wire                       clk,
  input  wire                       rst_n,
  // Slave AXI-S
  input  wire [AXIS_DATA_BITS-1:0]  s_axis_tdata,
  input  wire                       s_axis_tvalid,
  output wire                       s_axis_tready,
  // Master AXI-S
  output reg  [AXIS_DATA_BITS-1:0]  m_axis_tdata,
  output reg                        m_axis_tvalid,
  input  wire                       m_axis_tready,
  // Control
  input  wire                       ctl_rst, 
  // Config
  input  wire                       cfg_avg_en, 
  input  wire [CNT_BITS-1:0]        cfg_dec_factor, 
  input  wire [SHIFT_BITS-1:0]      cfg_dec_rshift 
);

////////////////////////////////////////////////////////////
// Parameters
////////////////////////////////////////////////////////////

localparam ACC_BITS = AXIS_DATA_BITS+CNT_BITS;

////////////////////////////////////////////////////////////
// Signals
////////////////////////////////////////////////////////////

wire signed [AXIS_DATA_BITS-1:0]  dec_data;
reg         [CNT_BITS-1:0]        dec_cnt;
reg  signed [ACC_BITS-1:0]        dec_acc;

assign s_axis_tready = 1;
assign dec_data = s_axis_tdata; 

////////////////////////////////////////////////////////////
// Name : Decimation Counter
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    dec_cnt <= 0;
  end else begin
    if (ctl_rst == 1) begin
      dec_cnt <= 0;
    end else begin
      if ((s_axis_tvalid == 1) && (s_axis_tready == 1)) begin
        if (dec_cnt == cfg_dec_factor) begin
          dec_cnt <= 0; 
        end else begin
          dec_cnt <= dec_cnt + 1;
        end
      end
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Decimator Accumulator
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    dec_acc <= 0;
  end else begin
    if (ctl_rst == 1) begin
      dec_acc <= 0;
    end else begin
      if ((s_axis_tvalid == 1) && (s_axis_tready == 1)) begin
        if (dec_cnt == cfg_dec_factor) begin
          dec_acc <= dec_data;
        end else begin
          dec_acc <= dec_acc + dec_data;
        end
      end
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Master AXI-S TDATA 
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (cfg_avg_en == 1) begin
    m_axis_tdata <= dec_acc >>> cfg_dec_rshift;
  end else begin
    m_axis_tdata <= s_axis_tdata;
  end
end

////////////////////////////////////////////////////////////
// Name : Master AXI-S TVALID 
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    m_axis_tvalid <= 0;
  end else begin
    if ((s_axis_tvalid == 1) && (s_axis_tready == 1) && (dec_cnt == cfg_dec_factor)) begin
      m_axis_tvalid <= 1;
    end else begin
      m_axis_tvalid <= 0;
    end
  end
end

endmodule