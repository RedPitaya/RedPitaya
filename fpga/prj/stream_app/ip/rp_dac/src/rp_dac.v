`timescale 1ns / 1ps

module rp_dac
  #(parameter S_AXI_REG_ADDR_BITS   = 12,
    parameter M_AXI_DAC_ADDR_BITS   = 32,
    parameter M_AXI_DAC_DATA_BITS   = 64,
    parameter M_AXI_DAC_DATA_BITS_O = 64,
    parameter DAC_DATA_BITS         = 14,
    parameter AXI_BURST_LEN         = 16,
    parameter EVENT_SRC_NUM         = 7,
    parameter TRIG_SRC_NUM          = 7,
    parameter ID_WIDTH              = 4)(    
  input  wire                                   clk,
  input  wire                                   rst_n,
  output wire                                   intr,

  //
  output wire [DAC_DATA_BITS-1:0]               dac_data_cha_o,
  output wire [DAC_DATA_BITS-1:0]               dac_data_chb_o,
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



  input  wire                                   m_axi_dac1_aclk       ,
  input  wire                                   m_axi_dac1_aresetn    ,
  output wire [ID_WIDTH-1: 0]                   m_axi_dac1_arid_o     ,
  output wire [M_AXI_DAC_ADDR_BITS-1: 0]        m_axi_dac1_araddr_o   ,
  output wire [3:0]                             m_axi_dac1_arlen_o    ,
  output wire [2:0]                             m_axi_dac1_arsize_o   ,
  output wire [1:0]                             m_axi_dac1_arburst_o  ,
  output wire [1:0]                             m_axi_dac1_arlock_o   ,
  output wire [3:0]                             m_axi_dac1_arcache_o  ,
  output wire [2:0]                             m_axi_dac1_arprot_o   ,
  output wire                                   m_axi_dac1_arvalid_o  ,
  input  wire                                   m_axi_dac1_arready_i  ,
  output wire [       3:0]                      m_axi_dac1_arqos_o    ,
  input  wire [ID_WIDTH-1: 0]                   m_axi_dac1_rid_i      ,
  input  wire [ M_AXI_DAC_DATA_BITS-1: 0]       m_axi_dac1_rdata_i    ,
  input  wire [    1: 0]                        m_axi_dac1_rresp_i    ,
  input  wire                                   m_axi_dac1_rlast_i    ,
  input  wire                                   m_axi_dac1_rvalid_i   ,
  output wire                                   m_axi_dac1_rready_o   ,

  input  wire                                   m_axi_dac2_aclk       ,    
  input  wire                                   m_axi_dac2_aresetn    ,    
  output wire [ID_WIDTH-1:0]                    m_axi_dac2_arid_o     ,
  output wire [M_AXI_DAC_ADDR_BITS-1: 0]        m_axi_dac2_araddr_o   ,
  output wire [3:0]                             m_axi_dac2_arlen_o    ,
  output wire [2:0]                             m_axi_dac2_arsize_o   ,
  output wire [1:0]                             m_axi_dac2_arburst_o  ,
  output wire [1:0]                             m_axi_dac2_arlock_o   ,
  output wire [3:0]                             m_axi_dac2_arcache_o  ,
  output wire [2:0]                             m_axi_dac2_arprot_o   ,
  output wire                                   m_axi_dac2_arvalid_o  ,
  input  wire                                   m_axi_dac2_arready_i  ,
  output wire [       3:0]                      m_axi_dac2_arqos_o    ,
  input  wire [ID_WIDTH-1:0]                    m_axi_dac2_rid_i      ,
  input  wire [ M_AXI_DAC_DATA_BITS-1: 0]       m_axi_dac2_rdata_i    ,
  input  wire [1:0]                             m_axi_dac2_rresp_i    ,
  input  wire                                   m_axi_dac2_rlast_i    ,
  input  wire                                   m_axi_dac2_rvalid_i   ,
  output wire                                   m_axi_dac2_rready_o
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
localparam DMA_STS_ADDR         = 8'h2C;

localparam DMA_BUF_SIZE_ADDR    = 8'h34;
localparam DMA_BUF1_ADR_CHA     = 8'h38;
localparam DMA_BUF2_ADR_CHA     = 8'h3C;
localparam DMA_BUF1_ADR_CHB     = 8'h40;
localparam DMA_BUF2_ADR_CHB     = 8'h44;

localparam SETDEC_CHA           = 8'h48; // diagnostic only, to test the ramp signal.
localparam SETDEC_CHB           = 8'h4C;
localparam ERRS_RST             = 8'h50;
localparam ERRS_CNT_CHA         = 8'h54;
localparam ERRS_CNT_CHB         = 8'h58;
localparam LOOPBACK_EN          = 8'h5C;

localparam DIAG_REG_ADDR1     = 8'h60;
localparam DIAG_REG_ADDR2     = 8'h64;
localparam DIAG_REG_ADDR3     = 8'h68;
localparam DIAG_REG_ADDR4     = 8'h6c;

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

reg [16-1:0]                    cfg_cha_setdec;
reg [16-1:0]                    cfg_chb_setdec;

reg [EVENT_SRC_NUM-1:0]         cfg_event_sts;
reg [EVENT_SRC_NUM-1:0]         cfg_event_sel;
reg [TRIG_SRC_NUM -1:0]         cfg_trig_mask;

reg [ 8-1:0]                    cfg_ctrl_reg_cha;
reg [ 8-1:0]                    cfg_ctrl_reg_chb;

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

reg  [31:0]                     errs_cnt_cha, errs_cnt_chb;
reg                             cfg_loopback_cha, cfg_loopback_chb;

reg ctrl_cha;
reg ctrl_chb;
wire sts_cha   = (reg_addr[8-1:0] == DMA_STS_ADDR  ) && (reg_wr_we == 1);
wire sts_chb   = (reg_addr[8-1:0] == DMA_STS_ADDR  ) && (reg_wr_we == 1);
wire event_val = (reg_addr[8-1:0] == EVENT_STS_ADDR    ) && (reg_wr_we == 1);
wire [31:0] ctrl_cha_o; 
wire [31:0] ctrl_chb_o;
wire [31:0] sts_cha_o; 
wire [31:0] sts_chb_o; 

wire [31:0] diag_reg;
wire [31:0] diag_reg2;

`ifdef SIMULATION
  assign reg_wr_we = reg_en & (reg_we == 4'h1);
`else
  assign reg_wr_we = reg_en & (reg_we == 4'hF);
`endif //SIMULATION

assign intr = 1'b0;


reg  [DAC_DATA_BITS-1:0]        dac_a_r, dac_b_r;
reg  [DAC_DATA_BITS-1:0]        dac_a_diff, dac_b_diff;    
always @(posedge clk)
begin
  dac_a_r <= dac_data_cha_o;
  dac_b_r <= dac_data_chb_o;
  dac_a_diff <= dac_data_cha_o - dac_a_r;
  dac_b_diff <= dac_data_chb_o - dac_b_r;
  if (~rst_n) begin
    errs_cnt_cha <= 'h0;
    errs_cnt_chb <= 'h0;
  end else begin
    if ((reg_addr[8-1:0] == ERRS_RST) && (reg_wr_we == 1))
      errs_cnt_cha <= 'h0;  
    else if ((dac_a_diff != cfg_cha_setdec) & (dac_a_diff != 'h0) & (dac_a_diff < 16'h7000))
      errs_cnt_cha <= errs_cnt_cha + 'h1;

    if ((reg_addr[8-1:0] == ERRS_RST) && (reg_wr_we == 1))
      errs_cnt_chb <= 'h0;
    else if ((dac_b_diff != cfg_chb_setdec) & (dac_b_diff != 'h0) & (dac_b_diff < 16'h7000))
      errs_cnt_chb <= errs_cnt_chb + 'h1;
  end

end

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
  .s_axi_bready   (s_axi_reg_bready),
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

    DAC_CHA_SCALE_OFFS:     reg_rd_data <= {2'h0, cfg_cha_offs, 2'h0, cfg_cha_scale};
    DAC_CHA_CNT_STEP:       reg_rd_data <= cfg_cha_step;
    DAC_CHA_CUR_RP:         reg_rd_data <= cfg_cha_rp;
    DAC_CHB_SCALE_OFFS:     reg_rd_data <= {2'h0, cfg_chb_offs, 2'h0, cfg_chb_scale};
    DAC_CHB_CNT_STEP:       reg_rd_data <= cfg_chb_step;
    DAC_CHB_CUR_RP:         reg_rd_data <= cfg_chb_rp;

    EVENT_STS_ADDR:         reg_rd_data <= cfg_event_sts;
    EVENT_SEL_ADDR:         reg_rd_data <= cfg_event_sel;
    TRIG_MASK_ADDR:         reg_rd_data <= cfg_trig_mask;

    DMA_CTRL_ADDR:          reg_rd_data <= {ctrl_chb_o[15:0], ctrl_cha_o[15:0]};      
    DMA_STS_ADDR:           reg_rd_data <= {sts_chb_o[15:0] , sts_cha_o[15:0]};   

    DMA_BUF_SIZE_ADDR:      reg_rd_data <= cfg_dma_buf_size;
    DMA_BUF1_ADR_CHA:       reg_rd_data <= cfg_buf1_adr_cha;
    DMA_BUF1_ADR_CHB:       reg_rd_data <= cfg_buf1_adr_chb;
    DMA_BUF2_ADR_CHA:       reg_rd_data <= cfg_buf2_adr_cha;
    DMA_BUF2_ADR_CHB:       reg_rd_data <= cfg_buf2_adr_chb;     
    ERRS_CNT_CHA:           reg_rd_data <= errs_cnt_cha;                            
    ERRS_CNT_CHB:           reg_rd_data <= errs_cnt_chb;    
    DIAG_REG_ADDR1:         reg_rd_data <= diag_reg;                            
    DIAG_REG_ADDR2:         reg_rd_data <= diag_reg2;                            
    DIAG_REG_ADDR3:         reg_rd_data <= diag_reg;                            
    DIAG_REG_ADDR4:         reg_rd_data <= diag_reg;                            
  endcase
end

always @(posedge clk)
begin
  if (rst_n == 0) begin
    dac_cha_conf <= 16'h0;    
    dac_chb_conf <= 16'h0;

    cfg_cha_scale       <= 14'h0;
    cfg_cha_step        <= 32'h0;
    cfg_chb_scale       <= 14'h0;
    cfg_chb_step        <= 32'h0;

    cfg_event_sts       <=  'h0;
    cfg_event_sel       <=  'h0;
    cfg_trig_mask       <=  'h0;

    cfg_ctrl_reg_cha    <=  8'h0;
    cfg_ctrl_reg_chb    <=  8'h0;


    cfg_dma_buf_size    <= 32'h0;    
    cfg_buf1_adr_cha    <= 32'h0;
    cfg_buf1_adr_chb    <= 32'h0;
    cfg_buf2_adr_cha    <= 32'h0;
    cfg_buf2_adr_chb    <= 32'h0;
    cfg_cha_setdec      <= 16'h1;
    cfg_chb_setdec      <= 16'h1;
    cfg_loopback_cha    <= 16'h0;
    cfg_loopback_chb    <= 16'h0;

  end else begin
    if ((reg_addr[8-1:0] == DAC_CONF_REG      ) && (reg_wr_we == 1)) begin dac_chb_conf       <= reg_wr_data[31:16];  dac_cha_conf  <= reg_wr_data[15:0]; end    
    if ((reg_addr[8-1:0] == DAC_CHA_SCALE_OFFS) && (reg_wr_we == 1)) begin cfg_cha_offs       <= reg_wr_data[29:16];  cfg_cha_scale <= reg_wr_data[13:0]; end    
    if ((reg_addr[8-1:0] == DAC_CHA_CNT_STEP  ) && (reg_wr_we == 1)) begin cfg_cha_step       <= reg_wr_data; end    
  
    if ((reg_addr[8-1:0] == DAC_CHB_SCALE_OFFS) && (reg_wr_we == 1)) begin cfg_chb_offs       <= reg_wr_data[29:16];  cfg_chb_scale <= reg_wr_data[13:0]; end    
    if ((reg_addr[8-1:0] == DAC_CHB_CNT_STEP  ) && (reg_wr_we == 1)) begin cfg_chb_step       <= reg_wr_data; end    

    if ((reg_addr[8-1:0] == EVENT_STS_ADDR    ) && (reg_wr_we == 1)) begin cfg_event_sts      <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == EVENT_SEL_ADDR    ) && (reg_wr_we == 1)) begin cfg_event_sel      <= reg_wr_data; end
    if ((reg_addr[8-1:0] == TRIG_MASK_ADDR    ) && (reg_wr_we == 1)) begin cfg_trig_mask      <= reg_wr_data; end
  
    if ((reg_addr[8-1:0] == DMA_CTRL_ADDR     ) && (reg_wr_we == 1)) begin {cfg_ctrl_reg_chb, cfg_ctrl_reg_cha} <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == DMA_BUF_SIZE_ADDR ) && (reg_wr_we == 1)) begin cfg_dma_buf_size   <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == DMA_BUF1_ADR_CHA  ) && (reg_wr_we == 1)) begin cfg_buf1_adr_cha   <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == DMA_BUF1_ADR_CHB  ) && (reg_wr_we == 1)) begin cfg_buf1_adr_chb   <= reg_wr_data; end   
    if ((reg_addr[8-1:0] == DMA_BUF2_ADR_CHA  ) && (reg_wr_we == 1)) begin cfg_buf2_adr_cha   <= reg_wr_data; end    
    if ((reg_addr[8-1:0] == DMA_BUF2_ADR_CHB  ) && (reg_wr_we == 1)) begin cfg_buf2_adr_chb   <= reg_wr_data; end   

    if ((reg_addr[8-1:0] == SETDEC_CHA        ) && (reg_wr_we == 1)) begin cfg_cha_setdec     <= reg_wr_data[15:0]; end    
    if ((reg_addr[8-1:0] == SETDEC_CHB        ) && (reg_wr_we == 1)) begin cfg_chb_setdec     <= reg_wr_data[15:0]; end 
    if ((reg_addr[8-1:0] == LOOPBACK_EN       ) && (reg_wr_we == 1)) begin cfg_loopback_cha   <= reg_wr_data[0];      cfg_loopback_chb <= reg_wr_data[4]; end   
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
  .M_AXI_DAC_DATA_BITS  (M_AXI_DAC_DATA_BITS_O),
  .DAC_DATA_BITS    (DAC_DATA_BITS), 
  .REG_ADDR_BITS    (REG_ADDR_BITS),
  .EVENT_SRC_NUM    (EVENT_SRC_NUM),
  .TRIG_SRC_NUM     (TRIG_SRC_NUM),
  .AXI_BURST_LEN    (AXI_BURST_LEN),
  .CH_NUM           (0))
  U_dac1(
  .clk              (clk),   
  .rst_n            (rst_n), 
  .axi_clk          (m_axi_dac1_aclk),   
  .axi_rstn         (m_axi_dac1_aresetn), 
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
  .dac_step         (cfg_cha_step),
  .dac_rp           (cfg_cha_rp),
  .dac_trig         (cfg_trig_mask),
  .dac_ctrl_reg     (cfg_ctrl_reg_cha),
  .dac_buf_size     (cfg_dma_buf_size),
  .dac_buf1_adr     (cfg_buf1_adr_cha),
  .dac_buf2_adr     (cfg_buf2_adr_cha),
  .dac_data_o       (dac_data_cha_o),
  .diag_reg         (diag_reg),
  .diag_reg2        (diag_reg2),
  .loopback_en      (cfg_loopback_cha),

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
  .m_axi_dac_arqos_o    (m_axi_dac1_arqos_o),
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
  .M_AXI_DAC_DATA_BITS  (M_AXI_DAC_DATA_BITS_O),
  .DAC_DATA_BITS    (DAC_DATA_BITS), 
  .REG_ADDR_BITS    (REG_ADDR_BITS),
  .EVENT_SRC_NUM    (EVENT_SRC_NUM),
  .TRIG_SRC_NUM     (TRIG_SRC_NUM),
  .AXI_BURST_LEN    (AXI_BURST_LEN),
  .CH_NUM           (1))
  U_dac2(
  .clk              (clk),   
  .rst_n            (rst_n), 
  .axi_clk          (m_axi_dac1_aclk),   
  .axi_rstn         (m_axi_dac1_aresetn), 
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
  .dac_step         (cfg_chb_step),
  .dac_rp           (cfg_chb_rp),
  .dac_trig         (cfg_trig_mask),
  .dac_ctrl_reg     (cfg_ctrl_reg_chb),
  .dac_buf_size     (cfg_dma_buf_size),
  .dac_buf1_adr     (cfg_buf1_adr_chb),
  .dac_buf2_adr     (cfg_buf2_adr_chb),
  .dac_data_o       (dac_data_chb_o),
  .loopback_en      (cfg_loopback_chb),
  
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
  .m_axi_dac_arqos_o    (m_axi_dac2_arqos_o),
  .m_axi_dac_rid_i      (m_axi_dac2_rid_i),
  .m_axi_dac_rdata_i    (m_axi_dac2_rdata_i),
  .m_axi_dac_rresp_i    (m_axi_dac2_rresp_i),
  .m_axi_dac_rlast_i    (m_axi_dac2_rlast_i),
  .m_axi_dac_rvalid_i   (m_axi_dac2_rvalid_i),
  .m_axi_dac_rready_o   (m_axi_dac2_rready_o));      

endmodule