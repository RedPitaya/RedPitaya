`timescale 1ns / 1ps

module rp_gpio #(
    parameter S_AXI_REG_ADDR_BITS   = 12,
    parameter M_AXI_GPIO_ADDR_BITS  = 32,
    parameter M_AXI_GPIO_DATA_BITS  = 64,
    parameter GPIO_BITS             = 8,
    parameter EVENT_SRC_NUM         = 5,
    parameter TRIG_SRC_NUM          = 6,
    parameter DCW = 17,  // decimation counter width
    parameter CW = 32,  // counter width
    parameter TN =  4,  // trigger number
    parameter TW = 64,   // timestamp width
    parameter DN = 2,
    type DT  = reg [  8-1:0],
    type DTO = reg [8+8-1:0]
    )( 
  input  wire                                   clk,
  input  wire                                   rst_n,
  output wire                                   intr,

  //
  inout  wire [GPIO_BITS-1:0]                   exp_p_io,
  inout  wire [GPIO_BITS-1:0]                   exp_n_io,

`ifdef SIMULATION
  output  wire [GPIO_BITS-1:0]                   dirp,
  output  wire [GPIO_BITS-1:0]                   dirn,

  input   wire [GPIO_BITS-1:0]                   gpiop_i,
  input   wire [GPIO_BITS-1:0]                   gpion_i,  

  output  wire [GPIO_BITS-1:0]                   gpiop_o,
  output  wire [GPIO_BITS-1:0]                   gpion_o,  
