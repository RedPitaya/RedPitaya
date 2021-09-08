`timescale 1ns / 1ps

module gpio_dma_mm2s_downsize
  #(parameter AXI_DATA_BITS   = 64,
    parameter AXIS_DATA_BITS  = 16,     
    parameter AXI_BURST_LEN   = 16)(    
  input  wire                       clk,
  input  wire                       rst,
  
  output reg  [7:0]                 req_data,
  output reg                        req_we,

  input  wire                       fifo_empty,
  input  wire                       fifo_full,

  input  wire [AXI_DATA_BITS-1:0]   fifo_rd_data,
  input  wire                       fifo_valid,
  output reg                        fifo_rd_re,      
  input  wire                       fifo_last,    
  input  wire                       valid_dis,    

  
  output reg  [AXIS_DATA_BITS-1:0]  m_axis_tdata,
  output reg                        m_axis_tvalid,
  input  wire                       m_axis_tready 
);

localparam MUX_MAX = AXI_DATA_BITS/AXIS_DATA_BITS;

reg  [1:0]  mux_sel;
reg  [6:0]  xfer_cnt;
wire [6:0]  req_len;
reg         tlast;
genvar      i;
reg  [2:0]  rd_cnt;
reg  [2:0]  empty_reg;
reg         fifo_re_reg, fifo_dis_reg;

assign req_len = xfer_cnt+1;
////////////////////////////////////////////////////////////
// Name : 
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst == 1) begin
    xfer_cnt <= 0;  
  end else begin
    if ((m_axis_tvalid == 1) && (m_axis_tready == 1)) begin
      if (xfer_cnt == AXI_BURST_LEN-1) begin
        xfer_cnt <= 0;      
      end else begin
        xfer_cnt <= xfer_cnt + 1;      
      end
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Request Data
// Sends the transfer count and indicates if this is the 
// last transfer.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  req_data <= {tlast, req_len};
end

////////////////////////////////////////////////////////////
// Name : 
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst == 1) begin
    req_we <= 0;  
  end else begin
    if (((m_axis_tvalid == 1) && (m_axis_tready == 1) && ((tlast == 1) || (xfer_cnt == AXI_BURST_LEN-1)))) begin
      req_we <= 1;      
    end else begin
      req_we <= 0;  
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Mux Select
// Selects which part of the output data should have the
// input sample assigned. 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst == 1) begin
    mux_sel <= 0;  
  end else begin
    if ((fifo_valid == 1) && (fifo_rd_re == 1)) begin
      if (mux_sel == MUX_MAX-1) begin
        mux_sel <= 0;
      end else begin
        mux_sel <= mux_sel + 1;
      end
    end
  end
end

////////////////////////////////////////////////////////////
// Name : TDATA
// 
////////////////////////////////////////////////////////////
/*
generate 
  for (i=0; i<MUX_MAX; i=i+1) begin : gen_data
    always @(posedge clk)
    begin
      if (mux_sel == i) begin
        m_axis_tdata  <=  fifo_rd_data[i*AXIS_DATA_BITS +: AXIS_DATA_BITS];
      end
    end
  end  
endgenerate
*/
////////////////////////////////////////////////////////////
// Name : TVALID
// Set the valid signal if all samples are assigned to the
// ouput data or the last sample has arrived.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst == 1)
    empty_reg <= 'd0;
  else 
    empty_reg <= {empty_reg[1:0], fifo_empty};
end

always @(posedge clk)
begin
  fifo_re_reg <= m_axis_tready;
  if (rst == 1)
    fifo_dis_reg <= 'h0;
  else begin
    if (m_axis_tready & ~fifo_re_reg)
      fifo_dis_reg <= 'h1;
    else if (~empty_reg[2])
      fifo_dis_reg <= 'h0;
  end
end

reg [9:0] reads_cnt;
always @(posedge clk)
begin
  if (rst == 1)
    reads_cnt <= 'h0;
  else begin
    if (m_axis_tready)
      reads_cnt <= 'h0;
    else
      reads_cnt <= reads_cnt + 1; 
  end
end

always @(posedge clk)
begin
  if (rst == 1) begin
    rd_cnt <= 'd0;
    m_axis_tvalid <= 'd0;
  end else begin 
    m_axis_tvalid <= ~valid_dis; // disables reading FIFOs
    if (~empty_reg[2]) begin
      if (rd_cnt == 'd0 && m_axis_tready) // downsize from 64 to 32 bit. 
        rd_cnt <= rd_cnt + 1;
      else if (rd_cnt >= 'd1 && m_axis_tready)
        rd_cnt <= 'd0;      

      fifo_rd_re <= (rd_cnt == 'd0) & m_axis_tready; 

      case(rd_cnt)
        'd1: m_axis_tdata <= fifo_rd_data[31: 0];
        'd0: m_axis_tdata <= fifo_rd_data[63:32];
      endcase
    end
  end 
end

////////////////////////////////////////////////////////////
// Name : TLAST
// Indicates that the last sample has been sent. 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst == 1) begin
    tlast <= 0;
  end else begin 
    if ((fifo_last == 1) && (fifo_valid == 1) && (fifo_rd_re == 1)) begin
      tlast <= 1;
    end 
  end
end


endmodule