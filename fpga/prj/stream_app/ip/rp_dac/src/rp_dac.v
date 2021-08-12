`timescale 1ns / 1ps

module rp_dac
  #(parameter S_AXI_REG_ADDR_BITS   = 12,
    parameter M_AXI_DAC_ADDR_BITS   = 32,
    parameter M_AXI_DAC_DATA_BITS   = 64,
    parameter DAC_DATA_BITS         = 14,
    parameter EVENT_SRC_NUM         = 7,
    parameter TRIG_SRC_NUM          = 7)(    
  input  wire                                   clk,
  input  wire                                   rst_n,
  output wire                                   intr,

  //
  output  wire [DAC_DATA_BITS-1:0]              dac_data_cha_o,
  output  wire [DAC_DATA_BITS-1:0]              dac_data_chb_o,  
  //
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_trig,
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_stop,
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_start,
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_reset,
  input  wire [TRIG_SRC_NUM-1:0]                trig_ip,
  //
  output wire [3:0]                             dac1_event_op,    
  output wire                                   dac1_trig_op,    
  // 
  output wire [3:0]                             dac2_event_op,      
  output wire                                   dac2_trig_op,  
  //    
  input  wire                                   s_axi_reg_aclk,    
  input  wire                                   s_axi_reg_aresetn,    
  input  wire [S_AXI_REG_ADDR_BITS-1:0]         s_axi_reg_awaddr,     
  input  wire [2:0]                             s_axi_reg_awprot,  
  input  wire                                   s_axi_reg_awvalid,    
  output wire                                   s_axi_reg_awready,                                   
  input  wire [31:0]                            s_axi_reg_wdata,   
  input  wire [3:0]                             s_axi_reg_wstrb,     
  input  wire                                   s_axi_reg_wvalid,     
  output wire                                   s_axi_reg_wready,     
  output wire [1:0]                             s_axi_reg_bresp,       
  output wire                                   s_axi_reg_bvalid,       
  input  wire                                   s_axi_reg_bready,   
  input  wire [S_AXI_REG_ADDR_BITS-1:0]         s_axi_reg_araddr,     
  input  wire [2:0]                             s_axi_reg_arprot,  
  input  wire                                   s_axi_reg_arvalid,    
  output wire                                   s_axi_reg_arready,         
  output wire [31:0]                            s_axi_reg_rdata, 
  output wire [1:0]                             s_axi_reg_rresp,
  output wire                                   s_axi_reg_rvalid,
  input  wire                                   s_axi_reg_rready, 

  input  wire                                   m_axi_dac1_aclk,    
  input  wire                                   m_axi_dac1_aresetn,    
  output wire [3:0]                             m_axi_dac1_arid_o     , // read address ID
  output wire [M_AXI_DAC_ADDR_BITS-1: 0]        m_axi_dac1_araddr_o   , // read address
  output wire [3:0]                             m_axi_dac1_arlen_o    , // read burst length
  output wire [2:0]                             m_axi_dac1_arsize_o   , // read burst size
  output wire [1:0]                             m_axi_dac1_arburst_o  , // read burst type
  output wire [1:0]                             m_axi_dac1_arlock_o   , // read lock type
  output wire [3:0]                             m_axi_dac1_arcache_o  , // read cache type
  output wire [2:0]                             m_axi_dac1_arprot_o   , // read protection type
  output wire                                   m_axi_dac1_arvalid_o  , // read address valid
  input  wire                                   m_axi_dac1_arready_i  , // read address ready
  input  wire [    3: 0]                        m_axi_dac1_rid_i      , // read response ID
  input  wire [M_AXI_DAC_DATA_BITS-1: 0]        m_axi_dac1_rdata_i    , // read data
  input  wire [    1: 0]                        m_axi_dac1_rresp_i    , // read response
  input  wire                                   m_axi_dac1_rlast_i    , // read last
  input  wire                                   m_axi_dac1_rvalid_i   , // read response valid
  output wire                                   m_axi_dac1_rready_o   , // read response ready

  input  wire                                   m_axi_dac2_aclk,    
  input  wire                                   m_axi_dac2_aresetn,    
  output wire [3:0]                             m_axi_dac2_arid_o     , // read address ID
  output wire [M_AXI_DAC_ADDR_BITS-1: 0]        m_axi_dac2_araddr_o   , // read address
  output wire [3:0]                             m_axi_dac2_arlen_o    , // read burst length
  output wire [2:0]                             m_axi_dac2_arsize_o   , // read burst size
  output wire [1:0]                             m_axi_dac2_arburst_o  , // read burst type
  output wire [1:0]                             m_axi_dac2_arlock_o   , // read lock type
  output wire [3:0]                             m_axi_dac2_arcache_o  , // read cache type
  output wire [2:0]                             m_axi_dac2_arprot_o   , // read protection type
  output wire                                   m_axi_dac2_arvalid_o  , // read address valid
  input  wire                                   m_axi_dac2_arready_i  , // read address ready
  input  wire [3:0]                             m_axi_dac2_rid_i      , // read response ID
  input  wire [M_AXI_DAC_DATA_BITS-1: 0]        m_axi_dac2_rdata_i    , // read data
  input  wire [1:0]                             m_axi_dac2_rresp_i    , // read response
  input  wire                                   m_axi_dac2_rlast_i    , // read last
  input  wire                                   m_axi_dac2_rvalid_i   , // read response valid
  output wire                                   m_axi_dac2_rready_o     // read response ready

  //axi4_if.read_ch                               m_axi_dac1,
  //axi4_if.read_ch                               m_axi_dac2,
);


////////////////////////////////////////////////////////////
// Parameters
////////////////////////////////////////////////////////////
    
localparam REG_ADDR_BITS  = 8;

// Regs address map
localparam DAC_CONF_REG         = 8'h0; 

localparam DAC_CHA_SCALE_OFFS   = 8'h4;
localparam DAC_CHA_CNT_STEP     = 8'h8;
localparam DAC_CHA_CUR_RP       = 8'hC;

localparam DAC_CHB_SCALE_OFFS   = 8'h10;
localparam DAC_CHB_CNT_STEP     = 8'h14;
localparam DAC_CHB_CUR_RP       = 8'h18;

localparam EVENT_STS_ADDR       = 8'h1C;
localparam EVENT_SEL_ADDR       = 8'h20;
localparam TRIG_MASK_ADDR       = 8'h24;

localparam DMA_CTRL_ADDR        = 8'h28;
localparam DMA_STS_ADDR_CHA     = 8'h2C;
localparam DMA_STS_ADDR_CHB     = 8'h30; 

localparam DMA_BUF_SIZE_ADDR    = 8'h34;
localparam DMA_BUF1_ADR_CHA     = 8'h38;
localparam DMA_BUF2_ADR_CHA     = 8'h3C;
localparam DMA_BUF1_ADR_CHB     = 8'h40;
localparam DMA_BUF2_ADR_CHB     = 8'h44;

/*
localparam DAC_CONF_REG         = 8'h0; 

localparam DAC_CHA_SCALE_OFFS   = 8'h4;
localparam DAC_CHA_WRAP         = 8'h8;
localparam DAC_CHA_START_OFFS   = 8'hC;
localparam DAC_CHA_CNT_STEP     = 8'h10;
localparam DAC_CHA_CUR_RP       = 8'h14;
localparam DAC_CHA_NUM_CYC      = 8'h18;
localparam DAC_CHA_NUM_REPS     = 8'h1C;
localparam DAC_CHA_BURST_DLY    = 8'h20;

localparam DAC_CHB_SCALE_OFFS   = 8'h24;
localparam DAC_CHB_WRAP         = 8'h28;
localparam DAC_CHB_START_OFFS   = 8'h2C;
localparam DAC_CHB_CNT_STEP     = 8'h30;
localparam DAC_CHB_CUR_RP       = 8'h34;
localparam DAC_CHB_NUM_CYC      = 8'h38;
localparam DAC_CHB_NUM_REPS     = 8'h3C;
localparam DAC_CHB_BURST_DLY    = 8'h40;

localparam EVENT_STS_ADDR       = 8'h50;
localparam EVENT_SEL_ADDR       = 8'h54;
localparam TRIG_MASK_ADDR       = 8'h58;

localparam DMA_CTRL_ADDR_CHA    = 8'h5C;
localparam DMA_CTRL_ADDR_CHB    = 8'h60;
localparam DMA_STS_ADDR_CHA     = 8'h64;
localparam DMA_STS_ADDR_CHB     = 8'h68; 

localparam DMA_BUF_SIZE_ADDR    = 8'h6C;
localparam DMA_BUF_ADR_CHA      = 8'h70;
localparam DMA_BUF_ADR_CHB      = 8'h74;
*/
////////////////////////////////////////////////////////////
// Signals
////////////////////////////////////////////////////////////
reg [16-1:0]                    dac_cha_conf;
reg [16-1:0]                    dac_chb_conf;

reg [DAC_DATA_BITS-1:0]         cfg_cha_scale;
reg [DAC_DATA_BITS-1:0]         cfg_cha_offs;
reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_cha_wrap;
reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_cha_start_offs;
reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_cha_step;
wire [M_AXI_DAC_ADDR_BITS-1:0]  cfg_cha_rp;
reg [16-1:0]                    cfg_cha_num_cyc;
reg [16-1:0]                    cfg_cha_num_reps;
reg [16-1:0]                    cfg_cha_burst_dly;

reg [DAC_DATA_BITS-1:0]         cfg_chb_scale;
reg [DAC_DATA_BITS-1:0]         cfg_chb_offs;
reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_chb_wrap;
reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_chb_start_offs;
reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_chb_step;
wire [M_AXI_DAC_ADDR_BITS-1:0]  cfg_chb_rp;
reg [16-1:0]                    cfg_chb_num_cyc;
reg [16-1:0]                    cfg_chb_num_reps;
reg [16-1:0]                    cfg_chb_burst_dly;

reg [EVENT_SRC_NUM-1:0]         cfg_event_sts;
reg [EVENT_SRC_NUM-1:0]         cfg_event_sel;
reg [TRIG_SRC_NUM -1:0]         cfg_trig_mask;

reg [ 8-1:0]                    cfg_ctrl_reg_cha;
reg [ 8-1:0]                    cfg_ctrl_reg_chb;
wire [ 5-1:0]                   cfg_sts_reg_cha;
wire [ 5-1:0]                   cfg_sts_reg_chb;

reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_dma_buf_size;
reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_buf1_adr_cha;
reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_buf1_adr_chb;
reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_buf2_adr_cha;
reg [M_AXI_DAC_ADDR_BITS-1:0]   cfg_buf2_adr_chb;

wire                            reg_clk;
wire                            reg_rst;
wire [S_AXI_REG_ADDR_BITS-1:0]  reg_addr;
wire                            reg_en;
wire [3:0]                      reg_we;
wire                            reg_wr_we;
wire [31:0]                     reg_wr_data;    
reg  [31:0]                     reg_rd_data;

reg ctrl_cha;
reg ctrl_chb;
wire sts_cha   = (reg_addr[8-1:0] == DMA_STS_ADDR_CHA  ) && (reg_wr_we == 1);
wire sts_chb   = (reg_addr[8-1:0] == DMA_STS_ADDR_CHB  ) && (reg_wr_we == 1);
wire event_val = (reg_addr[8-1:0] == EVENT_STS_ADDR    ) && (reg_wr_we == 1);
wire [31:0] ctrl_cha_o; 
wire [31:0] ctrl_chb_o;
wire [31:0] sts_cha_o; 
wire [31:0] sts_chb_o; 

`ifdef SIMULATION
  assign reg_wr_we = reg_en & (reg_we == 4'h1);
`else
  assign reg_wr_we = reg_en & (reg_we == 4'hF);
`endif //SIMULATION

assign intr = 1'b0;
////////////////////////////////////////////////////////////
// Name : Register Control
// 
////////////////////////////////////////////////////////////   
// probably stays the same as ADC stream //  
reg_ctrl U_reg_ctrl(
  .s_axi_aclk     (s_axi_reg_aclk),       
  .s_axi_aresetn  (s_axi_reg_aresetn), 
  .s_axi_awaddr   (s_axi_reg_awaddr),   
  .s_axi_awprot   (s_axi_reg_awprot),   
  .s_axi_awvalid  (s_axi_reg_awvalid), 
  .s_axi_awready  (s_axi_reg_awready), 
  .s_axi_wdata    (s_axi_reg_wdata),     
  .s_axi_wstrb    (s_axi_reg_wstrb),     
  .s_axi_wvalid   (s_axi_reg_wvalid),   
  .s_axi_wready   (s_axi_reg_wready),   
  .s_axi_bresp    (s_axi_reg_bresp),     
  .s_axi_bvalid   (s_axi_reg_bvalid),  
  .s_axi_bready   (s_axi_reg_bready),   // če se bready ne postavi na 1, se reg_ctrl ustavi 
  .s_axi_araddr   (s_axi_reg_araddr),   
  .s_axi_arprot   (s_axi_reg_arprot),   
  .s_axi_arvalid  (s_axi_reg_arvalid), 
  .s_axi_arready  (s_axi_reg_arready), 
  .s_axi_rdata    (s_axi_reg_rdata),     
  .s_axi_rresp    (s_axi_reg_rresp),     
  .s_axi_rvalid   (s_axi_reg_rvalid),   
  .s_axi_rready   (s_axi_reg_rready),   
  .bram_rst_a     (reg_rst),       
  .bram_clk_a     (reg_clk),       
  .bram_en_a      (reg_en),         
  .bram_we_a      (reg_we),         
  .bram_addr_a    (reg_addr),     
  .bram_wrdata_a  (reg_wr_data), 
  .bram_rddata_a  (reg_rd_data));  


////////////////////////////////////////////////////////////
// Name : Register Read Data
// 
////////////////////////////////////////////////////////////

always @(posedge clk)
begin
  case (reg_addr[8-1:0])
    DAC_CONF_REG:           reg_rd_data <= {dac_chb_conf, dac_cha_conf};

    DAC_CHA_SCALE_OFFS:     reg_rd_data <= {cfg_cha_scale, cfg_cha_offs};
    //DAC_CHA_WRAP:           reg_rd_data <= cfg_cha_wrap;
    //DAC_CHA_START_OFFS:     reg_rd_data <= cfg_cha_start_offs;
    DAC_CHA_CNT_STEP:       reg_rd_data <= cfg_cha_step;
    DAC_CHA_CUR_RP:         reg_rd_data <= cfg_cha_rp;
    //DAC_CHA_NUM_CYC:        reg_rd_data <= cfg_cha_num_cyc;
    //DAC_CHA_NUM_REPS:       reg_rd_data <= cfg_cha_num_reps;
    //DAC_CHA_BURST_DLY:      reg_rd_data <= cfg_cha_burst_dly;

    DAC_CHB_SCALE_OFFS:     reg_rd_data <= {cfg_chb_scale, cfg_chb_offs};
    //DAC_CHB_WRAP:           reg_rd_data <= cfg_chb_wrap;
    //DAC_CHB_START_OFFS:     reg_rd_data <= cfg_chb_start_offs;
    DAC_CHB_CNT_STEP:       reg_rd_data <= cfg_chb_step;
    DAC_CHB_CUR_RP:         reg_rd_data <= cfg_chb_rp;
    //DAC_CHB_NUM_CYC:        reg_rd_data <= cfg_chb_num_cyc;
    //DAC_CHB_NUM_REPS:       reg_rd_data <= cfg_chb_num_reps;
    //DAC_CHB_BURST_DLY:      reg_rd_data <= cfg_chb_burst_dly;

    EVENT_STS_ADDR:         reg_rd_data <= cfg_event_sts;
    EVENT_SEL_ADDR:         reg_rd_data <= cfg_event_sel;
    TRIG_MASK_ADDR:         reg_rd_data <= cfg_trig_mask;

    //DMA_CTRL_ADDR_CHA:      reg_rd_data <= ctrl_cha_o;    
    //DMA_CTRL_ADDR_CHB:      reg_rd_data <= ctrl_chb_o;
    DMA_CTRL_ADDR:          reg_rd_data <= {ctrl_chb_o, ctrl_cha_o};      
    DMA_STS_ADDR_CHA:       reg_rd_data <= sts_cha_o;   
    DMA_STS_ADDR_CHB:       reg_rd_data <= sts_chb_o;   

    DMA_BUF_SIZE_ADDR:      reg_rd_data <= cfg_dma_buf_size;
    DMA_BUF1_ADR_CHA:       reg_rd_data <= cfg_buf1_adr_cha;
    DMA_BUF1_ADR_CHB:       reg_rd_data <= cfg_buf1_adr_chb;
    DMA_BUF2_ADR_CHA:       reg_rd_data <= cfg_buf2_adr_cha;
    DMA_BUF2_ADR_CHB:       reg_rd_data <= cfg_buf2_adr_chb;                            
  endcase
end

always @(posedge clk)
begin
  if (rst_n == 0) begin
    dac_cha_conf <= 16'h0;    
    dac_chb_conf <= 16'h0;

    cfg_cha_scale       <= 14'h0;
    //cfg_cha_offs        <= 14'h0;
    //cfg_cha_wrap        <= 32'h0;    
    //cfg_cha_start_offs  <= 32'h0;
    cfg_cha_step        <= 32'h0;
    //cfg_cha_rp          <= 32'h0;
    //cfg_cha_num_cyc     <= 16'h0;
    //cfg_cha_num_reps    <= 16'h0;
    //cfg_cha_burst_dly   <= 16'h0;

    cfg_chb_scale       <= 14'h0;
    //cfg_chb_offs        <= 14'h0;
    //cfg_chb_wrap        <= 32'h0;    
    //cfg_chb_start_offs  <= 32'h0;
    cfg_chb_step        <= 32'h0;
    //cfg_chb_rp          <= 32'h0;
    //cfg_chb_num_cyc     <= 16'h0;
    //cfg_chb_num_reps    <= 16'h0;
    //cfg_chb_burst_dly   <= 16'h0;

    cfg_event_sts       <=  'h0;
    cfg_event_sel       <=  'h0;
    cfg_trig_mask       <=  'h0;

    cfg_ctrl_reg_cha    <=  8'h0;
    //cfg_ctrl_reg_chb    <=  6'h0;
    //cfg_sts_reg_cha     <=  5'h0;
    //cfg_sts_reg_chb     <=  5'h0;

    cfg_dma_buf_size    <= 32'h0;    
    cfg_buf1_adr_cha    <= 32'h0;
    cfg_buf1_adr_chb    <= 32'h0;
    cfg_buf2_adr_cha    <= 32'h0;
    cfg_buf2_adr_chb    <= 32'h0;
  end else begin
    if ((reg_addr[8-1:0] == DAC_CONF_REG      ) && (reg_wr_we == 1)) begin dac_chb_conf       <= reg_wr_data[31:16];  dac_cha_conf <= reg_wr_data[15:0]; end    

    if ((reg_addr[8-1:0] == DAC_CHA_SCALE_OFFS) && (reg_wr_we == 1)) begin cfg_cha_scale      <= reg_wr_data[29:16];  cfg_cha_offs <= reg_wr_data[13:0]; end    
    //if ((reg_addr[8-1:0] == DAC_CHA_WRAP      ) && (reg_wr_we == 1)) begin cfg_cha_wrap       <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DAC_CHA_START_OFFS) && (reg_wr_we == 1)) begin cfg_cha_start_offs <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == DAC_CHA_CNT_STEP  ) && (reg_wr_we == 1)) begin cfg_cha_step       <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DAC_CHA_CUR_RP    ) && (reg_wr_we == 1)) begin cfg_cha_rp         <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DAC_CHA_NUM_CYC   ) && (reg_wr_we == 1)) begin cfg_cha_num_cyc    <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DAC_CHA_NUM_REPS  ) && (reg_wr_we == 1)) begin cfg_cha_num_reps   <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DAC_CHA_BURST_DLY ) && (reg_wr_we == 1)) begin cfg_cha_burst_dly  <= reg_wr_data; end
  
    if ((reg_addr[8-1:0] == DAC_CHB_SCALE_OFFS) && (reg_wr_we == 1)) begin cfg_chb_scale      <= reg_wr_data[29:16];  cfg_chb_offs <= reg_wr_data[13:0]; end    
    //if ((reg_addr[8-1:0] == DAC_CHB_WRAP      ) && (reg_wr_we == 1)) begin cfg_chb_wrap       <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DAC_CHB_START_OFFS) && (reg_wr_we == 1)) begin cfg_chb_start_offs <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == DAC_CHB_CNT_STEP  ) && (reg_wr_we == 1)) begin cfg_chb_step       <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DAC_CHB_CUR_RP    ) && (reg_wr_we == 1)) begin cfg_chb_rp         <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DAC_CHB_NUM_CYC   ) && (reg_wr_we == 1)) begin cfg_chb_num_cyc    <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DAC_CHB_NUM_REPS  ) && (reg_wr_we == 1)) begin cfg_chb_num_reps   <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DAC_CHB_BURST_DLY ) && (reg_wr_we == 1)) begin cfg_chb_burst_dly  <= reg_wr_data; end    

    if ((reg_addr[8-1:0] == EVENT_STS_ADDR    ) && (reg_wr_we == 1)) begin cfg_event_sts      <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == EVENT_SEL_ADDR    ) && (reg_wr_we == 1)) begin cfg_event_sel      <= reg_wr_data; end
    if ((reg_addr[8-1:0] == TRIG_MASK_ADDR    ) && (reg_wr_we == 1)) begin cfg_trig_mask      <= reg_wr_data; end
  
    if ((reg_addr[8-1:0] == DMA_CTRL_ADDR     ) && (reg_wr_we == 1)) begin {cfg_ctrl_reg_chb, cfg_ctrl_reg_cha} <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DMA_CTRL_ADDR_CHB ) && (reg_wr_we == 1)) begin cfg_ctrl_reg_chb   <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DMA_STS_ADDR_CHA  ) && (reg_wr_we == 1)) begin cfg_sts_reg_cha    <= reg_wr_data; end    
    //if ((reg_addr[8-1:0] == DMA_STS_ADDR_CHB  ) && (reg_wr_we == 1)) begin cfg_sts_reg_chb    <= reg_wr_data; end    

    if ((reg_addr[8-1:0] == DMA_BUF_SIZE_ADDR ) && (reg_wr_we == 1)) begin cfg_dma_buf_size   <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == DMA_BUF1_ADR_CHA  ) && (reg_wr_we == 1)) begin cfg_buf1_adr_cha   <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == DMA_BUF1_ADR_CHB  ) && (reg_wr_we == 1)) begin cfg_buf1_adr_chb   <= reg_wr_data; end   
    if ((reg_addr[8-1:0] == DMA_BUF2_ADR_CHA  ) && (reg_wr_we == 1)) begin cfg_buf2_adr_cha   <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == DMA_BUF2_ADR_CHB  ) && (reg_wr_we == 1)) begin cfg_buf2_adr_chb   <= reg_wr_data; end   
  end