`endif
  //
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_trig,
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_stop,
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_start,
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_reset,
  input  wire [TRIG_SRC_NUM-1:0]                trig_ip,
  //
  output reg [3:0]                              la_event_op,    
  output wire                                   la_trig_op,    
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

  input  wire                                   m_axi_gpio_out_aclk,    
  input  wire                                   m_axi_gpio_out_aresetn,  
  
  output wire [3:0]                             m_axi_gpio_arid     , // read address ID
  output wire [M_AXI_GPIO_ADDR_BITS-1: 0]       m_axi_gpio_araddr   , // read address
  output wire [3:0]                             m_axi_gpio_arlen    , // read burst length
  output wire [2:0]                             m_axi_gpio_arsize   , // read burst size
  output wire [1:0]                             m_axi_gpio_arburst  , // read burst type
  output wire [1:0]                             m_axi_gpio_arlock   , // read lock type
  output wire [3:0]                             m_axi_gpio_arcache  , // read cache type
  output wire [2:0]                             m_axi_gpio_arprot   , // read protection type
  output wire                                   m_axi_gpio_arvalid  , // read address valid
  input  wire                                   m_axi_gpio_arready  , // read address ready
  input  wire [    3: 0]                        m_axi_gpio_rid      , // read response ID
  input  wire [M_AXI_GPIO_DATA_BITS-1: 0]       m_axi_gpio_rdata    , // read data
  input  wire [    1: 0]                        m_axi_gpio_rresp    , // read response
  input  wire                                   m_axi_gpio_rlast    , // read last
  input  wire                                   m_axi_gpio_rvalid   , // read response valid
  output wire                                   m_axi_gpio_rready   , // read response ready

  input  wire                                   m_axi_gpio_in_aclk,    
  input  wire                                   m_axi_gpio_in_aresetn,  
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


////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// streams
DT gpio_p_i;
DT gpio_n_i;

DT gpio_p_o;
DT gpio_n_o;

axi4_stream_if #(.DN (2), .DT (DT )) sti            (.ACLK (clk), .ARESETn (rst_n));
axi4_stream_if #(.DN (1), .DT (DTO)) sto            (.ACLK (clk), .ARESETn (rst_n));



// acquire regset

reg                         event_num_trig;
reg                         event_num_stop;
reg                         event_num_start;
reg                         event_num_reset;

reg                         event_op_trig;
reg                         event_op_stop;
reg                         event_op_start;
reg                         event_op_reset;
// control
reg           ctl_rst;
// configuration (mode)
reg           cfg_con;  // continuous
reg           cfg_aut;  // automatic

// configuration/status pre trigger
reg  [CW-1:0] cfg_pre;
reg  [CW-1:0] sts_pre;
// configuration/status post trigger
reg  [CW-1:0] cfg_pst;
reg  [CW-1:0] sts_pst;
// control/status/timestamp acquire
reg           ctl_acq;  // acquire start
reg           sts_acq;
reg  [TW-1:0] cts_acq;
// control/status/timestamp trigger
reg           sts_trg;
reg  [TW-1:0] cts_trg;
// control/status/timestamp stop
reg           ctl_stp;  // acquire stop
reg  [TW-1:0] cts_stp;

// trigger
reg  [TN-1:0] cfg_trg;  // trigger select

// trigger source configuration
DT              cfg_cmp_msk;  // comparator mask
DT              cfg_cmp_val;  // comparator value
DT              cfg_edg_pos;  // edge positive
DT              cfg_edg_neg;  // edge negative

// decimation configuration
reg [DCW-1:0] cfg_dec;  // decimation factor

// RLE configuration
reg           cfg_rle;  // RLE enable

// stream counter staus
reg  [CW-1:0] sts_cur;  // current     counter status
reg  [CW-1:0] sts_lst;  // last packet counter status

// bitwise input polarity
DT              cfg_pol;

//event selector
reg [EVENT_SRC_NUM-1: 0] event_sel;
reg [TRIG_SRC_NUM -1: 0] trig_sel;
reg [EVENT_SRC_NUM-1:0]         cfg_event_sts;

reg [ 8-1:0]                    cfg_ctrl_reg_in;
reg [ 8-1:0]                    cfg_ctrl_reg_out;
wire [32-1:0]                   dma_sts_reg_in;
wire [32-1:0]                   dma_sts_reg_out;

reg [16-1:0]                    gpio_outdat;


reg [M_AXI_GPIO_ADDR_BITS-1:0]   cfg_dma_buf_size;
reg [M_AXI_GPIO_ADDR_BITS-1:0]   cfg_buf1_adr_in;
reg [M_AXI_GPIO_ADDR_BITS-1:0]   cfg_buf1_adr_out;
reg [M_AXI_GPIO_ADDR_BITS-1:0]   cfg_buf2_adr_in;
reg [M_AXI_GPIO_ADDR_BITS-1:0]   cfg_buf2_adr_out;
reg [M_AXI_GPIO_ADDR_BITS-1:0]   cfg_dma_step;

wire [32-1:0]                    cfg_ctrl_reg_in_o;
wire [32-1:0]                    cfg_ctrl_reg_out_o;
wire [32-1:0]                    buf1_ms_cnt;
wire [32-1:0]                    buf2_ms_cnt;
wire [M_AXI_GPIO_ADDR_BITS-1:0]   gpio_in_wp;
wire [M_AXI_GPIO_ADDR_BITS-1:0]   gpio_out_rp;


// direction
DT              dir_p;
DT              dir_n;

`ifdef SIMULATION

assign sti.TDATA[0] = gpiop_i ;
assign sti.TDATA[1] = gpion_i ;

always @(posedge sto.ACLK) begin
    if(sto.TVALID)
        gpio_outdat <= sto.TDATA;
end

assign gpiop_o = gpio_outdat[15:8] ;
assign gpion_o = gpio_outdat[ 7:0] ;

assign dirp = dir_p;
assign dirn = dir_n;
`else
IOBUF iobuf_gpio_p [8-1:0] (.O (gpio_p_i), .IO(exp_p_io), .I(gpio_p_o), .T(dir_p));
IOBUF iobuf_gpio_n [8-1:0] (.O (gpio_n_i), .IO(exp_n_io), .I(gpio_n_o), .T(dir_n));

assign sti.TDATA[0] = gpio_p_i ;
assign sti.TDATA[1] = gpio_n_i ;

assign gpio_p_o = sto.TDATA[0][15:8] ;
assign gpio_n_o = sto.TDATA[0][ 7:0] ;

`endif


assign sto.TREADY = 1'b1;

