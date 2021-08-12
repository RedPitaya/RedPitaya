`timescale 1ns / 1ps

module rp_dma_s2mm_ctrl
  #(parameter AXI_ADDR_BITS = 32,  
    parameter AXI_DATA_BITS = 64,
    parameter AXI_BURST_LEN = 16,
    parameter FIFO_CNT_BITS = 9,
    parameter REG_ADDR_BITS = 1,
    parameter CTRL_ADDR     = 1)(
  input  wire                       m_axi_aclk,
  input  wire                       s_axis_aclk,
  input  wire                       m_axi_aresetn,      
  //
  output reg                        busy,
  output reg                        intr,  
  output reg                        mode,
  //
  input  wire [REG_ADDR_BITS-1:0]   reg_addr,
  input  wire [31:0]                reg_wr_data,
  input  wire                       reg_wr_we,    
  //
  output reg  [31:0]                reg_ctrl,
  output wire [31:0]                reg_sts,
  input  wire [31:0]                reg_dst_addr1,  
  input  wire [31:0]                reg_dst_addr2,  
  input  wire [31:0]                reg_buf_size,         
  //
  output reg                        fifo_rst,
  input  wire [7:0]                 req_data,
  input  wire                       req_we, 
  input  wire                       data_valid,
  output wire [31:0]                buf1_ms_cnt,
  output wire [31:0]                buf2_ms_cnt,
  input  wire                       buf_sel_in,
  output wire                       buf_sel_out,
  output reg                        fifo_dis,
  //
  output wire [(AXI_ADDR_BITS-1):0] m_axi_awaddr,     
  output reg  [7:0]                 m_axi_awlen,      
  output wire [2:0]                 m_axi_awsize,     
  output wire [1:0]                 m_axi_awburst,    
  output wire [2:0]                 m_axi_awprot,     
  output wire [3:0]                 m_axi_awcache,    
  output reg                        m_axi_awvalid,    
  input  wire                       m_axi_awready,    
  output wire                       m_axi_wvalid,    
  input  wire                       m_axi_wready,      
  output wire                       m_axi_wlast          
);

///////////////////////////////////////////////////////////
// Parameters
//////////////////////////////////////////////////////////// 

localparam IDLE             = 3'd0;
localparam FIFO_RST         = 3'd1;
localparam WAIT_DATA_RDY    = 3'd2;
localparam SEND_DMA_REQ     = 3'd3;
localparam WAIT_DATA_DONE   = 3'd4;
localparam WAIT_BUF_FULL    = 3'd5;

localparam CTRL_STRT        = 0;  // Control - Bit[0] : Start DMA
localparam CTRL_INTR_ACK    = 1;  // Control - Bit[1] : Interrupt ACK
localparam CTRL_BUF1_ACK    = 2;  // Control - Bit[2] : Buffer 1 ACK
localparam CTRL_BUF2_ACK    = 3;  // Control - Bit[3] : Buffer 2 ACK
localparam CTRL_RESET       = 4;  // Control - Bit[4] : Reset states and flags
localparam CTRL_MODE_NORM   = 8;  // Control - Bit[8] : Mode normal DMA
localparam CTRL_MODE_STREAM = 9;  // Control - Bit[9] : Mode streaming DMA

localparam STS_BUF1_FULL    = 0;  // Status = Bit[0] : Buffer 1 full
localparam STS_BUF2_FULL    = 1;  // Status = Bit[1] : Buffer 2 full
localparam STS_BUF1_OVF     = 2;  // Status = Bit[2] : Buffer 1 overflow
localparam STS_BUF2_OVF     = 3;  // Status = Bit[3] : Buffer 2 overflow
localparam STS_CURR_BUF     = 4;  // Currently selected buffer


localparam AXI_BURST_BYTES  = AXI_BURST_LEN*AXI_DATA_BITS/8;
localparam BUF_SIZE_BITS    = 17;

////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////// 

reg  [2:0]                state_cs;                // Current state
reg  [2:0]                state_ns;                // Next state  
reg  [199:0]              state_ascii;             // ASCII state
reg  [AXI_ADDR_BITS-1:0]  req_addr;
reg  [AXI_ADDR_BITS-1:0]  req_buf_addr;
reg                       req_buf_addr_sel;
reg                       req_buf_addr_sel_p1;
wire                      req_buf_addr_sel_pedge;
wire                      req_buf_addr_sel_nedge;
wire [7:0]                req_xfer_cnt;
reg                       req_xfer_last;
wire                      dat_ctrl_busy;
wire [7:0]                dat_ctrl_req_data;
wire                      dat_ctrl_req_we;
reg                       buf1_full;
reg                       buf2_full;
reg                       buf1_ovr;
reg                       buf2_ovr;
reg                       data_valid_reg;
reg  [31:0]               buf1_missed_samp;
reg  [31:0]               buf2_missed_samp;
reg  [3:0]                fifo_rst_cnt;
wire [7:0]                fifo_wr_data; 
wire                      fifo_wr_we;
wire [7:0]                fifo_rd_data;
reg                       fifo_rd_re;
wire                      fifo_empty;
reg                       next_buf_full;
reg                       fifo_rst_cntdwn;
reg                       transf_end;

assign m_axi_awaddr  = req_addr;
assign m_axi_awsize  = $clog2(AXI_DATA_BITS/8);   
assign m_axi_awburst = 2'b01;     // INCR
assign m_axi_awprot  = 3'b000;
assign m_axi_awcache = 4'b0011;

assign buf1_ms_cnt = buf1_missed_samp;
assign buf2_ms_cnt = buf2_missed_samp;

assign req_buf_addr_sel_pedge = req_buf_addr_sel & ~req_buf_addr_sel_p1;
assign req_buf_addr_sel_nedge = ~req_buf_addr_sel & req_buf_addr_sel_p1;

assign reg_sts[31:5] = 27'd0;
assign reg_sts[STS_BUF1_FULL] = buf1_full;
assign reg_sts[STS_BUF1_OVF] = buf1_ovr;
assign reg_sts[STS_BUF2_FULL] = buf2_full;
assign reg_sts[STS_BUF2_OVF] = buf2_ovr;
assign reg_sts[STS_CURR_BUF] = req_buf_addr_sel;


assign buf_sel_out = req_buf_addr_sel_p1;

////////////////////////////////////////////////////////////
// Name : Request FIFO 
// Stores the DMA requests.
////////////////////////////////////////////////////////////

fifo_axi_req U_fifo_axi_req(
  .wr_clk (s_axis_aclk),
  .rd_clk (m_axi_aclk),      
  .rst    (fifo_rst),    
  .din    (fifo_wr_data),     
  .wr_en  (fifo_wr_we),  
  .full   (),  
  .dout   (fifo_rd_data),   
  .rd_en  (fifo_rd_re), 
  .empty  (fifo_empty));
  
assign fifo_wr_data = req_data;
assign fifo_wr_we   = req_we && ~fifo_dis; // writing request buffer is only enabled when not waiting on clearing of the next buffer. 

////////////////////////////////////////////////////////////
// Name : Data Control
// 
////////////////////////////////////////////////////////////
  
rp_dma_s2mm_data_ctrl U_dma_s2mm_data_ctrl(
  .m_axi_aclk     (m_axi_aclk),            
  .m_axi_aresetn  (m_axi_aresetn),                  
  .fifo_rst       (fifo_rst),              
  .busy           (dat_ctrl_busy),
  .req_data       (dat_ctrl_req_data),              
  .req_we         (dat_ctrl_req_we),            
  .req_xfer_cnt   (req_xfer_cnt),                         
  .m_axi_wvalid   (m_axi_wvalid),          
  .m_axi_wready   (m_axi_wready),          
  .m_axi_wlast    (m_axi_wlast));
  
assign dat_ctrl_req_data = m_axi_awlen+1;
assign dat_ctrl_req_we   = (m_axi_awvalid & m_axi_awready) && ~fifo_dis; // writing data buffer is only enabled when not waiting on clearing of the next buffer. 
        
////////////////////////////////////////////////////////////
// Name : State machine seq logic
// Sets the current state to the next state.
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  if (m_axi_aresetn == 0) begin
    state_cs <= IDLE;
  end else begin
    state_cs <= state_ns;
  end
end

////////////////////////////////////////////////////////////
// Name : data_valid synchronisation
// data valid must be delayed by one clock to align with 
// axi signals
////////////////////////////////////////////////////////////
always @(posedge m_axi_aclk)
begin
  if (m_axi_aresetn == 0) begin
    data_valid_reg <= 1'b0;
  end else begin
    data_valid_reg <= data_valid;
  end
end

////////////////////////////////////////////////////////////
// Name : State machine ascii 
// Converts the state to ascii for debug.
////////////////////////////////////////////////////////////

always @(*)
begin
  case (state_cs)
    IDLE:           state_ascii = "IDLE";
    FIFO_RST:       state_ascii = "FIFO_RST";
    WAIT_DATA_RDY:  state_ascii = "WAIT_DATA_RDY";            
    SEND_DMA_REQ:   state_ascii = "SEND_DMA_REQ";       
    WAIT_DATA_DONE: state_ascii = "WAIT_DATA_DONE";
    WAIT_BUF_FULL:  state_ascii = "WAIT_BUF_FULL";  
  endcase
end

////////////////////////////////////////////////////////////
// Name : State machine comb logic
// Assigns the next state.
////////////////////////////////////////////////////////////

always @(*)
begin
  state_ns = state_cs;

  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      if (reg_ctrl[CTRL_STRT] == 1) begin
        state_ns = FIFO_RST;
      end
    end
    
    // FIFO_RST - Reset the data FIFO
    FIFO_RST: begin
      if (reg_ctrl[CTRL_RESET])
        state_ns <= IDLE;
      else if (fifo_rst_cnt == 15) begin
        state_ns = WAIT_DATA_RDY;
      end
    end
    
    // WAIT_DATA_RDY - Wait for the data to be buffered before sending the request
    WAIT_DATA_RDY: begin
      if (reg_ctrl[CTRL_RESET])
        state_ns <= IDLE;
      else if (fifo_empty == 0) begin
        state_ns = SEND_DMA_REQ;
      end
      
    end

    // SEND_DMA_REQ - Send the request 
    SEND_DMA_REQ: begin

        if (reg_ctrl[CTRL_RESET])
          state_ns <= IDLE;
        else if (transf_end) begin
          if (next_buf_full) // if next transfer results in overwriting the buffer, wait until the buffer is completely read out.
           state_ns = WAIT_BUF_FULL;
          else if (req_xfer_last == 1) begin // Test for the last transfer
            state_ns = WAIT_DATA_DONE;   
          end else begin
            state_ns = WAIT_DATA_RDY;     
          end  
        end  
    end    
    
    // WAIT_DATA_DONE - Wait until data from all requests has been sent
    WAIT_DATA_DONE: begin
      // Test if the data control is busy
      if (reg_ctrl[CTRL_RESET])
          state_ns <= IDLE;
      else if (dat_ctrl_busy == 0) begin
        state_ns = IDLE;  
      end
    end

    WAIT_BUF_FULL: begin
      if (reg_ctrl[CTRL_RESET])
          state_ns <= IDLE;
      else if (~next_buf_full) begin // if next buffer is full, then wait
        state_ns = FIFO_RST; // go back to filling FIFOs
      end
    end

  endcase
end

////////////////////////////////////////////////////////////
// Name : Control Register
// 
////////////////////////////////////////////////////////////
always @(posedge m_axi_aclk)
begin
  if (m_axi_aresetn == 0) begin
    reg_ctrl <= 0;
  end else begin
    if ((reg_addr[8-1:0] == CTRL_ADDR) && (reg_wr_we == 1)) begin
      reg_ctrl <= reg_wr_data;
    end else begin
      // Start
      if (reg_ctrl[CTRL_STRT] == 1) begin
        reg_ctrl[CTRL_STRT] <= 0;
      end

      // Intr ACK
      if (reg_ctrl[CTRL_INTR_ACK] == 1) begin
        reg_ctrl[CTRL_INTR_ACK] <= 0;
      end
      
      // Buf 1 ACK
      if (reg_ctrl[CTRL_BUF1_ACK]) begin
        reg_ctrl[CTRL_BUF1_ACK] <= 0;
      end

      // Buf 2 ACK
      if (reg_ctrl[CTRL_BUF2_ACK]) begin
        reg_ctrl[CTRL_BUF2_ACK] <= 0;
      end   
      
      // Reset state machine
      if (reg_ctrl[CTRL_RESET]) begin
        reg_ctrl[CTRL_RESET] <= 0;
      end   
      // Mode normal
      if (reg_ctrl[CTRL_MODE_NORM] == 1) begin
        reg_ctrl[CTRL_MODE_NORM] <= 0;
      end            

      // Mode streaming
      if (reg_ctrl[CTRL_MODE_STREAM] == 1) begin
        reg_ctrl[CTRL_MODE_STREAM] <= 0;
      end

    end
  end
end

////////////////////////////////////////////////////////////
// Transfer end
// All data has been transferred
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  if (m_axi_aresetn == 0)
    transf_end <= 0;
  else begin
    if ((m_axi_wvalid == 1) && (m_axi_wlast == 1))
      transf_end <= 1'b1;
    else 
      transf_end <= 1'b0;
  end
end

////////////////////////////////////////////////////////////
// FIFO reset countdown active
// only activates after data loss FIFO reset
// used to increase missed sample counter during FIFO reset
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs)
    WAIT_BUF_FULL: begin
      if (~next_buf_full) begin // if next buffer is full, then wait
        fifo_rst_cntdwn <= 1'b1; // go back to filling FIFOs
      end
    end

    FIFO_RST: begin
      if (fifo_rst_cnt == 15) begin
        fifo_rst_cntdwn <= 1'b0;
      end
      if (reg_ctrl[CTRL_RESET])
        fifo_rst_cntdwn <= 1'b0;
    end     

    default: begin
        fifo_rst_cntdwn <= 1'b0;
      end

  endcase
end  

////////////////////////////////////////////////////////////
// FIFO disable signal
// Prevents writing new data into FIFOs
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs)
    WAIT_BUF_FULL: begin
        fifo_dis <= 1'b1; // disable signal
      end
    
    default: begin
        fifo_dis <= 1'b0;
      end        
  endcase
end  

////////////////////////////////////////////////////////////
// Name : DMA Mode
// 0 = Normal
// 1 = Streaming
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  if (m_axi_aresetn == 0) begin
    mode <= 0;
  end else begin
    if (reg_ctrl[CTRL_MODE_NORM] == 1) begin
      mode <= 0;
    end
    
    if (reg_ctrl[CTRL_MODE_STREAM] == 1) begin
      mode <= 1;
    end    
  end
end

////////////////////////////////////////////////////////////
// next buffer is still full
////////////////////////////////////////////////////////////
 
always @(posedge m_axi_aclk)
begin
  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      next_buf_full <= 0;  
    end    

    default: begin
        if (reg_ctrl[CTRL_BUF1_ACK] || reg_ctrl[CTRL_BUF2_ACK])
            next_buf_full <= 0;
        else if ((req_addr+AXI_BURST_BYTES) >= (req_buf_addr[AXI_ADDR_BITS-1:0]+reg_buf_size[BUF_SIZE_BITS-1:0])) begin
          if (((req_buf_addr_sel == 0) && buf2_full) || ((req_buf_addr_sel == 1) && buf1_full)) begin // data loss is occuring
            next_buf_full <= 1;  
          end
        end 
      end          
  endcase
end  

////////////////////////////////////////////////////////////
// Name : Buffer 1 Full
// Set when buffer 1 is full.
////////////////////////////////////////////////////////////
 
always @(posedge m_axi_aclk)
begin
  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      buf1_full <= 0;  
    end 

    default: begin
      if (reg_ctrl[CTRL_BUF1_ACK] == 1) begin
        buf1_full <= 0;     
      end else begin
        if (transf_end) begin
          // Reset to the start of the buffer if we have reached the end
          if ((req_addr+AXI_BURST_BYTES) >= (req_buf_addr[AXI_ADDR_BITS-1:0]+reg_buf_size[BUF_SIZE_BITS-1:0])) begin
            if (req_buf_addr_sel == 0) begin // buffer 1 is full if next transfer goes over specified buffer size
              buf1_full <= 1;
            end
          end
        end   
      end     
    end
  endcase
end  

////////////////////////////////////////////////////////////
// Name : Buffer 1 number of missed samples
// Set when buffer 1 is full, but SW has not read out
// the buffer completely
////////////////////////////////////////////////////////////
 
always @(posedge m_axi_aclk)
begin
  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      buf1_missed_samp <= 32'b0;  
    end    

    default: begin
      // increase counter until SW confirms buffer was read
        if ((req_buf_addr_sel == 1 && (fifo_dis || fifo_rst_cntdwn)) && data_valid_reg && buf1_missed_samp < 32'hFFFFFFFF) begin // buffer1 is overflowing, there was a sample
          buf1_missed_samp <= buf1_missed_samp+32'd1;  
        end else if(req_buf_addr_sel_pedge) // number of missed samples is reset when writing into the buffer starts.
          buf1_missed_samp <= 32'd0;
        end       
  endcase
end  

////////////////////////////////////////////////////////////
// Name : Buffer 1 Overflow
// Set when buffer 1 is full and cannot be switched into 
// when still in buffer 2.
////////////////////////////////////////////////////////////
 
always @(posedge m_axi_aclk)
begin
  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      buf1_ovr <= 0;  
    end    

    default: begin
      if (reg_ctrl[CTRL_BUF1_ACK] == 1) begin
        buf1_ovr <= 0;     
      end else begin
          // Reset to the start of the buffer if we have reached the end
        if ((req_addr+AXI_BURST_BYTES) >= (req_buf_addr[AXI_ADDR_BITS-1:0]+reg_buf_size[BUF_SIZE_BITS-1:0])) begin
            if ((req_buf_addr_sel == 1) && buf1_full)
              buf1_ovr <= 1;  
        end
      end
    end   
  endcase
end  
      
////////////////////////////////////////////////////////////
// Name : Buffer 2 Full
// Set when buffer 2 is full.
////////////////////////////////////////////////////////////
 
always @(posedge m_axi_aclk)
begin
  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      buf2_full <= 0;  
    end    

    default: begin
      if (reg_ctrl[CTRL_BUF2_ACK] == 1) begin
        buf2_full <= 0;     
      end else begin
        if (transf_end) begin          // Reset to the start of the buffer if we have reached the end
          if ((req_addr+AXI_BURST_BYTES) >= (req_buf_addr[AXI_ADDR_BITS-1:0]+reg_buf_size[BUF_SIZE_BITS-1:0])) begin
            if (req_buf_addr_sel == 1) begin // buffer 2 is full if next transfer goes over specified buffer size
              buf2_full <= 1;  
            end
          end
        end   
      end     
    end
  endcase
end  

////////////////////////////////////////////////////////////
// Name : Buffer 2 number of missed samples
// Set when buffer 2 is full, but SW has not read out
// the buffer completely
////////////////////////////////////////////////////////////
 
always @(posedge m_axi_aclk)
begin
  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      buf2_missed_samp <= 32'b0;  
    end    

    default: begin
        if ((req_buf_addr_sel == 0 && (fifo_dis || fifo_rst_cntdwn)) && data_valid_reg && buf2_missed_samp < 32'hFFFFFFFF) begin // buffer2 is overflowing, there was a sample
          buf2_missed_samp <= buf2_missed_samp+32'd1;  
        end else if(req_buf_addr_sel_nedge) begin // number of missed samples is reset when writing into the buffer starts.
          buf2_missed_samp <= 32'd0;
        end   
      end     
  endcase
end     
////////////////////////////////////////////////////////////
// Name : Buffer 1 Overflow
// Set when buffer 1 is full and cannot be switched into 
// when still in buffer 2.
////////////////////////////////////////////////////////////
 
always @(posedge m_axi_aclk)
begin
  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      buf2_ovr <= 0;  
    end    

    default: begin
      if (reg_ctrl[CTRL_BUF2_ACK] == 1) begin
        buf2_ovr <= 0;     
      end else begin
        if ((req_addr+AXI_BURST_BYTES) >= (req_buf_addr[AXI_ADDR_BITS-1:0]+reg_buf_size[BUF_SIZE_BITS-1:0])) begin
            if ((req_buf_addr_sel == 0) && buf2_full) 
              buf2_ovr <= 1;  
        end
      end   
    end     
  endcase
end  
            
////////////////////////////////////////////////////////////
// Name : Request Address
// Holds the address of the buffer in memory where data will
// be written.
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      req_addr <= reg_dst_addr1;  
    end
    
    SEND_DMA_REQ: begin
      if (transf_end) begin
        // Swap the buffer if we have reached the end of the current one
        if ((req_addr+AXI_BURST_BYTES) >= (req_buf_addr[AXI_ADDR_BITS-1:0]+reg_buf_size[BUF_SIZE_BITS-1:0])) begin
          if ((req_buf_addr_sel == 0) && ~buf2_full) begin // only switch addresses when next buffer is read out
            req_addr <= reg_dst_addr2;          
          end 
          if ((req_buf_addr_sel == 1) && ~buf1_full) begin
            req_addr <= reg_dst_addr1;
          end        
        end else begin
          req_addr <= req_addr + AXI_BURST_BYTES;
        end
      end  
    end
    
    WAIT_BUF_FULL: begin
      if (~next_buf_full) begin
        // Swap the buffer if we have reached the end of the current one
          if ((req_buf_addr_sel == 0) && ~buf2_full) begin // only switch addresses when next buffer is read out
            req_addr <= reg_dst_addr2;          
          end 
          if ((req_buf_addr_sel == 1) && ~buf1_full) begin
            req_addr <= reg_dst_addr1;
          end        
      end   
    end      
  endcase
end

////////////////////////////////////////////////////////////
// Name : Request Buffer Address Select
// Selects the current buffer to use in memory.
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      req_buf_addr <= reg_dst_addr1;  
    end  
    
    SEND_DMA_REQ: begin
      if (transf_end) begin        // Reset to the start of the buffer if we have reached the end
        if ((req_addr+AXI_BURST_BYTES) >= (req_buf_addr[AXI_ADDR_BITS-1:0]+reg_buf_size[BUF_SIZE_BITS-1:0])) begin
          if ((req_buf_addr_sel == 0) && ~buf2_full) begin //only start writing to buf2 if it's empty
            req_buf_addr <= reg_dst_addr2;          
          end 
          if ((req_buf_addr_sel == 1) && ~buf1_full) begin //only start writing to buf1 if it's empty
            req_buf_addr <= reg_dst_addr1;
          end   
        end
      end  
    end
    
    WAIT_BUF_FULL: begin
      if (~next_buf_full) begin
        // Swap the buffer if we have reached the end of the current one
          if ((req_buf_addr_sel == 0) && ~buf2_full) begin // only switch addresses when next buffer is read out
            req_buf_addr <= reg_dst_addr2;          
          end 
          if ((req_buf_addr_sel == 1) && ~buf1_full) begin
            req_buf_addr <= reg_dst_addr1;
          end  
        end      
      end          
  endcase
end

////////////////////////////////////////////////////////////
// Name : Request Buffer Address Select
// Selects the current buffer to use in memory.
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs)
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      req_buf_addr_sel <= 0;  
    end  
    
    SEND_DMA_REQ: begin
      if (transf_end) begin        // Reset to the start of the buffer if we have reached the end
        if ((req_addr+AXI_BURST_BYTES) >= (req_buf_addr[AXI_ADDR_BITS-1:0]+reg_buf_size[BUF_SIZE_BITS-1:0])) begin
          if (~buf1_full && req_buf_addr_sel == 1'b1) 
            req_buf_addr_sel <= 1'b0;
        
          if (~buf2_full && req_buf_addr_sel == 1'b0)
            req_buf_addr_sel <= 1'b1;
        end
      end  
    end  
    
    FIFO_RST: begin // only happens when exiting FIFO reset state after data loss
      if (fifo_rst_cntdwn && (&fifo_rst_cnt)) begin
          req_buf_addr_sel <= ~req_buf_addr_sel;
        end      
      end        
  endcase
end

always @(posedge m_axi_aclk)
begin
  req_buf_addr_sel_p1 <= req_buf_addr_sel;
end

////////////////////////////////////////////////////////////
// Name : Request Transfer Last
// Set on the last transfer.
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs) 
    // WAIT_DATA_RDY - Wait until there is enough data to send an AXI transfer
    WAIT_DATA_RDY: begin
      req_xfer_last <= fifo_rd_data[7];
    end

  endcase   
end

////////////////////////////////////////////////////////////
// Name : Busy
// Signals when the DMA is busy.
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs) 
    // IDLE - Wait for the DMA start signal
    IDLE: begin
      if (reg_ctrl[CTRL_STRT] == 1) begin
        busy <= 1;
      end else begin
        busy <= 0;
      end
    end 
  endcase  
end

////////////////////////////////////////////////////////////
// Name : Interrupt
// Sends an interrupt to signal that the DMA has finished.
////////////////////////////////////////////////////////////
always @(posedge m_axi_aclk)
begin
  case (state_cs)
    IDLE: begin
        intr <= 1'b0;
      end

    default: begin
      if (m_axi_aresetn == 0) begin
        intr <= 0;
      end else begin
        if (reg_ctrl[CTRL_INTR_ACK] == 1) begin
          intr <= 0; 
        end else begin
        if (((state_cs == WAIT_DATA_DONE) && (dat_ctrl_busy == 0)) ||
              ((mode == 1) && 
              ((req_buf_addr_sel_pedge == 1 && buf_sel_in == 0) || (req_buf_addr_sel_nedge == 1 && buf_sel_in == 1)))) begin // Set if streaming mode and buffer is full
            //((req_buf_addr_sel_pedge == 1) || (req_buf_addr_sel_nedge == 1)))) begin
            intr <= 1;  // interrupt only triggers if the channel is not lagging behind. 
          end
        end
      end
    end
  endcase
end

////////////////////////////////////////////////////////////
// Name : FIFO Reset 
// Resets the Xilinx FIFOs
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs) 
    // FIFO_RST - Reset the data FIFO
    FIFO_RST: begin
      if (fifo_rst_cnt < 1) begin
        fifo_rst <= 1;  
      end else begin
        fifo_rst <= 0;
      end
    end   
  
    default: begin
      fifo_rst <= 0;
    end   
  endcase  
end

////////////////////////////////////////////////////////////
// Name : FIFO Reset Count
// The Xilinx FIFO needs a certain reset time.
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs) 
   // FIFO_RST - Reset the data FIFO
    FIFO_RST: begin
      fifo_rst_cnt <= fifo_rst_cnt + 1;
    end   
  
    default: begin
      fifo_rst_cnt <= 0;
    end   
  endcase   
end

////////////////////////////////////////////////////////////
// Name : FIFO Read Enable
// Reads requests from the FIFO when available. 
////////////////////////////////////////////////////////////

always @(*)
begin
  fifo_rd_re = 0;
        
  case (state_cs)   
    // WAIT_DATA_RDY - Wait until there is enough data to send an AXI transfer
    WAIT_DATA_RDY: begin
      if (fifo_empty == 0) begin
        fifo_rd_re = 1;
      end
    end
     
  endcase   
end

////////////////////////////////////////////////////////////
// Name : Master AXI AWVALID
// 
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  if (m_axi_aresetn == 0) begin
    m_axi_awvalid <= 0;
  end else begin
    case (state_cs) 
       // WAIT_DATA_RDY - Wait until there is enough data to send an AXI transfer
      WAIT_DATA_RDY: begin
        if (fifo_empty == 0) begin
          m_axi_awvalid <= 1;  
        end
      end
      
      SEND_DMA_REQ: begin
        if ((m_axi_awvalid == 1) && (m_axi_awready == 1)) begin
          m_axi_awvalid <= 0;  
        end  
      end          

      default: begin
        m_axi_awvalid <= 0;  
      end    
    endcase   
  end
end

////////////////////////////////////////////////////////////
// Name : Master AXI AWLEN
// 
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs) 
    // WAIT_DATA_RDY - Wait until there is enough data to send an AXI transfer
    WAIT_DATA_RDY: begin
      m_axi_awlen <= fifo_rd_data[6:0]-1;
    end

    

  endcase   
end

endmodule