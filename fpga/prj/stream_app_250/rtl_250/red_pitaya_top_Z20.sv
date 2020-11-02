////////////////////////////////////////////////////////////////////////////////
// Red Pitaya TOP module. It connects external pins and PS part with
// other application modules.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

/**
 * GENERAL DESCRIPTION:
 *
 * Top module connects PS part with rest of Red Pitaya applications.  
 *
 *                   /-------\      
 *   PS DDR <------> |  PS   |      AXI <-> custom bus
 *   PS MIO <------> |   /   | <------------+
 *   PS CLK -------> |  ARM  |              |
 *                   \-------/              |
 *                                          |
 *                            /-------\     |
 *                         -> | SCOPE | <---+
 *                         |  \-------/     |
 *                         |                |
 *            /--------\   |   /-----\      |
 *   ADC ---> |        | --+-> |     |      |
 *            | ANALOG |       | PID | <----+
 *   DAC <--- |        | <---- |     |      |
 *            \--------/   ^   \-----/      |
 *                         |                |
 *                         |  /-------\     |
 *                         -- |  ASG  | <---+ 
 *                            \-------/     |
 *                                          |
 *             /--------\                   |
 *    RX ----> |        |                   |
 *   SATA      | DAISY  | <-----------------+
 *    TX <---- |        | 
 *             \--------/ 
 *               |    |
 *               |    |
 *               (FREE)
 *
 * Inside analog module, ADC data is translated from unsigned neg-slope into
 * two's complement. Similar is done on DAC data.
 *
 * Scope module stores data from ADC into RAM, arbitrary signal generator (ASG)
 * sends data from RAM to DAC. MIMO PID uses ADC ADC as input and DAC as its output.
 *
 * Daisy chain connects with other boards with fast serial link. Data which is
 * send and received is at the moment undefined. This is left for the user.
 */

module red_pitaya_top_Z20 #(
  // identification
  bit [0:5*32-1] GITH = '0,
  // module numbers
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
  // LED
  output logic [ 8-1:0] led_o
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

wire                  clk_125;
wire                  clk_10;  
wire                  rstn_hk;

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

assign dac_spi_csb  = hk_spi_cs[1];
assign dac_spi_clk  = hk_spi_clk[1];
assign hk_spi_i[1]  = dac_spi_sdio;
assign dac_spi_sdio = hk_spi_t[1] ? 1'bz : hk_spi_o[1] ;
////////////////////////////////////////////////////////////////////////////////
// ADC IO
////////////////////////////////////////////////////////////////////////////////

// ADC sync is yet to drive
assign adc_sync_o = 1'bz ;

logic [2-1:0] [ 7-1:0] adc_dat_ibuf;
logic [2-1:0] [ 7-1:0] adc_dat_idly;
logic [2-1:0] [14-1:0] adc_dat_in;

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
      .C            ( clk_125               ),  // 1-bit input: Clock input
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
      .C            ( clk_125               ),  // 1-bit input: Clock input
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
     i_ddr0 (.Q1(adc_dat_in[0][GV]), .Q2(adc_dat_in[0][GV+1]), .C(adc_clk), .CE(1'b1), .D(adc_dat_idly[0][GV/2]), .R(1'b0), .S(1'b0) );
   IDDR #(.DDR_CLK_EDGE("SAME_EDGE_PIPELINED")) 
     i_ddr1 (.Q1(adc_dat_in[1][GV]), .Q2(adc_dat_in[1][GV+1]), .C(adc_clk), .CE(1'b1), .D(adc_dat_idly[1][GV/2]), .R(1'b0), .S(1'b0) );
end
endgenerate

// system bus
sys_bus_if   ps_sys      (.clk (clk_125), .rstn (rstn_hk));
sys_bus_if   sys         (.clk (clk_125), .rstn (rstn_hk));

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
  .SN (1),
  .SW (20)
) sys_bus_interconnect (
  .bus_m (ps_sys),
  .bus_s (sys)
);

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
  .clk_i           (clk_125),  // clock
  .rstn_i          (rstn_hk),  // reset - active low
  // LED
  .led_o           (led_hk),  // LED output
  // idelay control
  .idly_rst_o      (idly_rst    ),
  .idly_ce_o       (idly_ce     ),
  .idly_inc_o      (idly_inc    ),
  .idly_cnt_i      ({idly_cnt[7],idly_cnt[0]}),
  // global configuration
  .digital_loop    (),
  .pll_sys_i       (clk_10      ),    // system clock
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
  .sys_addr        (sys.addr ),
  .sys_wdata       (sys.wdata),
  .sys_wen         (sys.wen  ),
  .sys_ren         (sys.ren  ),
  .sys_rdata       (sys.rdata),
  .sys_err         (sys.err  ),
  .sys_ack         (sys.ack  )
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
        .m_axi_hk_ACLK    (axi_gp.ACLK   ),
        //.m_axi_hk_ARESETn (axi_gp.ARESETn),
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
        .clk_125(clk_125),
        .clk_10(clk_10),
        .rstn_hk(rstn_hk),
        .adc_clk_n(adc_clk_i[0]),
        .adc_clk_p(adc_clk_i[1]),
        .adc_data_ch1(adc_dat_in[0]),
        .adc_data_ch2(adc_dat_in[1]));

assign axi_gp.AWREGION = '0;
assign axi_gp.ARREGION = '0;


endmodule: red_pitaya_top_Z20
