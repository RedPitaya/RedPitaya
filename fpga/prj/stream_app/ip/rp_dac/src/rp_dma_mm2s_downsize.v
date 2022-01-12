`timescale 1ns / 1ps

module rp_dma_mm2s_downsize
  #(parameter AXI_DATA_BITS   = 64,
    parameter AXI_ADDR_BITS   = 32,
    parameter AXIS_DATA_BITS  = 16,     
    parameter AXI_BURST_LEN   = 15)(    
  input  wire                       clk,
  input  wire                       rst,

  input  wire                       fifo_empty,
  input  wire                       fifo_full,

  input  wire [AXI_DATA_BITS-1:0]   fifo_rd_data,
  output wire                       fifo_rd_re,      
  input [AXI_ADDR_BITS-1:0]         dac_pntr_step,
 
  output reg  [AXIS_DATA_BITS-1:0]  m_axis_tdata,
  output reg                        m_axis_tvalid
);

localparam NUM_SAMPS      = AXI_DATA_BITS/16;  // how many samples in one read from FIFO
localparam NUM_SAMPS_BITS = $clog2(NUM_SAMPS); // how many bits is the above number
localparam ADDR_DECS      = AXI_ADDR_BITS+16;  // to be able to get a finer pointer step

assign fifo_rd_re = (first_full & ~fifo_empty_r && (dac_rp_next_next[NUM_SAMPS_BITS+1+16] ^ dac_rp_next[NUM_SAMPS_BITS+1+16]));
reg  [ADDR_DECS-1:0]      dac_rp_curr;
wire [ADDR_DECS-1:0]      dac_rp_next      = dac_rp_curr+{15'h0, dac_pntr_step, 1'b0}; //shifted by 1 so that step 1 is a read of 1 address.
wire [ADDR_DECS-1:0]      dac_rp_next_next = dac_rp_next+{15'h0, dac_pntr_step, 1'b0}; //shifted by 1 so that step 1 is a read of 1 address.

reg [AXIS_DATA_BITS-1:0] samp_buf [0:NUM_SAMPS-1]; 
reg fifo_empty_r;
reg first_full;
////////////////////////////////////////////////////////////
// Name : First full
// must wait to fill up the FIFO before starting to read
////////////////////////////////////////////////////////////
always @(posedge clk)
begin
  if (rst == 0) begin
    first_full <= 'h0;
  end else begin 
    if (fifo_full & ~fifo_empty & ~first_full)
      first_full <= 'h1;
  end
end

////////////////////////////////////////////////////////////
// Name : Read pointer
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
fifo_empty_r <= fifo_empty;
  if (rst == 0) begin
    dac_rp_curr <= 'h0;
  end else begin 
    if (~fifo_empty_r & first_full) // wait for the FIFO to be full before starting to read, stop if empty
      dac_rp_curr <= dac_rp_next;
  end
end

genvar GV;
generate
for (GV = 0; GV < NUM_SAMPS; GV = GV + 1) begin : read_decoder
  always @(posedge clk) begin
    samp_buf[GV] <= fifo_rd_data[GV*16+13:GV*16];  
  end
end
endgenerate
////////////////////////////////////////////////////////////
// Name : TVALID
// Set the valid signal if all samples are assigned to the
// ouput data or the last sample has arrived.
////////////////////////////////////////////////////////////
always @(posedge clk)
begin
  if (rst == 0) begin
    m_axis_tvalid <= 'h0;
  end else begin 
    m_axis_tvalid <= first_full;
    m_axis_tdata  <= samp_buf[dac_rp_curr[NUM_SAMPS_BITS+16:17]];
  end
end

endmodule