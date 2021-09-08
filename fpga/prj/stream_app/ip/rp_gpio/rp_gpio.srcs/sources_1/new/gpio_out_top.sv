module gpio_out_top   #(
    parameter S_AXI_REG_ADDR_BITS   = 12,
    parameter M_AXI_GPIO_ADDR_BITS  = 32,
    parameter M_AXI_GPIO_DATA_BITS  = 64,
    parameter CTRL_ADDR             = 0,
    parameter STS_ADDR              = 0,
    parameter GPIO_BITS             = 8,
    parameter ALL_GPIO              = GPIO_BITS*2,
    parameter DCW                   = 17,  // decimation counter width
    parameter CW                    = 32,  // counter width
    parameter DN                    = 1,
    type DT                         = reg [8-1:0])
  ( 
  input                 clk,
  input                 rst_n,
  axi4_stream_if.s      sto,

  // control

  input  reg          ctl_rst,  // synchronous reset
  input  DT           cfg_pol,  // comparator mask
  input  reg          cfg_rle,  // enable RLE

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

  output wire [3:0]                       m_axi_gpio_arid_o     , // read address ID
  output wire [M_AXI_GPIO_ADDR_BITS-1: 0] m_axi_gpio_araddr_o   , // read address
  output wire [3:0]                       m_axi_gpio_arlen_o    , // read burst length
  output wire [2:0]                       m_axi_gpio_arsize_o   , // read burst size
  output wire [1:0]                       m_axi_gpio_arburst_o  , // read burst type
  output wire [1:0]                       m_axi_gpio_arlock_o   , // read lock type
  output wire [3:0]                       m_axi_gpio_arcache_o  , // read cache type
  output wire [2:0]                       m_axi_gpio_arprot_o   , // read protection type
  output wire                             m_axi_gpio_arvalid_o  , // read address valid
  input  wire                             m_axi_gpio_arready_i  , // read address ready
  input  wire [    3: 0]                  m_axi_gpio_rid_i      , // read response ID
  input  wire [M_AXI_GPIO_DATA_BITS-1: 0] m_axi_gpio_rdata_i    , // read data
  input  wire [    1: 0]                  m_axi_gpio_rresp_i    , // read response
  input  wire                             m_axi_gpio_rlast_i    , // read last
  input  wire                             m_axi_gpio_rvalid_i   , // read response valid
  output wire                             m_axi_gpio_rready_o     // read response ready     

);

axi4_stream_if #(.DN (1), .DT (reg [CW+ALL_GPIO-1:0])) sti1  (.ACLK (clk), .ARESETn (rst_n));  // output from FIFO reads


wire [32-1:0] gpio_data_o;
wire          gpio_rvalid;

////////////////////////////////////////////////////////////////////////////////
// bitwise input polarity
////////////////////////////////////////////////////////////////////////////////

// 0 0 0
// 0 1 1
// 1 0 1
// 1 1 0

wire sts_val   = (reg_addr[8-1:0] == STS_ADDR  ) && (reg_wr_we == 1);

reg ctrl_val;
always @(posedge clk) begin
  ctrl_val  <= (reg_addr[8-1:0] == CTRL_ADDR ) && (reg_wr_we == 1);
end

////////////////////////////////////////////////////////////
// Name : DMA MM2S
// 
////////////////////////////////////////////////////////////

gpio_dma_mm2s #(
  .AXI_ADDR_BITS  (M_AXI_GPIO_ADDR_BITS),
  .AXI_DATA_BITS  (M_AXI_GPIO_DATA_BITS),
  .AXIS_DATA_BITS (32),
  .AXI_BURST_LEN  (16))
  U_dma_mm2s(
  .m_axi_aclk     (clk),        
  .s_axis_aclk    (clk),      
  .aresetn        (rst_n),  
  .busy           (),
  .intr           (dma_intr),     
  .mode           (dma_mode),  

  .reg_ctrl       (reg_ctrl),
  .ctrl_val       (ctrl_val),
  .reg_sts        (reg_sts),
  .sts_val        (sts_val),  

  .dac_step         (gpio_step),
  .dac_rp           (gpio_rp),
  .dac_buf_size     (gpio_buf_size),
  .dac_buf1_adr     (gpio_buf1_adr),
  .dac_buf2_adr     (gpio_buf2_adr),

  .dac_trig         (ctl_trg),
  .dac_ctrl_reg     (gpio_ctrl_reg),
  .dac_sts_reg      (gpio_sts_reg),

  .dac_rdata_o      (gpio_data_o),
  .dac_rvalid_o     (gpio_rvalid),
  .gpio_rready_i    (new_read),
  .fifo_r           (fifo_rst),
  
  .m_axi_arid_o   (m_axi_gpio_arid_o), 
  .m_axi_araddr_o    (m_axi_gpio_araddr_o),  
  .m_axi_arlen_o   (m_axi_gpio_arlen_o), 
  .m_axi_arsize_o  (m_axi_gpio_arsize_o),
  .m_axi_arburst_o   (m_axi_gpio_arburst_o), 
  .m_axi_arlock_o  (m_axi_gpio_arlock_o),
  .m_axi_arcache_o  (m_axi_gpio_arcache_o),
  .m_axi_arprot_o  (m_axi_gpio_arprot_o),
  .m_axi_arvalid_o    (m_axi_gpio_arvalid_o),  
  .m_axi_arready_i    (m_axi_gpio_arready_i),  
  .m_axi_rid_i    (m_axi_gpio_rid_i),  
  .m_axi_rdata_i   (m_axi_gpio_rdata_i), 
  .m_axi_rresp_i   (m_axi_gpio_rresp_i), 
  .m_axi_rlast_i    (m_axi_gpio_rlast_i),  
  .m_axi_rvalid_i   (m_axi_gpio_rvalid_i), 
  .m_axi_rready_o   (m_axi_gpio_rready_o));
  
  
assign sti1.TDATA  = {gpio_data_o[31:16],gpio_data_o[7:0]};
assign sti1.TVALID = gpio_rvalid;
assign sti1.TLAST  = 1'b0;


rle_rev #(
  // counter properties
  .CW (8),
  // stream properties
  .DN (DN),
  .BW (ALL_GPIO),
  .DTI (reg [8+ALL_GPIO-1:0]),
  .DTO (reg [  ALL_GPIO-1:0])
) rle_rev1 (
  // input stream input/output
  .sti      (sti1),
  .sto      (sto),
  // configuration
  .ctl_rst  (ctl_rst),
  .cfg_ena  (cfg_rle),
  .new_read (new_read)
);

  endmodule