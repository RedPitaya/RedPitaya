`timescale 1ns / 1ps

module osc_top
  #(parameter M_AXI_ADDR_BITS   = 32, // DMA Address bits
    parameter M_AXI_DATA_BITS   = 64, // DMA data bits
    parameter S_AXIS_DATA_BITS  = 16, // Stream data bits
    parameter REG_ADDR_BITS     = 12, // Register interface address bits
    parameter DEC_CNT_BITS      = 17, // Decimator counter bits
    parameter DEC_SHIFT_BITS    = 4,  // Decimator shifter bits
    parameter TRIG_CNT_BITS     = 32, // Trigger counter bits
    parameter EVENT_SRC_NUM     = 1,  // Number of event sources
    parameter EVENT_NUM_LOG2    = $clog2(EVENT_SRC_NUM),
    parameter TRIG_SRC_NUM      = 1, // Number of trigger sources
    parameter CTRL_ADDR         = 1, // which address is control
    parameter CHAN_NUM          = 1)( // which channel
  input wire                              clk,
  input wire                              clk_adc,
  input wire                              rst_n,
  // Slave AXI-S
  input  wire [S_AXIS_DATA_BITS-1:0]      s_axis_tdata,
  input  wire                             s_axis_tvalid,
  //
  input  wire [EVENT_SRC_NUM-1:0]         event_ip_trig,
  input  wire [EVENT_SRC_NUM-1:0]         event_ip_stop,
  input  wire [EVENT_SRC_NUM-1:0]         event_ip_start,
  input  wire [EVENT_SRC_NUM-1:0]         event_ip_reset,
  //
  output reg                              event_op_trig,
  output reg                              event_op_stop,
  output reg                              event_op_start,
  output reg                              event_op_reset,   

  input  wire [TRIG_SRC_NUM-1:0]          trig_ip,
  output wire                             trig_op,
  output wire                             ctl_rst,
  //
  input  wire [REG_ADDR_BITS-1:0]         reg_addr,
  input  wire [31:0]                      reg_wr_data,
  input  wire                             reg_wr_we,  
  output reg  [31:0]                      reg_rd_data,
  //
  input  wire                             buf_sel_in,
  output wire                             buf_sel_out,
  //   
  output wire                             dma_intr,
  //
  output wire [(M_AXI_ADDR_BITS-1):0]     m_axi_awaddr,    
  output wire [7:0]                       m_axi_awlen,     
  output wire [2:0]                       m_axi_awsize,    
  output wire [1:0]                       m_axi_awburst,   
  output wire [2:0]                       m_axi_awprot,    
  output wire [3:0]                       m_axi_awcache,   
  output wire                             m_axi_awvalid,   
  input  wire                             m_axi_awready,   
  output wire [M_AXI_DATA_BITS-1:0]       m_axi_wdata,     
  output wire [((M_AXI_DATA_BITS/8)-1):0] m_axi_wstrb,     
  output wire                             m_axi_wlast,     
  output wire                             m_axi_wvalid,    
  input  wire                             m_axi_wready,    
  input  wire [1:0]                       m_axi_bresp,     
  input  wire                             m_axi_bvalid,    
  output wire                             m_axi_bready       
);

////////////////////////////////////////////////////////////
// Parameters
////////////////////////////////////////////////////////////

// Address map
localparam EVENT_STS_ADDR       = 8'h0;   //0 Event status address 
localparam EVENT_SEL_ADDR       = 8'h4;   //4 Event select address
localparam TRIG_MASK_ADDR       = 8'h8;   //8 Trigger mask address
localparam TRIG_PRE_SAMP_ADDR   = 8'h10;  //16 Trigger pre samples address
localparam TRIG_POST_SAMP_ADDR  = 8'h14;  //20 Trigger post samples address
localparam TRIG_PRE_CNT_ADDR    = 8'h18;  //24 Trigger pre count address
localparam TRIG_POST_CNT_ADDR   = 8'h1C;  //28 Trigger post count address
localparam TRIG_LOW_LEVEL_ADDR  = 8'h20;  //32 Trigger low level address
localparam TRIG_HIGH_LEVEL_ADDR = 8'h24;  //36 Trigger high level address
localparam TRIG_EDGE_ADDR       = 8'h28;  //40 Trigger edge address
localparam DEC_FACTOR_ADDR      = 8'h30;  //48 Decimation factor address
localparam DEC_RSHIFT_ADDR      = 8'h34;  //52 Decimation right shift address
localparam AVG_EN_ADDR          = 8'h38;  //56 Average enable address
localparam FILT_BYPASS_ADDR     = 8'h3C;  //60 Filter bypass address
localparam FILT_COEFF_AA_ADDR   = 8'h40;  //64 Filter coeff AA address
localparam FILT_COEFF_BB_ADDR   = 8'h44;  //68 Filter coeff BB address
localparam FILT_COEFF_KK_ADDR   = 8'h48;  //72 Filter coeff KK address
localparam FILT_COEFF_PP_ADDR   = 8'h4C;  //76 Filter coeff PP address

localparam DMA_CTRL_ADDR_CH1    = 8'h50;  //80 DMA control register
localparam DMA_STS_ADDR_CH1     = 8'h54;  //84 DMA status register
localparam DMA_BUF_SIZE_ADDR    = 8'h58;  //96 DMA buffer size
localparam BUF1_LOST_SAMP_CNT_CH1   = 8'h5C;  //108 Number of lost samples in buffer 1
localparam BUF2_LOST_SAMP_CNT_CH1   = 8'h60;  //112 Number of lost samples in buffer 2
localparam DMA_DST_ADDR1_CH1    = 8'h64;  //88 DMA destination address 1
localparam DMA_DST_ADDR2_CH1    = 8'h68;  //92 DMA destination address 2
localparam DMA_DST_ADDR1_CH2    = 8'h6C;  //88 DMA destination address 1
localparam DMA_DST_ADDR2_CH2    = 8'h70;  //92 DMA destination address 2
localparam CALIB_OFFSET_ADDR_CH1= 8'h74;  //100 Calibraton offset CH1
localparam CALIB_GAIN_ADDR_CH1  = 8'h78;  //104 Calibraton gain CH1
localparam CALIB_OFFSET_ADDR_CH2= 8'h7C;  //108 Calibraton offset CH2
localparam CALIB_GAIN_ADDR_CH2  = 8'h80;  //112 Calibraton gain CH2
localparam DMA_CTRL_ADDR_CH2    = 8'h8C;  //80 DMA control register CH2
localparam DMA_STS_ADDR_CH2     = 8'h90;  //84 DMA status register CH2
localparam BUF1_LOST_SAMP_CNT_CH2   = 8'h9C;  //108 Number of lost samples in buffer 1
localparam BUF2_LOST_SAMP_CNT_CH2   = 8'hA0;  //112 Number of lost samples in buffer 2
localparam CURR_WP_CH1 = 8'hA4;  //current write pointer CH1
localparam CURR_WP_CH2 = 8'hA8;  //current write pointer CH2
localparam CURR_DAT_CH1 = 8'hAC;  //current write pointer CH1
localparam CURR_DAT_CH2 = 8'hB0;  //current write pointer CH2

////////////////////////////////////////////////////////////
// Signals
////////////////////////////////////////////////////////////

wire                        dma_mode;

reg  [EVENT_NUM_LOG2-1:0]   cfg_event_sel;
reg  [TRIG_SRC_NUM-1:0]     cfg_trig_mask;
reg                         event_num_trig;
reg                         event_num_stop;
reg                         event_num_start;
reg                         event_num_reset;

wire                        event_sts_trig;
wire                        event_sts_stop;
wire                        event_sts_start;
wire                        event_sts_reset;
wire                        ctl_trg;

reg  [TRIG_CNT_BITS-1:0]    cfg_trig_pre_samp;
wire [TRIG_CNT_BITS-1:0]    sts_trig_pre_cnt;
wire                        sts_trig_pre_overflow;

reg  [TRIG_CNT_BITS-1:0]    cfg_trig_post_samp;
wire [TRIG_CNT_BITS-1:0]    sts_trig_post_cnt;
wire                        sts_trig_post_overflow;

reg  [S_AXIS_DATA_BITS-1:0] cfg_trig_low_level;
reg  [S_AXIS_DATA_BITS-1:0] cfg_trig_high_level;
reg                         cfg_trig_edge;  

reg                         cfg_avg_en; 
reg  [DEC_CNT_BITS-1:0]     cfg_dec_factor;  
reg  [DEC_SHIFT_BITS-1:0]   cfg_dec_rshift;  

reg                         cfg_filt_bypass;  
reg signed [17:0]           cfg_filt_coeff_aa; 
reg signed [24:0]           cfg_filt_coeff_bb; 
reg signed [24:0]           cfg_filt_coeff_kk; 
reg signed [24:0]           cfg_filt_coeff_pp; 

wire [31:0]                 cfg_dma_ctrl_reg;
wire [31:0]                 cfg_dma_sts_reg;
reg  [31:0]                 cfg_dma_dst_addr1;
reg  [31:0]                 cfg_dma_dst_addr2;
reg  [31:0]                 cfg_dma_buf_size;

reg  [15:0]                 cfg_calib_offset;
reg  [15:0]                 cfg_calib_gain;

wire [S_AXIS_DATA_BITS-1:0] calib_tdata;    
wire                        calib_tvalid;   
wire                        calib_tready;   

wire [S_AXIS_DATA_BITS-1:0] dec_tdata;    
wire                        dec_tvalid;   
wire                        dec_tready;   

wire [S_AXIS_DATA_BITS-1:0] trig_tdata;    
wire                        trig_tvalid;   
wire                        trig_tready;   

wire [S_AXIS_DATA_BITS-1:0] acq_tdata;    
wire                        acq_tvalid;   
wire                        acq_tready;   
wire                        acq_tlast;

wire  [31:0]                buf1_ms_cnt;
wire  [31:0]                buf2_ms_cnt;
 
////////////////////////////////////////////////////////////
// Name : Calibration
// 
////////////////////////////////////////////////////////////

osc_calib #(
  .AXIS_DATA_BITS   (S_AXIS_DATA_BITS))
  U_osc_calib(
  .clk              (clk_adc),
  // Slave AXI-S
  .s_axis_tdata     (s_axis_tdata),
  .s_axis_tvalid    (s_axis_tvalid),
  .s_axis_tready    (),
  // Master AXI-S
  .m_axis_tdata     (calib_tdata),
  .m_axis_tvalid    (calib_tvalid),
  .m_axis_tready    (calib_tready),
  // Config
  .cfg_calib_offset (cfg_calib_offset), 
  .cfg_calib_gain   (cfg_calib_gain));

////////////////////////////////////////////////////////////
// Name : Decimation
// 
////////////////////////////////////////////////////////////

osc_decimator #(
  .AXIS_DATA_BITS (S_AXIS_DATA_BITS), 
  .CNT_BITS       (17),
  .SHIFT_BITS     (4))
  U_osc_decimator(
  .clk            (clk_adc),                   
  .rst_n          (rst_n),        
  .s_axis_tdata   (calib_tdata),          
  .s_axis_tvalid  (calib_tvalid),     
  .s_axis_tready  (calib_tready),                                                                 
  .m_axis_tdata   (dec_tdata),          
  .m_axis_tvalid  (dec_tvalid),    
  .m_axis_tready  (dec_tready),      
  .ctl_rst        (event_num_reset),                                                                     
  .cfg_avg_en     (cfg_avg_en),            
  .cfg_dec_factor (cfg_dec_factor),        
  .cfg_dec_rshift (cfg_dec_rshift));       

////////////////////////////////////////////////////////////
// Name : Trigger
// 
////////////////////////////////////////////////////////////

osc_trigger #(
  .AXIS_DATA_BITS       (S_AXIS_DATA_BITS),
  .TRIG_LEVEL_BITS      (S_AXIS_DATA_BITS))
  U_osc_trigger(
  .clk                  (clk_adc),                         
  .rst_n                (rst_n),                                                    
  .ctl_rst              (event_num_reset),                                                    
  .cfg_trig_low_level   (cfg_trig_low_level),          
  .cfg_trig_high_level  (cfg_trig_high_level),         
  .cfg_trig_edge        (cfg_trig_edge),                                                 
  .trig                 (trig_op),                                                    
  .s_axis_tdata         (dec_tdata),                
  .s_axis_tvalid        (dec_tvalid),               
  .s_axis_tready        (dec_tready),                                                          
  .m_axis_tdata         (trig_tdata),                
  .m_axis_tvalid        (trig_tvalid),  
  .m_axis_tready        (trig_tready));                  

////////////////////////////////////////////////////////////
// Name : Acquire
// 
////////////////////////////////////////////////////////////

osc_acquire #(
  .AXIS_DATA_BITS         (S_AXIS_DATA_BITS),
  .CNT_BITS               (TRIG_CNT_BITS))
  U_osc_acq(
  .clk                    (clk_adc),
  .rst_n                  (rst_n),
  .s_axis_tdata           (trig_tdata),     
  .s_axis_tvalid          (trig_tvalid), 
  .s_axis_tready          (trig_tready),                                
  .m_axis_tdata           (acq_tdata),     
  .m_axis_tvalid          (acq_tvalid),    
  .m_axis_tready          (acq_tready),
  .m_axis_tlast           (acq_tlast),  
  .ctl_start              (event_num_start), 
  .ctl_rst                (event_num_reset),   
  .ctl_stop               (event_num_stop),   
  .ctl_trig               (ctl_trg),   
  .cfg_mode               (dma_mode),
  .cfg_trig_pre_samp      (cfg_trig_pre_samp),  
  .cfg_trig_post_samp     (cfg_trig_post_samp),   
  .sts_start              (event_sts_start), 
  .sts_stop               (event_sts_stop),
  .sts_trig               (event_sts_trig),
  .sts_trig_pre_cnt       (sts_trig_pre_cnt),
  .sts_trig_pre_overflow  (sts_trig_pre_overflow),  
  .sts_trig_post_cnt      (sts_trig_post_cnt),
  .sts_trig_post_overflow (sts_trig_post_overflow));    
  
////////////////////////////////////////////////////////////
// Name : DMA S2MM
// 
////////////////////////////////////////////////////////////
  
rp_dma_s2mm #(
  .AXI_ADDR_BITS  (M_AXI_ADDR_BITS),
  .AXI_DATA_BITS  (M_AXI_DATA_BITS),
  .AXIS_DATA_BITS (S_AXIS_DATA_BITS),
  .AXI_BURST_LEN  (16),
  .REG_ADDR_BITS  (REG_ADDR_BITS),
  .CTRL_ADDR      (CTRL_ADDR))
  U_dma_s2mm(
  .m_axi_aclk     (clk),        
  .s_axis_aclk    (clk_adc),      
  .aresetn        (rst_n),  
  .busy           (),
  .intr           (dma_intr),     
  .mode           (dma_mode),  
  .reg_addr       (reg_addr),          
  .reg_wr_data    (reg_wr_data),       
  .reg_wr_we      (reg_wr_we),   
  .reg_ctrl       (cfg_dma_ctrl_reg),
  .reg_sts        (cfg_dma_sts_reg),
  .reg_dst_addr1  (cfg_dma_dst_addr1),
  .reg_dst_addr2  (cfg_dma_dst_addr2),
  .reg_buf_size   (cfg_dma_buf_size),
  .buf1_ms_cnt    (buf1_ms_cnt),
  .buf2_ms_cnt    (buf2_ms_cnt),
  .buf_sel_in     (buf_sel_in),
  .buf_sel_out    (buf_sel_out),
  .m_axi_awaddr   (m_axi_awaddr), 
  .m_axi_awlen    (m_axi_awlen),  
  .m_axi_awsize   (m_axi_awsize), 
  .m_axi_awburst  (m_axi_awburst),
  .m_axi_awprot   (m_axi_awprot), 
  .m_axi_awcache  (m_axi_awcache),
  .m_axi_awvalid  (m_axi_awvalid),
  .m_axi_awready  (m_axi_awready),
  .m_axi_wdata    (m_axi_wdata),  
  .m_axi_wstrb    (m_axi_wstrb),  
  .m_axi_wlast    (m_axi_wlast),  
  .m_axi_wvalid   (m_axi_wvalid), 
  .m_axi_wready   (m_axi_wready), 
  .m_axi_bresp    (m_axi_bresp),  
  .m_axi_bvalid   (m_axi_bvalid), 
  .m_axi_bready   (m_axi_bready), 
  .s_axis_tdata   (acq_tdata),    
  .s_axis_tvalid  (acq_tvalid),  
  .s_axis_tready  (acq_tready),  
  .s_axis_tlast   (acq_tlast));     
  
always @(posedge clk)
begin
  if (rst_n == 0) begin
    event_num_trig  <= 0;    
    event_num_start <= 0;   
    event_num_stop  <= 0;    
    event_num_reset <= 0;   
  end else begin
    event_num_trig  <= event_ip_trig[cfg_event_sel];    
    event_num_start <= event_ip_start[cfg_event_sel];   
    event_num_stop  <= event_ip_stop[cfg_event_sel];     
    event_num_reset <= event_ip_reset[cfg_event_sel];        
  end  
end

assign ctl_rst = event_num_reset;
assign event_sts_reset = 0;

assign ctl_trg = event_num_trig | |(trig_ip & cfg_trig_mask);

////////////////////////////////////////////////////////////
// Name : 
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    event_op_trig   <= 0;
    event_op_stop   <= 0;
    event_op_start  <= 0;
    event_op_reset  <= 0;
  end else begin
    if ((reg_addr[8-1:0] == EVENT_STS_ADDR) && (reg_wr_we == 1)) begin
      event_op_trig   <= reg_wr_data[3];
      event_op_stop   <= reg_wr_data[2];
      event_op_start  <= reg_wr_data[1];
      event_op_reset  <= reg_wr_data[0];    
    end else begin
      event_op_trig   <= 0;
      event_op_stop   <= 0;
      event_op_start  <= 0;
      event_op_reset  <= 0;     
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Event Select
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_event_sel <= 0;
  end else begin
    if ((reg_addr[8-1:0] == EVENT_SEL_ADDR) && (reg_wr_we == 1)) begin
      cfg_event_sel <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Trigger Mask
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_trig_mask <= 0;
  end else begin
    if ((reg_addr[8-1:0] == TRIG_MASK_ADDR) && (reg_wr_we == 1)) begin
      cfg_trig_mask <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Trigger Pre Samples
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_trig_pre_samp <= 0;
  end else begin
    if ((reg_addr[8-1:0] == TRIG_PRE_SAMP_ADDR) && (reg_wr_we == 1)) begin
      cfg_trig_pre_samp <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Trigger Post Samples
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_trig_post_samp <= 0;
  end else begin
    if ((reg_addr[8-1:0] == TRIG_POST_SAMP_ADDR) && (reg_wr_we == 1)) begin
      cfg_trig_post_samp <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Trigger Low Level
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_trig_low_level <= 0;
  end else begin
    if ((reg_addr[8-1:0] == TRIG_LOW_LEVEL_ADDR) && (reg_wr_we == 1)) begin
      cfg_trig_low_level <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Trigger High Level
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_trig_high_level <= 0;
  end else begin
    if ((reg_addr[8-1:0] == TRIG_HIGH_LEVEL_ADDR) && (reg_wr_we == 1)) begin
      cfg_trig_high_level <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Trigger Edge
// 0 = +ve, 1= -ve
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_trig_edge <= 0;
  end else begin
    if ((reg_addr[8-1:0] == TRIG_EDGE_ADDR) && (reg_wr_we == 1)) begin
      cfg_trig_edge <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Decimation Factor
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_dec_factor <= 0;
  end else begin
    if ((reg_addr[8-1:0] == DEC_FACTOR_ADDR) && (reg_wr_we == 1)) begin
      cfg_dec_factor <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Decimation Right Shift
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_dec_rshift <= 0;
  end else begin
    if ((reg_addr[8-1:0] == DEC_RSHIFT_ADDR) && (reg_wr_we == 1)) begin
      cfg_dec_rshift <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Average Enable
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_avg_en <= 0;
  end else begin
    if ((reg_addr[8-1:0] == AVG_EN_ADDR) && (reg_wr_we == 1)) begin
      cfg_avg_en <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Filter Bypass
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_filt_bypass <= 0;
  end else begin
    if ((reg_addr[8-1:0] == FILT_BYPASS_ADDR) && (reg_wr_we == 1)) begin
      cfg_filt_bypass <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Filter Coeff AA
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_filt_coeff_aa <= 0;
  end else begin
    if ((reg_addr[8-1:0] == FILT_COEFF_AA_ADDR) && (reg_wr_we == 1)) begin
      cfg_filt_coeff_aa <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Filter Coeff BB
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_filt_coeff_bb <= 0;
  end else begin
    if ((reg_addr[8-1:0] == FILT_COEFF_BB_ADDR) && (reg_wr_we == 1)) begin
      cfg_filt_coeff_bb <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Filter Coeff KK
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_filt_coeff_kk <= 0;
  end else begin
    if ((reg_addr[8-1:0] == FILT_COEFF_KK_ADDR) && (reg_wr_we == 1)) begin
      cfg_filt_coeff_kk <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : Filter Coeff PP
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_filt_coeff_pp <= 0;
  end else begin
    if ((reg_addr[8-1:0] == FILT_COEFF_PP_ADDR) && (reg_wr_we == 1)) begin
      cfg_filt_coeff_pp <= reg_wr_data;
    end 
  end
end

////////////////////////////////////////////////////////////
// Name : DMA Destination Address 1
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_dma_dst_addr1 <= 0;
  end else begin
    if (((reg_addr[8-1:0] == DMA_DST_ADDR1_CH2 && CHAN_NUM == 'd2) || (reg_addr[8-1:0] == DMA_DST_ADDR1_CH1 && CHAN_NUM == 'd1)) && (reg_wr_we == 1)) begin
      cfg_dma_dst_addr1 <= reg_wr_data;
    end
  end
end

////////////////////////////////////////////////////////////
// Name : DMA Destination Address 2
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_dma_dst_addr2 <= 0;
  end else begin
    if (((reg_addr[8-1:0] == DMA_DST_ADDR2_CH2 && CHAN_NUM == 'd2) || (reg_addr[8-1:0] == DMA_DST_ADDR2_CH1 && CHAN_NUM == 'd1)) && (reg_wr_we == 1)) begin
      cfg_dma_dst_addr2 <= reg_wr_data;
    end
  end
end

////////////////////////////////////////////////////////////
// Name : DMA Buffer Size
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_dma_buf_size <= 0;
  end else begin
    if ((reg_addr[8-1:0] == DMA_BUF_SIZE_ADDR) && (reg_wr_we == 1)) begin
      cfg_dma_buf_size <= reg_wr_data;
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Calibration Offset
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_calib_offset <= 0;
  end else begin
    if (((reg_addr[8-1:0] == CALIB_OFFSET_ADDR_CH1 && CHAN_NUM == 'd1) || (reg_addr[8-1:0] == CALIB_OFFSET_ADDR_CH2 && CHAN_NUM == 'd2)) && (reg_wr_we == 1)) begin
      cfg_calib_offset <= reg_wr_data[S_AXIS_DATA_BITS-1:0];
    end
  end
end

////////////////////////////////////////////////////////////
// Name : Calibration Gain
// 
////////////////////////////////////////////////////////////
always @(posedge clk)
begin
  if (rst_n == 0) begin
    cfg_calib_gain <= 16'h8000; //gain is 1 by default.
  end else begin
    if (((reg_addr[8-1:0] == CALIB_GAIN_ADDR_CH1 && CHAN_NUM == 'd1) || (reg_addr[8-1:0] == CALIB_GAIN_ADDR_CH2 && CHAN_NUM == 'd2)) && (reg_wr_we == 1)) begin
      cfg_calib_gain <= reg_wr_data[S_AXIS_DATA_BITS-1:0];
    end
    
  end
end

////////////////////////////////////////////////////////////
// Name : Register Read Data
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  case (reg_addr[8-1:0])
    EVENT_STS_ADDR:         reg_rd_data <= {28'd0, event_sts_trig, event_sts_stop, event_sts_start, event_sts_reset};
    EVENT_SEL_ADDR:         reg_rd_data <= cfg_event_sel;
    TRIG_MASK_ADDR:         reg_rd_data <= cfg_trig_mask;
    TRIG_PRE_SAMP_ADDR:     reg_rd_data <= cfg_trig_pre_samp;
    TRIG_POST_SAMP_ADDR:    reg_rd_data <= cfg_trig_post_samp;
    TRIG_PRE_CNT_ADDR:      reg_rd_data <= {sts_trig_pre_overflow, sts_trig_pre_cnt[30:0]};
    TRIG_POST_CNT_ADDR:     reg_rd_data <= {sts_trig_post_overflow, sts_trig_post_cnt[30:0]};
    TRIG_LOW_LEVEL_ADDR:    reg_rd_data <= cfg_trig_low_level;     
    TRIG_HIGH_LEVEL_ADDR:   reg_rd_data <= cfg_trig_high_level;  
    TRIG_EDGE_ADDR:         reg_rd_data <= cfg_trig_edge;  
    DEC_FACTOR_ADDR:        reg_rd_data <= cfg_dec_factor;  
    DEC_RSHIFT_ADDR:        reg_rd_data <= cfg_dec_rshift;  
    AVG_EN_ADDR:            reg_rd_data <= cfg_avg_en;  
    CALIB_OFFSET_ADDR_CH1:  reg_rd_data <= cfg_calib_offset;
    CALIB_GAIN_ADDR_CH1:    reg_rd_data <= cfg_calib_gain;
    CALIB_OFFSET_ADDR_CH2:  reg_rd_data <= cfg_calib_offset;
    CALIB_GAIN_ADDR_CH2:    reg_rd_data <= cfg_calib_gain;
    FILT_BYPASS_ADDR:       reg_rd_data <= cfg_filt_bypass;                          
    FILT_COEFF_AA_ADDR:     reg_rd_data <= cfg_filt_coeff_aa;  
    FILT_COEFF_BB_ADDR:     reg_rd_data <= cfg_filt_coeff_bb;  
    FILT_COEFF_KK_ADDR:     reg_rd_data <= cfg_filt_coeff_kk;  
    FILT_COEFF_PP_ADDR:     reg_rd_data <= cfg_filt_coeff_pp;     
    DMA_CTRL_ADDR_CH1:      reg_rd_data <= cfg_dma_ctrl_reg;    
    DMA_STS_ADDR_CH1:       reg_rd_data <= cfg_dma_sts_reg;    
    DMA_CTRL_ADDR_CH2:      reg_rd_data <= cfg_dma_ctrl_reg;    
    DMA_STS_ADDR_CH2:       reg_rd_data <= cfg_dma_sts_reg;   
    DMA_DST_ADDR1_CH1:      reg_rd_data <= cfg_dma_dst_addr1; 
    DMA_DST_ADDR2_CH1:      reg_rd_data <= cfg_dma_dst_addr2; 
    DMA_DST_ADDR1_CH2:      reg_rd_data <= cfg_dma_dst_addr1; 
    DMA_DST_ADDR2_CH2:      reg_rd_data <= cfg_dma_dst_addr2; 
    DMA_BUF_SIZE_ADDR:      reg_rd_data <= cfg_dma_buf_size; 
    BUF1_LOST_SAMP_CNT_CH1: reg_rd_data <= buf1_ms_cnt; 
    BUF2_LOST_SAMP_CNT_CH1: reg_rd_data <= buf2_ms_cnt; 
    BUF1_LOST_SAMP_CNT_CH2: reg_rd_data <= buf1_ms_cnt; 
    BUF2_LOST_SAMP_CNT_CH2: reg_rd_data <= buf2_ms_cnt; 
    CURR_WP_CH1:            reg_rd_data <= m_axi_awaddr;
    CURR_WP_CH2:            reg_rd_data <= m_axi_awaddr;
    default                 reg_rd_data <= 32'd0;                                
  endcase
end

////////////////////////////////////////////////////////////
// Name : DMA Mode
// 0 = Normal
// 1 = Streaming
////////////////////////////////////////////////////////////

//always @(posedge clk)
//begin
//  if (rst_n == 0) begin
//    dma_mode <= 0;
//  end else begin
//    if (cfg_dma_ctrl_reg[DMA_CTRL_STRT] == 1) begin
//      dma_mode <= cfg_dma_ctrl_reg[DMA_CTRL_MODE];
//    end
//  end
//end

endmodule