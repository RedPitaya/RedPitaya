`timescale 1 ps / 1 ps

module system_wrapper
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
    FCLK_CLK0,
    FCLK_CLK1,
    FCLK_CLK2,
    FCLK_CLK3,
    FCLK_RESET0_N,
    FCLK_RESET1_N,
    FCLK_RESET2_N,
    FCLK_RESET3_N,
    FIXED_IO_ddr_vrn,
    FIXED_IO_ddr_vrp,
    FIXED_IO_mio,
    FIXED_IO_ps_clk,
    FIXED_IO_ps_porb,
    FIXED_IO_ps_srstb,
    M_AXI_GP0_araddr,
    M_AXI_GP0_arburst,
    M_AXI_GP0_arcache,
    M_AXI_GP0_arid,
    M_AXI_GP0_arlen,
    M_AXI_GP0_arlock,
    M_AXI_GP0_arprot,
    M_AXI_GP0_arqos,
    M_AXI_GP0_arready,
    M_AXI_GP0_arsize,
    M_AXI_GP0_arvalid,
    M_AXI_GP0_awaddr,
    M_AXI_GP0_awburst,
    M_AXI_GP0_awcache,
    M_AXI_GP0_awid,
    M_AXI_GP0_awlen,
    M_AXI_GP0_awlock,
    M_AXI_GP0_awprot,
    M_AXI_GP0_awqos,
    M_AXI_GP0_awready,
    M_AXI_GP0_awsize,
    M_AXI_GP0_awvalid,
    M_AXI_GP0_bid,
    M_AXI_GP0_bready,
    M_AXI_GP0_bresp,
    M_AXI_GP0_bvalid,
    M_AXI_GP0_rdata,
    M_AXI_GP0_rid,
    M_AXI_GP0_rlast,
    M_AXI_GP0_rready,
    M_AXI_GP0_rresp,
    M_AXI_GP0_rvalid,
    M_AXI_GP0_wdata,
    M_AXI_GP0_wid,
    M_AXI_GP0_wlast,
    M_AXI_GP0_wready,
    M_AXI_GP0_wstrb,
    M_AXI_GP0_wvalid,
    SPI0_MISO_I,
    SPI0_MISO_O,
    SPI0_MISO_T,
    SPI0_MOSI_I,
    SPI0_MOSI_O,
    SPI0_MOSI_T,
    SPI0_SCLK_I,
    SPI0_SCLK_O,
    SPI0_SCLK_T,
    SPI0_SS1_O,
    SPI0_SS2_O,
    SPI0_SS_I,
    SPI0_SS_O,
    SPI0_SS_T,
    S_AXI_HP0_aclk,
    S_AXI_HP0_araddr,
    S_AXI_HP0_arburst,
    S_AXI_HP0_arcache,
    S_AXI_HP0_arid,
    S_AXI_HP0_arlen,
    S_AXI_HP0_arlock,
    S_AXI_HP0_arprot,
    S_AXI_HP0_arqos,
    S_AXI_HP0_arready,
    S_AXI_HP0_arsize,
    S_AXI_HP0_arvalid,
    S_AXI_HP0_awaddr,
    S_AXI_HP0_awburst,
    S_AXI_HP0_awcache,
    S_AXI_HP0_awid,
    S_AXI_HP0_awlen,
    S_AXI_HP0_awlock,
    S_AXI_HP0_awprot,
    S_AXI_HP0_awqos,
    S_AXI_HP0_awready,
    S_AXI_HP0_awsize,
    S_AXI_HP0_awvalid,
    S_AXI_HP0_bid,
    S_AXI_HP0_bready,
    S_AXI_HP0_bresp,
    S_AXI_HP0_bvalid,
    S_AXI_HP0_rdata,
    S_AXI_HP0_rid,
    S_AXI_HP0_rlast,
    S_AXI_HP0_rready,
    S_AXI_HP0_rresp,
    S_AXI_HP0_rvalid,
    S_AXI_HP0_wdata,
    S_AXI_HP0_wid,
    S_AXI_HP0_wlast,
    S_AXI_HP0_wready,
    S_AXI_HP0_wstrb,
    S_AXI_HP0_wvalid,
    S_AXI_HP1_aclk,
    S_AXI_HP1_araddr,
    S_AXI_HP1_arburst,
    S_AXI_HP1_arcache,
    S_AXI_HP1_arid,
    S_AXI_HP1_arlen,
    S_AXI_HP1_arlock,
    S_AXI_HP1_arprot,
    S_AXI_HP1_arqos,
    S_AXI_HP1_arready,
    S_AXI_HP1_arsize,
    S_AXI_HP1_arvalid,
    S_AXI_HP1_awaddr,
    S_AXI_HP1_awburst,
    S_AXI_HP1_awcache,
    S_AXI_HP1_awid,
    S_AXI_HP1_awlen,
    S_AXI_HP1_awlock,
    S_AXI_HP1_awprot,
    S_AXI_HP1_awqos,
    S_AXI_HP1_awready,
    S_AXI_HP1_awsize,
    S_AXI_HP1_awvalid,
    S_AXI_HP1_bid,
    S_AXI_HP1_bready,
    S_AXI_HP1_bresp,
    S_AXI_HP1_bvalid,
    S_AXI_HP1_rdata,
    S_AXI_HP1_rid,
    S_AXI_HP1_rlast,
    S_AXI_HP1_rready,
    S_AXI_HP1_rresp,
    S_AXI_HP1_rvalid,
    S_AXI_HP1_wdata,
    S_AXI_HP1_wid,
    S_AXI_HP1_wlast,
    S_AXI_HP1_wready,
    S_AXI_HP1_wstrb,
    S_AXI_HP1_wvalid);
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
  output FCLK_CLK0;
  output FCLK_CLK1;
  output FCLK_CLK2;
  output FCLK_CLK3;
  output FCLK_RESET0_N;
  output FCLK_RESET1_N;
  output FCLK_RESET2_N;
  output FCLK_RESET3_N;
  inout FIXED_IO_ddr_vrn;
  inout FIXED_IO_ddr_vrp;
  inout [53:0]FIXED_IO_mio;
  inout FIXED_IO_ps_clk;
  inout FIXED_IO_ps_porb;
  inout FIXED_IO_ps_srstb;
  output [31:0]M_AXI_GP0_araddr;
  output [1:0]M_AXI_GP0_arburst;
  output [3:0]M_AXI_GP0_arcache;
  output [11:0]M_AXI_GP0_arid;
  output [3:0]M_AXI_GP0_arlen;
  output [1:0]M_AXI_GP0_arlock;
  output [2:0]M_AXI_GP0_arprot;
  output [3:0]M_AXI_GP0_arqos;
  input M_AXI_GP0_arready;
  output [2:0]M_AXI_GP0_arsize;
  output M_AXI_GP0_arvalid;
  output [31:0]M_AXI_GP0_awaddr;
  output [1:0]M_AXI_GP0_awburst;
  output [3:0]M_AXI_GP0_awcache;
  output [11:0]M_AXI_GP0_awid;
  output [3:0]M_AXI_GP0_awlen;
  output [1:0]M_AXI_GP0_awlock;
  output [2:0]M_AXI_GP0_awprot;
  output [3:0]M_AXI_GP0_awqos;
  input M_AXI_GP0_awready;
  output [2:0]M_AXI_GP0_awsize;
  output M_AXI_GP0_awvalid;
  input [11:0]M_AXI_GP0_bid;
  output M_AXI_GP0_bready;
  input [1:0]M_AXI_GP0_bresp;
  input M_AXI_GP0_bvalid;
  input [31:0]M_AXI_GP0_rdata;
  input [11:0]M_AXI_GP0_rid;
  input M_AXI_GP0_rlast;
  output M_AXI_GP0_rready;
  input [1:0]M_AXI_GP0_rresp;
  input M_AXI_GP0_rvalid;
  output [31:0]M_AXI_GP0_wdata;
  output [11:0]M_AXI_GP0_wid;
  output M_AXI_GP0_wlast;
  input M_AXI_GP0_wready;
  output [3:0]M_AXI_GP0_wstrb;
  output M_AXI_GP0_wvalid;
  input SPI0_MISO_I;
  output SPI0_MISO_O;
  output SPI0_MISO_T;
  input SPI0_MOSI_I;
  output SPI0_MOSI_O;
  output SPI0_MOSI_T;
  input SPI0_SCLK_I;
  output SPI0_SCLK_O;
  output SPI0_SCLK_T;
  output SPI0_SS1_O;
  output SPI0_SS2_O;
  input SPI0_SS_I;
  output SPI0_SS_O;
  output SPI0_SS_T;
  input S_AXI_HP0_aclk;
  input [31:0]S_AXI_HP0_araddr;
  input [1:0]S_AXI_HP0_arburst;
  input [3:0]S_AXI_HP0_arcache;
  input [5:0]S_AXI_HP0_arid;
  input [3:0]S_AXI_HP0_arlen;
  input [1:0]S_AXI_HP0_arlock;
  input [2:0]S_AXI_HP0_arprot;
  input [3:0]S_AXI_HP0_arqos;
  output S_AXI_HP0_arready;
  input [2:0]S_AXI_HP0_arsize;
  input S_AXI_HP0_arvalid;
  input [31:0]S_AXI_HP0_awaddr;
  input [1:0]S_AXI_HP0_awburst;
  input [3:0]S_AXI_HP0_awcache;
  input [5:0]S_AXI_HP0_awid;
  input [3:0]S_AXI_HP0_awlen;
  input [1:0]S_AXI_HP0_awlock;
  input [2:0]S_AXI_HP0_awprot;
  input [3:0]S_AXI_HP0_awqos;
  output S_AXI_HP0_awready;
  input [2:0]S_AXI_HP0_awsize;
  input S_AXI_HP0_awvalid;
  output [5:0]S_AXI_HP0_bid;
  input S_AXI_HP0_bready;
  output [1:0]S_AXI_HP0_bresp;
  output S_AXI_HP0_bvalid;
  output [63:0]S_AXI_HP0_rdata;
  output [5:0]S_AXI_HP0_rid;
  output S_AXI_HP0_rlast;
  input S_AXI_HP0_rready;
  output [1:0]S_AXI_HP0_rresp;
  output S_AXI_HP0_rvalid;
  input [63:0]S_AXI_HP0_wdata;
  input [5:0]S_AXI_HP0_wid;
  input S_AXI_HP0_wlast;
  output S_AXI_HP0_wready;
  input [7:0]S_AXI_HP0_wstrb;
  input S_AXI_HP0_wvalid;
  input S_AXI_HP1_aclk;
  input [31:0]S_AXI_HP1_araddr;
  input [1:0]S_AXI_HP1_arburst;
  input [3:0]S_AXI_HP1_arcache;
  input [5:0]S_AXI_HP1_arid;
  input [3:0]S_AXI_HP1_arlen;
  input [1:0]S_AXI_HP1_arlock;
  input [2:0]S_AXI_HP1_arprot;
  input [3:0]S_AXI_HP1_arqos;
  output S_AXI_HP1_arready;
  input [2:0]S_AXI_HP1_arsize;
  input S_AXI_HP1_arvalid;
  input [31:0]S_AXI_HP1_awaddr;
  input [1:0]S_AXI_HP1_awburst;
  input [3:0]S_AXI_HP1_awcache;
  input [5:0]S_AXI_HP1_awid;
  input [3:0]S_AXI_HP1_awlen;
  input [1:0]S_AXI_HP1_awlock;
  input [2:0]S_AXI_HP1_awprot;
  input [3:0]S_AXI_HP1_awqos;
  output S_AXI_HP1_awready;
  input [2:0]S_AXI_HP1_awsize;
  input S_AXI_HP1_awvalid;
  output [5:0]S_AXI_HP1_bid;
  input S_AXI_HP1_bready;
  output [1:0]S_AXI_HP1_bresp;
  output S_AXI_HP1_bvalid;
  output [63:0]S_AXI_HP1_rdata;
  output [5:0]S_AXI_HP1_rid;
  output S_AXI_HP1_rlast;
  input S_AXI_HP1_rready;
  output [1:0]S_AXI_HP1_rresp;
  output S_AXI_HP1_rvalid;
  input [63:0]S_AXI_HP1_wdata;
  input [5:0]S_AXI_HP1_wid;
  input S_AXI_HP1_wlast;
  output S_AXI_HP1_wready;
  input [7:0]S_AXI_HP1_wstrb;
  input S_AXI_HP1_wvalid;

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
  wire FCLK_CLK0;
  wire FCLK_CLK1;
  wire FCLK_CLK2;
  wire FCLK_CLK3;
  wire FCLK_RESET0_N;
  wire FCLK_RESET1_N;
  wire FCLK_RESET2_N;
  wire FCLK_RESET3_N;
  wire FIXED_IO_ddr_vrn;
  wire FIXED_IO_ddr_vrp;
  wire [53:0]FIXED_IO_mio;
  wire FIXED_IO_ps_clk;
  wire FIXED_IO_ps_porb;
  wire FIXED_IO_ps_srstb;
  wire [31:0]M_AXI_GP0_araddr;
  wire [1:0]M_AXI_GP0_arburst;
  wire [3:0]M_AXI_GP0_arcache;
  wire [11:0]M_AXI_GP0_arid;
  wire [3:0]M_AXI_GP0_arlen;
  wire [1:0]M_AXI_GP0_arlock;
  wire [2:0]M_AXI_GP0_arprot;
  wire [3:0]M_AXI_GP0_arqos;
  wire M_AXI_GP0_arready;
  wire [2:0]M_AXI_GP0_arsize;
  wire M_AXI_GP0_arvalid;
  wire [31:0]M_AXI_GP0_awaddr;
  wire [1:0]M_AXI_GP0_awburst;
  wire [3:0]M_AXI_GP0_awcache;
  wire [11:0]M_AXI_GP0_awid;
  wire [3:0]M_AXI_GP0_awlen;
  wire [1:0]M_AXI_GP0_awlock;
  wire [2:0]M_AXI_GP0_awprot;
  wire [3:0]M_AXI_GP0_awqos;
  wire M_AXI_GP0_awready;
  wire [2:0]M_AXI_GP0_awsize;
  wire M_AXI_GP0_awvalid;
  wire [11:0]M_AXI_GP0_bid;
  wire M_AXI_GP0_bready;
  wire [1:0]M_AXI_GP0_bresp;
  wire M_AXI_GP0_bvalid;
  wire [31:0]M_AXI_GP0_rdata;
  wire [11:0]M_AXI_GP0_rid;
  wire M_AXI_GP0_rlast;
  wire M_AXI_GP0_rready;
  wire [1:0]M_AXI_GP0_rresp;
  wire M_AXI_GP0_rvalid;
  wire [31:0]M_AXI_GP0_wdata;
  wire [11:0]M_AXI_GP0_wid;
  wire M_AXI_GP0_wlast;
  wire M_AXI_GP0_wready;
  wire [3:0]M_AXI_GP0_wstrb;
  wire M_AXI_GP0_wvalid;
  wire SPI0_MISO_I;
  wire SPI0_MISO_O;
  wire SPI0_MISO_T;
  wire SPI0_MOSI_I;
  wire SPI0_MOSI_O;
  wire SPI0_MOSI_T;
  wire SPI0_SCLK_I;
  wire SPI0_SCLK_O;
  wire SPI0_SCLK_T;
  wire SPI0_SS1_O;
  wire SPI0_SS2_O;
  wire SPI0_SS_I;
  wire SPI0_SS_O;
  wire SPI0_SS_T;
  wire S_AXI_HP0_aclk;
  wire [31:0]S_AXI_HP0_araddr;
  wire [1:0]S_AXI_HP0_arburst;
  wire [3:0]S_AXI_HP0_arcache;
  wire [5:0]S_AXI_HP0_arid;
  wire [3:0]S_AXI_HP0_arlen;
  wire [1:0]S_AXI_HP0_arlock;
  wire [2:0]S_AXI_HP0_arprot;
  wire [3:0]S_AXI_HP0_arqos;
  wire S_AXI_HP0_arready;
  wire [2:0]S_AXI_HP0_arsize;
  wire S_AXI_HP0_arvalid;
  wire [31:0]S_AXI_HP0_awaddr;
  wire [1:0]S_AXI_HP0_awburst;
  wire [3:0]S_AXI_HP0_awcache;
  wire [5:0]S_AXI_HP0_awid;
  wire [3:0]S_AXI_HP0_awlen;
  wire [1:0]S_AXI_HP0_awlock;
  wire [2:0]S_AXI_HP0_awprot;
  wire [3:0]S_AXI_HP0_awqos;
  wire S_AXI_HP0_awready;
  wire [2:0]S_AXI_HP0_awsize;
  wire S_AXI_HP0_awvalid;
  wire [5:0]S_AXI_HP0_bid;
  wire S_AXI_HP0_bready;
  wire [1:0]S_AXI_HP0_bresp;
  wire S_AXI_HP0_bvalid;
  wire [63:0]S_AXI_HP0_rdata;
  wire [5:0]S_AXI_HP0_rid;
  wire S_AXI_HP0_rlast;
  wire S_AXI_HP0_rready;
  wire [1:0]S_AXI_HP0_rresp;
  wire S_AXI_HP0_rvalid;
  wire [63:0]S_AXI_HP0_wdata;
  wire [5:0]S_AXI_HP0_wid;
  wire S_AXI_HP0_wlast;
  wire S_AXI_HP0_wready;
  wire [7:0]S_AXI_HP0_wstrb;
  wire S_AXI_HP0_wvalid;
  wire S_AXI_HP1_aclk;
  wire [31:0]S_AXI_HP1_araddr;
  wire [1:0]S_AXI_HP1_arburst;
  wire [3:0]S_AXI_HP1_arcache;
  wire [5:0]S_AXI_HP1_arid;
  wire [3:0]S_AXI_HP1_arlen;
  wire [1:0]S_AXI_HP1_arlock;
  wire [2:0]S_AXI_HP1_arprot;
  wire [3:0]S_AXI_HP1_arqos;
  wire S_AXI_HP1_arready;
  wire [2:0]S_AXI_HP1_arsize;
  wire S_AXI_HP1_arvalid;
  wire [31:0]S_AXI_HP1_awaddr;
  wire [1:0]S_AXI_HP1_awburst;
  wire [3:0]S_AXI_HP1_awcache;
  wire [5:0]S_AXI_HP1_awid;
  wire [3:0]S_AXI_HP1_awlen;
  wire [1:0]S_AXI_HP1_awlock;
  wire [2:0]S_AXI_HP1_awprot;
  wire [3:0]S_AXI_HP1_awqos;
  wire S_AXI_HP1_awready;
  wire [2:0]S_AXI_HP1_awsize;
  wire S_AXI_HP1_awvalid;
  wire [5:0]S_AXI_HP1_bid;
  wire S_AXI_HP1_bready;
  wire [1:0]S_AXI_HP1_bresp;
  wire S_AXI_HP1_bvalid;
  wire [63:0]S_AXI_HP1_rdata;
  wire [5:0]S_AXI_HP1_rid;
  wire S_AXI_HP1_rlast;
  wire S_AXI_HP1_rready;
  wire [1:0]S_AXI_HP1_rresp;
  wire S_AXI_HP1_rvalid;
  wire [63:0]S_AXI_HP1_wdata;
  wire [5:0]S_AXI_HP1_wid;
  wire S_AXI_HP1_wlast;
  wire S_AXI_HP1_wready;
  wire [7:0]S_AXI_HP1_wstrb;
  wire S_AXI_HP1_wvalid;

system system_i
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
        .FCLK_CLK0(FCLK_CLK0),
        .FCLK_CLK1(FCLK_CLK1),
        .FCLK_CLK2(FCLK_CLK2),
        .FCLK_CLK3(FCLK_CLK3),
        .FCLK_RESET0_N(FCLK_RESET0_N),
        .FCLK_RESET1_N(FCLK_RESET1_N),
        .FCLK_RESET2_N(FCLK_RESET2_N),
        .FCLK_RESET3_N(FCLK_RESET3_N),
        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
        .M_AXI_GP0_araddr(M_AXI_GP0_araddr),
        .M_AXI_GP0_arburst(M_AXI_GP0_arburst),
        .M_AXI_GP0_arcache(M_AXI_GP0_arcache),
        .M_AXI_GP0_arid(M_AXI_GP0_arid),
        .M_AXI_GP0_arlen(M_AXI_GP0_arlen),
        .M_AXI_GP0_arlock(M_AXI_GP0_arlock),
        .M_AXI_GP0_arprot(M_AXI_GP0_arprot),
        .M_AXI_GP0_arqos(M_AXI_GP0_arqos),
        .M_AXI_GP0_arready(M_AXI_GP0_arready),
        .M_AXI_GP0_arsize(M_AXI_GP0_arsize),
        .M_AXI_GP0_arvalid(M_AXI_GP0_arvalid),
        .M_AXI_GP0_awaddr(M_AXI_GP0_awaddr),
        .M_AXI_GP0_awburst(M_AXI_GP0_awburst),
        .M_AXI_GP0_awcache(M_AXI_GP0_awcache),
        .M_AXI_GP0_awid(M_AXI_GP0_awid),
        .M_AXI_GP0_awlen(M_AXI_GP0_awlen),
        .M_AXI_GP0_awlock(M_AXI_GP0_awlock),
        .M_AXI_GP0_awprot(M_AXI_GP0_awprot),
        .M_AXI_GP0_awqos(M_AXI_GP0_awqos),
        .M_AXI_GP0_awready(M_AXI_GP0_awready),
        .M_AXI_GP0_awsize(M_AXI_GP0_awsize),
        .M_AXI_GP0_awvalid(M_AXI_GP0_awvalid),
        .M_AXI_GP0_bid(M_AXI_GP0_bid),
        .M_AXI_GP0_bready(M_AXI_GP0_bready),
        .M_AXI_GP0_bresp(M_AXI_GP0_bresp),
        .M_AXI_GP0_bvalid(M_AXI_GP0_bvalid),
        .M_AXI_GP0_rdata(M_AXI_GP0_rdata),
        .M_AXI_GP0_rid(M_AXI_GP0_rid),
        .M_AXI_GP0_rlast(M_AXI_GP0_rlast),
        .M_AXI_GP0_rready(M_AXI_GP0_rready),
        .M_AXI_GP0_rresp(M_AXI_GP0_rresp),
        .M_AXI_GP0_rvalid(M_AXI_GP0_rvalid),
        .M_AXI_GP0_wdata(M_AXI_GP0_wdata),
        .M_AXI_GP0_wid(M_AXI_GP0_wid),
        .M_AXI_GP0_wlast(M_AXI_GP0_wlast),
        .M_AXI_GP0_wready(M_AXI_GP0_wready),
        .M_AXI_GP0_wstrb(M_AXI_GP0_wstrb),
        .M_AXI_GP0_wvalid(M_AXI_GP0_wvalid),
        .SPI0_MISO_I(SPI0_MISO_I),
        .SPI0_MISO_O(SPI0_MISO_O),
        .SPI0_MISO_T(SPI0_MISO_T),
        .SPI0_MOSI_I(SPI0_MOSI_I),
        .SPI0_MOSI_O(SPI0_MOSI_O),
        .SPI0_MOSI_T(SPI0_MOSI_T),
        .SPI0_SCLK_I(SPI0_SCLK_I),
        .SPI0_SCLK_O(SPI0_SCLK_O),
        .SPI0_SCLK_T(SPI0_SCLK_T),
        .SPI0_SS1_O(SPI0_SS1_O),
        .SPI0_SS2_O(SPI0_SS2_O),
        .SPI0_SS_I(SPI0_SS_I),
        .SPI0_SS_O(SPI0_SS_O),
        .SPI0_SS_T(SPI0_SS_T),
        .S_AXI_HP0_aclk(S_AXI_HP0_aclk),
        .S_AXI_HP0_araddr(S_AXI_HP0_araddr),
        .S_AXI_HP0_arburst(S_AXI_HP0_arburst),
        .S_AXI_HP0_arcache(S_AXI_HP0_arcache),
        .S_AXI_HP0_arid(S_AXI_HP0_arid),
        .S_AXI_HP0_arlen(S_AXI_HP0_arlen),
        .S_AXI_HP0_arlock(S_AXI_HP0_arlock),
        .S_AXI_HP0_arprot(S_AXI_HP0_arprot),
        .S_AXI_HP0_arqos(S_AXI_HP0_arqos),
        .S_AXI_HP0_arready(S_AXI_HP0_arready),
        .S_AXI_HP0_arsize(S_AXI_HP0_arsize),
        .S_AXI_HP0_arvalid(S_AXI_HP0_arvalid),
        .S_AXI_HP0_awaddr(S_AXI_HP0_awaddr),
        .S_AXI_HP0_awburst(S_AXI_HP0_awburst),
        .S_AXI_HP0_awcache(S_AXI_HP0_awcache),
        .S_AXI_HP0_awid(S_AXI_HP0_awid),
        .S_AXI_HP0_awlen(S_AXI_HP0_awlen),
        .S_AXI_HP0_awlock(S_AXI_HP0_awlock),
        .S_AXI_HP0_awprot(S_AXI_HP0_awprot),
        .S_AXI_HP0_awqos(S_AXI_HP0_awqos),
        .S_AXI_HP0_awready(S_AXI_HP0_awready),
        .S_AXI_HP0_awsize(S_AXI_HP0_awsize),
        .S_AXI_HP0_awvalid(S_AXI_HP0_awvalid),
        .S_AXI_HP0_bid(S_AXI_HP0_bid),
        .S_AXI_HP0_bready(S_AXI_HP0_bready),
        .S_AXI_HP0_bresp(S_AXI_HP0_bresp),
        .S_AXI_HP0_bvalid(S_AXI_HP0_bvalid),
        .S_AXI_HP0_rdata(S_AXI_HP0_rdata),
        .S_AXI_HP0_rid(S_AXI_HP0_rid),
        .S_AXI_HP0_rlast(S_AXI_HP0_rlast),
        .S_AXI_HP0_rready(S_AXI_HP0_rready),
        .S_AXI_HP0_rresp(S_AXI_HP0_rresp),
        .S_AXI_HP0_rvalid(S_AXI_HP0_rvalid),
        .S_AXI_HP0_wdata(S_AXI_HP0_wdata),
        .S_AXI_HP0_wid(S_AXI_HP0_wid),
        .S_AXI_HP0_wlast(S_AXI_HP0_wlast),
        .S_AXI_HP0_wready(S_AXI_HP0_wready),
        .S_AXI_HP0_wstrb(S_AXI_HP0_wstrb),
        .S_AXI_HP0_wvalid(S_AXI_HP0_wvalid),
        .S_AXI_HP1_aclk(S_AXI_HP1_aclk),
        .S_AXI_HP1_araddr(S_AXI_HP1_araddr),
        .S_AXI_HP1_arburst(S_AXI_HP1_arburst),
        .S_AXI_HP1_arcache(S_AXI_HP1_arcache),
        .S_AXI_HP1_arid(S_AXI_HP1_arid),
        .S_AXI_HP1_arlen(S_AXI_HP1_arlen),
        .S_AXI_HP1_arlock(S_AXI_HP1_arlock),
        .S_AXI_HP1_arprot(S_AXI_HP1_arprot),
        .S_AXI_HP1_arqos(S_AXI_HP1_arqos),
        .S_AXI_HP1_arready(S_AXI_HP1_arready),
        .S_AXI_HP1_arsize(S_AXI_HP1_arsize),
        .S_AXI_HP1_arvalid(S_AXI_HP1_arvalid),
        .S_AXI_HP1_awaddr(S_AXI_HP1_awaddr),
        .S_AXI_HP1_awburst(S_AXI_HP1_awburst),
        .S_AXI_HP1_awcache(S_AXI_HP1_awcache),
        .S_AXI_HP1_awid(S_AXI_HP1_awid),
        .S_AXI_HP1_awlen(S_AXI_HP1_awlen),
        .S_AXI_HP1_awlock(S_AXI_HP1_awlock),
        .S_AXI_HP1_awprot(S_AXI_HP1_awprot),
        .S_AXI_HP1_awqos(S_AXI_HP1_awqos),
        .S_AXI_HP1_awready(S_AXI_HP1_awready),
        .S_AXI_HP1_awsize(S_AXI_HP1_awsize),
        .S_AXI_HP1_awvalid(S_AXI_HP1_awvalid),
        .S_AXI_HP1_bid(S_AXI_HP1_bid),
        .S_AXI_HP1_bready(S_AXI_HP1_bready),
        .S_AXI_HP1_bresp(S_AXI_HP1_bresp),
        .S_AXI_HP1_bvalid(S_AXI_HP1_bvalid),
        .S_AXI_HP1_rdata(S_AXI_HP1_rdata),
        .S_AXI_HP1_rid(S_AXI_HP1_rid),
        .S_AXI_HP1_rlast(S_AXI_HP1_rlast),
        .S_AXI_HP1_rready(S_AXI_HP1_rready),
        .S_AXI_HP1_rresp(S_AXI_HP1_rresp),
        .S_AXI_HP1_rvalid(S_AXI_HP1_rvalid),
        .S_AXI_HP1_wdata(S_AXI_HP1_wdata),
        .S_AXI_HP1_wid(S_AXI_HP1_wid),
        .S_AXI_HP1_wlast(S_AXI_HP1_wlast),
        .S_AXI_HP1_wready(S_AXI_HP1_wready),
        .S_AXI_HP1_wstrb(S_AXI_HP1_wstrb),
        .S_AXI_HP1_wvalid(S_AXI_HP1_wvalid));
endmodule
