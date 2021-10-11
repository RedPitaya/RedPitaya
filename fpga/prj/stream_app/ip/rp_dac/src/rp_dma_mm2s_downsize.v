`timescale 1ns / 1ps

module rp_dma_mm2s_downsize
  #(parameter AXI_DATA_BITS   = 64,
    parameter AXIS_DATA_BITS  = 16,     
    parameter AXI_BURST_LEN   = 15)(    
  input  wire                       clk,
  input  wire                       rst,

  input  wire                       fifo_empty,
  input  wire [AXI_DATA_BITS-1:0]   fifo_rd_data,
  output wire                       fifo_rd_re,      

  input  wire                       word_sel,    
 
  output reg  [AXIS_DATA_BITS-1:0]  m_axis_tdata,
  output reg                        m_axis_tvalid
);

localparam MUX_MAX = AXI_DATA_BITS/AXIS_DATA_BITS;

assign fifo_rd_re = ~fifo_empty;


////////////////////////////////////////////////////////////
// Name : TVALID
// Set the valid signal if all samples are assigned to the
// ouput data or the last sample has arrived.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst == 0) begin
    m_axis_tvalid <= 1'b1;
  end else begin 
    m_axis_tvalid <= 1'b1;
    m_axis_tdata  <= word_sel ? fifo_rd_data[29:16]: fifo_rd_data[13: 0];
  end
end

endmodule