end 

always @(posedge clk ) begin
  ctrl_cha  <= (reg_addr[8-1:0] == DMA_CTRL_ADDR ) && (reg_wr_we == 1);
  ctrl_chb  <= (reg_addr[8-1:0] == DMA_CTRL_ADDR ) && (reg_wr_we == 1);
end


`ifdef SIMULATION
  assign reg_wr_we = reg_en & (reg_we == 4'h1);
`else
  assign reg_wr_we = reg_en & (reg_we == 4'hF);
`endif //SIMULATION

////////////////////////////////////////////////////////////
// Name : DAC 1
// 
////////////////////////////////////////////////////////////     
dac_top #(
  .M_AXI_DAC_ADDR_BITS  (M_AXI_DAC_ADDR_BITS),
  .M_AXI_DAC_DATA_BITS  (M_AXI_DAC_DATA_BITS),
  .DAC_DATA_BITS    (DAC_DATA_BITS), 
  .REG_ADDR_BITS    (REG_ADDR_BITS),
  .EVENT_SRC_NUM    (EVENT_SRC_NUM),
  .TRIG_SRC_NUM     (TRIG_SRC_NUM),
  .CH_NUM           (0))
  U_dac1(
  .clk              (clk),   
  .rst_n            (rst_n), 
  .event_ip_trig    (event_ip_trig),  
  .event_ip_stop    (event_ip_stop),  
  .event_ip_start   (event_ip_start), 
  .event_ip_reset   (event_ip_reset),  
  .event_op_trig    (dac1_event_op[0]),
  .event_op_stop    (dac1_event_op[1]),
  .event_op_start   (dac1_event_op[2]),
  .event_op_reset   (dac1_event_op[3]),
  .event_sel        (cfg_event_sel),
  .event_val        (event_val),
  .trig_ip          (trig_ip),
  .trig_op          (dac1_trig_op),  
  .reg_ctrl         (ctrl_cha_o),
  .ctrl_val         (ctrl_cha),
  .reg_sts          (sts_cha_o),
  .sts_val          (sts_cha),  
  .dac_conf         (dac_cha_conf),
  .dac_scale        (cfg_cha_scale),
  .dac_offs         (cfg_cha_offs),
  //.dac_wrap         (cfg_cha_wrap),
  //.dac_start_offs   (cfg_cha_start_offs),
  .dac_step         (cfg_cha_step),
  .dac_rp           (cfg_cha_rp),
  //.dac_num_cyc      (cfg_cha_num_cyc),
  //.dac_num_reps     (cfg_cha_num_reps),
  //.dac_burst_dly    (cfg_cha_burst_dly),
  .dac_trig         (cfg_trig_mask),
  .dac_ctrl_reg     (cfg_ctrl_reg_cha),
  .dac_sts_reg      (cfg_sts_reg_cha),
  .dac_buf_size     (cfg_dma_buf_size),
  .dac_buf1_adr     (cfg_buf1_adr_cha),
  .dac_buf2_adr     (cfg_buf2_adr_cha),
  .dac_data_o       (dac_data_cha_o),
  .m_axi_dac_arid_o     (m_axi_dac1_arid_o),
  .m_axi_dac_araddr_o   (m_axi_dac1_araddr_o),
  .m_axi_dac_arlen_o    (m_axi_dac1_arlen_o),
  .m_axi_dac_arsize_o   (m_axi_dac1_arsize_o),
  .m_axi_dac_arburst_o  (m_axi_dac1_arburst_o),
  .m_axi_dac_arlock_o   (m_axi_dac1_arlock_o),
  .m_axi_dac_arcache_o  (m_axi_dac1_arcache_o),
  .m_axi_dac_arprot_o   (m_axi_dac1_arprot_o),
  .m_axi_dac_arvalid_o  (m_axi_dac1_arvalid_o),
  .m_axi_dac_arready_i  (m_axi_dac1_arready_i),
  .m_axi_dac_rid_i      (m_axi_dac1_rid_i),
  .m_axi_dac_rdata_i    (m_axi_dac1_rdata_i),
  .m_axi_dac_rresp_i    (m_axi_dac1_rresp_i),
  .m_axi_dac_rlast_i    (m_axi_dac1_rlast_i),
  .m_axi_dac_rvalid_i   (m_axi_dac1_rvalid_i),
  .m_axi_dac_rready_o   (m_axi_dac1_rready_o));
////////////////////////////////////////////////////////////
// Name : DAC 2
// 
////////////////////////////////////////////////////////////     

dac_top #(
  .M_AXI_DAC_ADDR_BITS  (M_AXI_DAC_ADDR_BITS),
  .M_AXI_DAC_DATA_BITS  (M_AXI_DAC_DATA_BITS),
  .DAC_DATA_BITS    (DAC_DATA_BITS), 
  .REG_ADDR_BITS    (REG_ADDR_BITS),
  .EVENT_SRC_NUM    (EVENT_SRC_NUM),
  .TRIG_SRC_NUM     (TRIG_SRC_NUM),
  .CH_NUM           (13))
  U_dac2(
  .clk              (clk),   
  .rst_n            (rst_n), 
  .event_ip_trig    (event_ip_trig),  
  .event_ip_stop    (event_ip_stop),  
  .event_ip_start   (event_ip_start), 
  .event_ip_reset   (event_ip_reset),  
  .event_op_trig    (dac2_event_op[0]),
  .event_op_stop    (dac2_event_op[1]),
  .event_op_start   (dac2_event_op[2]),
  .event_op_reset   (dac2_event_op[3]),
  .event_sel        (cfg_event_sel),
  .event_val        (event_val),
  .trig_ip          (trig_ip),
  .trig_op          (dac2_trig_op),  
  .reg_ctrl         (ctrl_chb_o),
  .ctrl_val         (ctrl_chb),
  .reg_sts          (sts_chb_o),
  .sts_val          (sts_chb), 
  .dac_conf         (dac_chb_conf),
  .dac_scale        (cfg_chb_scale),
  .dac_offs         (cfg_chb_offs),
  //.dac_wrap         (cfg_chb_wrap),
  //.dac_start_offs   (cfg_chb_start_offs),
  .dac_step         (cfg_chb_step),
  .dac_rp           (cfg_chb_rp),
  //.dac_num_cyc      (cfg_chb_num_cyc),
  //.dac_num_reps     (cfg_chb_num_reps),
  //.dac_burst_dly    (cfg_chb_burst_dly),
  .dac_trig         (cfg_trig_mask),
  .dac_ctrl_reg     (cfg_ctrl_reg_chb),
  .dac_sts_reg      (cfg_sts_reg_chb),
  .dac_buf_size     (cfg_dma_buf_size),
  .dac_buf1_adr     (cfg_buf1_adr_chb),
  .dac_buf2_adr     (cfg_buf2_adr_chb),
  .dac_data_o       (dac_data_chb_o),
  .m_axi_dac_arid_o     (m_axi_dac2_arid_o),
  .m_axi_dac_araddr_o   (m_axi_dac2_araddr_o),
  .m_axi_dac_arlen_o    (m_axi_dac2_arlen_o),
  .m_axi_dac_arsize_o   (m_axi_dac2_arsize_o),
  .m_axi_dac_arburst_o  (m_axi_dac2_arburst_o),
  .m_axi_dac_arlock_o   (m_axi_dac2_arlock_o),
  .m_axi_dac_arcache_o  (m_axi_dac2_arcache_o),
  .m_axi_dac_arprot_o   (m_axi_dac2_arprot_o),
  .m_axi_dac_arvalid_o  (m_axi_dac2_arvalid_o),
  .m_axi_dac_arready_i  (m_axi_dac2_arready_i),
  .m_axi_dac_rid_i      (m_axi_dac2_rid_i),
  .m_axi_dac_rdata_i    (m_axi_dac2_rdata_i),
  .m_axi_dac_rresp_i    (m_axi_dac2_rresp_i),
  .m_axi_dac_rlast_i    (m_axi_dac2_rlast_i),
  .m_axi_dac_rvalid_i   (m_axi_dac2_rvalid_i),
  .m_axi_dac_rready_o   (m_axi_dac2_rready_o));      
          
endmodule