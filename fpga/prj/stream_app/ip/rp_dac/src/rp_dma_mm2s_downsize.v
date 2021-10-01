`timescale 1ns / 1ps

module rp_dma_mm2s_downsize
  #(parameter AXI_DATA_BITS   = 64,
    parameter AXIS_DATA_BITS  = 16,     
    parameter AXI_BURST_LEN   = 126)(    
  input  wire                       clk,
  input  wire                       rst,
  
  output reg  [7:0]                 req_data,
  output reg                        req_we,

  input  wire                       fifo_empty,

  input  wire [AXI_DATA_BITS-1:0]   fifo_rd_data,
  input  wire                       fifo_valid,
  output wire                       fifo_rd_re,      
  input  wire                       fifo_last,    
  
  output reg  [AXIS_DATA_BITS-1:0]  m_axis_tdata,
  output reg                       m_axis_tvalid,
  input  wire                       m_axis_tready 
);

localparam MUX_MAX = AXI_DATA_BITS/AXIS_DATA_BITS;

reg  [6:0]  xfer_cnt;
wire [6:0]  req_len;
reg         tlast;
genvar      i;
reg  [2:0]  rd_cnt;
reg  [4:0]  empty_reg;


//assign req_len = xfer_cnt+1;
assign req_len = AXI_BURST_LEN;

assign fifo_rd_re = ~fifo_empty;

////////////////////////////////////////////////////////////
// Name : 
// 
////////////////////////////////////////////////////////////
/*
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
*/
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
    /*if (((m_axis_tvalid == 1) && (m_axis_tready == 1) && ((tlast == 1) || (xfer_cnt == AXI_BURST_LEN-1)))) begin
      req_we <= 1;      
    end else begin
      req_we <= 0;  
    end*/
    req_we <= 1;
  end
end

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
    empty_reg <= {empty_reg[3:0], fifo_empty};
end

always @(posedge clk)
begin
  if (rst == 1) begin
    rd_cnt <= 'd0;
    m_axis_tvalid <= 1'b1;
  end else begin 
    //if (~empty_reg[4]) begin
    if (~fifo_empty) begin
      /*if (rd_cnt < 'd4)
        rd_cnt <= rd_cnt + 1;
      else
        rd_cnt <= 'd0;
      
      fifo_rd_re    <= (rd_cnt == 'd0);
      m_axis_tvalid <= (rd_cnt > 'd0);

      case(rd_cnt)
        'd1: m_axis_tdata <= fifo_rd_data[13: 0];
        'd2: m_axis_tdata <= fifo_rd_data[29:16];
        'd3: m_axis_tdata <= fifo_rd_data[45:32];
        'd4: m_axis_tdata <= fifo_rd_data[61:48];
      endcase      
*/
      m_axis_tvalid <= 1'b1;
      m_axis_tdata <= fifo_rd_data[13: 0];
    end

      //fifo_rd_re <= ~fifo_empty;
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
    if ((fifo_last == 1) && (fifo_valid == 1)) begin
      tlast <= 1;
    end 
  end
end


endmodule