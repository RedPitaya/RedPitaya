`timescale 1ns / 1ps
module dac_sim #(
      // time period
  realtime  TP = 8.0ns,  // 250MHz
  realtime  AXIP = 5.0ns,  // 200MHz
  realtime  RP = 100.1ns  // ~10MHz
);
// ADC
logic [2-1:0] [ 7-1:0] adc_dat;
logic         [ 2-1:0] adc_dco;
// DAC
logic [2-1:0] [14-1:0] dac_dat;     // DAC combined data
logic                  dac_clk;     // DAC clock
logic                  dac_rst;     // DAC reset
// PDM DAC
logic         [ 4-1:0] dac_pwm;     // 1-bit PDM DAC
// XADC
logic         [ 5-1:0] vinp;        // voltages p
logic         [ 5-1:0] vinn;        // voltages n
// Expansion connector
wire          [ 9-1:0] exp_p_io;
wire          [ 9-1:0] exp_n_io;
wire                   exp_9_io;
// Expansion output data/enable
logic         [ 9-1:0] exp_p_od, exp_p_oe;
logic         [ 9-1:0] exp_n_od, exp_n_oe;
logic                  exp_9_od, exp_9_oe;
// SATA
logic         [ 4-1:0] daisy_p;
logic         [ 4-1:0] daisy_n;

// LED
wire          [ 8-1:0] led;

logic         [ 2-1:0] temp_prot;
logic                  pll_lo;
logic                  pll_hi;
logic                  pll_ref;
logic                  trig;

logic                  intr;

logic               clk ;
logic               axi_clk ;

logic               clkn;
wire               rstn_out;
logic               rstn;

localparam OSC_DW = 32;
localparam REG_DW = 32;
localparam OSC_AW = 32;
localparam REG_AW = 12;
localparam IW = 12;
localparam LW = 4;
localparam LW2 = 8;

localparam GEN1_EVENT = 0;
localparam GEN2_EVENT = 1;
localparam OSC1_EVENT = 2;
localparam OSC2_EVENT = 3;
localparam LA_EVENT = 4;

// clock
initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

initial        pll_ref = 1'b0;
always #(RP/2) pll_ref = ~pll_ref;

initial          axi_clk = 1'b0;
always #(AXIP/2) axi_clk = ~axi_clk;

// default clocking 
default clocking cb @ (posedge clk);
  input  rstn;
  input  exp_p_od, exp_p_oe;
  input  exp_n_od, exp_n_oe;
endclocking: cb

// reset
initial begin
        rstn = 1'b0;
  ##4;  rstn = 1'b1;
end

// clock cycle counter
int unsigned cyc=0;
always_ff @ (posedge clk)
cyc <= cyc+1;

axi4_if #(.DW (REG_DW), .AW (REG_AW), .IW (IW), .LW (LW)) axi_reg (
  .ACLK    (clk   ),  .ARESETn (rstn)
);

axi4_if #(.DW (REG_DW), .AW (REG_AW), .IW (IW), .LW (LW)) axi_syncd (
  .ACLK    (clk   ),  .ARESETn (rstn)
);

axi4_if #(.DW (OSC_DW), .AW (OSC_AW), .IW (IW), .LW (LW2)) axi_dac1 (
  .ACLK    (axi_clk   ),  .ARESETn (rstn)
);


top_tc_dac top_tc_dac();

axi_bus_model #(.AW (REG_AW), .DW (REG_DW), .IW (IW), .LW (LW)) axi_bm_reg  (axi_reg );
axi_bus_model #(.AW (OSC_AW), .DW (OSC_DW), .IW (IW), .LW (LW2)) axi_bm_osc2 (axi_dac1);

axi4_sync sync (
.axi_i(axi_reg),
.axi_o(axi_syncd)
);

initial begin
  ##500;

   //top_tc.test_hk                 (0<<20, 32'h55);
   //top_tc.test_sata               (5<<20, 32'h55);
   //top_tc.test_osc                (32'h40100000, OSC1_EVENT);
   top_tc_dac.test_dac2                (32'h40200000, GEN1_EVENT);
//   top_tc.test_asg                (2<<20, 32'h40090000, 2);


  ##1600000000;
  $finish();
end
/*
reg dac_dat;
always @(posedge clk) begin
    dac_dat <= axi_dac1.ARADDR;

end*/

/*
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
  input  wire [M_AXI_DAC_ADDR_BITS-1: 0]        m_axi_dac1_rdata_i    , // read data
  input  wire [    1: 0]                             , // read response
  input  wire                                   m_axi_dac1_rlast_i    , // read last
  input  wire                                   m_axi_dac1_rvalid_i   , // read response valid
  output wire                                   m_axi_dac1_rready_o   , // read response ready
*/
   axi_slave_model #(
    .AXI_DW           (  32          ) , // data width (8,16,...,1024)
    .AXI_AW           (  32          ) , // address width ()
    .AXI_ID           (   0          ) , // master ID
    .AXI_IW           (   4          ) , // master ID width   
    .AXI_LW           (   8          ) , // burst length width, 256 max
    .AXI_SW           (   8          )   // strobe width - 1 bit for every data byte
) i_slave_model (     
    .axi_clk_i       (     axi_clk     ), // global clock
    .axi_rstn_i      (     rstn    ), // global reset
/*                    
    .axi_awid_i     (   m_sm_awid      ), // write address ID
    .axi_awaddr_i   (   m_sm_awaddr    ), // write address
    .axi_awlen_i    (   m_sm_awlen     ), // write burst length
    .axi_awsize_i   (   m_sm_awsize    ), // write burst size
    .axi_awburst_i  (   m_sm_awburst   ), // write burst type
    .axi_awlock_i   (   m_sm_awlock    ), // write lock type
    .axi_awcache_i  (   m_sm_awcache   ), // write cache type
    .axi_awprot_i   (   m_sm_awprot    ), // write protection type
    .axi_awvalid_i  (   m_sm_awvalid   ), // write address valid
    .axi_awready_o  (   m_sm_awready   ), // write ready
    
    
    .axi_wid_i      (   m_sm_wid       ), // write data ID
    .axi_wdata_i    (   m_sm_wdata     ), // write data
    .axi_wstrb_i    (   m_sm_wstrb     ), // write strobes
    .axi_wlast_i    (   m_sm_wlast     ), // write last
    .axi_wvalid_i   (   m_sm_wvalid    ), // write valid
    .axi_wready_o   (   m_sm_wready    ), // write ready
    
    
    .axi_bid_o       (   m_sm_bid       ), // write response ID
    .axi_bresp_o     (   m_sm_bresp     ), // write response
    .axi_bvalid_o    (   m_sm_bval      ), // write response valid
    .axi_bready_i    (   m_sm_brea      ), // write response ready
*/   
    .axi_arid_i     (  axi_dac1.ARID       ), // read address ID
    .axi_araddr_i   (  axi_dac1.ARADDR     ), // read address
    .axi_arlen_i    (  axi_dac1.ARLEN      ), // read burst length
    .axi_arsize_i   (  axi_dac1.ARSIZE     ), // read burst size
    .axi_arburst_i  (  axi_dac1.ARBURST    ), // read burst type
    .axi_arlock_i   (  axi_dac1.ARLOCK     ), // read lock type
    .axi_arcache_i  (  axi_dac1.ARCACHE    ), // read cache type
    .axi_arprot_i   (  axi_dac1.ARPROT     ), // read protection type
    .axi_arvalid_i  (  axi_dac1.ARVALID    ), // read address valid
    .axi_arready_o  (  axi_dac1.ARREADY    ), // read address ready
    
    
    .axi_rid_o      (   axi_dac1.RID       ), // read response ID
    .axi_rdata_o    (   axi_dac1.RDATA     ), // read data
    .axi_rresp_o    (   axi_dac1.RRESP     ), // read response
    .axi_rlast_o    (   axi_dac1.RLAST     ), // read last
    .axi_rvalid_o   (   axi_dac1.RVALID    ), // read response valid
    .axi_rready_i   (   axi_dac1.RREADY    )  // read response ready
    
    ); 
    
  // ne pozabi define SIMULATION
  // s_axi_awready na 1
  rp_dac #(
    .M_AXI_DAC_DATA_BITS  (32),
    .M_AXI_DAC_ADDR_BITS  (32)

  ) rp_dac_mod (    
  .clk(clk),
  .rst_n(rstn),
  .intr(),
  .dac_data_cha_o(),
  .dac_data_chb_o(),  
  .event_ip_trig(),
  .event_ip_stop(),
  .event_ip_start(),
  .event_ip_reset(),
  .trig_ip(),
  .dac1_event_op(),    
  .dac1_trig_op(),    
  .dac2_event_op(),      
  .dac2_trig_op(),  

  .s_axi_reg_aclk(clk),    
  .s_axi_reg_aresetn(rstn), 

  .s_axi_reg_araddr(axi_syncd.ARADDR),
  .s_axi_reg_arprot(axi_syncd.ARPROT),
  .s_axi_reg_arready(axi_syncd.ARREADY),
  .s_axi_reg_arvalid(axi_syncd.ARVALID),
  .s_axi_reg_awaddr(axi_syncd.AWADDR),
  .s_axi_reg_awprot(axi_syncd.AWPROT),
  .s_axi_reg_awready(axi_syncd.AWREADY),
  .s_axi_reg_awvalid(axi_syncd.AWVALID),
  .s_axi_reg_bready(axi_syncd.BREADY),
  .s_axi_reg_bresp(axi_syncd.BRESP),
  .s_axi_reg_bvalid(axi_syncd.BVALID),
  .s_axi_reg_rdata(axi_syncd.RDATA),
  .s_axi_reg_rready(axi_syncd.RREADY),
  .s_axi_reg_rresp(axi_syncd.RRESP),
  .s_axi_reg_rvalid(axi_syncd.RVALID),
  .s_axi_reg_wdata(axi_syncd.WDATA),
  .s_axi_reg_wready(axi_syncd.WREADY),
  .s_axi_reg_wstrb(axi_syncd.WSTRB),
  .s_axi_reg_wvalid(axi_syncd.WVALID),
  /*
  .s_axi_reg_arburst(axi_syncd.ARBURST),
  .s_axi_reg_arcache(axi_syncd.ARCACHE),
  .s_axi_reg_arid(axi_syncd.ARID),
  .s_axi_reg_arlen(axi_syncd.ARLEN),
  .s_axi_reg_arlock(axi_syncd.ARLOCK),
  .s_axi_reg_arqos(axi_syncd.ARQOS),
  .s_axi_reg_arsize(axi_syncd.ARSIZE),
  .s_axi_reg_awburst(axi_syncd.AWBURST),
  .s_axi_reg_awcache(axi_syncd.AWCACHE),
  .s_axi_reg_awid(axi_syncd.AWID),
  .s_axi_reg_awlen(axi_syncd.AWLEN),
  .s_axi_reg_awlock(axi_syncd.AWLOCK),
  .s_axi_reg_awqos(axi_syncd.AWQOS),
  .s_axi_reg_awsize(axi_syncd.AWSIZE),
  .s_axi_reg_bid(axi_syncd.BID),
  .s_axi_reg_rid(axi_syncd.RID),
  .s_axi_reg_rlast(axi_syncd.RLAST),
  .s_axi_reg_wid(axi_syncd.WID),
  .s_axi_reg_wlast(axi_syncd.WLAST),
*/
  .m_axi_dac1_aclk(axi_clk),
  .m_axi_dac1_aresetn(rstn),
  .m_axi_dac1_arid_o(axi_dac1.ARID),
  .m_axi_dac1_araddr_o(axi_dac1.ARADDR),
  .m_axi_dac1_arlen_o(axi_dac1.ARLEN),
  .m_axi_dac1_arsize_o(axi_dac1.ARSIZE),
  .m_axi_dac1_arburst_o(axi_dac1.ARBURST),
  .m_axi_dac1_arlock_o(axi_dac1.ARLOCK),
  .m_axi_dac1_arcache_o(axi_dac1.ARCACHE),
  .m_axi_dac1_arprot_o(axi_dac1.ARPROT),
  .m_axi_dac1_arvalid_o(axi_dac1.ARVALID),
  .m_axi_dac1_arready_i(axi_dac1.ARREADY),
  .m_axi_dac1_rid_i(axi_dac1.RID),
  .m_axi_dac1_rdata_i(axi_dac1.RDATA),
  .m_axi_dac1_rresp_i(axi_dac1.RRESP),
  .m_axi_dac1_rlast_i(axi_dac1.RLAST),
  .m_axi_dac1_rvalid_i(axi_dac1.RVALID),
  .m_axi_dac1_rready_o(axi_dac1.RREADY),

  .m_axi_dac2_arid_o(),
  .m_axi_dac2_araddr_o(),
  .m_axi_dac2_arlen_o(),
  .m_axi_dac2_arsize_o(),
  .m_axi_dac2_arburst_o(),
  .m_axi_dac2_arlock_o(),
  .m_axi_dac2_arcache_o(),
  .m_axi_dac2_arprot_o(),
  .m_axi_dac2_arvalid_o(),
  .m_axi_dac2_arready_i(),
  .m_axi_dac2_rid_i(),
  .m_axi_dac2_rdata_i(),
  .m_axi_dac2_rresp_i(),
  .m_axi_dac2_rlast_i(),
  .m_axi_dac2_rvalid_i(),
  .m_axi_dac2_rready_o()
);

endmodule