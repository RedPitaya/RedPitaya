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

module red_pitaya_top_sim #(
  // identification
  bit [0:5*32-1] GITH = '0,
  // module numbers
  parameter ADC_DATA_BITS = 12,
  int unsigned MNA = 2,  // number of acquisition modules
  int unsigned MNG = 2   // number of generator   modules
)(
  // PS connections
  inout  logic [54-1:0] FIXED_IO_mio     ,
  inout  logic          FIXED_IO_ps_clk  ,
  inout  logic          FIXED_IO_ps_porb ,
  inout  logic          FIXED_IO_ps_srstb,
  inout  logic          FIXED_IO_ddr_vrn ,
  inout  logic          FIXED_IO_ddr_vrp ,
  // DDR
  inout  logic [15-1:0] DDR_addr   ,
  inout  logic [ 3-1:0] DDR_ba     ,
  inout  logic          DDR_cas_n  ,
  inout  logic          DDR_ck_n   ,
  inout  logic          DDR_ck_p   ,
  inout  logic          DDR_cke    ,
  inout  logic          DDR_cs_n   ,
  inout  logic [ 4-1:0] DDR_dm     ,
  inout  logic [32-1:0] DDR_dq     ,
  inout  logic [ 4-1:0] DDR_dqs_n  ,
  inout  logic [ 4-1:0] DDR_dqs_p  ,
  inout  logic          DDR_odt    ,
  inout  logic          DDR_ras_n  ,
  inout  logic          DDR_reset_n,
  inout  logic          DDR_we_n   ,

  // Red Pitaya periphery
  input  logic          trig_i ,
  input  logic          pll_ref_i,
  output logic          pll_hi_o,
  output logic          pll_lo_o,
  input  logic [ 2-1:0] temp_prot_i ,
  // ADC
  input  logic           [14-1:0] cnter_in,  // ADC data

  input  logic [MNA-1:0] [ 7-1:0] adc_dat_p_i,  // ADC data
  input  logic [MNA-1:0] [ 7-1:0] adc_dat_n_i,  // ADC data
  input  logic           [ 2-1:0] adc_clk_i,  // ADC clock {p,n}
  output logic                    adc_spi_csb, // ADC spi CS
  inout  logic                    adc_spi_sdio, // ADC spi DIO
  output logic                    adc_spi_clk, // ADC spi CLK
  output logic                    adc_sync_o, // ADC sync
  // DAC
  output logic [1:0][14-1:0] dac_dat_o,  // DAC combined data
  input  logic               dac_dco_i,  // DAC clock
  output logic               dac_reset_o,  // DAC reset
  output logic               dac_spi_csb, // DAC spi CS
  inout  logic               dac_spi_sdio, // DAC spi DIO
  output logic               dac_spi_clk, // DAC spi CLK
  // PWM DAC
  output logic [ 4-1:0] dac_pwm_o  ,  // 1-bit PWM DAC
  // XADC
  input  logic [ 5-1:0] vinp_i     ,  // voltages p
  input  logic [ 5-1:0] vinn_i     ,  // voltages n
  // Expansion connector
  inout  logic          exp_9_io   ,
  inout  logic [ 9-1:0] exp_p_io   ,
  inout  logic [ 9-1:0] exp_n_io   ,
  // SATA connector
  output logic [ 2-1:0] daisy_p_o  ,  // line 1 is clock capable
  output logic [ 2-1:0] daisy_n_o  ,
  input  logic [ 2-1:0] daisy_p_i  ,  // line 1 is clock capable
  input  logic [ 2-1:0] daisy_n_i  ,

  input                 rst_in,
  output                rstn_out,
  output                clkout_125,
  output                clkout_625,


  output [31:0]M_AXI_OSC_araddr,
  output [1:0]M_AXI_OSC_arburst,
  output [3:0]M_AXI_OSC_arcache,
  output [0:0]M_AXI_OSC_arid,
  output [3:0]M_AXI_OSC_arlen,
  output [1:0]M_AXI_OSC_arlock,
  output [2:0]M_AXI_OSC_arprot,
  output [3:0]M_AXI_OSC_arqos,
  input M_AXI_OSC_arready,
  output [2:0]M_AXI_OSC_arsize,
  output M_AXI_OSC_arvalid,
  output [31:0]M_AXI_OSC_awaddr,
  output [1:0]M_AXI_OSC_awburst,
  output [3:0]M_AXI_OSC_awcache,
  output [0:0]M_AXI_OSC_awid,
  output [3:0]M_AXI_OSC_awlen,
  output [1:0]M_AXI_OSC_awlock,
  output [2:0]M_AXI_OSC_awprot,
  output [3:0]M_AXI_OSC_awqos,
  input M_AXI_OSC_awready,
  output [2:0]M_AXI_OSC_awsize,
  output M_AXI_OSC_awvalid,
  input [0:0]M_AXI_OSC_bid,
  output M_AXI_OSC_bready,
  input [1:0]M_AXI_OSC_bresp,
  input M_AXI_OSC_bvalid,
  input [63:0]M_AXI_OSC_rdata,
  input [0:0]M_AXI_OSC_rid,
  input M_AXI_OSC_rlast,
  output M_AXI_OSC_rready,
  input [1:0]M_AXI_OSC_rresp,
  input M_AXI_OSC_rvalid,
  output [63:0]M_AXI_OSC_wdata,
  output [0:0]M_AXI_OSC_wid,
  output M_AXI_OSC_wlast,
  input M_AXI_OSC_wready,
  output [7:0]M_AXI_OSC_wstrb,
  output M_AXI_OSC_wvalid,
  
  input [31:0]S_AXI_REG_araddr,
  input [1:0]S_AXI_REG_arburst,
  input [3:0]S_AXI_REG_arcache,
  input [11:0]S_AXI_REG_arid,
  input [3:0]S_AXI_REG_arlen,
  input [1:0]S_AXI_REG_arlock,
  input [2:0]S_AXI_REG_arprot,
  input [3:0]S_AXI_REG_arqos,
  output [0:0]S_AXI_REG_arready,
  input [2:0]S_AXI_REG_arsize,
  input [0:0]S_AXI_REG_arvalid,
  input [31:0]S_AXI_REG_awaddr,
  input [1:0]S_AXI_REG_awburst,
  input [3:0]S_AXI_REG_awcache,
  input [11:0]S_AXI_REG_awid,
  input [3:0]S_AXI_REG_awlen,
  input [1:0]S_AXI_REG_awlock,
  input [2:0]S_AXI_REG_awprot,
  input [3:0]S_AXI_REG_awqos,
  output [0:0]S_AXI_REG_awready,
  input [2:0]S_AXI_REG_awsize,
  input [0:0]S_AXI_REG_awvalid,
  output [11:0]S_AXI_REG_bid,
  input [0:0]S_AXI_REG_bready,
  output [1:0]S_AXI_REG_bresp,
  output [0:0]S_AXI_REG_bvalid,
  output [31:0]S_AXI_REG_rdata,
  output [11:0]S_AXI_REG_rid,
  output [0:0]S_AXI_REG_rlast,
  input [0:0]S_AXI_REG_rready,
  output [1:0]S_AXI_REG_rresp,
  output [0:0]S_AXI_REG_rvalid,
  input [31:0]S_AXI_REG_wdata,
  input [11:0]S_AXI_REG_wid,
  input [0:0]S_AXI_REG_wlast,
  output [0:0]S_AXI_REG_wready,
  input [3:0]S_AXI_REG_wstrb,
  input [0:0]S_AXI_REG_wvalid,

  // LED
  output logic [ 8-1:0] led_o
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// PLL signals
logic                 adc_clk_in;
logic                 pll_adc_clk;
logic                 pll_adc_clk2d;
logic                 pll_adc_10mhz;
logic                 pll_ser_clk;
logic                 pll_pwm_clk;
logic                 pll_locked;
// fast serial signals
logic                 ser_clk ;
// PWM clock and reset
logic                 pwm_clk ;
logic                 pwm_rstn;

// ADC clock/reset
logic                 adc_clk;
logic                 adc_clk2d;
logic                 adc_10mhz;
logic                 adc_rstn;

logic [4-1:0] fclk ; //[0]-125MHz, [1]-250MHz, [2]-50MHz, [3]-200MHz
logic [4-1:0] frstn;

// GPIO interface
gpio_if #(.DW (24)) gpio ();

wire [2-1:0] hk_spi_cs  ;
wire [2-1:0] hk_spi_clk ;
wire [2-1:0] hk_spi_i   ;
wire [2-1:0] hk_spi_o   ;
wire [2-1:0] hk_spi_t   ;

assign adc_spi_csb  = hk_spi_cs[0];
assign adc_spi_clk  = hk_spi_clk[0];
assign hk_spi_i[0]  = adc_spi_sdio;
assign adc_spi_sdio = hk_spi_t[0] ? 1'bz : hk_spi_o[0] ;

//assign dac_spi_csb  = hk_spi_cs[1];
//assign dac_spi_clk  = hk_spi_clk[1];
assign hk_spi_i[1]  = dac_spi_sdio;
assign dac_spi_sdio = hk_spi_t[1] ? 1'bz : hk_spi_o[1] ;

////////////////////////////////////////////////////////////////////////////////
// PLL
////////////////////////////////////////////////////////////////////////////////

// diferential clock input
IBUFDS i_clk (.I (adc_clk_i[1]), .IB (adc_clk_i[0]), .O (adc_clk_in));  // differential clock input

red_pitaya_pll pll (
  // inputs
  .clk         (adc_clk_in),  // clock
  .rstn        (frstn[0]  ),  // reset - active low
  // output clocks
  .clk_adc     (pll_adc_clk   ),  // ADC clock
  .clk_adc2d   (pll_adc_clk2d ),  // ADC clock divided by 2
  .clk_10mhz   (pll_adc_10mhz ),  // ADC divided to 10MHz
  .clk_ser     (pll_ser_clk   ),  // fast serial clock
  .clk_pdm     (pll_pwm_clk   ),  // PWM clock
  // status outputs
  .pll_locked  (pll_locked)
);

BUFG bufg_adc_clk    (.O (adc_clk   ), .I (pll_adc_clk   ));
BUFG bufg_adc_clk2d  (.O (adc_clk2d ), .I (pll_adc_clk2d ));
BUFG bufg_adc_10MHz  (.O (adc_10mhz ), .I (pll_adc_10mhz ));
BUFG bufg_ser_clk    (.O (ser_clk   ), .I (pll_ser_clk   ));
BUFG bufg_pwm_clk    (.O (pwm_clk   ), .I (pll_pwm_clk   ));

// ADC reset (active low)
always @(posedge adc_clk2d)
adc_rstn <=  frstn[0] &  pll_locked & idly_rdy;

//assign clkout_125 = adc_clk2d;
assign frstn = {~rst_in,~rst_in,~rst_in,~rst_in};
////////////////////////////////////////////////////////////////////////////////
// ADC IO
////////////////////////////////////////////////////////////////////////////////

// ADC sync is yet to drive
assign adc_sync_o = 1'bz ;

logic [2-1:0] [ 7-1:0] adc_dat_ibuf;
logic [2-1:0] [ 7-1:0] adc_dat_idly;
logic [2-1:0] [14-1:0] adc_dat_in;
logic [2-1:0] [14-1:0] adc_dat_sw;


genvar GV;
generate
for(GV = 0 ; GV < 7 ; GV = GV +1)
begin:adc_ipad
  IBUFDS i_dat0 (.I (adc_dat_p_i[0][GV]), .IB (adc_dat_n_i[0][GV]), .O (adc_dat_ibuf[0][GV]));  // differential data input
  IBUFDS i_dat1 (.I (adc_dat_p_i[1][GV]), .IB (adc_dat_n_i[1][GV]), .O (adc_dat_ibuf[1][GV]));  // differential data input
end
endgenerate


logic [2*7-1:0] idly_rst ;
logic [2*7-1:0] idly_ce  ;
logic [2*7-1:0] idly_inc ;
logic [2*7-1:0] [5-1:0] idly_cnt ;

IDELAYCTRL i_idelayctrl (
  .RDY(idly_rdy),   // 1-bit output: Ready output
  .REFCLK(fclk[3]), // 1-bit input: Reference clock input
  .RST(!frstn[3])   // 1-bit input: Active high reset input
);

generate
for(GV = 0 ; GV < 7 ; GV = GV +1)
begin:adc_idly

   //(* IODELAY_GROUP = adc_inputs *)
   IDELAYE2 #(
      .DELAY_SRC("IDATAIN"),           // Delay input (IDATAIN, DATAIN)
      .HIGH_PERFORMANCE_MODE("TRUE"),  // Reduced jitter ("TRUE"), Reduced power ("FALSE")
      .IDELAY_TYPE("VARIABLE"),        // FIXED, VARIABLE, VAR_LOAD, VAR_LOAD_PIPE
      .IDELAY_VALUE(0),                // Input delay tap setting (0-31)
      .PIPE_SEL("FALSE"),              // Select pipelined mode, FALSE, TRUE
      .REFCLK_FREQUENCY(200.0),        // IDELAYCTRL clock input frequency in MHz (190.0-210.0, 290.0-310.0).
      .SIGNAL_PATTERN("DATA")          // DATA, CLOCK input signal
   )
   i_dlya (
      .CNTVALUEOUT  ( idly_cnt[GV]          ),  // 5-bit output: Counter value output
      .DATAOUT      ( adc_dat_idly[0][GV]   ),  // 1-bit output: Delayed data output
      .C            ( adc_clk2d             ),  // 1-bit input: Clock input
      .CE           ( idly_ce[GV]           ),  // 1-bit input: Active high enable increment/decrement input
      .CINVCTRL     ( 1'b0                  ),  // 1-bit input: Dynamic clock inversion input
      .CNTVALUEIN   ( 5'h0                  ),  // 5-bit input: Counter value input
      .DATAIN       ( 1'b0                  ),  // 1-bit input: Internal delay data input
      .IDATAIN      ( adc_dat_ibuf[0][GV]   ),  // 1-bit input: Data input from the I/O
      .INC          ( idly_inc[GV]          ),  // 1-bit input: Increment / Decrement tap delay input
      .LD           ( idly_rst[GV]          ),  // 1-bit input: Load IDELAY_VALUE input
      .LDPIPEEN     ( 1'b0                  ),  // 1-bit input: Enable PIPELINE register to load data input
      .REGRST       ( 1'b0                  )   // 1-bit input: Active-high reset tap-delay input
   );

   //(* IODELAY_GROUP = adc_inputs *)
   IDELAYE2 #(
      .DELAY_SRC("IDATAIN"),           // Delay input (IDATAIN, DATAIN)
      .HIGH_PERFORMANCE_MODE("TRUE"),  // Reduced jitter ("TRUE"), Reduced power ("FALSE")
      .IDELAY_TYPE("VARIABLE"),        // FIXED, VARIABLE, VAR_LOAD, VAR_LOAD_PIPE
      .IDELAY_VALUE(0),                // Input delay tap setting (0-31)
      .PIPE_SEL("FALSE"),              // Select pipelined mode, FALSE, TRUE
      .REFCLK_FREQUENCY(200.0),        // IDELAYCTRL clock input frequency in MHz (190.0-210.0, 290.0-310.0).
      .SIGNAL_PATTERN("DATA")          // DATA, CLOCK input signal
   )
   i_dlyb (
      .CNTVALUEOUT  ( idly_cnt[GV+7]        ),  // 5-bit output: Counter value output
      .DATAOUT      ( adc_dat_idly[1][GV]   ),  // 1-bit output: Delayed data output
      .C            ( adc_clk2d             ),  // 1-bit input: Clock input
      .CE           ( idly_ce[GV+7]         ),  // 1-bit input: Active high enable increment/decrement input
      .CINVCTRL     ( 1'b0                  ),  // 1-bit input: Dynamic clock inversion input
      .CNTVALUEIN   ( 5'h0                  ),  // 5-bit input: Counter value input
      .DATAIN       ( 1'b0                  ),  // 1-bit input: Internal delay data input
      .IDATAIN      ( adc_dat_ibuf[1][GV]   ),  // 1-bit input: Data input from the I/O
      .INC          ( idly_inc[GV+7]        ),  // 1-bit input: Increment / Decrement tap delay input
      .LD           ( idly_rst[GV+7]        ),  // 1-bit input: Load IDELAY_VALUE input
      .LDPIPEEN     ( 1'b0                  ),  // 1-bit input: Enable PIPELINE register to load data input
      .REGRST       ( 1'b0                  )   // 1-bit input: Active-high reset tap-delay input
   );
end
endgenerate


generate
for(GV = 0 ; GV < 14 ; GV = GV +2)
begin:adc_iddr
   IDDR #(.DDR_CLK_EDGE("SAME_EDGE_PIPELINED")) 
     i_ddr0 (.Q1(adc_dat_in[0][GV]), .Q2(adc_dat_in[0][GV+1]), .C(adc_clk), .CE(1'b1), .D(adc_dat_ibuf[0][GV/2]), .R(1'b0), .S(1'b0) );
   IDDR #(.DDR_CLK_EDGE("SAME_EDGE_PIPELINED")) 
     i_ddr1 (.Q1(adc_dat_in[1][GV]), .Q2(adc_dat_in[1][GV+1]), .C(adc_clk), .CE(1'b1), .D(adc_dat_ibuf[1][GV/2]), .R(1'b0), .S(1'b0) );
end
endgenerate

// system bus
sys_bus_if   ps_sys      (.clk (adc_clk2d), .rstn (adc_rstn));
sys_bus_if   sys [8-1:0] (.clk (adc_clk2d), .rstn (adc_rstn));

// silence unused busses
generate
for (genvar i=1; i<8; i++) begin: for_sys
  sys_bus_stub sys_bus_stub_1_7 (sys[i]);
end: for_sys
endgenerate

axi4_if #(.DW (32), .AW (32), .IW (12), .LW (4)) axi_gp (.ACLK (ps_sys.clk), .ARESETn (ps_sys.rstn));

axi4_slave #(
  .DW (32),
  .AW (32),
  .IW (12)
) axi_slave_gp0 (
  // AXI bus
  .axi       (axi_gp),
  // system read/write channel
  .bus       (ps_sys)
);

sys_bus_interconnect #(
  .SN (8),
  .SW (20)
) sys_bus_interconnect (
  .bus_m (ps_sys),
  .bus_s (sys)
);
reg [13:0]        cnter; // ta signal gre na vhod A
reg [14:0]        oflw;

always @(posedge adc_clk) begin
    if (adc_rstn==0)
        oflw <= 16'b0;
    else if (oflw==16'h3FFF)
        oflw <= 16'b0;
    else
        oflw <= oflw + 16'd8; 
    
    
    if (adc_rstn==0)
        cnter <= 16'b0;
    else if (cnter==14'h1FFF)
        cnter <= 16'b0;
    else
        cnter <= cnter + 16'd8; 
        

 /*   if (adc_rstn==0)
        cnter <= 14'b0;
    else if (~oflw[14] && oflw > 15'h1FFF)
        cnter <= 14'h1FFF;
    else if (oflw[14] && oflw < 15'h5FFF)
        cnter <= 14'h2000;
    else
        cnter <= oflw[13:0];*/
end
// data loopback
always @(posedge adc_clk)
begin
  adc_dat_sw[0] <= { adc_dat_in[1][14-1:2] , 2'h0 }; // switch adc_b->ch_a
  adc_dat_sw[1] <= { adc_dat_in[0][14-1:2] , 2'h0 }; // switch adc_a->ch_b
end

////////////////////////////////////////////////////////////////////////////////
//  House Keeping
////////////////////////////////////////////////////////////////////////////////

logic [  9-1: 0] exp_p_in , exp_n_in ;
logic [  9-1: 0] exp_p_out, exp_n_out;
logic [  9-1: 0] exp_p_dir, exp_n_dir;
logic            exp_9_in,  exp_9_out, exp_9_dir;
logic [  8-1: 0] led_hk;

red_pitaya_hk #(.DWE(10))

i_hk (
  // system signals
  .clk_i           (adc_clk2d),  // clock
  .rstn_i          (adc_rstn),  // reset - active low
  // LED
  .led_o           (led_hk),  // LED output
  // idelay control
  .idly_rst_o      (idly_rst    ),
  .idly_ce_o       (idly_ce     ),
  .idly_inc_o      (idly_inc    ),
  .idly_cnt_i      ({idly_cnt[7],idly_cnt[0]}),
  // global configuration
  .digital_loop    (),
  .pll_sys_i       (adc_10mhz   ),    // system clock
  .pll_ref_i       (pll_ref_i   ),    // reference clock
  .pll_hi_o        (pll_hi_o    ),    // PLL high
  .pll_lo_o        (pll_lo_o    ),    // PLL low
  // SPI
  .spi_cs_o        (hk_spi_cs   ),
  .spi_clk_o       (hk_spi_clk  ),
  .spi_miso_i      (hk_spi_i    ),
  .spi_mosi_t      (hk_spi_t    ),
  .spi_mosi_o      (hk_spi_o    ),
  // Expansion connector
  .exp_p_dat_i     ({exp_9_in, exp_p_in }),  // input data
  .exp_p_dat_o     ({exp_9_out,exp_p_out}),  // output data
  .exp_p_dir_o     ({exp_9_dir,exp_p_dir}),  // 1-output enable
  .exp_n_dat_i     ({1'b0,     exp_n_in }),
  .exp_n_dat_o     (           exp_n_out ),
  .exp_n_dir_o     (           exp_n_dir ),
   // System bus
  .sys_addr        (sys[0].addr ),
  .sys_wdata       (sys[0].wdata),
  .sys_wen         (sys[0].wen  ),
  .sys_ren         (sys[0].ren  ),
  .sys_rdata       (sys[0].rdata),
  .sys_err         (sys[0].err  ),
  .sys_ack         (sys[0].ack  )
);

////////////////////////////////////////////////////////////////////////////////
// LED
////////////////////////////////////////////////////////////////////////////////

assign led_o = led_hk[7:0];


////////////////////////////////////////////////////////////////////////////////
// GPIO
////////////////////////////////////////////////////////////////////////////////

IOBUF i_iobufp [ 9-1:0] (.O(exp_p_in), .IO(exp_p_io), .I(exp_p_out), .T(~exp_p_dir) );
IOBUF i_iobufn [ 9-1:0] (.O(exp_n_in), .IO(exp_n_io), .I(exp_n_out), .T(~exp_n_dir) );
IOBUF i_iobuf9          (.O(exp_9_in), .IO(exp_9_io), .I(exp_9_out), .T(~exp_9_dir) );

assign gpio.i[15: 8] = exp_p_in[7:0];
assign gpio.i[23:16] = exp_n_in[7:0];


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

        .adc_data_ch1(cnter[13:(14-ADC_DATA_BITS)]),
        .adc_data_ch2(adc_dat_sw[1][13:(14-ADC_DATA_BITS)]),

        .m_axi_hk_arvalid (axi_gp.ARVALID),
        .m_axi_hk_awvalid (axi_gp.AWVALID),
        .m_axi_hk_bready  (axi_gp.BREADY ),
        .m_axi_hk_rready  (axi_gp.RREADY ),
        .m_axi_hk_wlast   (axi_gp.WLAST  ),
        .m_axi_hk_wvalid  (axi_gp.WVALID ),
        .m_axi_hk_arid    (axi_gp.ARID   ),
        .m_axi_hk_awid    (axi_gp.AWID   ),
        .m_axi_hk_wid     (axi_gp.WID    ),
        .m_axi_hk_arburst (axi_gp.ARBURST),
        .m_axi_hk_arlock  (axi_gp.ARLOCK ),
        .m_axi_hk_arsize  (axi_gp.ARSIZE ),
        .m_axi_hk_awburst (axi_gp.AWBURST),
        .m_axi_hk_awlock  (axi_gp.AWLOCK ),
        .m_axi_hk_awsize  (axi_gp.AWSIZE ),
        .m_axi_hk_arprot  (axi_gp.ARPROT ),
        .m_axi_hk_awprot  (axi_gp.AWPROT ),
        .m_axi_hk_araddr  (axi_gp.ARADDR ),
        .m_axi_hk_awaddr  (axi_gp.AWADDR ),
        .m_axi_hk_wdata   (axi_gp.WDATA  ),
        .m_axi_hk_arcache (axi_gp.ARCACHE),
        .m_axi_hk_arlen   (axi_gp.ARLEN  ),
        .m_axi_hk_arqos   (axi_gp.ARQOS  ),
        .m_axi_hk_awcache (axi_gp.AWCACHE),
        .m_axi_hk_awlen   (axi_gp.AWLEN  ),
        .m_axi_hk_awqos   (axi_gp.AWQOS  ),
        .m_axi_hk_wstrb   (axi_gp.WSTRB  ),
        .m_axi_hk_arready (axi_gp.ARREADY),
        .m_axi_hk_awready (axi_gp.AWREADY),
        .m_axi_hk_bvalid  (axi_gp.BVALID ),
        .m_axi_hk_rlast   (axi_gp.RLAST  ),
        .m_axi_hk_rvalid  (axi_gp.RVALID ),
        .m_axi_hk_wready  (axi_gp.WREADY ),
        .m_axi_hk_bid     (axi_gp.BID    ),
        .m_axi_hk_rid     (axi_gp.RID    ),
        .m_axi_hk_bresp   (axi_gp.BRESP  ),
        .m_axi_hk_rresp   (axi_gp.RRESP  ),
        .m_axi_hk_rdata   (axi_gp.RDATA  ),

        .clkin_125(adc_clk2d),
        .clkin_250(adc_clk),
        .clkout_625(clkout_625),
        .clkout_125(clkout_125),


        .fclk_clk0(fclk[0]),
        .fclk_clk1(fclk[1]),
        .fclk_clk2(fclk[2]),
        .fclk_clk3(fclk[3]),
        /*.frstn_0(frstn[0]),
        .frstn_1(frstn[1]),
        .frstn_2(frstn[2]),
        .frstn_3(frstn[3]),    */    

        .rst_in(rst_in),
        .rstn_out(rstn_out));

assign axi_gp.AWREGION = '0;
assign axi_gp.ARREGION = '0;

endmodule