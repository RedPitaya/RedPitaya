`timescale 1ns / 1ps

module dac_top
  #(parameter M_AXI_DAC_ADDR_BITS   = 32, // DMA Address bits
    parameter M_AXI_DAC_DATA_BITS   = 64, // DMA data bits
    parameter DAC_DATA_BITS     = 14, // ADC data bits
    parameter REG_ADDR_BITS     = 12, // Register interface address bits
    parameter TRIG_CNT_BITS     = 32, // Trigger counter bits
    parameter EVENT_SRC_NUM     = 1,  // Number of event sources
    parameter EVENT_NUM_LOG2    = $clog2(EVENT_SRC_NUM),
    parameter TRIG_SRC_NUM      = 1,  // Number of trigger sources
    parameter CH_NUM            = 0
)( // which channel
  input wire                              clk,
  input wire                              rst_n,
  input wire                              axi_clk,
  input wire                              axi_rstn,
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

  input  wire [TRIG_SRC_NUM-1:0]          event_sel,
  input  wire                             event_val, 

  input  wire [TRIG_SRC_NUM-1:0]          trig_ip,
  output wire                             trig_op,

  output      [31:0]                    reg_ctrl,
  input  wire                           ctrl_val, 
  output      [31:0]                    reg_sts,
  input  wire                           sts_val, 

  input  [16-1:0]                       dac_conf,

  input [DAC_DATA_BITS-1:0]               dac_scale,
  input [DAC_DATA_BITS-1:0]               dac_offs,
  input [M_AXI_DAC_ADDR_BITS-1:0]         dac_step,
  input [M_AXI_DAC_ADDR_BITS-1:0]         dac_buf_size,
  input [M_AXI_DAC_ADDR_BITS-1:0]         dac_buf1_adr,
  input [M_AXI_DAC_ADDR_BITS-1:0]         dac_buf2_adr,
  output [M_AXI_DAC_ADDR_BITS-1:0]        dac_rp,


  input  [ 5-1:0]                         dac_trig,
  input  [ 8-1:0]                         dac_ctrl_reg,
  output [ 5-1:0]                         dac_sts_reg,


  output [DAC_DATA_BITS-1:0]              dac_data_o,
  output [32-1:0]                         diag_reg,
  output [32-1:0]                         diag_reg2,

  //
  output wire [3:0]                       m_axi_dac_arid_o     , // read address ID
  output wire [M_AXI_DAC_ADDR_BITS-1: 0]  m_axi_dac_araddr_o   , // read address
  output wire [7:0]                       m_axi_dac_arlen_o    , // read burst length
  output wire [2:0]                       m_axi_dac_arsize_o   , // read burst size
  output wire [1:0]                       m_axi_dac_arburst_o  , // read burst type
  output wire [1:0]                       m_axi_dac_arlock_o   , // read lock type
  output wire [3:0]                       m_axi_dac_arcache_o  , // read cache type
  output wire [2:0]                       m_axi_dac_arprot_o   , // read protection type
  output wire                             m_axi_dac_arvalid_o  , // read address valid
  input  wire                             m_axi_dac_arready_i  , // read address ready
  input  wire [    3: 0]                  m_axi_dac_rid_i      , // read response ID
  input  wire [M_AXI_DAC_DATA_BITS-1: 0]  m_axi_dac_rdata_i    , // read data
  input  wire [    1: 0]                  m_axi_dac_rresp_i    , // read response
  input  wire                             m_axi_dac_rlast_i    , // read last
  input  wire                             m_axi_dac_rvalid_i   , // read response valid
  output wire                             m_axi_dac_rready_o     // read response ready
);

////////////////////////////////////////////////////////////
// Parameters
////////////////////////////////////////////////////////////

// DAC configuration reg
localparam TRIG_SEL         = 3;
localparam OUT_ZERO         = 7;
  


////////////////////////////////////////////////////////////
// Signals
////////////////////////////////////////////////////////////

wire                        dma_mode;

reg                         event_num_trig;
reg                         event_num_stop;
reg                         event_num_start;
reg                         event_num_reset;

wire                        event_sts_trig;
wire                        event_sts_stop;
wire                        event_sts_start;
wire                        event_sts_reset;
wire                        ctl_trg;
wire dac_rvalid;           
wire [DAC_DATA_BITS-1:0]    dac_data_raw;
wire set_zero = dac_conf[OUT_ZERO];
  
////////////////////////////////////////////////////////////
// Name : DMA S2MM
// 
////////////////////////////////////////////////////////////
  
rp_dma_mm2s #(
  .AXI_ADDR_BITS  (M_AXI_DAC_ADDR_BITS),
  .AXI_DATA_BITS  (M_AXI_DAC_DATA_BITS),
  .AXIS_DATA_BITS (DAC_DATA_BITS),
  .AXI_BURST_LEN  (16))
  U_dma_mm2s(
  .m_axi_aclk     (axi_clk),        
  .s_axis_aclk    (clk),      
  .aresetn        (rst_n),  
  .busy           (),
  .intr           (dma_intr),     
  .mode           (dma_mode),  

  .reg_ctrl       (reg_ctrl),
  .ctrl_val       (ctrl_val),
  .reg_sts        (reg_sts),
  .sts_val        (sts_val),  

  .dac_step         (dac_step),
  .dac_rp           (dac_rp),
  .dac_buf_size     (dac_buf_size),
  .dac_buf1_adr     (dac_buf1_adr),
  .dac_buf2_adr     (dac_buf2_adr),

  .dac_trig         (ctl_trg),
  .dac_ctrl_reg     (dac_ctrl_reg),
  .dac_sts_reg      (dac_sts_reg),

  .dac_rdata_o      (dac_data_raw),
  .dac_rvalid_o     (dac_rvalid),
  .diag_reg         (diag_reg),
  .diag_reg2        (diag_reg2),
 
  .m_axi_arid_o     (m_axi_dac_arid_o), 
  .m_axi_araddr_o   (m_axi_dac_araddr_o),  
  .m_axi_arlen_o    (m_axi_dac_arlen_o), 
  .m_axi_arsize_o   (m_axi_dac_arsize_o),
  .m_axi_arburst_o  (m_axi_dac_arburst_o), 
  .m_axi_arlock_o   (m_axi_dac_arlock_o),
  .m_axi_arcache_o  (m_axi_dac_arcache_o),
  .m_axi_arprot_o   (m_axi_dac_arprot_o),
  .m_axi_arvalid_o  (m_axi_dac_arvalid_o),  
  .m_axi_arready_i  (m_axi_dac_arready_i),  
  .m_axi_rid_i      (m_axi_dac_rid_i),  
  .m_axi_rdata_i    (m_axi_dac_rdata_i), 
  .m_axi_rresp_i    (m_axi_dac_rresp_i), 
  .m_axi_rlast_i    (m_axi_dac_rlast_i),  
  .m_axi_rvalid_i   (m_axi_dac_rvalid_i), 
  .m_axi_rready_o   (m_axi_dac_rready_o));     

dac_calib #(
  .AXIS_DATA_BITS (DAC_DATA_BITS))
  U_osc_calib(
  .dac_clk_i      (clk),
  .dac_rstn_i     (rst_n),

  .dac_o          (dac_data_o),
  .dac_rdata_i    (dac_data_raw),
  .dac_rvalid_i   (dac_rvalid),
  // conf
  .set_amp_i      (dac_scale),
  .set_dc_i       (dac_offs),
  .set_zero_i     (set_zero));


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

assign ctl_rst = event_num_reset;
assign event_sts_reset = 0;

assign ctl_trg = event_num_trig | |(trig_ip & dac_trig);

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
    if (event_val) begin
      event_op_trig   <= event_sel[3];
      event_op_stop   <= event_sel[2];
      event_op_start  <= event_sel[1];
      event_op_reset  <= event_sel[0];    
    end else begin
      event_op_trig   <= 0;
      event_op_stop   <= 0;
      event_op_start  <= 0;
      event_op_reset  <= 0;     
    end
  end
end       

endmodule