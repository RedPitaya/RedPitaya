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

module red_pitaya_top #(
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

// GPIO parameter
localparam int unsigned GDW = 8+8;

logic [4-1:0] fclk ; //[0]-125MHz, [1]-250MHz, [2]-50MHz, [3]-200MHz
logic [4-1:0] frstn;
logic         idly_rdy;

// AXI masters
logic            axi1_clk    , axi0_clk    ;
logic            axi1_rstn   , axi0_rstn   ;
logic [ 32-1: 0] axi1_waddr  , axi0_waddr  ;
logic [ 64-1: 0] axi1_wdata  , axi0_wdata  ;
logic [  8-1: 0] axi1_wsel   , axi0_wsel   ;
logic            axi1_wvalid , axi0_wvalid ;
logic [  4-1: 0] axi1_wlen   , axi0_wlen   ;
logic            axi1_wfixed , axi0_wfixed ;
logic            axi1_werr   , axi0_werr   ;
logic            axi1_wrdy   , axi0_wrdy   ;

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

// stream bus type
localparam type SBA_T = logic signed [14-1:0];  // acquire
localparam type SBG_T = logic signed [14-1:0];  // generate


// DAC signals
logic        [14-1:0] dac_dat_a, dac_dat_b;
logic        [14-1:0] dac_a    , dac_b    ;
logic signed [15-1:0] dac_a_sum, dac_b_sum;

// ASG
SBG_T [2-1:0]            asg_dat;

// PID
SBA_T [2-1:0]            pid_dat;

// configuration
logic                    digital_loop;

// system bus
sys_bus_if   ps_sys      (.clk (adc_clk2d), .rstn (adc_rstn));
sys_bus_if   sys [8-1:0] (.clk (adc_clk2d), .rstn (adc_rstn));

// GPIO interface
gpio_if #(.DW (24)) gpio ();

// SPI0
spi_if #(.DW (2)) spi0 ();

////////////////////////////////////////////////////////////////////////////////
// PLL (clock and reset)
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

// PWM reset (active low)
always @(posedge pwm_clk)
pwm_rstn <=  frstn[0] &  pll_locked & idly_rdy;

////////////////////////////////////////////////////////////////////////////////
//  Connections to PS
////////////////////////////////////////////////////////////////////////////////

red_pitaya_ps ps (
  .FIXED_IO_mio       (  FIXED_IO_mio                ),
  .FIXED_IO_ps_clk    (  FIXED_IO_ps_clk             ),
  .FIXED_IO_ps_porb   (  FIXED_IO_ps_porb            ),
  .FIXED_IO_ps_srstb  (  FIXED_IO_ps_srstb           ),
  .FIXED_IO_ddr_vrn   (  FIXED_IO_ddr_vrn            ),
  .FIXED_IO_ddr_vrp   (  FIXED_IO_ddr_vrp            ),
  // DDR
  .DDR_addr      (DDR_addr    ),
  .DDR_ba        (DDR_ba      ),
  .DDR_cas_n     (DDR_cas_n   ),
  .DDR_ck_n      (DDR_ck_n    ),
  .DDR_ck_p      (DDR_ck_p    ),
  .DDR_cke       (DDR_cke     ),
  .DDR_cs_n      (DDR_cs_n    ),
  .DDR_dm        (DDR_dm      ),
  .DDR_dq        (DDR_dq      ),
  .DDR_dqs_n     (DDR_dqs_n   ),
  .DDR_dqs_p     (DDR_dqs_p   ),
  .DDR_odt       (DDR_odt     ),
  .DDR_ras_n     (DDR_ras_n   ),
  .DDR_reset_n   (DDR_reset_n ),
  .DDR_we_n      (DDR_we_n    ),
  // system signals
  .fclk_clk_o    (fclk        ),
  .fclk_rstn_o   (frstn       ),
  // ADC analog inputs
  .vinp_i        (vinp_i      ),
  .vinn_i        (vinn_i      ),
  // GPIO
  .gpio          (gpio),
  // SPI0
  .spi0          (spi0),
  // system read/write channel
  .bus           (ps_sys      ),
  // AXI masters
  .axi1_clk_i    (axi1_clk    ),  .axi0_clk_i    (axi0_clk    ),  // global clock
  .axi1_rstn_i   (axi1_rstn   ),  .axi0_rstn_i   (axi0_rstn   ),  // global reset
  .axi1_waddr_i  (axi1_waddr  ),  .axi0_waddr_i  (axi0_waddr  ),  // system write address
  .axi1_wdata_i  (axi1_wdata  ),  .axi0_wdata_i  (axi0_wdata  ),  // system write data
  .axi1_wsel_i   (axi1_wsel   ),  .axi0_wsel_i   (axi0_wsel   ),  // system write byte select
  .axi1_wvalid_i (axi1_wvalid ),  .axi0_wvalid_i (axi0_wvalid ),  // system write data valid
  .axi1_wlen_i   (axi1_wlen   ),  .axi0_wlen_i   (axi0_wlen   ),  // system write burst length
  .axi1_wfixed_i (axi1_wfixed ),  .axi0_wfixed_i (axi0_wfixed ),  // system write burst type (fixed / incremental)
  .axi1_werr_o   (axi1_werr   ),  .axi0_werr_o   (axi0_werr   ),  // system write error
  .axi1_wrdy_o   (axi1_wrdy   ),  .axi0_wrdy_o   (axi0_wrdy   )   // system write ready
);



