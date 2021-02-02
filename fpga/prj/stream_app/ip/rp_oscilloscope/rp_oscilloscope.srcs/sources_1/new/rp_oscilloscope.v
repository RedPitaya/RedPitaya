`timescale 1ns / 1ps

module rp_oscilloscope
  #(parameter S_AXI_REG_ADDR_BITS   = 12,
    parameter M_AXI_OSC1_ADDR_BITS  = 32,
    parameter M_AXI_OSC1_DATA_BITS  = 64,
    parameter M_AXI_OSC2_ADDR_BITS  = 32,
    parameter M_AXI_OSC2_DATA_BITS  = 64,
    parameter ADC_DATA_BITS         = 14,
    parameter EVENT_SRC_NUM         = 7,
    parameter TRIG_SRC_NUM          = 7)(    
  input  wire                                   clk,
  input  wire                                   rst_n,
  output wire                                   intr,

  //
  input  wire [ADC_DATA_BITS-1:0]               adc_data_ch1,
  input  wire [ADC_DATA_BITS-1:0]               adc_data_ch2,  
  //
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_trig,
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_stop,
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_start,
  input  wire [EVENT_SRC_NUM-1:0]               event_ip_reset,
  input  wire [TRIG_SRC_NUM-1:0]                trig_ip,
  //
  output wire [3:0]                             osc1_event_op,    
  output wire                                   osc1_trig_op,    
  // 
  output wire [3:0]                             osc2_event_op,      
  output wire                                   osc2_trig_op,  
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
  
  input  wire                                   m_axi_osc1_aclk,    
  input  wire                                   m_axi_osc1_aresetn,     
  output wire [(M_AXI_OSC1_ADDR_BITS-1):0]      m_axi_osc1_awaddr,    
  output wire [7:0]                             m_axi_osc1_awlen,     
  output wire [2:0]                             m_axi_osc1_awsize,    
  output wire [1:0]                             m_axi_osc1_awburst,   
  output wire [2:0]                             m_axi_osc1_awprot,    
  output wire [3:0]                             m_axi_osc1_awcache,   
  output wire                                   m_axi_osc1_awvalid,   
  input  wire                                   m_axi_osc1_awready,   
  output wire [M_AXI_OSC1_DATA_BITS-1:0]        m_axi_osc1_wdata,     
  output wire [((M_AXI_OSC1_DATA_BITS/8)-1):0]  m_axi_osc1_wstrb,     
  output wire                                   m_axi_osc1_wlast,     
  output wire                                   m_axi_osc1_wvalid,    
  input  wire                                   m_axi_osc1_wready,    
  input  wire [1:0]                             m_axi_osc1_bresp,     
  input  wire                                   m_axi_osc1_bvalid,    
  output wire                                   m_axi_osc1_bready,   
  //
  input  wire                                   m_axi_osc2_aclk,    
  input  wire                                   m_axi_osc2_aresetn,   
  output wire [(M_AXI_OSC2_ADDR_BITS-1):0]      m_axi_osc2_awaddr,    
  output wire [7:0]                             m_axi_osc2_awlen,     
  output wire [2:0]                             m_axi_osc2_awsize,    
  output wire [1:0]                             m_axi_osc2_awburst,   
  output wire [2:0]                             m_axi_osc2_awprot,    
  output wire [3:0]                             m_axi_osc2_awcache,   
  output wire                                   m_axi_osc2_awvalid,   
  input  wire                                   m_axi_osc2_awready,   
  output wire [M_AXI_OSC2_DATA_BITS-1:0]        m_axi_osc2_wdata,     
  output wire [((M_AXI_OSC2_DATA_BITS/8)-1):0]  m_axi_osc2_wstrb,     
  output wire                                   m_axi_osc2_wlast,     
  output wire                                   m_axi_osc2_wvalid,    
  input  wire                                   m_axi_osc2_wready,    
  input  wire [1:0]                             m_axi_osc2_bresp,     
  input  wire                                   m_axi_osc2_bvalid,    
  output wire                                   m_axi_osc2_bready   
);

////////////////////////////////////////////////////////////
// Parameters
////////////////////////////////////////////////////////////
    
localparam REG_ADDR_BITS  = 8;

////////////////////////////////////////////////////////////
// Signals
////////////////////////////////////////////////////////////
    
wire                            reg_clk;
wire                            reg_rst;
wire [S_AXI_REG_ADDR_BITS-1:0]  reg_addr;
wire                            reg_en;
wire [3:0]                      reg_we;
wire                            reg_wr_we;
wire [31:0]                     reg_wr_data;    
wire [31:0]                     reg_rd_data;

wire [REG_ADDR_BITS-1:0]        osc1_reg_addr;
reg                             osc1_reg_wr_we;
wire [31:0]                     osc1_reg_wr_data;    
wire [31:0]                     osc1_reg_rd_data;
wire                            osc1_dma_intr;

wire [REG_ADDR_BITS-1:0]        osc2_reg_addr;
reg                             osc2_reg_wr_we;
wire [31:0]                     osc2_reg_wr_data;    
wire [31:0]                     osc2_reg_rd_data;
wire                            osc2_dma_intr;

reg  signed [16-1:0]            adc_data_ch1_signed;        
reg  signed [16-1:0]            adc_data_ch2_signed;        

wire signed [15:0]              s_axis_osc1_tdata;
wire signed [15:0]              s_axis_osc2_tdata;

wire                            adr_is_setting;
wire                            adr_is_ch1, adr_is_ch2;
wire                            adr_is_cal_ch1, adr_is_cal_ch2;
wire                            adr_is_dma_ch1, adr_is_dma_ch2;
wire                            adr_is_diag_ch1, adr_is_diag_ch2;
wire                            adr_is_cntms_ch1, adr_is_cntms_ch2;
wire                            adr_is_filt_ch1, adr_is_filt_ch2;
wire                            buf_sel_ch1, buf_sel_ch2;

wire                            adc_sign_ch1 = ~adc_data_ch1[ADC_DATA_BITS-1];
wire                            adc_sign_ch2 = ~adc_data_ch2[ADC_DATA_BITS-1];

always @(posedge clk)
begin
  adc_data_ch1_signed <= {adc_data_ch1[ADC_DATA_BITS-1], ~{adc_data_ch1[ADC_DATA_BITS-2:0],{(16-ADC_DATA_BITS){adc_sign_ch1}}}};  
end

assign s_axis_osc1_tdata = adc_data_ch1_signed;

always @(posedge clk)
begin
  adc_data_ch2_signed <= {adc_data_ch2[ADC_DATA_BITS-1], ~{adc_data_ch2[ADC_DATA_BITS-2:0],{(16-ADC_DATA_BITS){adc_sign_ch2}}}}; 
end

assign s_axis_osc2_tdata = adc_data_ch2_signed;

assign intr = osc1_dma_intr | osc2_dma_intr;

`ifdef SIMULATION
  assign reg_wr_we = reg_en & (reg_we == 4'h1);
`else
  assign reg_wr_we = reg_en & (reg_we == 4'hF);
`endif //SIMULATION


assign adr_is_setting = (reg_addr[REG_ADDR_BITS-1:0] <= 8'h58);

assign adr_is_cal_ch1= (reg_addr[REG_ADDR_BITS-1:0] == 8'h74 || reg_addr[REG_ADDR_BITS-1:0] == 8'h78);
assign adr_is_cal_ch2= (reg_addr[REG_ADDR_BITS-1:0] == 8'h7C || reg_addr[REG_ADDR_BITS-1:0] == 8'h80);

assign adr_is_diag_ch1= (reg_addr[REG_ADDR_BITS-1:0] == 8'hA4);
assign adr_is_diag_ch2= (reg_addr[REG_ADDR_BITS-1:0] == 8'hA8);

assign adr_is_dma_ch1= (reg_addr[REG_ADDR_BITS-1:0] == 8'h64 || reg_addr[REG_ADDR_BITS-1:0] == 8'h68);
assign adr_is_dma_ch2= (reg_addr[REG_ADDR_BITS-1:0] == 8'h6C || reg_addr[REG_ADDR_BITS-1:0] == 8'h70);

assign adr_is_cntms_ch1= (reg_addr[REG_ADDR_BITS-1:0] == 8'h5C || reg_addr[REG_ADDR_BITS-1:0] == 8'h60);
assign adr_is_cntms_ch2= (reg_addr[REG_ADDR_BITS-1:0] == 8'h9C || reg_addr[REG_ADDR_BITS-1:0] == 8'hA0);

assign adr_is_filt_ch1= (reg_addr[REG_ADDR_BITS-1:0] >= 8'hC0 && reg_addr[REG_ADDR_BITS-1:0] <= 8'hCC);
assign adr_is_filt_ch2= (reg_addr[REG_ADDR_BITS-1:0] >= 8'hD0 && reg_addr[REG_ADDR_BITS-1:0] <= 8'hDC);

assign adr_is_ch1     = (adr_is_dma_ch1 || adr_is_cal_ch1 || adr_is_diag_ch1 || adr_is_filt_ch1);
assign adr_is_ch2     = (adr_is_dma_ch2 || adr_is_cal_ch2 || adr_is_diag_ch1 || adr_is_filt_ch2);

////////////////////////////////////////////////////////////
// Name : Register Control
// 
////////////////////////////////////////////////////////////   
  
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
// Name : Oscilloscope 1
// 
////////////////////////////////////////////////////////////     

osc_top #(
  .M_AXI_ADDR_BITS  (M_AXI_OSC1_ADDR_BITS),
  .M_AXI_DATA_BITS  (M_AXI_OSC1_DATA_BITS),
  .S_AXIS_DATA_BITS (16), 
  .REG_ADDR_BITS    (REG_ADDR_BITS),
  .EVENT_SRC_NUM    (EVENT_SRC_NUM),
  .TRIG_SRC_NUM     (TRIG_SRC_NUM),
  .CTRL_ADDR        ('h50),
  .CHAN_NUM         (1))
  U_osc1(
  .clk              (m_axi_osc1_aclk),   
  .rst_n            (m_axi_osc1_aresetn), 
  .s_axis_tdata     (s_axis_osc1_tdata), 
  .s_axis_tvalid    (1'b1),
  .event_ip_trig    (event_ip_trig),  
  .event_ip_stop    (event_ip_stop),  
  .event_ip_start   (event_ip_start), 
  .event_ip_reset   (event_ip_reset),  
  .event_op_trig    (osc1_event_op[0]),
  .event_op_stop    (osc1_event_op[1]),
  .event_op_start   (osc1_event_op[2]),
  .event_op_reset   (osc1_event_op[3]),
  .trig_ip          (trig_ip),
  .trig_op          (osc1_trig_op),  
  .ctl_rst          (),
  .reg_addr         (osc1_reg_addr),   
  .reg_wr_data      (osc1_reg_wr_data),
  .reg_wr_we        (osc1_reg_wr_we),  
  .reg_rd_data      (osc1_reg_rd_data),
  .buf_sel_in       (buf_sel_ch2),
  .buf_sel_out      (buf_sel_ch1),
  .dma_intr         (osc1_dma_intr),
  .m_axi_awaddr     (m_axi_osc1_awaddr), 
  .m_axi_awlen      (m_axi_osc1_awlen),  
  .m_axi_awsize     (m_axi_osc1_awsize), 
  .m_axi_awburst    (m_axi_osc1_awburst),
  .m_axi_awprot     (m_axi_osc1_awprot), 
  .m_axi_awcache    (m_axi_osc1_awcache),
  .m_axi_awvalid    (m_axi_osc1_awvalid),
  .m_axi_awready    (m_axi_osc1_awready),
  .m_axi_wdata      (m_axi_osc1_wdata),  
  .m_axi_wstrb      (m_axi_osc1_wstrb),  
  .m_axi_wlast      (m_axi_osc1_wlast),  
  .m_axi_wvalid     (m_axi_osc1_wvalid), 
  .m_axi_wready     (m_axi_osc1_wready), 
  .m_axi_bresp      (m_axi_osc1_bresp),  
  .m_axi_bvalid     (m_axi_osc1_bvalid), 
  .m_axi_bready     (m_axi_osc1_bready));

////////////////////////////////////////////////////////////
// Name : Oscilloscope 2
// 
////////////////////////////////////////////////////////////     

osc_top #(
  .M_AXI_ADDR_BITS  (M_AXI_OSC1_ADDR_BITS),
  .M_AXI_DATA_BITS  (M_AXI_OSC1_DATA_BITS),
  .S_AXIS_DATA_BITS (16), 
  .REG_ADDR_BITS    (REG_ADDR_BITS),  
  .EVENT_SRC_NUM    (EVENT_SRC_NUM),
  .TRIG_SRC_NUM     (TRIG_SRC_NUM),
  .CTRL_ADDR        ('h50),
  .CHAN_NUM         (2))
  U_osc2(
  .clk              (m_axi_osc2_aclk),   
  .rst_n            (m_axi_osc2_aresetn), 
  .s_axis_tdata     (s_axis_osc2_tdata), 
  .s_axis_tvalid    (1'b1),
  .event_ip_trig    (event_ip_trig),  
  .event_ip_stop    (event_ip_stop),  
  .event_ip_start   (event_ip_start), 
  .event_ip_reset   (event_ip_reset),  
  .event_op_trig    (osc2_event_op[0]),
  .event_op_stop    (osc2_event_op[1]),
  .event_op_start   (osc2_event_op[2]),
  .event_op_reset   (osc2_event_op[3]),
  .trig_ip          (trig_ip),
  .trig_op          (osc2_trig_op),  
  .ctl_rst          (),
  .reg_addr         (osc2_reg_addr),   
  .reg_wr_data      (osc2_reg_wr_data),
  .reg_wr_we        (osc2_reg_wr_we),  
  .reg_rd_data      (osc2_reg_rd_data),
  .buf_sel_in       (buf_sel_ch1),
  .buf_sel_out      (buf_sel_ch2),
  .dma_intr         (osc2_dma_intr),
  .m_axi_awaddr     (m_axi_osc2_awaddr), 
  .m_axi_awlen      (m_axi_osc2_awlen),  
  .m_axi_awsize     (m_axi_osc2_awsize), 
  .m_axi_awburst    (m_axi_osc2_awburst),
  .m_axi_awprot     (m_axi_osc2_awprot), 
  .m_axi_awcache    (m_axi_osc2_awcache),
  .m_axi_awvalid    (m_axi_osc2_awvalid),
  .m_axi_awready    (m_axi_osc2_awready),
  .m_axi_wdata      (m_axi_osc2_wdata),  
  .m_axi_wstrb      (m_axi_osc2_wstrb),  
  .m_axi_wlast      (m_axi_osc2_wlast),  
  .m_axi_wvalid     (m_axi_osc2_wvalid), 
  .m_axi_wready     (m_axi_osc2_wready), 
  .m_axi_bresp      (m_axi_osc2_bresp),  
  .m_axi_bvalid     (m_axi_osc2_bvalid), 
  .m_axi_bready     (m_axi_osc2_bready));

assign osc1_reg_addr    = reg_addr[REG_ADDR_BITS-1:0];
assign osc1_reg_wr_data = reg_wr_data;

always @(*)
begin
  osc1_reg_wr_we = 0;
  
  if ((reg_wr_we == 1) && (adr_is_ch1 || adr_is_setting)) begin
    osc1_reg_wr_we = 1;
  end
end

assign osc2_reg_addr    = reg_addr[REG_ADDR_BITS-1:0];
assign osc2_reg_wr_data = reg_wr_data;

always @(*)
begin
  osc2_reg_wr_we = 0;
  
  if ((reg_wr_we == 1) && (adr_is_ch2 || adr_is_setting)) begin
    osc2_reg_wr_we = 1;
  end
end

assign reg_rd_data = (adr_is_ch1 || adr_is_cntms_ch1 || adr_is_setting) ? osc1_reg_rd_data : osc2_reg_rd_data;
        
          
endmodule