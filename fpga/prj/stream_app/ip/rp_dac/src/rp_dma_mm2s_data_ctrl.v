`timescale 1ns / 1ps

module rp_dma_mm2s_data_ctrl(
  input  wire       m_axi_aclk,
  input  wire       m_axi_aresetn,
  input  wire       fifo_rst,
  input  wire       fifo_full,
  //
  //
  input  wire [8:0] req_data,
  input  wire       req_we,
  //  
  input  wire       m_axi_rvalid,    
  output reg        m_axi_rready,    
  input  wire       m_axi_rlast                        
);

///////////////////////////////////////////////////////////
// Parameters
//////////////////////////////////////////////////////////// 

localparam IDLE           = 2'd0;
localparam SEND_AXI_DATA  = 2'd1;
localparam WAIT_FULL      = 2'd2;

////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////// 

reg  [1:0]    state_cs;                // Current state
reg  [1:0]    state_ns;                // Next state  
reg  [199:0]  state_ascii;             // ASCII state
reg           req_st;
reg  [8:0]    req_xfer_cnt;


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
// Name : State machine ascii 
// Converts the state to ascii for debug.
////////////////////////////////////////////////////////////

always @(*)
begin
  case (state_cs)
    IDLE:           state_ascii = "IDLE";
    SEND_AXI_DATA:  state_ascii = "SEND_AXI_DATA";
    WAIT_FULL:      state_ascii = "WAIT_FULL";
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
    // IDLE - Wait for a request in the FIFO
    IDLE: begin
      if (req_we == 1 || req_st) begin
        state_ns = SEND_AXI_DATA;
      end
    end

    // Just wait the buffer is not full
    WAIT_FULL: begin
      if (~fifo_full) begin
        if (req_st)
          state_ns = SEND_AXI_DATA;        
        else
          state_ns = IDLE;
      end      
    end

    // SEND_AXI_DATA - Transfer the AXI data 
    SEND_AXI_DATA: begin
      if ((m_axi_rvalid == 1) && (m_axi_rready == 1) && (m_axi_rlast == 1)) begin
        if (fifo_full)
          state_ns = WAIT_FULL;
        else
          state_ns = IDLE;
      end      
    end
  endcase
end

always @(posedge m_axi_aclk)
begin
  case (state_cs) 
    WAIT_FULL: begin
      if (req_we)
        req_st <= 'b1;
    end     

    default: begin
      req_st <= 'b0;
    end  
  endcase   
end
////////////////////////////////////////////////////////////
// Name : Transfer Count
// Counts the AXI transfers in the request.
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  case (state_cs) 
    // IDLE - Wait for a request in the FIFO
    IDLE: begin
      req_xfer_cnt <= req_data;
    end  
    
    // SEND_AXI_DATA - Transfer the AXI data 
    SEND_AXI_DATA: begin
      if ((m_axi_rvalid == 1) && (m_axi_rready == 1)) begin
        if (m_axi_rlast == 1) begin
          req_xfer_cnt <= req_data; 
        end else begin
          req_xfer_cnt <= req_xfer_cnt - 1;     
        end       
      end      
    end      
  endcase   
end

////////////////////////////////////////////////////////////
// Name : AXI RREADY
// Able to transfer as there is data in the FIFO
////////////////////////////////////////////////////////////

always @(posedge m_axi_aclk)
begin
  if (m_axi_aresetn == 0)
      m_axi_rready <= 1'b0;
  else begin
    case (state_cs) 
      // IDLE - Wait for a request in the FIFO
      IDLE: begin
        m_axi_rready <= 1'b0;
      end  

      WAIT_FULL: begin
        m_axi_rready <= 1'b0;
      end  

      // SEND_AXI_DATA - Transfer the AXI data 
      SEND_AXI_DATA: begin
        if (m_axi_rvalid == 1)
          m_axi_rready <= |req_xfer_cnt;    
      end      
    endcase   
  end
end
      

endmodule