assign spi0.sck_i   = 1'b0 ;
assign spi0.ss_i    = 1'b1 ;

//reg spi0_in ;
//always @(*) begin
//  case ({spi0.ss2_o, spi0.ss1_o})
//    2'b10 : spi0_in = adc_spi_sdio ;
//    2'b01 : spi0_in = dac_spi_sdio ;
//   default: spi0_in = 1'b0 ;
//  endcase
//end

//assign adc_spi_csb   = spi0.ss1_o;
//assign adc_spi_clk   = spi0.sck_o;
//assign adc_spi_sdio  = spi0.io_t[0] ? 1'bz : spi0.io_o[0]; // MOSI <=> IO[0]
//assign dac_spi_csb   = spi0.ss2_o;
//assign dac_spi_clk   = spi0.sck_o;
//assign dac_spi_sdio  = spi0.io_t[0] ? 1'bz : spi0.io_o[0]; // MOSI <=> IO[0]

//assign spi0.io_i[1]  = spi0_in ; // MISO <=> IO[1]
//assign spi0.io_i[0]  = spi0_in ;


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
// system bus decoder & multiplexer (it breaks memory addresses into 8 regions)
////////////////////////////////////////////////////////////////////////////////

sys_bus_interconnect #(
  .SN (8),
  .SW (20)
) sys_bus_interconnect (
  .bus_m (ps_sys),
  .bus_s (sys)
);

// silence unused busses
generate
for (genvar i=6; i<8; i++) begin: for_sys
  sys_bus_stub sys_bus_stub_6_7 (sys[i]);
end: for_sys
endgenerate

////////////////////////////////////////////////////////////////////////////////
// Analog mixed signals (PDM analog outputs)
////////////////////////////////////////////////////////////////////////////////

logic [4-1:0] [24-1:0] pwm_cfg;

red_pitaya_ams i_ams (
  // power test
  .clk_i           (adc_clk2d),  // clock
  .rstn_i          (adc_rstn),  // reset - active low
  // PWM configuration
  .dac_a_o         (pwm_cfg[0]),
  .dac_b_o         (pwm_cfg[1]),
  .dac_c_o         (pwm_cfg[2]),
  .dac_d_o         (pwm_cfg[3]),
  // System bus
  .sys_addr        (sys[4].addr ),
  .sys_wdata       (sys[4].wdata),
  .sys_wen         (sys[4].wen  ),
  .sys_ren         (sys[4].ren  ),
  .sys_rdata       (sys[4].rdata),
  .sys_err         (sys[4].err  ),
  .sys_ack         (sys[4].ack  )
);

red_pitaya_pwm pwm [4-1:0] (
  // system signals
  .clk   (pwm_clk ),
  .rstn  (pwm_rstn),
  // configuration
  .cfg   (pwm_cfg),
  // PWM outputs
  .pwm_o (dac_pwm_o),
  .pwm_s ()
);



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


