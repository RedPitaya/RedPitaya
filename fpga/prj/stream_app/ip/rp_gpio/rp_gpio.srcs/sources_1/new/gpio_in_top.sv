module gpio_in_top   #(
    parameter S_AXI_REG_ADDR_BITS   = 12,
    parameter M_AXI_GPIO_ADDR_BITS  = 32,
    parameter M_AXI_GPIO_DATA_BITS  = 64,
    parameter GPIO_BITS             = 8,
    parameter DCW = 17,  // decimation counter width
    parameter CW = 32,  // counter width
    parameter TN =  4,  // trigger number
    parameter TW = 64,   // timestamp width
    parameter DN = 1,
    type DT = reg [8-1:0])( 
  input                 clk,
  input                 rst_n,
  axi4_stream_if.d  sti,
  // control
  input  reg          ctl_rst,  // synchronous reset
  input  reg [CW-1:0] cfg_dec,  // decimation factor
  input  DT             cfg_pol,  // comparator mask
  input  DT             cfg_cmp_msk,  // comparator mask
  input  DT             cfg_cmp_val,  // comparator value
  input  DT             cfg_edg_pos,  // edge positive
  input  DT             cfg_edg_neg,  // edge negative
  output reg          irq_trg,  // trigger
  output reg          irq_stp,  // stop
  output reg          trg_out,
  output reg [CW-1:0] sts_cur,  // current     counter status
  output reg [CW-1:0] sts_lst,  // last packet counter status
  input  reg [TN-1:0] cfg_trg,  // trigger mask
  input  reg          cfg_con,  // continuous
  input  reg          cfg_aut,  // automatic
  input  reg          cfg_rle,  // enable RLE
  input  reg [CW-1:0] cfg_pre,
  output reg [CW-1:0] sts_pre,
  input  reg [CW-1:0] cfg_pst,
  output reg [CW-1:0] sts_pst,
  input  reg          ctl_acq,  // acquire start
  output reg          sts_acq,
  output reg [TW-1:0] cts_acq,
  input  reg [TN-1:0] ctl_trg,
  output reg          sts_trg,
  output reg [TW-1:0] cts_trg,
  input  reg          ctl_stp,  // acquire stop
  output reg [TW-1:0] cts_stp,

  input  wire [S_AXI_REG_ADDR_BITS-1:0]   reg_addr,
  input  wire [31:0]                reg_wr_data,
  input  wire                       reg_wr_we, 

  //
  output                                dma_intr,
  output      [31:0]                    reg_ctrl,
  output      [31:0]                    reg_sts,
  input [M_AXI_GPIO_ADDR_BITS-1:0]      gpio_step,
  input [M_AXI_GPIO_ADDR_BITS-1:0]      gpio_buf_size,
  input [M_AXI_GPIO_ADDR_BITS-1:0]      gpio_buf1_adr,
  input [M_AXI_GPIO_ADDR_BITS-1:0]      gpio_buf2_adr,
  input [32-1:0]                        buf1_ms_cnt,
  input [32-1:0]                        buf2_ms_cnt,
  output [M_AXI_GPIO_ADDR_BITS-1:0]     gpio_wp,
  input  [ 8-1:0]                       gpio_ctrl_reg,
  output [ 5-1:0]                       gpio_sts_reg,

  output wire [(M_AXI_GPIO_ADDR_BITS-1):0]      m_axi_gpio_awaddr,    
  output wire [7:0]                             m_axi_gpio_awlen,     
  output wire [2:0]                             m_axi_gpio_awsize,    
  output wire [1:0]                             m_axi_gpio_awburst,   
  output wire [2:0]                             m_axi_gpio_awprot,    
  output wire [3:0]                             m_axi_gpio_awcache,   
  output wire                                   m_axi_gpio_awvalid,   
  input  wire                                   m_axi_gpio_awready,   
  output wire [M_AXI_GPIO_ADDR_BITS-1:0]        m_axi_gpio_wdata,     
  output wire [((M_AXI_GPIO_DATA_BITS/8)-1):0]  m_axi_gpio_wstrb,     
  output wire                                   m_axi_gpio_wlast,     
  output wire                                   m_axi_gpio_wvalid,    
  input  wire                                   m_axi_gpio_wready,    
  input  wire [1:0]                             m_axi_gpio_bresp,     
  input  wire                                   m_axi_gpio_bvalid,    
  output wire                                   m_axi_gpio_bready  

);

