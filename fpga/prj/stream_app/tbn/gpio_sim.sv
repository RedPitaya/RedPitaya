`timescale 1ns / 1ps
module gpio_sim #(
      // time period
  realtime  TP = 8.0ns,  // 250MHz
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
//reg          [ 8-1:0] exp_p_io=8'h10;
wire          [ 8-1:0] exp_p_io;
wire          [ 8-1:0] exp_n_io;

wire          [ 8-1:0] outval_p;
wire          [ 8-1:0] outval_n;

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
logic               clkn;
wire               rstn_out;
logic               rstn;

localparam OSC_DW = 64;
localparam REG_DW = 32;
localparam OSC_AW = 32;
localparam REG_AW = 12;
localparam IW = 4;
localparam LW = 8;

localparam GPIO_IN_CTRL_ADDR  = 'h8C;
localparam GPIO_OUT_CTRL_ADDR = 'h90;

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

axi4_if #(.DW (OSC_DW), .AW (OSC_AW), .IW (IW), .LW (LW)) axi_osc1 (
  .ACLK    (clk   ),  .ARESETn (rstn)
);


top_tc_gpio top_tc_gpio();

axi_bus_model #(.AW (REG_AW), .DW (REG_DW), .IW (IW), .LW (LW)) axi_bm_reg  (axi_reg );
axi_slave_model #(.AXI_AW (OSC_AW), .AXI_DW (OSC_DW), .AXI_IW (IW), .AXI_SW (8), .AXI_ID(0)) 
axi_bm_osc1 (
   // global signals
  .axi_clk_i      (clk), // global clock
  .axi_rstn_i     (rstn), // global reset
   // axi write address channel
  .axi_awid_i     (axi_osc1.AWID), // write address ID
  .axi_awaddr_i   (axi_osc1.AWADDR), // write address
  .axi_awlen_i    (axi_osc1.AWLEN), // write burst length
  .axi_awsize_i   (axi_osc1.AWSIZE), // write burst size
  .axi_awburst_i  (axi_osc1.AWBURST), // write burst type
  .axi_awlock_i   (axi_osc1.AWLOCK), // write lock type
  .axi_awcache_i  (axi_osc1.AWCACHE), // write cache type
  .axi_awprot_i   (axi_osc1.AWPROT), // write protection type
  .axi_awvalid_i  (axi_osc1.AWVALID), // write address valid
  .axi_awready_o  (axi_osc1.AWREADY), // write ready
   // axi write data channel
  .axi_wid_i      (axi_osc1.WID), // write data ID
  .axi_wdata_i    (axi_osc1.WDATA), // write data
  .axi_wstrb_i    (axi_osc1.WSTRB), // write strobes
  .axi_wlast_i    (axi_osc1.WLAST), // write last
  .axi_wvalid_i   (axi_osc1.WVALID), // write valid
  .axi_wready_o   (axi_osc1.WREADY), // write ready
   // axi write response channel
  .axi_bid_o      (axi_osc1.BID), // write response ID
  .axi_bresp_o    (axi_osc1.BRESP), // write response
  .axi_bvalid_o   (axi_osc1.BVALID), // write response valid
  .axi_bready_i   (axi_osc1.BREADY), // write response ready
   // axi read address channel
  .axi_arid_i     (axi_osc1.ARID), // read address ID
  .axi_araddr_i   (axi_osc1.ARADDR), // read address
  .axi_arlen_i    (axi_osc1.ARLEN), // read burst length
  .axi_arsize_i   (axi_osc1.ARSIZE), // read burst size
  .axi_arburst_i  (axi_osc1.ARBURST), // read burst type
  .axi_arlock_i   (axi_osc1.ARLOCK), // read lock type
  .axi_arcache_i  (axi_osc1.ARCACHE), // read cache type
  .axi_arprot_i   (axi_osc1.ARPROT), // read protection type
  .axi_arvalid_i  (axi_osc1.ARVALID), // read address valid
  .axi_arready_o  (axi_osc1.ARREADY), // read address ready
   // axi read data channel
  .axi_rid_o      (axi_osc1.RID), // read response ID
  .axi_rdata_o    (axi_osc1.RDATA), // read data
  .axi_rresp_o    (axi_osc1.RRESP), // read response
  .axi_rlast_o    (axi_osc1.RLAST), // read last
  .axi_rvalid_o   (axi_osc1.RVALID), // read response valid
  .axi_rready_i   (axi_osc1.RREADY) // read response ready
);


axi4_sync sync (
.axi_i(axi_reg),
.axi_o(axi_syncd)
);

wire [7:0] dirp;
wire [7:0] dirn;

initial begin
  ##500;

   top_tc_gpio.test_gpio (32'h40100000, GPIO_OUT_CTRL_ADDR, LA_EVENT);

  ##1600000000;
  $finish();
end
wire          [ 8-1:0] gpio_p_i;
wire          [ 8-1:0] gpio_n_i;

reg          [ 8-1:0] gpio_p_o;
reg          [ 8-1:0] gpio_n_o;

reg          [10-1:0] gpio_cnt;

//assign gpio_p_o = 8'h01;
//assign gpio_n_o = 8'hFE;
/*
always @(posedge clk) begin
  if (~rstn) begin
    gpio_cnt <= 'h0;
    gpio_p_o <= 'h0;
    gpio_n_o <= 'h0;
  end else begin
    if (gpio_cnt >= 10'd100) begin
      gpio_cnt <= 'h0;
      gpio_p_o <= gpio_p_o + 1;
      gpio_n_o <= gpio_n_o - 1;      
    end else
      gpio_cnt <= gpio_cnt + 1;
  end
end
wire [31:0] read_dat1={8'd100, gpio_p_o,   8'd100, gpio_n_o  };
wire [31:0] read_dat2={8'd100, gpio_p_o+1, 8'd100, gpio_n_o-1};
wire [63:0] read_dat ={read_dat1, read_dat2};
*/
/*


genvar GV;
generate
for(GV = 0 ; GV < 8 ; GV = GV +1) begin
//pullup(exp_p_io[GV]);
//pullup(exp_n_io[GV]);

assign exp_p_io[GV] = (dirp[GV]) ? outval_p[GV] : 1'bz;
assign exp_n_io[GV] = (~dirn[GV]) ? outval_n[GV] : 1'bz;

end
endgenerate
*/

/*
pullup(sda_io) ;
assign sda_tri = sda_o  ;

assign sda_io  = sda_tri ? 1'bz : sda_o ;
assign sda_i   = sda_io ;
*/
//assign exp_p_io = 8'h01;


  // ne pozabi define SIMULATION
  // s_axi_awready na 1

wire [4:0] la_event_op;
wire [4:0] rp_concat_0_event_reset;
wire [4:0] rp_concat_0_event_start;
wire [4:0] rp_concat_0_event_stop;
wire [4:0] rp_concat_0_event_trig;
wire [5:0]rp_concat_0_trig;
  
  rp_concat rp_concat
       (.event_reset(rp_concat_0_event_reset),
        .event_start(rp_concat_0_event_start),
        .event_stop(rp_concat_0_event_stop),
        .event_trig(rp_concat_0_event_trig),
        .ext_trig_ip(rp_concat_ext_trig_ip),
        .gen1_event_ip(4'h0),
        .gen1_trig_ip(1'b0),
        .gen2_event_ip(4'h0),
        .gen2_trig_ip(1'b0),
        .la_event_ip(la_event_op),
        .la_trig_ip(la_trig_op),
        .osc1_event_ip(4'h0),
        .osc1_trig_ip(1'b0),
        .osc2_event_ip(4'h0),
        .osc2_trig_ip(1'b0),
        .trig(rp_concat_0_trig));

  rp_gpio rp_gpio_mod (    
  .clk(clk),
  .rst_n(rstn),
  .intr(),

  .gpiop_i(gpio_p_o),
  .gpion_i(gpio_n_o),
  
  .dirp(dirp),
  .dirn(dirn),

  .event_ip_reset(rp_concat_0_event_reset),
  .event_ip_start(rp_concat_0_event_start),
  .event_ip_stop(rp_concat_0_event_stop),
  .event_ip_trig(rp_concat_0_event_trig),
  .trig_ip(rp_concat_0_trig),
  .la_event_op(la_event_op),    
  .la_trig_op(la_trig_op),    
   

  .s_axi_reg_aclk(clk),    
  .s_axi_reg_aresetn(rstn), 
/*
  .s_axi_reg_araddr(axi_reg.ARADDR),
  .s_axi_reg_arprot(axi_reg.ARPROT),
  .s_axi_reg_arready(axi_reg.ARREADY),
  .s_axi_reg_arvalid(axi_reg.ARVALID),
  .s_axi_reg_awaddr(axi_reg.AWADDR),
  .s_axi_reg_awprot(axi_reg.AWPROT),
  .s_axi_reg_awready(axi_reg.AWREADY),
  .s_axi_reg_awvalid(axi_reg.AWVALID),
  .s_axi_reg_bready(axi_reg.BREADY),
  .s_axi_reg_bresp(axi_reg.BRESP),
  .s_axi_reg_bvalid(axi_reg.BVALID),
  .s_axi_reg_rdata(axi_reg.RDATA),
  .s_axi_reg_rready(axi_reg.RREADY),
  .s_axi_reg_rresp(axi_reg.RRESP),
  .s_axi_reg_rvalid(axi_reg.RVALID),
  .s_axi_reg_wdata(axi_reg.WDATA),
  .s_axi_reg_wready(axi_reg.WREADY),
  .s_axi_reg_wstrb(axi_reg.WSTRB),
  .s_axi_reg_wvalid(axi_reg.WVALID),
*/

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


        .m_axi_gpio_araddr(axi_osc1.ARADDR),
        .m_axi_gpio_arburst(axi_osc1.ARBURST),
        .m_axi_gpio_arcache(axi_osc1.ARCACHE),
        .m_axi_gpio_arid(axi_osc1.ARID),
        .m_axi_gpio_arlen(axi_osc1.ARLEN),
        .m_axi_gpio_arlock(axi_osc1.ARLOCK),
        .m_axi_gpio_arprot(axi_osc1.ARPROT),
        .m_axi_gpio_arready(axi_osc1.ARREADY),
        .m_axi_gpio_arsize(axi_osc1.ARSIZE),
        .m_axi_gpio_arvalid(axi_osc1.ARVALID),
        .m_axi_gpio_rid(axi_osc1.RID),
        .m_axi_gpio_rlast(axi_osc1.RLAST),
        .m_axi_gpio_rready(axi_osc1.RREADY),
        .m_axi_gpio_rresp(axi_osc1.RRESP),
        .m_axi_gpio_rvalid(axi_osc1.RVALID),
        .m_axi_gpio_rdata(axi_osc1.RDATA),
        .m_axi_gpio_out_aclk(clk),
        .m_axi_gpio_out_aresetn(rstn),
        
        .m_axi_gpio_in_aclk(clk),
        .m_axi_gpio_in_aresetn(rstn),
        
        .m_axi_gpio_awaddr(axi_osc1.AWADDR),
        .m_axi_gpio_awburst(axi_osc1.AWBURST),
        .m_axi_gpio_awcache(axi_osc1.AWCACHE),
        .m_axi_gpio_awlen(axi_osc1.AWLEN),
        .m_axi_gpio_awprot(axi_osc1.AWPROT),
        .m_axi_gpio_awready(axi_osc1.AWREADY),
        .m_axi_gpio_awsize(axi_osc1.AWSIZE),
        .m_axi_gpio_awvalid(axi_osc1.AWVALID),
        .m_axi_gpio_bready(axi_osc1.BREADY),
        .m_axi_gpio_bresp(axi_osc1.BRESP),
        .m_axi_gpio_bvalid(axi_osc1.BVALID),
        .m_axi_gpio_wdata(axi_osc1.WDATA),
        .m_axi_gpio_wlast(axi_osc1.WLAST),
        .m_axi_gpio_wready(axi_osc1.WREADY),
        .m_axi_gpio_wstrb(axi_osc1.WSTRB),
        .m_axi_gpio_wvalid(axi_osc1.WVALID)
);

endmodule