// delay input ADC signals
//(* IODELAY_GROUP = adc_inputs *) // Specifies group name for associated IDELAYs/ODELAYs and IDELAYCTRL
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
     i_ddr0 (.Q1(adc_dat_in[0][GV]), .Q2(adc_dat_in[0][GV+1]), .C(adc_clk), .CE(1'b1), .D(adc_dat_idly[0][GV/2]), .R(1'b0), .S(1'b0) );
   IDDR #(.DDR_CLK_EDGE("SAME_EDGE_PIPELINED")) 
     i_ddr1 (.Q1(adc_dat_in[1][GV]), .Q2(adc_dat_in[1][GV+1]), .C(adc_clk), .CE(1'b1), .D(adc_dat_idly[1][GV/2]), .R(1'b0), .S(1'b0) );
end
endgenerate



    
// data loopback
always @(posedge adc_clk)
begin
  adc_dat_sw[0] <= digital_loop ? dac_a : { adc_dat_in[1][14-1:2] , 2'h0 }; // switch adc_b->ch_a
  adc_dat_sw[1] <= digital_loop ? dac_b : { adc_dat_in[0][14-1:2] , 2'h0 }; // switch adc_a->ch_b
end

////////////////////////////////////////////////////////////////////////////////
// DAC IO
////////////////////////////////////////////////////////////////////////////////

// driving DAC reset
always @(posedge adc_clk)
  dac_reset_o <= !adc_rstn;

// Sumation of ASG and PID signal perform saturation before sending to DAC 
assign dac_a_sum = asg_dat[0] + pid_dat[0];
assign dac_b_sum = asg_dat[1] + pid_dat[1];

// saturation
assign dac_a = (^dac_a_sum[15-1:15-2]) ? {dac_a_sum[15-1], {13{~dac_a_sum[15-1]}}} :  ~dac_a_sum[14-1:0];
assign dac_b = (^dac_b_sum[15-1:15-2]) ? {dac_b_sum[15-1], {13{~dac_b_sum[15-1]}}} :  ~dac_b_sum[14-1:0];

always @(posedge adc_clk) begin
  dac_dat_a <= dac_a;
  dac_dat_b <= dac_b;
end

// IO outputs
logic dac_dco_bufg ;
BUFG bufg_dac_dco    (.O (dac_dco_bufg ), .I(dac_dco_i));

//always @(posedge dac_dco_bufg) begin
always @(posedge adc_clk) begin
  dac_dat_o[0] <= dac_dat_b ; // switch ch_b->dac_a
  dac_dat_o[1] <= dac_dat_a ; // switch ch_a->dac_b
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
  .digital_loop    (digital_loop),
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

////////////////////////////////////////////////////////////////////////////////
// oscilloscope
////////////////////////////////////////////////////////////////////////////////

logic trig_asg_out;

red_pitaya_scope i_scope (
  // ADC
  .adc_a_i       (adc_dat_sw[0]  ),  // CH 1
  .adc_b_i       (adc_dat_sw[1]  ),  // CH 2
  .adc_clk_i     (adc_clk        ),  // clock
  .adc_clk2d_i   (adc_clk2d      ),  // clock divided
  .adc_rstn_i    (adc_rstn       ),  // reset - active low
  .trig_ext_i    (trig_i         ),  // external trigger
  .trig_asg_i    (trig_asg_out   ),  // ASG trigger
  // AXI0 master                 // AXI1 master
  .axi0_clk_o    (axi0_clk   ),  .axi1_clk_o    (axi1_clk   ),
  .axi0_rstn_o   (axi0_rstn  ),  .axi1_rstn_o   (axi1_rstn  ),
  .axi0_waddr_o  (axi0_waddr ),  .axi1_waddr_o  (axi1_waddr ),
  .axi0_wdata_o  (axi0_wdata ),  .axi1_wdata_o  (axi1_wdata ),
  .axi0_wsel_o   (axi0_wsel  ),  .axi1_wsel_o   (axi1_wsel  ),
  .axi0_wvalid_o (axi0_wvalid),  .axi1_wvalid_o (axi1_wvalid),
  .axi0_wlen_o   (axi0_wlen  ),  .axi1_wlen_o   (axi1_wlen  ),
  .axi0_wfixed_o (axi0_wfixed),  .axi1_wfixed_o (axi1_wfixed),
  .axi0_werr_i   (axi0_werr  ),  .axi1_werr_i   (axi1_werr  ),
  .axi0_wrdy_i   (axi0_wrdy  ),  .axi1_wrdy_i   (axi1_wrdy  ),
  // System bus
  .sys_addr      (sys[1].addr ),
  .sys_wdata     (sys[1].wdata),
  .sys_wen       (sys[1].wen  ),
  .sys_ren       (sys[1].ren  ),
  .sys_rdata     (sys[1].rdata),
  .sys_err       (sys[1].err  ),
  .sys_ack       (sys[1].ack  )
);

////////////////////////////////////////////////////////////////////////////////
//  DAC arbitrary signal generator
////////////////////////////////////////////////////////////////////////////////


red_pitaya_asg i_asg (
   // DAC
  .dac_a_o         (asg_dat[0]  ),  // CH 1
  .dac_b_o         (asg_dat[1]  ),  // CH 2
  .dac_clk_i       (adc_clk     ),  // clock
  .dac_rstn_i      (adc_rstn    ),  // reset - active low
  .trig_a_i        (trig_i      ),
  .trig_b_i        (trig_i      ),
  .trig_out_o      (trig_asg_out),
  .temp_prot_i     (temp_prot_i ),
  // System bus
  .sys_clk         (adc_clk2d   ),
  .sys_rstn        (adc_rstn    ), 
  .sys_addr        (sys[2].addr ),
  .sys_wdata       (sys[2].wdata),
  .sys_wen         (sys[2].wen  ),
  .sys_ren         (sys[2].ren  ),
  .sys_rdata       (sys[2].rdata),
  .sys_err         (sys[2].err  ),
  .sys_ack         (sys[2].ack  )
);

////////////////////////////////////////////////////////////////////////////////
//  MIMO PID controller
////////////////////////////////////////////////////////////////////////////////

//red_pitaya_pid i_pid (
//   // signals
//  .clk_i           (adc_clk2d ),  // clock
//  .rstn_i          (adc_rstn  ),  // reset - active low
//  .dat_a_i         (adc_dat[0]),  // in 1
//  .dat_b_i         (adc_dat[1]),  // in 2
//  .dat_a_o         (pid_dat[0]),  // out 1
//  .dat_b_o         (pid_dat[1]),  // out 2
//  // System bus
//  .sys_addr        (sys[3].addr ),
//  .sys_wdata       (sys[3].wdata),
//  .sys_wen         (sys[3].wen  ),
//  .sys_ren         (sys[3].ren  ),
//  .sys_rdata       (sys[3].rdata),
//  .sys_err         (sys[3].err  ),
//  .sys_ack         (sys[3].ack  )
//);

assign sys[3].ack = 1'b1 ;
assign sys[3].err = 1'b0 ;
assign sys[3].rdata = 32'h0 ;

assign pid_dat[0] = 14'h0 ;
assign pid_dat[1] = 14'h0 ;



////////////////////////////////////////////////////////////////////////////////
// Daisy test code
////////////////////////////////////////////////////////////////////////////////

wire daisy_rx_rdy ;
wire dly_clk = fclk[3]; // 200MHz clock from PS - used for IDELAY (optionaly)

red_pitaya_daisy i_daisy (
   // SATA connector
  .daisy_p_o       (  daisy_p_o                  ),  // line 1 is clock capable
  .daisy_n_o       (  daisy_n_o                  ),
  .daisy_p_i       (  daisy_p_i                  ),  // line 1 is clock capable
  .daisy_n_i       (  daisy_n_i                  ),
   // Data
  .ser_clk_i       (  ser_clk                    ),  // high speed serial
  .dly_clk_i       (  dly_clk                    ),  // delay clock
   // TX
  .par_clk_i       (  adc_clk2d                  ),  // data paralel clock
  .par_rstn_i      (  adc_rstn                   ),  // reset - active low
  .par_rdy_o       (  daisy_rx_rdy               ),
  .par_dv_i        (  daisy_rx_rdy               ),
  .par_dat_i       (  16'h1234                   ),
   // RX
  .par_clk_o       (                             ),
  .par_rstn_o      (                             ),
  .par_dv_o        (                             ),
  .par_dat_o       (                             ),

  .debug_o         (/*led_o*/                    ),
   // System bus
  .sys_clk_i       (  adc_clk2d                  ),  // clock
  .sys_rstn_i      (  adc_rstn                   ),  // reset - active low
  .sys_addr_i      (  sys[5].addr                ),
  .sys_sel_i       (                             ),
  .sys_wdata_i     (  sys[5].wdata               ),
  .sys_wen_i       (  sys[5].wen                 ),
  .sys_ren_i       (  sys[5].ren                 ),
  .sys_rdata_o     (  sys[5].rdata               ),
  .sys_err_o       (  sys[5].err                 ),
  .sys_ack_o       (  sys[5].ack                 )
);



endmodule: red_pitaya_top
