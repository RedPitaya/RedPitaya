////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya top FPGA module
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module top_tb #(
  // time period
  realtime  TP = 4.0ns,  // 250MHz
  realtime  RP = 100.1ns,  // ~10MHz
  // DUT configuration
  int unsigned DAC_DW = 14, // ADC data width
  int unsigned RSZ = 14  // RAM size is 2**RSZ
);

////////////////////////////////////////////////////////////////////////////////
// IO port signals
////////////////////////////////////////////////////////////////////////////////

// PS connections
wire  [54-1:0] FIXED_IO_mio     ;
wire           FIXED_IO_ps_clk  ;
wire           FIXED_IO_ps_porb ;
wire           FIXED_IO_ps_srstb;
wire           FIXED_IO_ddr_vrn ;
wire           FIXED_IO_ddr_vrp ;
// DDR
wire  [15-1:0] DDR_addr   ;
wire  [ 3-1:0] DDR_ba     ;
wire           DDR_cas_n  ;
wire           DDR_ck_n   ;
wire           DDR_ck_p   ;
wire           DDR_cke    ;
wire           DDR_cs_n   ;
wire  [ 4-1:0] DDR_dm     ;
wire  [32-1:0] DDR_dq     ;
wire  [ 4-1:0] DDR_dqs_n  ;
wire  [ 4-1:0] DDR_dqs_p  ;
wire           DDR_odt    ;
wire           DDR_ras_n  ;
wire           DDR_reset_n;
wire           DDR_we_n   ;

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
logic               clkn;
wire               rstn_out;
logic               rstn;

//glbl glbl();

localparam OSC_DW = 64;
localparam REG_DW = 32;
localparam OSC_AW = 32;
localparam REG_AW = 12;
localparam IW = 12;
localparam LW = 8;

localparam GEN1_EVENT = 0;
localparam GEN2_EVENT = 1;
localparam OSC1_EVENT = 2;
localparam OSC2_EVENT = 3;
localparam LA_EVENT = 4;

/*axi4_if #(.DW (REG_DW), .AW (REG_AW), .IW (IW), .LW (LW)) axi_reg (
  .ACLK    (clk   ),  .ARESETn (rstn)
);

axi4_if #(.DW (OSC_DW), .AW (OSC_AW), .IW (IW), .LW (LW)) axi_osc1 (
  .ACLK    (clk   ),  .ARESETn (rstn)
);*/

axi4_if #(.DW (REG_DW), .AW (REG_AW), .IW (IW), .LW (LW)) axi_reg (
  .ACLK    (clkout_625   ),  .ARESETn (rstn_out)
);

axi4_if #(.DW (OSC_DW), .AW (OSC_AW), .IW (IW), .LW (LW)) axi_osc1 (
  .ACLK    (clkout_125   ),  .ARESETn (rstn_out)
);

/*axi4_if #(.DW (OSC_DW), .AW (OSC_AW), .IW (IW), .LW (LW)) axi_osc2 (
  .ACLK    (clk   ),  .ARESETn (rstn)
);*/



axi_bus_model #(.AW (REG_AW), .DW (REG_DW), .IW (IW), .LW (LW)) axi_bm_reg  (axi_reg );
axi_bus_model #(.AW (OSC_AW), .DW (OSC_DW), .IW (IW), .LW (LW)) axi_bm_osc1 (axi_osc1);
//axi_bus_model #(.AW (OSC_AW), .DW (OSC_DW), .IW (IW), .LW (LW)) axi_bm_osc2 (axi_osc2);



////////////////////////////////////////////////////////////////////////////////
// Clock and reset generation
////////////////////////////////////////////////////////////////////////////////


assign clkn = ~clk;
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






////////////////////////////////////////////////////////////////////////////////
// initializtion
////////////////////////////////////////////////////////////////////////////////

initial begin
  exp_p_od = '0;
  exp_n_od = '0;
  exp_p_oe = '0;
  exp_n_oe = '0;
end

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

//initial begin
//  ##6000;
//  $display("ERROR: timeout!");
//  $finish();
//end