assign sti.TVALID = 1'b1;
assign sti.TKEEP  = 1'b1;
assign sti.TLAST  = 1'b0;

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////
wire                            reg_clk;
wire                            reg_rst;
wire [S_AXI_REG_ADDR_BITS-1:0]  reg_addr;
wire                            reg_en;
wire [3:0]                      reg_we;
wire                            reg_wr_we;
wire [31:0]                     reg_wr_data;    
reg  [31:0]                     reg_rd_data;

`ifdef SIMULATION
  assign reg_wr_we = reg_en & (reg_we == 4'h1);
`else
  assign reg_wr_we = reg_en & (reg_we == 4'hF);
`endif


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
  
// write access
always @(posedge clk)
if (~rst_n) begin
  // acquire regset
  cfg_con <= 1'b0;
  cfg_aut <= 1'b0;
  //cfg_trg <= 'h0;
  cfg_pre <= 'h0;
  cfg_pst <= 'h0;

  // trigger detection
  cfg_cmp_msk <= 'h0;
  cfg_cmp_val <= 'h0;
  cfg_edg_pos <= 'h0;
  cfg_edg_neg <= 'h0;

  // filter/dacimation
  cfg_dec <= 'h0;

  // RLE
  cfg_rle <= 1'b0;

  // bitwise input polarity
  cfg_pol <= 'h0;

  dir_p   <= 'h0;
  dir_n   <= 'h0;

  event_sel     <= 'h0;
  trig_sel      <= 'h0;
  cfg_event_sts <= 'h0;

  cfg_ctrl_reg_in   <= 'h0;
  cfg_ctrl_reg_out  <= 'h0;

  cfg_dma_buf_size  <= 'h0;
  cfg_buf1_adr_in   <= 'h0;
  cfg_buf1_adr_out  <= 'h0;
  cfg_buf2_adr_in   <= 'h0;
  cfg_buf2_adr_out  <= 'h0;
  cfg_dma_step      <= 'h1;

end else begin
  if (reg_wr_we) begin
    // acquire regset
    if (reg_addr[8-1:0]=='h04)   cfg_con <= reg_wr_data[0];
    if (reg_addr[8-1:0]=='h04)   cfg_aut <= reg_wr_data[1];
    if (reg_addr[8-1:0]=='h10)   cfg_pre <= reg_wr_data[CW-1:0];
    if (reg_addr[8-1:0]=='h14)   cfg_pst <= reg_wr_data[CW-1:0];

    // trigger detection
    if (reg_addr[8-1:0]=='h40)   cfg_cmp_msk <= reg_wr_data;
    if (reg_addr[8-1:0]=='h44)   cfg_cmp_val <= reg_wr_data;
    if (reg_addr[8-1:0]=='h48)   cfg_edg_pos <= reg_wr_data;
    if (reg_addr[8-1:0]=='h4C)   cfg_edg_neg <= reg_wr_data;

    // decimation
    if (reg_addr[8-1:0]=='h50)   cfg_dec <= reg_wr_data[DCW-1:0];

    // RLE
    if (reg_addr[8-1:0]=='h54)   cfg_rle <= reg_wr_data[0];

    // bitwise input polarity
    if (reg_addr[8-1:0]=='h60)   cfg_pol <= reg_wr_data;

    // GPIO direction
    if (reg_addr[8-1:0]=='h70)   dir_p   <= reg_wr_data;
    if (reg_addr[8-1:0]=='h74)   dir_n   <= reg_wr_data;

    // trigger selector
    if (reg_addr[8-1:0]=='h80)   event_sel      <= reg_wr_data;
    if (reg_addr[8-1:0]=='h84)   trig_sel       <= reg_wr_data;
    if (reg_addr[8-1:0]=='h88)   cfg_event_sts  <= reg_wr_data;

    if (reg_addr[8-1:0]=='h8C)   cfg_ctrl_reg_in  <= reg_wr_data;
    if (reg_addr[8-1:0]=='h90)   cfg_ctrl_reg_out <= reg_wr_data;


    if (reg_addr[8-1:0]=='h9C)   cfg_dma_buf_size <= reg_wr_data;
    if (reg_addr[8-1:0]=='hA0)   cfg_buf1_adr_in  <= reg_wr_data;
    if (reg_addr[8-1:0]=='hA4)   cfg_buf1_adr_out <= reg_wr_data;
    if (reg_addr[8-1:0]=='hA8)   cfg_buf2_adr_in  <= reg_wr_data;
    if (reg_addr[8-1:0]=='hAC)   cfg_buf2_adr_out <= reg_wr_data;
    if (reg_addr[8-1:0]=='hC0)   cfg_dma_step     <= reg_wr_data;

  end
end


// read access
always_ff @(posedge clk)
begin
  casez (reg_addr[8-1:0])
    // acquire regset
    'h00 : reg_rd_data <= {{32-  4{1'b0}},~sts_acq, sts_acq, sts_trg, 1'b0};
    'h04 : reg_rd_data <= {{32-  2{1'b0}}, cfg_aut, cfg_con};
    'h10 : reg_rd_data <=              32'(cfg_pre); // number of samples pre trigger
    'h14 : reg_rd_data <=              32'(cfg_pst); // number of samples post trigger
    'h18 : reg_rd_data <=              32'(sts_pre); // current number of pre trig samples
    'h1c : reg_rd_data <=              32'(sts_pst); // current number of post trig samples
    'h20 : reg_rd_data <=              32'(cts_acq >>  0);
    'h24 : reg_rd_data <=              32'(cts_acq >> 32);
    'h28 : reg_rd_data <=              32'(cts_trg >>  0);
    'h2c : reg_rd_data <=              32'(cts_trg >> 32);
    'h30 : reg_rd_data <=              32'(cts_stp >>  0);
    'h34 : reg_rd_data <=              32'(cts_stp >> 32);

    // trigger detection
    'h40 : reg_rd_data <=                  cfg_cmp_msk;
    'h44 : reg_rd_data <=                  cfg_cmp_val;
    'h48 : reg_rd_data <=                  cfg_edg_pos;
    'h4c : reg_rd_data <=                  cfg_edg_neg;

    // decimation
    'h50 : reg_rd_data <= {{32-DCW{1'b0}}, cfg_dec};

    // RLE configuration
    'h54 : reg_rd_data <= {{32-  1{1'b0}}, cfg_rle};

    // stream counter status
    'h58 : reg_rd_data <=              32'(sts_cur);
    'h5c : reg_rd_data <=              32'(sts_lst);

    // bitwise input polarity
    'h60 : reg_rd_data <=                cfg_pol;

    // GPIO direction
    'h70 : reg_rd_data <=                  dir_p;
    'h74 : reg_rd_data <=                  dir_n;

    // DMA controls
    'h80 : reg_rd_data <=                  event_sel;
    'h84 : reg_rd_data <=                  trig_sel;
    'h88 : reg_rd_data <=                  cfg_event_sts;
    'h8C : reg_rd_data <=                  cfg_ctrl_reg_in_o;
    'h90 : reg_rd_data <=                  cfg_ctrl_reg_out_o;
    'h94 : reg_rd_data <=                  dma_sts_reg_in;
    'h98 : reg_rd_data <=                  dma_sts_reg_out;
    'h9C : reg_rd_data <=                  cfg_dma_buf_size;
    'hA0 : reg_rd_data <=                  cfg_buf1_adr_in;
    'hA4 : reg_rd_data <=                  cfg_buf1_adr_out;
    'hA8 : reg_rd_data <=                  cfg_buf2_adr_in;
    'hAC : reg_rd_data <=                  cfg_buf2_adr_out;
    'hB0 : reg_rd_data <=                  buf1_ms_cnt;
    'hB4 : reg_rd_data <=                  buf2_ms_cnt;
    'hB8 : reg_rd_data <=                  gpio_in_wp;
    'hBC : reg_rd_data <=                  gpio_out_rp;
    'hC0 : reg_rd_data <=                  cfg_dma_step;

    default : reg_rd_data <= '0;
  endcase
end

////////////////////////////////////////////////////////////////////////////////
// GPIO acq
////////////////////////////////////////////////////////////////////////////////

gpio_in_top #(
  .DN (DN),
  .DT (DT),
  .TN (TRIG_SRC_NUM),
  .M_AXI_GPIO_ADDR_BITS (M_AXI_GPIO_ADDR_BITS),
  .M_AXI_GPIO_DATA_BITS (M_AXI_GPIO_DATA_BITS),
  .S_AXI_REG_ADDR_BITS  (S_AXI_REG_ADDR_BITS),
  .CW (8)
) gpio_in_top (
  .clk(m_axi_gpio_in_aclk),
  .rst_n(m_axi_gpio_in_aresetn),

  // input data stream
  .sti(sti),
  //interrupt
  .irq_trg  (trg_intr),
  .dma_intr (dma_intr),
  //.irq_stp  (irq_stp),

  // processing settings
  .cfg_dec  (cfg_dec),
  .cfg_pol  (cfg_pol),
  .cfg_cmp_msk (cfg_cmp_msk),
  .cfg_cmp_val (cfg_cmp_val),
  .cfg_edg_pos (cfg_edg_pos),
  .cfg_edg_neg (cfg_edg_neg),
  .cfg_con  (1'b1),
  .cfg_aut  (1'b0),
  .cfg_pre  (cfg_pre),
  .sts_pre  (sts_pre),
  .cfg_pst  (cfg_pst),
  .sts_pst  (sts_pst),
  .sts_acq  (sts_acq),
  .cts_acq  (cts_acq),
  .sts_trg  (sts_trg),
  .cts_trg  (cts_trg),
  .cts_stp  (cts_stp),
  .cfg_rle  (cfg_rle),
  .sts_cur  (sts_cur),
  .sts_lst  (sts_lst),

  // trigger
  .cfg_trg  (trig_sel),
  .ctl_trg  (trig_ip),
  .trg_out  (la_trig_op),

  // DMA stuff
  .reg_wr_data      (reg_wr_data),       
  .reg_wr_we        (reg_wr_we),   
  .reg_addr         (reg_addr),          

  .reg_ctrl         (cfg_ctrl_reg_in_o),
  
  .ctl_rst          (event_num_reset),
  .ctl_acq          (event_num_start),
  .ctl_stp          (event_num_stop),
  .reg_sts          (dma_sts_reg_in),
  .gpio_ctrl_reg     (cfg_ctrl_reg_in),
  .gpio_sts_reg      (cfg_sts_reg_in),
  .gpio_buf_size     (cfg_dma_buf_size),
  .gpio_buf1_adr     (cfg_buf1_adr_in),
  .gpio_buf2_adr     (cfg_buf2_adr_in),
  .buf1_ms_cnt      (buf1_ms_cnt),
  .buf2_ms_cnt      (buf2_ms_cnt),
  .gpio_wp          (gpio_in_wp),

  // AXI
  .m_axi_gpio_awaddr     (m_axi_gpio_awaddr), 
  .m_axi_gpio_awlen      (m_axi_gpio_awlen),  
  .m_axi_gpio_awsize     (m_axi_gpio_awsize), 
  .m_axi_gpio_awburst    (m_axi_gpio_awburst),
  .m_axi_gpio_awprot     (m_axi_gpio_awprot), 
  .m_axi_gpio_awcache    (m_axi_gpio_awcache),
  .m_axi_gpio_awvalid    (m_axi_gpio_awvalid),
  .m_axi_gpio_awready    (m_axi_gpio_awready),
  .m_axi_gpio_wdata      (m_axi_gpio_wdata),  
  .m_axi_gpio_wstrb      (m_axi_gpio_wstrb),  
  .m_axi_gpio_wlast      (m_axi_gpio_wlast),  
  .m_axi_gpio_wvalid     (m_axi_gpio_wvalid), 
  .m_axi_gpio_wready     (m_axi_gpio_wready), 
  .m_axi_gpio_bresp      (m_axi_gpio_bresp),  
  .m_axi_gpio_bvalid     (m_axi_gpio_bvalid), 
  .m_axi_gpio_bready     (m_axi_gpio_bready)
);

////////////////////////////////////////////////////////////////////////////////
// GPIO output
////////////////////////////////////////////////////////////////////////////////
gpio_out_top #(
  .DN (DN),
  .DT (DT),
  .GPIO_BITS (GPIO_BITS),
  .M_AXI_GPIO_ADDR_BITS (M_AXI_GPIO_ADDR_BITS),
  .M_AXI_GPIO_DATA_BITS (M_AXI_GPIO_DATA_BITS),
  .S_AXI_REG_ADDR_BITS  (S_AXI_REG_ADDR_BITS),
  .CTRL_ADDR('h90),
  .STS_ADDR('h98),
  .CW (8)
) gpio_out_top (
  .clk              (m_axi_gpio_out_aclk),
  .rst_n            (m_axi_gpio_out_aresetn),

  // output data stream
  .sto              (sto),

  // processing settings
  .cfg_pol          (cfg_pol),
  .cfg_rle          (cfg_rle),

  // DMA stuff
  .reg_wr_data      (reg_wr_data),       
  .reg_wr_we        (reg_wr_we),   
  .reg_addr         (reg_addr),
  .reg_ctrl         (cfg_ctrl_reg_out_o),
  
  .ctl_rst          (event_num_reset),
  .reg_sts          (dma_sts_reg_out),
  .gpio_ctrl_reg    (cfg_ctrl_reg_out),
  .gpio_sts_reg     (cfg_sts_reg_out),
  .gpio_step        (cfg_dma_step),
  .gpio_buf_size    (cfg_dma_buf_size),
  .gpio_buf1_adr    (cfg_buf1_adr_out),
  .gpio_buf2_adr    (cfg_buf2_adr_out),
  .buf1_ms_cnt      (buf1_ms_cnt),
  .buf2_ms_cnt      (buf2_ms_cnt),
  .gpio_wp          (gpio_out_wp),

  // AXI
  .m_axi_gpio_arid_o    (m_axi_gpio_arid), 
  .m_axi_gpio_araddr_o  (m_axi_gpio_araddr),  
  .m_axi_gpio_arlen_o   (m_axi_gpio_arlen), 
  .m_axi_gpio_arsize_o  (m_axi_gpio_arsize),
  .m_axi_gpio_arburst_o (m_axi_gpio_arburst), 
  .m_axi_gpio_arlock_o  (m_axi_gpio_arlock),
  .m_axi_gpio_arcache_o (m_axi_gpio_arcache),
  .m_axi_gpio_arprot_o  (m_axi_gpio_arprot),
  .m_axi_gpio_arvalid_o (m_axi_gpio_arvalid),  
  .m_axi_gpio_arready_i (m_axi_gpio_arready),  
  .m_axi_gpio_rid_i     (m_axi_gpio_rid),  
  .m_axi_gpio_rdata_i   (m_axi_gpio_rdata), 
  .m_axi_gpio_rresp_i   (m_axi_gpio_rresp), 
  .m_axi_gpio_rlast_i   (m_axi_gpio_rlast),  
  .m_axi_gpio_rvalid_i  (m_axi_gpio_rvalid), 
  .m_axi_gpio_rready_o  (m_axi_gpio_rready)   
);


always @(posedge clk)
begin
  if (rst_n == 0) begin
    event_num_trig  <= 0;    
    event_num_start <= 0;   
    event_num_stop  <= 0;    
    event_num_reset <= 0;   
  end else begin
    event_num_trig  <= event_ip_trig[event_sel];    
    event_num_start <= event_ip_start[event_sel];   
    event_num_stop  <= event_ip_stop[event_sel];     
    event_num_reset <= event_ip_reset[event_sel];        
  end  
end

assign ctl_trg = event_num_trig | |(trig_ip & trig_sel);
assign intr = dma_intr;

////////////////////////////////////////////////////////////
// Name : 
// 
////////////////////////////////////////////////////////////
wire event_val = (reg_addr[8-1:0] == 8'h88) && (reg_wr_we == 1);

always @(posedge clk)
begin
  if (rst_n == 0) begin
    la_event_op <= 'h0;
  end else begin
    if (event_val) begin
      la_event_op <= cfg_event_sts;   
    end else begin
      la_event_op <= 'h0;
    end
  end
end       
endmodule