axi4_stream_if #(.DN (DN), .DT (DT)) stn            (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from negator
axi4_stream_if #(.DN (DN), .DT (DT)) std            (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from decimator
axi4_stream_if #(.DN (DN), .DT (DT)) stt            (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from trigger
axi4_stream_if #(.DN (DN), .DT (DT)) sta_str        (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from acquire
axi4_stream_if #(.DN (1), .DT (DT)) sta            (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from acquire
axi4_stream_if #(.DN (1), .DT (DT)) sta2           (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // from acquire

axi4_stream_if #(.DN (1), .DT (reg [8+8-1:0])) sto1  (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // output
axi4_stream_if #(.DN (1), .DT (reg [8+8-1:0])) sto2  (.ACLK (sti.ACLK), .ARESETn (sti.ARESETn));  // output

////////////////////////////////////////////////////////////////////////////////
// Decimation
////////////////////////////////////////////////////////////////////////////////

str_dec #(
  .DN (DN),
  .CW (DCW)
) dec (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_dec  (cfg_dec),
  // streams
  .sti      (sti),
  .sto      (std)
);

////////////////////////////////////////////////////////////////////////////////
// bitwise input polarity
////////////////////////////////////////////////////////////////////////////////

// 0 0 0
// 0 1 1
// 1 0 1
// 1 1 0

assign stn.TDATA  = std.TDATA ^ cfg_pol;
assign stn.TKEEP  = std.TKEEP ;
assign stn.TLAST  = std.TLAST ;
assign stn.TVALID = std.TVALID;

assign std.TREADY = stn.TREADY;

////////////////////////////////////////////////////////////////////////////////
// Edge detection (trigger source)
////////////////////////////////////////////////////////////////////////////////

la_trg #(
  .DT (DT)
) la_trg (
  // control
  .ctl_rst  (ctl_rst),
  // configuration
  .cfg_cmp_msk (cfg_cmp_msk),
  .cfg_cmp_val (cfg_cmp_val),
  .cfg_edg_pos (cfg_edg_pos),
  .cfg_edg_neg (cfg_edg_neg),
  // output triggers
  .sts_trg  (trg_out),
  // stream monitor
  .sti      (stn),
  .sto      (stt)
);

wire [TW-1:0] cts;
cts cts_i (
  // system signals
  .clk  (clk),
  .rstn (rst_n),
  // counter
  .cts  (cts)
);

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

old_acq #(
  .DN (DN),
  .TN (TN),
  .TW (TW),
  .CW (CW)
) acq (
  // stream input/output
  .sti      (stt),
  .sto      (sta_str),
  // current time stamp
  .cts      (cts),
  // interrupts
  .irq_trg  (irq_trg),
  .irq_stp  (irq_stp),
  // control
  .ctl_rst  (ctl_rst),
  // configuration (mode)
  .cfg_trg  (cfg_trg),
  .cfg_con  (cfg_con),
  .cfg_aut  (cfg_aut),
  // configuration/status pre trigger
  .cfg_pre  (cfg_pre),
  .sts_pre  (sts_pre),
  // configuration/status post trigger
  .cfg_pst  (cfg_pst),
  .sts_pst  (sts_pst),
  // control/status/timestamp acquire
  .ctl_acq  (ctl_acq),
  .sts_acq  (sts_acq),
  .cts_acq  (cts_acq),
  // control/status/timestamp trigger
  .ctl_trg  (trg_ext),
  .sts_trg  (sts_trg),
  .cts_trg  (cts_trg),
  // control/status/timestamp stop
  .ctl_stp  (ctl_stp),
  .cts_stp  (cts_stp)
);

assign sta.TDATA  = sta_str.TDATA [0];
assign sta.TKEEP  = sta_str.TKEEP ;
assign sta.TLAST  = sta_str.TLAST ;
assign sta.TVALID = sta_str.TVALID;
assign sta_str.TREADY = sta.TREADY;

