`timescale 1ns / 1ps

module rp_dma_s2mm
  #(parameter AXI_ADDR_BITS   = 32,                    
    parameter AXI_DATA_BITS   = 64,                        
    parameter AXIS_DATA_BITS  = 16,              
    parameter AXI_BURST_LEN   = 16,
    parameter REG_ADDR_BITS   = 1,
    parameter CTRL_ADDR       = 1)(    
  input  wire                           m_axi_aclk,
  input  wire                           s_axis_aclk,   
  input  wire                           aresetn,      
  //
  output wire                           busy,
  output wire                           intr,
  output wire                           mode,
  //
  input  wire [REG_ADDR_BITS-1:0]       reg_addr,
  input  wire [31:0]                    reg_wr_data,
  input  wire                           reg_wr_we,      
  //
  output wire [31:0]                    reg_ctrl,
  output wire [31:0]                    reg_sts,  
  input  wire [31:0]                    reg_dst_addr1,  
  input  wire [31:0]                    reg_dst_addr2,  
  input  wire [31:0]                    reg_buf_size,
  //
  output wire [31:0]                    buf1_ms_cnt,
  output wire [31:0]                    buf2_ms_cnt,
  input  wire                           buf_sel_in,
  output wire                           buf_sel_out,
  //                                        
  output wire [(AXI_ADDR_BITS-1):0]     m_axi_awaddr,     
  output wire [7:0]                     m_axi_awlen,      
  output wire [2:0]                     m_axi_awsize,     
  output wire [1:0]                     m_axi_awburst,    
  output wire [2:0]                     m_axi_awprot,     
  output wire [3:0]                     m_axi_awcache,    
  output wire                           m_axi_awvalid,    
  input  wire                           m_axi_awready,     
  output wire [AXI_DATA_BITS-1:0]       m_axi_wdata,                                            
  output wire [((AXI_DATA_BITS/8)-1):0] m_axi_wstrb,      
  output wire                           m_axi_wlast,      
  output wire                           m_axi_wvalid,     
  input  wire                           m_axi_wready,     
  input  wire [1:0]                     m_axi_bresp,      
  input  wire                           m_axi_bvalid,     
  output wire                           m_axi_bready,            
  // 
  input  wire [AXIS_DATA_BITS-1:0]      s_axis_tdata,
  input  wire                           s_axis_tvalid,
  output wire                           s_axis_tready,      
  input  wire                           s_axis_tlast                       
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
wire                      fifo_dis;

assign m_axi_wdata  = fifo_rd_data;
assign m_axi_wstrb  = {AXI_DATA_BITS/8{1'b1}};
assign m_axi_bready = m_axi_bvalid;

////////////////////////////////////////////////////////////
// Name : DMA S2MM Control
// Accepts DMA requests and sends data over the AXI bus.
////////////////////////////////////////////////////////////

rp_dma_s2mm_ctrl #(
  .AXI_ADDR_BITS  (AXI_ADDR_BITS),
  .AXI_DATA_BITS  (AXI_DATA_BITS),
  .AXI_BURST_LEN  (AXI_BURST_LEN),
  .FIFO_CNT_BITS  (FIFO_CNT_BITS),
  .REG_ADDR_BITS  (REG_ADDR_BITS),
  .CTRL_ADDR      (CTRL_ADDR))
  U_dma_s2mm_ctrl(
  .m_axi_aclk     (m_axi_aclk),         
  .s_axis_aclk    (s_axis_aclk),       
  .m_axi_aresetn  (aresetn),     
  .busy           (busy),
  .intr           (intr),      
  .mode           (mode),    
  .reg_addr       (reg_addr),          
  .reg_wr_data    (reg_wr_data),       
  .reg_wr_we      (reg_wr_we),           
  .reg_ctrl       (reg_ctrl),  
  .reg_sts        (reg_sts),  
  .reg_dst_addr1  (reg_dst_addr1),  
  .reg_dst_addr2  (reg_dst_addr2),  
  .reg_buf_size   (reg_buf_size),    
  .fifo_rst       (fifo_rst),                 
  .req_data       (req_data),
  .req_we         (req_we), 
  .data_valid     (s_axis_tvalid),
  .buf1_ms_cnt    (buf1_ms_cnt),
  .buf2_ms_cnt    (buf2_ms_cnt),
  .buf_sel_in     (buf_sel_in),
  .buf_sel_out    (buf_sel_out),
  .fifo_dis       (fifo_dis),
  .m_axi_awaddr   (m_axi_awaddr),       
  .m_axi_awlen    (m_axi_awlen),        
  .m_axi_awsize   (m_axi_awsize),       
  .m_axi_awburst  (m_axi_awburst),      
  .m_axi_awprot   (m_axi_awprot),       
  .m_axi_awcache  (m_axi_awcache),      
  .m_axi_awvalid  (m_axi_awvalid),      
  .m_axi_awready  (m_axi_awready), 
  .m_axi_wvalid   (m_axi_wvalid),    
  .m_axi_wready   (m_axi_wready),     
  .m_axi_wlast    (m_axi_wlast));      

////////////////////////////////////////////////////////////
// Name : DMA S2MM Upsizer
// Packs input data into the AXI bus width. 
////////////////////////////////////////////////////////////

rp_dma_s2mm_upsize #(
  .AXI_DATA_BITS  (AXI_DATA_BITS),
  .AXIS_DATA_BITS (AXIS_DATA_BITS))
  U_dma_s2mm_upsize(
  .clk            (s_axis_aclk),              
  .rst            (fifo_rst),    
  .req_data       (req_data),
  .req_we         (req_we),       
  .s_axis_tdata   (s_axis_tdata),      
  .s_axis_tvalid  (s_axis_tvalid),     
  .s_axis_tready  (s_axis_tready),     
  .s_axis_tlast   (s_axis_tlast),                 
  .m_axis_tdata   (fifo_wr_data),      
  .m_axis_tvalid  (fifo_wr_we),     
  .m_axis_tready  (1'b1));      

////////////////////////////////////////////////////////////
// Name : Data FIFO
// Stores the data to transfer.
////////////////////////////////////////////////////////////

fifo_axi_data 
  U_fifo_axi_data(
  .wr_clk         (s_axis_aclk),               
  .rd_clk         (m_axi_aclk),               
  .rst            (fifo_rst),     
  .din            (fifo_wr_data),                     
  .wr_en          (fifo_wr_we && ~fifo_dis),               
  .full           (),   
  .dout           (fifo_rd_data),    
  .rd_en          (fifo_rd_re),                                 
  .empty          (fifo_empty),                 
  .rd_data_count  (fifo_rd_cnt), 
  .wr_rst_busy    (),     
  .rd_rst_busy    ());
  
assign fifo_rd_re = m_axi_wvalid & m_axi_wready;  
 
endmodule