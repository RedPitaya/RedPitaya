//Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2018.3 (win64) Build 2405991 Thu Dec  6 23:38:27 MST 2018
//Date        : Thu Oct 17 12:18:07 2019
//Host        : Jon-PC running 64-bit major release  (build 9200)
//Command     : generate_target system_wrapper.bd
//Design      : system_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module red_pitaya_top_sim
   (DDR_addr,
    DDR_ba,
    DDR_cas_n,
    DDR_ck_n,
    DDR_ck_p,
    DDR_cke,
    DDR_cs_n,
    DDR_dm,
    DDR_dq,
    DDR_dqs_n,
    DDR_dqs_p,
    DDR_odt,
    DDR_ras_n,
    DDR_reset_n,
    DDR_we_n,
    FIXED_IO_ddr_vrn,
    FIXED_IO_ddr_vrp,
    FIXED_IO_mio,
    FIXED_IO_ps_clk,
    FIXED_IO_ps_porb,
    FIXED_IO_ps_srstb,

    M_AXI_OSC_araddr,
    M_AXI_OSC_arburst,
    M_AXI_OSC_arcache,
    M_AXI_OSC_arid,
    M_AXI_OSC_arlen,
    M_AXI_OSC_arlock,
    M_AXI_OSC_arprot,
    M_AXI_OSC_arqos,
    M_AXI_OSC_arready,
    M_AXI_OSC_arsize,
    M_AXI_OSC_arvalid,
    M_AXI_OSC_awaddr,
    M_AXI_OSC_awburst,
    M_AXI_OSC_awcache,
    M_AXI_OSC_awid,
    M_AXI_OSC_awlen,
    M_AXI_OSC_awlock,
    M_AXI_OSC_awprot,
    M_AXI_OSC_awqos,
    M_AXI_OSC_awready,
    M_AXI_OSC_awsize,
    M_AXI_OSC_awvalid,
    M_AXI_OSC_bid,
    M_AXI_OSC_bready,
    M_AXI_OSC_bresp,
    M_AXI_OSC_bvalid,
    M_AXI_OSC_rdata,
    M_AXI_OSC_rid,
    M_AXI_OSC_rlast,
    M_AXI_OSC_rready,
    M_AXI_OSC_rresp,
    M_AXI_OSC_rvalid,
    M_AXI_OSC_wdata,
    M_AXI_OSC_wid,
    M_AXI_OSC_wlast,
    M_AXI_OSC_wready,
    M_AXI_OSC_wstrb,
    M_AXI_OSC_wvalid,

    S_AXI_REG_araddr,
    S_AXI_REG_arburst,
    S_AXI_REG_arcache,
    S_AXI_REG_arid,
    S_AXI_REG_arlen,
    S_AXI_REG_arlock,
    S_AXI_REG_arprot,
    S_AXI_REG_arqos,
    S_AXI_REG_arready,
    S_AXI_REG_arsize,
    S_AXI_REG_arvalid,
    S_AXI_REG_awaddr,
    S_AXI_REG_awburst,
    S_AXI_REG_awcache,
    S_AXI_REG_awid,
    S_AXI_REG_awlen,
    S_AXI_REG_awlock,
    S_AXI_REG_awprot,
    S_AXI_REG_awqos,
    S_AXI_REG_awready,
    S_AXI_REG_awsize,
    S_AXI_REG_awvalid,
    S_AXI_REG_bid,
    S_AXI_REG_bready,
    S_AXI_REG_bresp,
    S_AXI_REG_bvalid,
    S_AXI_REG_rdata,
    S_AXI_REG_rid,
    S_AXI_REG_rlast,
    S_AXI_REG_rready,
    S_AXI_REG_rresp,
    S_AXI_REG_rvalid,
    S_AXI_REG_wdata,
    S_AXI_REG_wid,
    S_AXI_REG_wlast,
    S_AXI_REG_wready,
    S_AXI_REG_wstrb,
    S_AXI_REG_wvalid,

    clkout_625,
    clkout_125,
    rstn_out,
    rst_in,

    adc_clk_n,
    adc_clk_p,
    adc_data_ch1,
    adc_data_ch2);

  inout [14:0]DDR_addr;
  inout [2:0]DDR_ba;
  inout DDR_cas_n;
  inout DDR_ck_n;
  inout DDR_ck_p;
  inout DDR_cke;
  inout DDR_cs_n;
  inout [3:0]DDR_dm;
  inout [31:0]DDR_dq;
  inout [3:0]DDR_dqs_n;
  inout [3:0]DDR_dqs_p;
  inout DDR_odt;
  inout DDR_ras_n;
  inout DDR_reset_n;
  inout DDR_we_n;
  inout FIXED_IO_ddr_vrn;
  inout FIXED_IO_ddr_vrp;
  inout [53:0]FIXED_IO_mio;
  inout FIXED_IO_ps_clk;
  inout FIXED_IO_ps_porb;
  inout FIXED_IO_ps_srstb;

  output [31:0]M_AXI_OSC_araddr;
  output [1:0]M_AXI_OSC_arburst;
  output [3:0]M_AXI_OSC_arcache;
  output [0:0]M_AXI_OSC_arid;
  output [3:0]M_AXI_OSC_arlen;
  output [1:0]M_AXI_OSC_arlock;
  output [2:0]M_AXI_OSC_arprot;
  output [3:0]M_AXI_OSC_arqos;
  input M_AXI_OSC_arready;
  output [2:0]M_AXI_OSC_arsize;
  output M_AXI_OSC_arvalid;
  output [31:0]M_AXI_OSC_awaddr;
  output [1:0]M_AXI_OSC_awburst;
  output [3:0]M_AXI_OSC_awcache;
  output [0:0]M_AXI_OSC_awid;
  output [3:0]M_AXI_OSC_awlen;
  output [1:0]M_AXI_OSC_awlock;
  output [2:0]M_AXI_OSC_awprot;
  output [3:0]M_AXI_OSC_awqos;
  input M_AXI_OSC_awready;
  output [2:0]M_AXI_OSC_awsize;
  output M_AXI_OSC_awvalid;
  input [0:0]M_AXI_OSC_bid;
  output M_AXI_OSC_bready;
  input [1:0]M_AXI_OSC_bresp;
  input M_AXI_OSC_bvalid;
  input [63:0]M_AXI_OSC_rdata;
  input [0:0]M_AXI_OSC_rid;
  input M_AXI_OSC_rlast;
  output M_AXI_OSC_rready;
  input [1:0]M_AXI_OSC_rresp;
  input M_AXI_OSC_rvalid;
  output [63:0]M_AXI_OSC_wdata;
  output [0:0]M_AXI_OSC_wid;
  output M_AXI_OSC_wlast;
  input M_AXI_OSC_wready;
  output [7:0]M_AXI_OSC_wstrb;
  output M_AXI_OSC_wvalid;

  input [31:0]S_AXI_REG_araddr;
  input [1:0]S_AXI_REG_arburst;
  input [3:0]S_AXI_REG_arcache;
  input [11:0]S_AXI_REG_arid;
  input [3:0]S_AXI_REG_arlen;
  input [1:0]S_AXI_REG_arlock;
  input [2:0]S_AXI_REG_arprot;
  input [3:0]S_AXI_REG_arqos;
  output S_AXI_REG_arready;
  input [2:0]S_AXI_REG_arsize;
  input S_AXI_REG_arvalid;
  input [31:0]S_AXI_REG_awaddr;
  input [1:0]S_AXI_REG_awburst;
  input [3:0]S_AXI_REG_awcache;
  input [11:0]S_AXI_REG_awid;
  input [3:0]S_AXI_REG_awlen;
  input [1:0]S_AXI_REG_awlock;
  input [2:0]S_AXI_REG_awprot;
  input [3:0]S_AXI_REG_awqos;
  output S_AXI_REG_awready;
  input [2:0]S_AXI_REG_awsize;
  input S_AXI_REG_awvalid;
  output [11:0]S_AXI_REG_bid;
  input S_AXI_REG_bready;
  output [1:0]S_AXI_REG_bresp;
  output S_AXI_REG_bvalid;
  output [31:0]S_AXI_REG_rdata;
  output [11:0]S_AXI_REG_rid;
  output S_AXI_REG_rlast;
  input S_AXI_REG_rready;
  output [1:0]S_AXI_REG_rresp;
  output S_AXI_REG_rvalid;
  input [31:0]S_AXI_REG_wdata;
  input [11:0]S_AXI_REG_wid;
  input S_AXI_REG_wlast;
  output S_AXI_REG_wready;
  input [3:0]S_AXI_REG_wstrb;
  input S_AXI_REG_wvalid;

  output clkout_625;
  output clkout_125;
  output rstn_out;
  input  rst_in;

  input adc_clk_n;
  input adc_clk_p;
  input [15:0]adc_data_ch1;
  input [15:0]adc_data_ch2;

  wire [14:0]DDR_addr;
  wire [2:0]DDR_ba;
  wire DDR_cas_n;
  wire DDR_ck_n;
  wire DDR_ck_p;
  wire DDR_cke;
  wire DDR_cs_n;
  wire [3:0]DDR_dm;
  wire [31:0]DDR_dq;
  wire [3:0]DDR_dqs_n;
  wire [3:0]DDR_dqs_p;
  wire DDR_odt;
  wire DDR_ras_n;
  wire DDR_reset_n;
  wire DDR_we_n;
  wire FIXED_IO_ddr_vrn;
  wire FIXED_IO_ddr_vrp;
  wire [53:0]FIXED_IO_mio;
  wire FIXED_IO_ps_clk;
  wire FIXED_IO_ps_porb;
  wire FIXED_IO_ps_srstb;

  wire [31:0]M_AXI_OSC_araddr;
  wire [1:0]M_AXI_OSC_arburst;
  wire [3:0]M_AXI_OSC_arcache;
  wire [0:0]M_AXI_OSC_arid;
  wire [3:0]M_AXI_OSC_arlen;
  wire [1:0]M_AXI_OSC_arlock;
  wire [2:0]M_AXI_OSC_arprot;
  wire [3:0]M_AXI_OSC_arqos;
  wire M_AXI_OSC_arready;
  wire [2:0]M_AXI_OSC_arsize;
  wire M_AXI_OSC_arvalid;
  wire [31:0]M_AXI_OSC_awaddr;
  wire [1:0]M_AXI_OSC_awburst;
  wire [3:0]M_AXI_OSC_awcache;
  wire [0:0]M_AXI_OSC_awid;
  wire [3:0]M_AXI_OSC_awlen;
  wire [1:0]M_AXI_OSC_awlock;
  wire [2:0]M_AXI_OSC_awprot;
  wire [3:0]M_AXI_OSC_awqos;
  wire M_AXI_OSC_awready;
  wire [2:0]M_AXI_OSC_awsize;
  wire M_AXI_OSC_awvalid;
  wire [0:0]M_AXI_OSC_bid;
  wire M_AXI_OSC_bready;
  wire [1:0]M_AXI_OSC_bresp;
  wire M_AXI_OSC_bvalid;
  wire [63:0]M_AXI_OSC_rdata;
  wire [0:0]M_AXI_OSC_rid;
  wire M_AXI_OSC_rlast;
  wire M_AXI_OSC_rready;
  wire [1:0]M_AXI_OSC_rresp;
  wire M_AXI_OSC_rvalid;
  wire [63:0]M_AXI_OSC_wdata;
  wire [0:0]M_AXI_OSC_wid;
  wire M_AXI_OSC_wlast;
  wire M_AXI_OSC_wready;
  wire [7:0]M_AXI_OSC_wstrb;
  wire M_AXI_OSC_wvalid;

  wire [31:0]S_AXI_REG_araddr;
  wire [1:0]S_AXI_REG_arburst;
  wire [3:0]S_AXI_REG_arcache;
  wire [11:0]S_AXI_REG_arid;
  wire [3:0]S_AXI_REG_arlen;
  wire [1:0]S_AXI_REG_arlock;
  wire [2:0]S_AXI_REG_arprot;
  wire [3:0]S_AXI_REG_arqos;
  wire S_AXI_REG_arready;
  wire [2:0]S_AXI_REG_arsize;
  wire S_AXI_REG_arvalid;
  wire [31:0]S_AXI_REG_awaddr;
  wire [1:0]S_AXI_REG_awburst;
  wire [3:0]S_AXI_REG_awcache;
  wire [11:0]S_AXI_REG_awid;
  wire [3:0]S_AXI_REG_awlen;
  wire [1:0]S_AXI_REG_awlock;
  wire [2:0]S_AXI_REG_awprot;
  wire [3:0]S_AXI_REG_awqos;
  wire S_AXI_REG_awready;
  wire [2:0]S_AXI_REG_awsize;
  wire S_AXI_REG_awvalid;
  wire [11:0]S_AXI_REG_bid;
  wire S_AXI_REG_bready;
  wire [1:0]S_AXI_REG_bresp;
  wire S_AXI_REG_bvalid;
  wire [31:0]S_AXI_REG_rdata;
  wire [11:0]S_AXI_REG_rid;
  wire S_AXI_REG_rlast;
  wire S_AXI_REG_rready;
  wire [1:0]S_AXI_REG_rresp;
  wire S_AXI_REG_rvalid;
  wire [31:0]S_AXI_REG_wdata;
  wire [11:0]S_AXI_REG_wid;
  wire S_AXI_REG_wlast;
  wire S_AXI_REG_wready;
  wire [3:0]S_AXI_REG_wstrb;
  wire S_AXI_REG_wvalid;

  wire clkout_625;
  wire clkout_125;
  wire rstn_out;
  wire rst_in;

  wire adc_clk_n;
  wire adc_clk_p;
  wire [15:0]adc_data_ch1;
  wire [15:0]adc_data_ch2;

  system_wrapper system_wrapper_i
       (.DDR_addr(DDR_addr),
        .DDR_ba(DDR_ba),
        .DDR_cas_n(DDR_cas_n),
        .DDR_ck_n(DDR_ck_n),
        .DDR_ck_p(DDR_ck_p),
        .DDR_cke(DDR_cke),
        .DDR_cs_n(DDR_cs_n),
        .DDR_dm(DDR_dm),
        .DDR_dq(DDR_dq),
        .DDR_dqs_n(DDR_dqs_n),
        .DDR_dqs_p(DDR_dqs_p),
        .DDR_odt(DDR_odt),
        .DDR_ras_n(DDR_ras_n),
        .DDR_reset_n(DDR_reset_n),
        .DDR_we_n(DDR_we_n),
        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),

        .M_AXI_OSC_araddr(M_AXI_OSC_araddr),
        .M_AXI_OSC_arburst(M_AXI_OSC_arburst),
        .M_AXI_OSC_arcache(M_AXI_OSC_arcache),
        .M_AXI_OSC_arid(M_AXI_OSC_arid),
        .M_AXI_OSC_arlen(M_AXI_OSC_arlen),
        .M_AXI_OSC_arlock(M_AXI_OSC_arlock),
        .M_AXI_OSC_arprot(M_AXI_OSC_arprot),
        .M_AXI_OSC_arqos(M_AXI_OSC_arqos),
        .M_AXI_OSC_arready(M_AXI_OSC_arready),
        .M_AXI_OSC_arsize(M_AXI_OSC_arsize),
        .M_AXI_OSC_arvalid(M_AXI_OSC_arvalid),
        .M_AXI_OSC_awaddr(M_AXI_OSC_awaddr),
        .M_AXI_OSC_awburst(M_AXI_OSC_awburst),
        .M_AXI_OSC_awcache(M_AXI_OSC_awcache),
        .M_AXI_OSC_awid(M_AXI_OSC_awid),
        .M_AXI_OSC_awlen(M_AXI_OSC_awlen),
        .M_AXI_OSC_awlock(M_AXI_OSC_awlock),
        .M_AXI_OSC_awprot(M_AXI_OSC_awprot),
        .M_AXI_OSC_awqos(M_AXI_OSC_awqos),
        .M_AXI_OSC_awready(M_AXI_OSC_awready),
        .M_AXI_OSC_awsize(M_AXI_OSC_awsize),
        .M_AXI_OSC_awvalid(M_AXI_OSC_awvalid),
        .M_AXI_OSC_bid(M_AXI_OSC_bid),
        .M_AXI_OSC_bready(M_AXI_OSC_bready),
        .M_AXI_OSC_bresp(M_AXI_OSC_bresp),
        .M_AXI_OSC_bvalid(M_AXI_OSC_bvalid),
        .M_AXI_OSC_rdata(M_AXI_OSC_rdata),
        .M_AXI_OSC_rid(M_AXI_OSC_rid),
        .M_AXI_OSC_rlast(M_AXI_OSC_rlast),
        .M_AXI_OSC_rready(M_AXI_OSC_rready),
        .M_AXI_OSC_rresp(M_AXI_OSC_rresp),
        .M_AXI_OSC_rvalid(M_AXI_OSC_rvalid),
        .M_AXI_OSC_wdata(M_AXI_OSC_wdata),
        .M_AXI_OSC_wid(M_AXI_OSC_wid),
        .M_AXI_OSC_wlast(M_AXI_OSC_wlast),
        .M_AXI_OSC_wready(M_AXI_OSC_wready),
        .M_AXI_OSC_wstrb(M_AXI_OSC_wstrb),
        .M_AXI_OSC_wvalid(M_AXI_OSC_wvalid),

        .S_AXI_REG_araddr(S_AXI_REG_araddr),
        .S_AXI_REG_arburst(S_AXI_REG_arburst),
        .S_AXI_REG_arcache(S_AXI_REG_arcache),
        .S_AXI_REG_arid(S_AXI_REG_arid),
        .S_AXI_REG_arlen(S_AXI_REG_arlen),
        .S_AXI_REG_arlock(S_AXI_REG_arlock),
        .S_AXI_REG_arprot(S_AXI_REG_arprot),
        .S_AXI_REG_arqos(S_AXI_REG_arqos),
        .S_AXI_REG_arready(S_AXI_REG_arready),
        .S_AXI_REG_arsize(S_AXI_REG_arsize),
        .S_AXI_REG_arvalid(S_AXI_REG_arvalid),
        .S_AXI_REG_awaddr(S_AXI_REG_awaddr),
        .S_AXI_REG_awburst(S_AXI_REG_awburst),
        .S_AXI_REG_awcache(S_AXI_REG_awcache),
        .S_AXI_REG_awid(S_AXI_REG_awid),
        .S_AXI_REG_awlen(S_AXI_REG_awlen),
        .S_AXI_REG_awlock(S_AXI_REG_awlock),
        .S_AXI_REG_awprot(S_AXI_REG_awprot),
        .S_AXI_REG_awqos(S_AXI_REG_awqos),
        .S_AXI_REG_awready(S_AXI_REG_awready),
        .S_AXI_REG_awsize(S_AXI_REG_awsize),
        .S_AXI_REG_awvalid(S_AXI_REG_awvalid),
        .S_AXI_REG_bid(S_AXI_REG_bid),
        .S_AXI_REG_bready(S_AXI_REG_bready),
        .S_AXI_REG_bresp(S_AXI_REG_bresp),
        .S_AXI_REG_bvalid(S_AXI_REG_bvalid),
        .S_AXI_REG_rdata(S_AXI_REG_rdata),
        .S_AXI_REG_rid(S_AXI_REG_rid),
        .S_AXI_REG_rlast(S_AXI_REG_rlast),
        .S_AXI_REG_rready(S_AXI_REG_rready),
        .S_AXI_REG_rresp(S_AXI_REG_rresp),
        .S_AXI_REG_rvalid(S_AXI_REG_rvalid),
        .S_AXI_REG_wdata(S_AXI_REG_wdata),
        .S_AXI_REG_wid(S_AXI_REG_wid),
        .S_AXI_REG_wlast(S_AXI_REG_wlast),
        .S_AXI_REG_wready(S_AXI_REG_wready),
        .S_AXI_REG_wstrb(S_AXI_REG_wstrb),
        .S_AXI_REG_wvalid(S_AXI_REG_wvalid),

        .clkout_625(clkout_625),
        .clkout_125(clkout_125),
        .rstn_out(rstn_out),
        .rst_in(rst_in),

        .adc_clk_n(adc_clk_n),
        .adc_clk_p(adc_clk_p),
        .adc_data_ch1(adc_data_ch1[16-1:2]),
        .adc_data_ch2(adc_data_ch2[16-1:2]));
endmodule