assign sta2.TDATA  = sta_str.TDATA [1];
assign sta2.TKEEP  = sta_str.TKEEP ;
assign sta2.TLAST  = sta_str.TLAST ;
assign sta2.TVALID = sta_str.TVALID;

rle #(
  // counter properties
  .CW (8),
  // stream properties
  .DN (DN),
  .DTI (reg [  8-1:0]),
  .DTO (reg [8+8-1:0])
) rle (
  // input stream input/output
  .sti      (sta),
  .sto      (sto1),
  // configuration
  .ctl_rst  (ctl_rst),
  .cfg_ena  (cfg_rle)
);

rle #(
  // counter properties
  .CW (8),
  // stream properties
  .DN (DN),
  .DTI (reg [  8-1:0]),
  .DTO (reg [8+8-1:0])
) rle2 (
  // input stream input/output
  .sti      (sta2),
  .sto      (sto2),
  // configuration
  .ctl_rst  (ctl_rst),
  .cfg_ena  (cfg_rle)
);

axi4_stream_cnt #(
  .DN (DN),
  .CW (CW)
) axi4_stream_cnt (
  // control
  .ctl_rst  (ctl_rst),
  // counter staus
  .sts_cur  (sts_cur),
  .sts_lst  (sts_lst),
  // stream monitor
  .str      (sto1)
);
assign sto2.TREADY = sto1.TREADY;
////////////////////////////////////////////////////////////
// Name : DMA S2MM
// 
////////////////////////////////////////////////////////////

gpio_dma_s2mm #(
  .AXI_ADDR_BITS  (M_AXI_GPIO_ADDR_BITS),
  .AXI_DATA_BITS  (M_AXI_GPIO_DATA_BITS),
  .AXIS_DATA_BITS (32),
  .AXI_BURST_LEN  (16),
  .REG_ADDR_BITS  (S_AXI_REG_ADDR_BITS),
  .CTRL_ADDR      (8'h8C))
  U_dma_s2mm(
  .m_axi_aclk     (clk),        
  .s_axis_aclk    (clk),      
  .aresetn        (rst_n),  
  .busy           (),
  .intr           (dma_intr),     
  .reg_addr       (reg_addr),          
  .reg_wr_data    (reg_wr_data),       
  .reg_wr_we      (reg_wr_we),   
  .reg_ctrl       (reg_ctrl),
  .reg_sts        (reg_sts),
  .reg_dst_addr1  (gpio_buf1_adr),
  .reg_dst_addr2  (gpio_buf2_adr),
  .reg_buf_size   (gpio_buf_size),
  .buf1_ms_cnt    (buf1_ms_cnt),
  .buf2_ms_cnt    (buf2_ms_cnt),
  .buf_sel_out    (buf_sel),
  .buf_sel_in     (buf_sel),

  .m_axi_awaddr   (m_axi_gpio_awaddr), 
  .m_axi_awlen    (m_axi_gpio_awlen),  
  .m_axi_awsize   (m_axi_gpio_awsize), 
  .m_axi_awburst  (m_axi_gpio_awburst),
  .m_axi_awprot   (m_axi_gpio_awprot), 
  .m_axi_awcache  (m_axi_gpio_awcache),
  .m_axi_awvalid  (m_axi_gpio_awvalid),
  .m_axi_awready  (m_axi_gpio_awready),
  .m_axi_wdata    (m_axi_gpio_wdata),  
  .m_axi_wstrb    (m_axi_gpio_wstrb),  
  .m_axi_wlast    (m_axi_gpio_wlast),  
  .m_axi_wvalid   (m_axi_gpio_wvalid), 
  .m_axi_wready   (m_axi_gpio_wready), 
  .m_axi_bresp    (m_axi_gpio_bresp),  
  .m_axi_bvalid   (m_axi_gpio_bvalid), 
  .m_axi_bready   (m_axi_gpio_bready), 

  .s_axis_tdata   ({sto2.TDATA,sto1.TDATA}),    
  .s_axis_tvalid  (sto1.TVALID),  
  .s_axis_tready  (sto1.TREADY),  
  .s_axis_tlast   (sto1.TLAST));  

endmodule