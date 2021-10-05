`timescale 1ns / 1ps

module rp_dma_mm2s
  #(parameter AXI_ADDR_BITS   = 32,                    
    parameter AXI_DATA_BITS   = 64,                        
    parameter AXIS_DATA_BITS  = 16,              
    parameter AXI_BURST_LEN   = 16,
    parameter REG_ADDR_BITS   = 1)(    
  input  wire                           m_axi_aclk,
  input  wire                           s_axis_aclk,   
  input  wire                           aresetn,      
  //
  output wire                           busy,
  output wire                           intr,
  output wire                           mode,  
  //
  output      [31:0]                    reg_ctrl,
  input  wire                           ctrl_val, 
  output      [31:0]                    reg_sts,
  input  wire                           sts_val, 

  //input [AXI_ADDR_BITS-1:0]             dac_wrap,
  //input [AXI_ADDR_BITS-1:0]             dac_start_offs,
  input [AXI_ADDR_BITS-1:0]             dac_step,
  input [AXI_ADDR_BITS-1:0]             dac_buf_size,
  input [AXI_ADDR_BITS-1:0]             dac_buf1_adr,
  input [AXI_ADDR_BITS-1:0]             dac_buf2_adr,
  output [AXI_ADDR_BITS-1:0]            dac_rp,


  input                                 dac_trig,
  input  [ 8-1:0]                       dac_ctrl_reg,
  output [ 5-1:0]                       dac_sts_reg,

  //
  output wire [AXIS_DATA_BITS-1: 0]     dac_rdata_o,
  output wire                           dac_rvalid_o,
  output [32-1:0]                       diag_reg,

  // 
  output wire [3:0]                       m_axi_arid_o     , // read address ID
  output wire [AXI_ADDR_BITS-1: 0]        m_axi_araddr_o   , // read address
  output wire [7:0]                       m_axi_arlen_o    , // read burst length
  output wire [2:0]                       m_axi_arsize_o   , // read burst size
  output wire [1:0]                       m_axi_arburst_o  , // read burst type
  output wire [1:0]                       m_axi_arlock_o   , // read lock type
  output wire [3:0]                       m_axi_arcache_o  , // read cache type
  output wire [2:0]                       m_axi_arprot_o   , // read protection type
  output wire                             m_axi_arvalid_o  , // read address valid
  input  wire                             m_axi_arready_i  , // read address ready
  input  wire [    3: 0]                  m_axi_rid_i      , // read response ID
  input  wire [AXI_DATA_BITS-1: 0]        m_axi_rdata_i    , // read data
  input  wire [    1: 0]                  m_axi_rresp_i    , // read response
  input  wire                             m_axi_rlast_i    , // read last
  input  wire                             m_axi_rvalid_i   , // read response valid
  output wire                             m_axi_rready_o     // read response ready                   
 );

////////////////////////////////////////////////////////////
// Parameters
////////////////////////////////////////////////////////////

localparam FIFO_CNT_BITS = 10;  // Size of the FIFO data counter

////////////////////////////////////////////////////////////
// Signals
////////////////////////////////////////////////////////////

wire                      fifo_rst;
wire [AXI_DATA_BITS-1:0]  fifo_wr_data; 
wire                      fifo_wr_we;
wire [AXI_DATA_BITS-1:0]  fifo_rd_data;
wire                      fifo_rd_re;
wire                      fifo_empty;
wire [FIFO_CNT_BITS-1:0]  fifo_rd_cnt;
wire [7:0]                req_data;
wire                      req_we;
wire                      fifo_we_dat;
wire                      dac_word_wr;
wire                      dac_word_rd;
//wire                      fifo_dis;

wire [AXIS_DATA_BITS-1:0] downsized_data;
wire                      downsized_valid;

// DMA control reg
localparam CTRL_STRT            = 0;
localparam CTRL_RESET           = 1;
localparam CTRL_MODE_NORM       = 4;
localparam CTRL_MODE_STREAM     = 5;

wire                      ctrl_start = dac_ctrl_reg[CTRL_STRT];
wire                      ctrl_reset = dac_ctrl_reg[CTRL_RESET];
wire                      ctrl_norm  = dac_ctrl_reg[CTRL_MODE_NORM];
wire                      ctrl_strm  = dac_ctrl_reg[CTRL_MODE_STREAM];

assign dac_rdata_o  = downsized_data;
//assign dac_rdata_o  = m_axi_araddr_o[13:0];

assign dac_rvalid_o = downsized_valid;

assign fifo_wr_data  = m_axi_rdata_i;
//assign fifo_wr_we = m_axi_rvalid_i && m_axi_rready_o && fifo_we_dat;
assign fifo_wr_we = we_dat_r2;
//assign m_axi_rready_o  = 1'b1;
assign m_axi_arlock_o  = 2'b00;


//assign m_axi_dac_araddr_o  = req_addr;
assign m_axi_arsize_o  = $clog2(AXI_DATA_BITS/8);   
assign m_axi_arburst_o = 2'b01;     // INCR
assign m_axi_arprot_o  = 3'b000;
assign m_axi_arcache_o = 4'b0011;
assign m_axi_arid_o = 4'h0;


/* not needed in MM2S
  input  wire [M_AXI_DAC_ADDR_BITS-1: 0]  m_axi_dac_rdata_i    , // read data
*/  

/* needed in MM2S
  output wire [M_AXI_DAC_ADDR_BITS-1: 0]  m_axi_dac_araddr_o   , // read address
  output wire [3:0]                       m_axi_dac_arlen_o    , // read burst length
  output wire                             m_axi_dac_arvalid_o  , // read address valid
  input  wire                             m_axi_dac_arready_i  , // read address ready
  input  wire                             m_axi_dac_rvalid_i   , // read response valid      
  input  wire                             m_axi_dac_rlast_i    , // read last
  output wire                             m_axi_dac_rready_o     // read response ready                   
*/

/* UNUSED OUTPUTS
  output wire [3:0]                       m_axi_dac_arid_o     , // read address ID
  output wire [2:0]                       m_axi_dac_arsize_o   , // read burst size
  output wire [1:0]                       m_axi_dac_arburst_o  , // read burst type
  output wire [3:0]                       m_axi_dac_arcache_o  , // read cache type
  output wire [2:0]                       m_axi_dac_arprot_o   , // read protection type
  output wire [1:0]                       m_axi_dac_arlock_o   , // read lock type
*/

/* UNUSED INPUTS
  input  wire [    3: 0]                  m_axi_dac_rid_i      , // read response ID
  input  wire [    1: 0]                  m_axi_dac_rresp_i    , // read response
*/
////////////////////////////////////////////////////////////
// Name : DMA MM2S Control
// Accepts DMA requests and sends data over the AXI bus.
////////////////////////////////////////////////////////////

rp_dma_mm2s_ctrl #(
  .AXI_ADDR_BITS  (AXI_ADDR_BITS),
  .AXI_DATA_BITS  (AXI_DATA_BITS),
  .AXI_BURST_LEN  (AXI_BURST_LEN),
  .FIFO_CNT_BITS  (FIFO_CNT_BITS),
  .REG_ADDR_BITS  (REG_ADDR_BITS))
  U_dma_mm2s_ctrl(
  .m_axi_aclk     (m_axi_aclk),         
  .s_axis_aclk    (s_axis_aclk),       
  .m_axi_aresetn  (aresetn),     
  .busy           (busy),
  .intr           (intr),      
  .mode           (mode),    
  .req_data       (req_data),
  .req_we         (req_we),
  .reg_ctrl       (reg_ctrl),
  .ctrl_val       (ctrl_val),
  .reg_sts        (reg_sts),
  .sts_val        (sts_val),  
  .ctrl_reset     (ctrl_reset),
  .ctrl_start     (ctrl_start),
  .ctrl_norm      (ctrl_norm),
  .ctrl_strm      (ctrl_norm),
  .data_valid     (downsized_valid),
  //.dac_conf         (dac_conf),
  //.dac_wrap         (dac_wrap),
  //.dac_start_offs   (dac_start_offs),
  .dac_pntr_step    (dac_step),
  .dac_rp           (dac_rp),
  .dac_word         (dac_word),
  .dac_buf_size     (dac_buf_size),
  .dac_buf1_adr     (dac_buf1_adr),
  .dac_buf2_adr     (dac_buf2_adr),
  .dac_trig         (dac_trig),
  .dac_ctrl_reg     (dac_ctrl_reg),
  .dac_sts_reg      (dac_sts_reg),
  //.fifo_dis       (fifo_dis),
  .fifo_rst         (fifo_rst),
  .fifo_we_dat      (fifo_we_dat),
  .diag_reg         (diag_reg),
  .m_axi_dac_araddr_o   (m_axi_araddr_o),       
  .m_axi_dac_arlen_o    (m_axi_arlen_o),      
  .m_axi_dac_arvalid_o  (m_axi_arvalid_o), 
  .m_axi_dac_rready_o   (m_axi_rready_o),            
  .m_axi_dac_arready_i  (m_axi_arready_i), 
  .m_axi_dac_rvalid_i   (m_axi_rvalid_i),    
  .m_axi_dac_rlast_i    (m_axi_rlast_i));      
 
////////////////////////////////////////////////////////////
// Name : DMA MM2S Downsize 
// Packs input data into the AXI bus width. 
////////////////////////////////////////////////////////////

reg valid_reg;
reg last_reg;
always @(posedge s_axis_aclk) begin
//  valid_reg <= fifo_rd_re;
valid_reg <= m_axi_rvalid_i;
last_reg <= 1'b0;
end

reg we_dat_r1, we_dat_r2;
always @(posedge s_axis_aclk) begin
//  valid_reg <= fifo_rd_re;
we_dat_r1 <= m_axi_rvalid_i && m_axi_rready_o && fifo_we_dat;
we_dat_r2 <= we_dat_r1;
end
rp_dma_mm2s_downsize #(
  .AXI_DATA_BITS  (AXI_DATA_BITS),
  .AXIS_DATA_BITS (AXIS_DATA_BITS))
  U_dma_mm2s_downsize(
  .clk            (s_axis_aclk),              
  .rst            (fifo_rst),    
  .req_data       (req_data),
  .req_we         (req_we),       
  .fifo_empty     (fifo_empty),
  .fifo_rd_data   (fifo_rd_data),      
  .fifo_valid     (valid_reg),     
  .fifo_rd_re     (fifo_rd_re),     
  .fifo_last      (last_reg), 
  .word_sel       (dac_word_rd),
  .m_axis_tdata   (downsized_data),      
  .m_axis_tvalid  (downsized_valid),     
  .m_axis_tready  (fifo_empty));      


////////////////////////////////////////////////////////////
// Name : Data FIFO
// Stores the data to transfer.
////////////////////////////////////////////////////////////

fifo_axi_data 
  U_fifo_axi_data(
  .wr_clk         (m_axi_aclk),               
  .rd_clk         (s_axis_aclk),               
  .rst            (fifo_rst),     
  .din            ({dac_word_wr, fifo_wr_data}),                     
  .wr_en          (fifo_wr_we),// && ~fifo_dis),               
  .full           (fifo_full),   
  .dout           ({dac_word_rd, fifo_rd_data}),    
  .rd_en          (fifo_rd_re),                                 
  .empty          (fifo_empty),                 
  .rd_data_count  (fifo_rd_cnt), 
  .wr_rst_busy    (),     
  .rd_rst_busy    ());
  
//assign fifo_rd_re = m_axi_wvalid & m_axi_wready;  
 
endmodule