initial begin
  ##500;

   //top_tc.test_hk                 (0<<20, 32'h55);
   //top_tc.test_sata               (5<<20, 32'h55);
   top_tc.test_osc                (32'h40100000, OSC1_EVENT);

//   top_tc.test_asg                (2<<20, 32'h40090000, 2);


  ##1600000000;
  $finish();
end



////////////////////////////////////////////////////////////////////////////////
// signal generation
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned DWM = 14;
localparam int unsigned CWM = 14;
localparam int unsigned CWF = 16;

//int buf_len = 2**CWM;
int buf_len = 'hff+1;
real freq  = 10_000; // 10kHz
real phase = 0; // DEG

always begin
  trig <= 1'b0;
  ##100000;
  trig <= 1'b1;
  ##1200;
  trig <= 1'b0;
end


always begin
  temp_prot <= 2'b00;
  ##50000;
  temp_prot <= 2'b10;
  ##1000;
  temp_prot <= 2'b00;
end


//localparam int unsigned SIZ_REF = 64;
//
//bit [16-1:0] dat_ref [SIZ_REF];
//
//initial begin
//  logic signed [16-1:0] dat;
//  for (int unsigned i=0; i<SIZ_REF; i++) begin
//      dat = -SIZ_REF/2+i;
//      dat_ref[i] = {dat[16-1], ~dat[16-2:0]};
//  end
//end

bit [14-1:0] dat_ref [2*15];

initial begin
  for (int unsigned i=0; i<31; i++) begin
    dat_ref [i] = {i, 2'b0};
    dat_ref [16-1-i] = {1'b1, 15'(1<<i)};
    dat_ref [16  +i] = {1'b0, 15'(1<<i)};
  end
end

// ADC
logic [2-1:0] [14-1:0] adc_dr ;
assign adc_dr[0] =  dat_ref[cyc % $size(dat_ref)];
assign adc_dr[1] = ~dat_ref[cyc % $size(dat_ref)];

always @(clk) begin
  if (clk==1) begin
    #(0.1);
    adc_dat[0] <= {adc_dr[0][12], adc_dr[0][10], adc_dr[0][8], adc_dr[0][6], adc_dr[0][4], adc_dr[0][2]};
    adc_dat[1] <= {adc_dr[1][12], adc_dr[1][10], adc_dr[1][8], adc_dr[1][6], adc_dr[1][4], adc_dr[1][2]};
  end else begin
    #(0.1);
    adc_dat[0] <= {adc_dr[0][13], adc_dr[0][11], adc_dr[0][9], adc_dr[0][7], adc_dr[0][5], adc_dr[0][3]};
    adc_dat[1] <= {adc_dr[1][13], adc_dr[1][11], adc_dr[1][9], adc_dr[1][7], adc_dr[1][5], adc_dr[1][3]};
  end
end

always @(clk) begin
  if (clk==1) begin
    #(0.7);
    adc_dco[1] <= 1;
    adc_dco[0] <= 0;
  end else begin
    #(0.7);
    adc_dco[1] <= 0;
    adc_dco[0] <= 1;
  end
end

// XADC
assign vinp = '0;
assign vinn = '0;

// Expansion connector
//assign exp_p_io = 8'h0;
//assign exp_n_io = 8'h0;

// LED


assign #0.2 daisy_p[3] = daisy_p[1] ;
assign #0.2 daisy_n[3] = daisy_n[1] ;
assign #0.2 daisy_p[2] = daisy_p[0] ;
assign #0.2 daisy_n[2] = daisy_n[0] ;




wire [11:0] wdat1; 
wire [11:0] wdat2;
wire [11:0] wdat3;
wire [11:0] wdat4;

wire [15:0] wdat5; 
wire [15:0] wdat6;
wire [15:0] wdat7;
wire [15:0] wdat8;
assign wdat1 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc1_wdata[11: 0];
assign wdat2 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc1_wdata[27:16];
assign wdat3 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc1_wdata[43:32];
assign wdat4 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc1_wdata[59:48];

assign wdat5 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc2_wdata[15: 0];
assign wdat6 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc2_wdata[31:16];
assign wdat7 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc2_wdata[47:32];
assign wdat8 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc2_wdata[63:48];

reg [15:0] cnter;
always @(clk) begin

    if (rstn==0)
        cnter <= 16'b0;
    //else if (cnter==13'hFFF && clk==1)
    //    cnter <= 13'b0;
    else if (clk == 1)
        cnter <= cnter + 16'b1; 

end







////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

// module under test

 red_pitaya_top_sim red_pitaya_top_sim
       (.DDR_addr(),
        .DDR_ba(),
        .DDR_cas_n(),
        .DDR_ck_n(),
        .DDR_ck_p(),
        .DDR_cke(),
        .DDR_cs_n(),
        .DDR_dm(),
        .DDR_dq(),
        .DDR_dqs_n(),
        .DDR_dqs_p(),
        .DDR_odt(),
        .DDR_ras_n(),
        .DDR_reset_n(),
        .DDR_we_n(),
        .FIXED_IO_ddr_vrn(),
        .FIXED_IO_ddr_vrp(),
        .FIXED_IO_mio(),
        .FIXED_IO_ps_clk(),
        .FIXED_IO_ps_porb(),
        .FIXED_IO_ps_srstb(),

        .M_AXI_OSC_araddr(axi_osc1.ARADDR),
        .M_AXI_OSC_arburst(axi_osc1.ARBURST),
        .M_AXI_OSC_arcache(axi_osc1.ARCACHE),
        .M_AXI_OSC_arid(axi_osc1.ARID),
        .M_AXI_OSC_arlen(axi_osc1.ARLEN),
        .M_AXI_OSC_arlock(axi_osc1.ARLOCK),
        .M_AXI_OSC_arprot(axi_osc1.ARPROT),
        .M_AXI_OSC_arqos(axi_osc1.ARQOS),
        .M_AXI_OSC_arready(axi_osc1.ARREADY),
        .M_AXI_OSC_arsize(axi_osc1.ARSIZE),
        .M_AXI_OSC_arvalid(axi_osc1.ARVALID),
        .M_AXI_OSC_awaddr(axi_osc1.AWADDR),
        .M_AXI_OSC_awburst(axi_osc1.AWBURST),
        .M_AXI_OSC_awcache(axi_osc1.AWCACHE),
        .M_AXI_OSC_awid(axi_osc1.AWID),
        .M_AXI_OSC_awlen(axi_osc1.AWLEN),
        .M_AXI_OSC_awlock(axi_osc1.AWLOCK),
        .M_AXI_OSC_awprot(axi_osc1.AWPROT),
        .M_AXI_OSC_awqos(axi_osc1.AWQOS),
        .M_AXI_OSC_awready(axi_osc1.AWREADY),
        .M_AXI_OSC_awsize(axi_osc1.AWSIZE),
        .M_AXI_OSC_awvalid(axi_osc1.AWVALID),
        .M_AXI_OSC_bid(axi_osc1.BID),
        .M_AXI_OSC_bready(axi_osc1.BREADY),
        .M_AXI_OSC_bresp(axi_osc1.BRESP),
        .M_AXI_OSC_bvalid(axi_osc1.BVALID),
        .M_AXI_OSC_rdata(axi_osc1.RDATA),
        .M_AXI_OSC_rid(axi_osc1.RID),
        .M_AXI_OSC_rlast(axi_osc1.RLAST),
        .M_AXI_OSC_rready(axi_osc1.RREADY),
        .M_AXI_OSC_rresp(axi_osc1.RRESP),
        .M_AXI_OSC_rvalid(axi_osc1.RVALID),
        .M_AXI_OSC_wdata(axi_osc1.WDATA),
        .M_AXI_OSC_wid(axi_osc1.WID),
        .M_AXI_OSC_wlast(axi_osc1.WLAST),
        .M_AXI_OSC_wready(axi_osc1.WREADY),
        .M_AXI_OSC_wstrb(axi_osc1.WSTRB),
        .M_AXI_OSC_wvalid(axi_osc1.WVALID),

        .S_AXI_REG_araddr(axi_reg.ARADDR),
        .S_AXI_REG_arburst(axi_reg.ARBURST),
        .S_AXI_REG_arcache(axi_reg.ARCACHE),
        .S_AXI_REG_arid(axi_reg.ARID),
        .S_AXI_REG_arlen(axi_reg.ARLEN),
        .S_AXI_REG_arlock(axi_reg.ARLOCK),
        .S_AXI_REG_arprot(axi_reg.ARPROT),
        .S_AXI_REG_arqos(axi_reg.ARQOS),
        .S_AXI_REG_arready(axi_reg.ARREADY),
        .S_AXI_REG_arsize(axi_reg.ARSIZE),
        .S_AXI_REG_arvalid(axi_reg.ARVALID),
        .S_AXI_REG_awaddr(axi_reg.AWADDR),
        .S_AXI_REG_awburst(axi_reg.AWBURST),
        .S_AXI_REG_awcache(axi_reg.AWCACHE),
        .S_AXI_REG_awid(axi_reg.AWID),
        .S_AXI_REG_awlen(axi_reg.AWLEN),
        .S_AXI_REG_awlock(axi_reg.AWLOCK),
        .S_AXI_REG_awprot(axi_reg.AWPROT),
        .S_AXI_REG_awqos(axi_reg.AWQOS),
        .S_AXI_REG_awready(axi_reg.AWREADY),
        .S_AXI_REG_awsize(axi_reg.AWSIZE),
        .S_AXI_REG_awvalid(axi_reg.AWVALID),
        .S_AXI_REG_bid(axi_reg.BID),
        .S_AXI_REG_bready(axi_reg.BREADY),
        .S_AXI_REG_bresp(axi_reg.BRESP),
        .S_AXI_REG_bvalid(axi_reg.BVALID),
        .S_AXI_REG_rdata(axi_reg.RDATA),
        .S_AXI_REG_rid(axi_reg.RID),
        .S_AXI_REG_rlast(axi_reg.RLAST),
        .S_AXI_REG_rready(axi_reg.RREADY),
        .S_AXI_REG_rresp(axi_reg.RRESP),
        .S_AXI_REG_rvalid(axi_reg.RVALID),
        .S_AXI_REG_wdata(axi_reg.WDATA),
        .S_AXI_REG_wid(axi_reg.WID),
        .S_AXI_REG_wlast(axi_reg.WLAST),
        .S_AXI_REG_wready(axi_reg.WREADY),
        .S_AXI_REG_wstrb(axi_reg.WSTRB),
        .S_AXI_REG_wvalid(axi_reg.WVALID),

        .clkout_625(clkout_625),
        .clkout_125(clkout_125),
        .rstn_out(rstn_out),
        .rst_in(~rstn),

        .adc_clk_n(clkn),
        .adc_clk_p(clk),
        //.adc_data_ch1({1'b0,cnter,2'b0}),
        .adc_data_ch1(16'h7000),
        .adc_data_ch2({cnter[15:1],1'b0}));


/*rp_concat #(
  .EVENT_SRC_NUM(5),
  .TRIG_SRC_NUM(5)
) rp_concat (
  .event_reset(rp_oscilloscope.event_ip_reset),
  .event_start(rp_oscilloscope.event_ip_start),
  .event_stop(rp_oscilloscope.event_ip_stop),
  .event_trig(rp_oscilloscope.event_ip_trig),
  .gen1_event_ip(4'b0),
  .gen1_trig_ip(1'b0),
  .gen2_event_ip(4'b0),
  .gen2_trig_ip(1'b0),
  .la_event_ip(4'b0),
  .la_trig_ip(1'b0),
  .osc1_event_ip(rp_oscilloscope.osc1_event_op),
  .osc1_trig_ip(rp_oscilloscope.osc1_trig_op),
  .osc2_event_ip(rp_oscilloscope.osc2_event_op),
  .osc2_trig_ip(rp_oscilloscope.osc2_trig_op),
  .trig(rp_oscilloscope.trig_ip)
);

rp_oscilloscope #(
  .S_AXI_REG_ADDR_BITS(REG_AW),
  .M_AXI_OSC1_ADDR_BITS(OSC_AW),
  .M_AXI_OSC1_DATA_BITS(OSC_DW),
  .M_AXI_OSC2_ADDR_BITS(OSC_AW),
  .M_AXI_OSC2_DATA_BITS(OSC_DW),
  .ADC_DATA_BITS(14),
  .EVENT_SRC_NUM(5),
  .TRIG_SRC_NUM(5)
) rp_oscilloscope (
  
  .clk(clk),
  .rst_n(rstn),

  .adc_data_ch1(adc_dr[0]),
  .adc_data_ch2(adc_dr[1]),
  
  .event_ip_reset(rp_concat.event_reset),
  .event_ip_start(rp_concat.event_start),
  .event_ip_stop(rp_concat.event_stop),
  .event_ip_trig(rp_concat.event_trig),
  .trig_ip(rp_concat.trig),
  
  .osc1_event_op(rp_concat.osc1_event_ip),
  .osc1_trig_op(rp_concat.osc1_trig_ip),
  .osc2_event_op(rp_concat.osc2_event_ip),
  .osc2_trig_op(rp_concat.osc2_trig_ip),

  .intr(intr),

  .m_axi_osc1_aclk(clk),
  .m_axi_osc1_aresetn(rstn),
  .m_axi_osc1_awaddr(axi_osc1.AWADDR),
  .m_axi_osc1_awburst(axi_osc1.AWBURST),
  .m_axi_osc1_awcache(axi_osc1.AWCACHE),
  .m_axi_osc1_awlen(axi_osc1.AWLEN),
  .m_axi_osc1_awprot(axi_osc1.AWPROT),
  .m_axi_osc1_awready(axi_osc1.AWREADY),
  .m_axi_osc1_awsize(axi_osc1.AWSIZE),
  .m_axi_osc1_awvalid(axi_osc1.AWVALID),
  .m_axi_osc1_bready(axi_osc1.BREADY),
  .m_axi_osc1_bresp(axi_osc1.BRESP),
  .m_axi_osc1_bvalid(axi_osc1.BVALID),
  .m_axi_osc1_wdata(axi_osc1.WDATA),
  .m_axi_osc1_wlast(axi_osc1.WLAST),
  .m_axi_osc1_wready(axi_osc1.WREADY),
  .m_axi_osc1_wstrb(axi_osc1.WSTRB),
  .m_axi_osc1_wvalid(axi_osc1.WVALID),

  .m_axi_osc2_aclk(clk),
  .m_axi_osc2_aresetn(rstn),
  .m_axi_osc2_awaddr(axi_osc2.AWADDR),
  .m_axi_osc2_awburst(axi_osc2.AWBURST),
  .m_axi_osc2_awcache(axi_osc2.AWCACHE),
  .m_axi_osc2_awlen(axi_osc2.AWLEN),
  .m_axi_osc2_awprot(axi_osc2.AWPROT),
  .m_axi_osc2_awready(axi_osc2.AWREADY),
  .m_axi_osc2_awsize(axi_osc2.AWSIZE),
  .m_axi_osc2_awvalid(axi_osc2.AWVALID),
  .m_axi_osc2_bready(axi_osc2.BREADY),
  .m_axi_osc2_bresp(axi_osc2.BRESP),
  .m_axi_osc2_bvalid(axi_osc2.BVALID),
  .m_axi_osc2_wdata(axi_osc2.WDATA),
  .m_axi_osc2_wlast(axi_osc2.WLAST),
  .m_axi_osc2_wready(axi_osc2.WREADY),
  .m_axi_osc2_wstrb(axi_osc2.WSTRB),
  .m_axi_osc2_wvalid(axi_osc2.WVALID),

  .s_axi_reg_aclk(clk),
  .s_axi_reg_aresetn(rstn),
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
  .s_axi_reg_wvalid(axi_reg.WVALID)
);*/

bufif1 bufif_exp_p_io [9-1:0] (exp_p_io, exp_p_od, exp_p_oe);
bufif1 bufif_exp_n_io [9-1:0] (exp_n_io, exp_n_od, exp_n_oe);
bufif1 bufif_exp_9_io         (exp_9_io, exp_9_od, exp_9_oe);
// testcases
top_tc top_tc();


////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("top_tb.vcd");
  $dumpvars(0, top_tb);
end



endmodule: top_tb
