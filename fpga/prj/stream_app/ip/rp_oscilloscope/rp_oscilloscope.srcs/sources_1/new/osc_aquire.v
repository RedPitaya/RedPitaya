`timescale 1ns / 1ps

module osc_acquire
  #(parameter AXIS_DATA_BITS  = 16,
    parameter CNT_BITS        = 32)( // No. of bits for the counters
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
  output reg                        m_axis_tlast,    
  // Control
  input  wire                       ctl_start, 
  input  wire                       ctl_rst,
  input  wire                       ctl_stop, 
  input  wire                       ctl_trig,
  // Config
  input  wire                       cfg_mode, 
  input  wire [CNT_BITS-1:0]        cfg_trig_pre_samp, 
  input  wire [CNT_BITS-1:0]        cfg_trig_post_samp,
  // Status
  output reg                        sts_start,
  output wire                       sts_stop,     
  output reg                        sts_trig,
  output reg  [CNT_BITS-1:0]        sts_trig_pre_cnt,
  output reg                        sts_trig_pre_overflow,  
  output reg  [CNT_BITS-1:0]        sts_trig_post_cnt,
  output reg                        sts_trig_post_overflow         
);

////////////////////////////////////////////////////////////
// Parameters
//////////////////////////////////////////////////////////// 

localparam IDLE                 = 3'd0;
localparam PRE_SAMP_FILL        = 3'd1;
localparam PRE_SAMP_WAIT_TRIG   = 3'd2;
localparam POST_SAMP_WAIT_TRIG  = 3'd3;
localparam POST_SAMP_FILL       = 3'd4;
localparam STRM_SAMP            = 3'd5;

localparam NORM_MODE            = 0;
localparam STREAM_MODE          = 1;

////////////////////////////////////////////////////////////
// Signals
////////////////////////////////////////////////////////////

reg  [2:0]    state_cs;             
reg  [2:0]    state_ns;          
reg  [199:0]  state_ascii;      
reg           sts_acquire;
reg           sts_last;

assign s_axis_tready  = 1;
assign sts_stop       = ~sts_start;

////////////////////////////////////////////////////////////
// Name : State machine seq logic
// Sets the current state to the next state.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    state_cs <= IDLE;
  end else begin
    if (ctl_stop == 1) begin
      state_cs <= IDLE; 
    end else begin
      state_cs <= state_ns;
    end
  end
end

////////////////////////////////////////////////////////////
// Name : State machine ascii 
// Converts the state to ascii for debug.
////////////////////////////////////////////////////////////

always @(*)
begin
  case (state_cs)
    IDLE:                 state_ascii = "IDLE";
    PRE_SAMP_FILL:        state_ascii = "PRE_SAMP_FILL";
    PRE_SAMP_WAIT_TRIG:   state_ascii = "PRE_SAMP_WAIT_TRIG";
    POST_SAMP_WAIT_TRIG:  state_ascii = "POST_SAMP_WAIT_TRIG";
    POST_SAMP_FILL:       state_ascii = "POST_SAMP_FILL";        
    STRM_SAMP:            state_ascii = "STRM_SAMP";                                                                                     
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
    // IDLE - 
    IDLE: begin
      if (ctl_start == 1) begin
        if (cfg_mode == STREAM_MODE) begin
          //state_ns = STRM_SAMP;  
          state_ns = PRE_SAMP_WAIT_TRIG;  
        end else begin     
          // Check if the pre buffer is enabled
          if (cfg_trig_pre_samp != 0) begin
            state_ns = PRE_SAMP_FILL;
          end else begin
            state_ns = POST_SAMP_WAIT_TRIG;        
          end
        end
      end
    end
    
    // PRE_SAMP_FILL - Fills the pre sample buffer 
    PRE_SAMP_FILL: begin
      if ((s_axis_tvalid == 1) && (sts_trig_pre_cnt == cfg_trig_pre_samp-1)) begin
        state_ns = PRE_SAMP_WAIT_TRIG;  
      end
    end
    
    // PRE_SAMP_WAIT_TRIG - Wait until we get a trigger
    PRE_SAMP_WAIT_TRIG: begin
      if (ctl_trig == 1) begin
        if (cfg_mode == STREAM_MODE) begin
          state_ns = STRM_SAMP;  
        end else begin
          state_ns = POST_SAMP_FILL;
        end  
      end
    end    
    
    // POST_SAMP_WAIT_TRIG - Wait until we get a trigger
    POST_SAMP_WAIT_TRIG: begin
      if (ctl_trig == 1) begin
        state_ns = POST_SAMP_FILL;  
      end
    end        
    
    // POST_SAMP_FILL - Fills the post sample buffer 
    POST_SAMP_FILL: begin
      if ((s_axis_tvalid == 1) && (sts_trig_post_cnt == cfg_trig_post_samp-1)) begin
        state_ns = IDLE;  
      end
    end    
    
    // STRM_SAMP - Streaming mode, data is continuously sent to the DMA
    STRM_SAMP: begin
      if (ctl_stop == 1) begin
        state_ns = IDLE;   
      end   
    end
  endcase
end

////////////////////////////////////////////////////////////
// Name : Status Start
// Set when the process is running.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    sts_start <= 0;
  end else begin
    if (sts_last == 1) begin
      sts_start <= 0;
    end else begin
      if (ctl_start == 1) begin 
        sts_start <= 1; 
      end    
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Status Acquire
// Set when data is being captured.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    sts_acquire <= 0;
  end else begin
    if (sts_last == 1) begin
      sts_acquire <= 0;
    end else begin
      // Start acquire if the pre buffer is enabled,
      // there is no pre buffer and a trigger has been detected or
      // streaming mode
      if (((ctl_start == 1) && (cfg_trig_pre_samp != 0)) ||
          ((state_cs == POST_SAMP_WAIT_TRIG) && (ctl_trig == 1)) ||
          ((ctl_start == 1) && (cfg_mode == STREAM_MODE))) begin
        sts_acquire <= 1; 
      end    
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Status Last
// Set on the last data sample. 
////////////////////////////////////////////////////////////

always @(*)
begin
  sts_last = 0;
  
  if (ctl_stop == 1) begin
    sts_last = 1;    
  end else begin
    case (state_cs)
      // POST_SAMP_FILL - Fills the post sample buffer 
      POST_SAMP_FILL: begin
        if ((s_axis_tvalid == 1) && (sts_trig_post_cnt == cfg_trig_post_samp-1)) begin
          sts_last = 1;  
        end
      end    
    endcase  
  end
end

////////////////////////////////////////////////////////////
// Name : Pre Sample Count
// Counts the pre sample data.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    sts_trig_pre_cnt <= 0;
  end else begin
    if (ctl_rst == 1) begin
      sts_trig_pre_cnt <= 0; 
    end else begin
      if ((s_axis_tvalid == 1) && ((state_cs == PRE_SAMP_FILL) || (state_cs == PRE_SAMP_WAIT_TRIG))) begin
        sts_trig_pre_cnt <= sts_trig_pre_cnt + 1; 
      end
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Pre Sample Overflow
// Set when the counter overflows.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    sts_trig_pre_overflow <= 0;
  end else begin
    if (ctl_rst == 1) begin
      sts_trig_pre_overflow <= 0; 
    end else begin
      if ((sts_trig_pre_overflow == 0) && (sts_trig_pre_cnt == (2**CNT_BITS)-1)) begin
        sts_trig_pre_overflow <= 1;
      end
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Post Sample Count
// Counts the post sample data.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    sts_trig_post_cnt <= 0;
  end else begin
    if (ctl_rst == 1) begin
      sts_trig_post_cnt <= 0; 
    end else begin
      if ((s_axis_tvalid == 1) && (state_cs == POST_SAMP_FILL)) begin
        sts_trig_post_cnt <= sts_trig_post_cnt + 1; 
      end
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Post Sample Overflow
// Set when the counter overflows.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    sts_trig_post_overflow <= 0;
  end else begin
    if (ctl_rst == 1) begin
      sts_trig_post_overflow <= 0; 
    end else begin
      if ((sts_trig_post_overflow == 0) && (sts_trig_post_cnt == (2**CNT_BITS)-1)) begin
        sts_trig_post_overflow <= 1;
      end
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Status Trigger
// Set when the trigger is detected in the trigger detect 
// state.
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    sts_trig <= 0;
  end else begin
    if (ctl_rst == 1) begin
      sts_trig <= 0; 
    end else begin
      case (state_cs)
        // PRE_SAMP_WAIT_TRIG - Wait until we get a trigger
        PRE_SAMP_WAIT_TRIG: begin
          if (ctl_trig == 1) begin
            sts_trig <= 1;
          end
        end    
        
        // POST_SAMP_WAIT_TRIG - Wait until we get a trigger
        POST_SAMP_WAIT_TRIG: begin
          if (ctl_trig == 1) begin
            sts_trig <= 1;
          end
        end        
      endcase      
    end
  end
end

always @(posedge clk)
begin
  if (rst_n == 0) begin
    m_axis_tvalid <= 0;
  end else begin
    if ((sts_acquire == 1) && (s_axis_tvalid == 1)) begin
      m_axis_tvalid <= 1;
    end else begin
      m_axis_tvalid <= 0;
    end
  end  
end

always @(posedge clk)
begin
  m_axis_tlast <= sts_last;
end

always @(posedge clk)
begin
  m_axis_tdata <= s_axis_tdata;
end

endmodule