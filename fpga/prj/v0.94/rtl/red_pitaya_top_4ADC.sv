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

module red_pitaya_top_4ADC #(
  // identification
  bit [0:5*32-1] GITH = '0,
  // module numbers
  int unsigned MNA = 4  // number of acquisition modules
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

  // ADC
  input  logic [MNA-1:0] [ 7-1:0] adc_dat_i,  // ADC data
  input  logic [  2-1:0] [ 2-1:0] adc_clk_i,  // ADC clock {p,n}
  //output logic           [ 2-1:0] adc_clk_o,  // optional ADC clock source (unused) [0] = p; [1] = n
  //output logic                    adc_cdcs_o, // ADC clock duty cycle stabilizer

  // SPI interface to ADC
  output                spi_cs_o   ,
  output                spi_clk_o  ,
  output                spi_mosi_o ,
  // PLL control
  input  logic          pll_ref_i  ,
  output logic          pll_hi_o   ,
  output logic          pll_lo_o   ,
  // PWM DAC
  output logic [ 4-1:0] dac_pwm_o  ,  // 1-bit PWM DAC
  // XADC
  input  logic [ 5-1:0] vinp_i     ,  // voltages p
  input  logic [ 5-1:0] vinn_i     ,  // voltages n
  // Expansion connector
  inout  logic [ 8-1:0] exp_p_io   ,
  inout  logic [ 8-1:0] exp_n_io   ,
  // SATA connector
  output logic [ 2-1:0] daisy_p_o  ,  // line 1 is clock capable
  output logic [ 2-1:0] daisy_n_o  ,
  input  logic [ 2-1:0] daisy_p_i  ,  // line 1 is clock capable
  input  logic [ 2-1:0] daisy_n_i  ,
  // LED
  inout  logic [ 8-1:0] led_o
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// GPIO parameter
localparam int unsigned GDW = 8+8;

logic [4-1:0] fclk ; //[0]-125MHz, [1]-250MHz, [2]-50MHz, [3]-200MHz
logic [4-1:0] frstn;
logic         idly_rdy;

// AXI masters 0, 1
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

// AXI masters 2, 3
logic            axi3_clk    , axi2_clk    ;
logic            axi3_rstn   , axi2_rstn   ;
logic [ 32-1: 0] axi3_waddr  , axi2_waddr  ;
logic [ 64-1: 0] axi3_wdata  , axi2_wdata  ;
logic [  8-1: 0] axi3_wsel   , axi2_wsel   ;
logic            axi3_wvalid , axi2_wvalid ;
logic [  4-1: 0] axi3_wlen   , axi2_wlen   ;
logic            axi3_wfixed , axi2_wfixed ;
logic            axi3_werr   , axi2_werr   ;
logic            axi3_wrdy   , axi2_wrdy   ;
// PLL signals
logic [  2-1: 0]      adc_clk_in;
logic [  2-1: 0]      pll_adc_clk;
logic                 pll_ser_clk;
logic                 pll_pwm_clk;
logic [  2-1: 0]      pll_locked;
logic                 adc_10mhz;

// fast serial signals
logic                 ser_clk ;
// PWM clock and reset
logic                 pwm_clk ;
logic                 pwm_rstn;

// ADC clock/reset
logic       adc_clk_01 , adc_clk_23 ;
logic       adc_rstn_01, adc_rstn_23;

// stream bus type
localparam type SBA_T = logic signed [14-1:0];
SBA_T [MNA-1:0]          adc_dat, adc_dat_r;

// configuration
logic                    digital_loop;

// system bus
sys_bus_if   ps_sys      (.clk (adc_clk_01), .rstn (adc_rstn_01));
sys_bus_if   sys [8-1:0] (.clk (adc_clk_01), .rstn (adc_rstn_01));
sys_bus_if   sys_adc_23  (.clk (adc_clk_23), .rstn (adc_rstn_23));

// GPIO interface
gpio_if #(.DW (24)) gpio ();

////////////////////////////////////////////////////////////////////////////////
// PLL (clock and reset)
////////////////////////////////////////////////////////////////////////////////

// diferential clock input
IBUFDS i_clk_01 (.I (adc_clk_i[0][1]), .IB (adc_clk_i[0][0]), .O (adc_clk_in[0]));  // differential clock input
IBUFDS i_clk_23 (.I (adc_clk_i[1][1]), .IB (adc_clk_i[1][0]), .O (adc_clk_in[1]));  // differential clock input

red_pitaya_pll_4adc pll_01 (
  // inputs
  .clk         (adc_clk_in[0]),  // clock
  .rstn        (frstn[0]  ),  // reset - active low
  // output clocks
  .clk_adc     (pll_adc_clk[0]),  // ADC clock
  .clk_10mhz   (pll_adc_10mhz ),  // ADC divided to 10MHz
  .clk_ser     (pll_ser_clk   ),  // fast serial clock
  .clk_pdm     (pll_pwm_clk   ),  // PWM clock
  // status outputs
  .pll_locked  (pll_locked[0])
);

red_pitaya_pll_4adc pll_23 (
  // inputs
  .clk         (adc_clk_in[1]),  // clock
  .rstn        (frstn[0]  ),  // reset - active low
  // output clocks
  .clk_adc     (pll_adc_clk[1]),  // ADC clock
  // status outputs
  .pll_locked  (pll_locked[1])
);


BUFG bufg_adc_clk_01 (.O (adc_clk_01), .I (pll_adc_clk[0]));
BUFG bufg_adc_clk_23 (.O (adc_clk_23), .I (pll_adc_clk[1]));
BUFG bufg_adc_10MHz  (.O (adc_10mhz ), .I (pll_adc_10mhz ));
BUFG bufg_ser_clk    (.O (ser_clk   ), .I (pll_ser_clk   ));
BUFG bufg_pwm_clk    (.O (pwm_clk   ), .I (pll_pwm_clk   ));

wire [2-1:0] adc_clks;
assign adc_clks={adc_clk_23, adc_clk_01};

// ADC reset (active low)
always @(posedge adc_clk_01)
adc_rstn_01 <=  frstn[0] &  pll_locked[0] & idly_rdy;

always @(posedge adc_clk_23)
adc_rstn_23 <=  frstn[0] &  pll_locked[1] & idly_rdy;

// PWM reset (active low)
always @(posedge pwm_clk)
pwm_rstn <=  frstn[0] &  pll_locked[0] & idly_rdy;

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
  // system read/write channel
  .bus           (ps_sys      ),
  // AXI masters 0, 1 
  .axi1_clk_i    (axi1_clk    ),  .axi0_clk_i    (axi0_clk    ),  // global clock
  .axi1_rstn_i   (axi1_rstn   ),  .axi0_rstn_i   (axi0_rstn   ),  // global reset
  .axi1_waddr_i  (axi1_waddr  ),  .axi0_waddr_i  (axi0_waddr  ),  // system write address
  .axi1_wdata_i  (axi1_wdata  ),  .axi0_wdata_i  (axi0_wdata  ),  // system write data
  .axi1_wsel_i   (axi1_wsel   ),  .axi0_wsel_i   (axi0_wsel   ),  // system write byte select
  .axi1_wvalid_i (axi1_wvalid ),  .axi0_wvalid_i (axi0_wvalid ),  // system write data valid
  .axi1_wlen_i   (axi1_wlen   ),  .axi0_wlen_i   (axi0_wlen   ),  // system write burst length
  .axi1_wfixed_i (axi1_wfixed ),  .axi0_wfixed_i (axi0_wfixed ),  // system write burst type (fixed / incremental)
  .axi1_werr_o   (axi1_werr   ),  .axi0_werr_o   (axi0_werr   ),  // system write error
  .axi1_wrdy_o   (axi1_wrdy   ),  .axi0_wrdy_o   (axi0_wrdy   ),  // system write ready
  // AXI masters 2, 3 
  .axi3_clk_i    (axi3_clk    ),  .axi2_clk_i    (axi2_clk    ),  // global clock
  .axi3_rstn_i   (axi3_rstn   ),  .axi2_rstn_i   (axi2_rstn   ),  // global reset
  .axi3_waddr_i  (axi3_waddr  ),  .axi2_waddr_i  (axi2_waddr  ),  // system write address
  .axi3_wdata_i  (axi3_wdata  ),  .axi2_wdata_i  (axi2_wdata  ),  // system write data
  .axi3_wsel_i   (axi3_wsel   ),  .axi2_wsel_i   (axi2_wsel   ),  // system write byte select
  .axi3_wvalid_i (axi3_wvalid ),  .axi2_wvalid_i (axi2_wvalid ),  // system write data valid
  .axi3_wlen_i   (axi3_wlen   ),  .axi2_wlen_i   (axi2_wlen   ),  // system write burst length
  .axi3_wfixed_i (axi3_wfixed ),  .axi2_wfixed_i (axi2_wfixed ),  // system write burst type (fixed / incremental)
  .axi3_werr_o   (axi3_werr   ),  .axi2_werr_o   (axi2_werr   ),  // system write error
  .axi3_wrdy_o   (axi3_wrdy   ),  .axi2_wrdy_o   (axi2_wrdy   )   // system write ready
);

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
sys_bus_stub sys_bus_stub_3 (sys[3]);
generate
for (genvar i=6; i<8; i++) begin: for_sys
  sys_bus_stub sys_bus_stub_5_7 (sys[i]);
end: for_sys
endgenerate

////////////////////////////////////////////////////////////////////////////////
// ADC IO
////////////////////////////////////////////////////////////////////////////////

// DDR inputs
// falling edge: odd bits    rising edge: even bits   
// 0: CH1 falling edge data  1: CH1 rising edge data
// 2: CH2 falling edge data  3: CH2 rising edge data
// 4: CH3 falling edge data  5: CH3 rising edge data
// 6: CH4 falling edge data  7: CH4 rising edge data

// delay input ADC signals
//(* IODELAY_GROUP = adc_inputs *) // Specifies group name for associated IDELAYs/ODELAYs and IDELAYCTRL
logic [4*7-1:0] idly_rst ;
logic [4*7-1:0] idly_ce  ;
logic [4*7-1:0] idly_inc ;
logic [4*7-1:0] [5-1:0] idly_cnt ;

IDELAYCTRL i_idelayctrl (
  .RDY(idly_rdy),   // 1-bit output: Ready output
  .REFCLK(fclk[3]), // 1-bit input: Reference clock input
  .RST(!frstn[3])   // 1-bit input: Active high reset input
);

genvar GV;
genvar GVC;
genvar GVD;

generate
for (GVC = 0; GVC < 4; GVC = GVC + 1) begin : channels
  for (GV = 0; GV < 7; GV = GV + 1) begin : adc_decode
    logic          adc_dat_idly;
    logic [ 2-1:0] adc_dat_ddr;

//    logic [8-1:0] [ 7-1:0] adc_dat_ddr;
//    logic [4-1:0] [ 7-1:0] adc_dat_idly;
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
   i_dly (
      .CNTVALUEOUT  ( idly_cnt[GV+GVC*7]    ),  // 5-bit output: Counter value output
      .DATAOUT      ( adc_dat_idly          ),  // 1-bit output: Delayed data output
      .C            ( adc_clk_in[GVC/2]     ),  // 1-bit input: Clock input
      .CE           ( idly_ce[GV+GVC*7]     ),  // 1-bit input: Active high enable increment/decrement input
      .CINVCTRL     ( 1'b0                  ),  // 1-bit input: Dynamic clock inversion input
      .CNTVALUEIN   ( 5'h0                  ),  // 5-bit input: Counter value input
      .DATAIN       ( 1'b0                  ),  // 1-bit input: Internal delay data input
      .IDATAIN      ( adc_dat_i[GVC][GV]    ),  // 1-bit input: Data input from the I/O
      .INC          ( idly_inc[GV+GVC*7]    ),  // 1-bit input: Increment / Decrement tap delay input
      .LD           ( idly_rst[GV+GVC*7]    ),  // 1-bit input: Load IDELAY_VALUE input
      .LDPIPEEN     ( 1'b0                  ),  // 1-bit input: Enable PIPELINE register to load data input
      .REGRST       ( 1'b0                  )   // 1-bit input: Active-high reset tap-delay input
   );
  
    IDDR #(.DDR_CLK_EDGE("SAME_EDGE")) iddr_adc_dat_0 (.D(adc_dat_idly), .Q1({adc_dat_ddr[1]}), .Q2({adc_dat_ddr[0]}), .C(adc_clks[GVC/2]), .CE(1'b1), .R(1'b0), .S(1'b0));
    assign adc_dat_raw[GVC][2*GV  ] = adc_dat_ddr[0];
    assign adc_dat_raw[GVC][2*GV+1] = adc_dat_ddr[1];
  end 
end
endgenerate

logic [4-1:0] [14-1:0] adc_dat_raw;

always @(posedge adc_clk_01) begin
  adc_dat_r[0] <= {adc_dat_raw[0][14-1], ~adc_dat_raw[0][14-2:0]};
  adc_dat_r[1] <= {adc_dat_raw[1][14-1], ~adc_dat_raw[1][14-2:0]};

  adc_dat  [0] <= adc_dat_r[0];
  adc_dat  [1] <= adc_dat_r[1];
end

always @(posedge adc_clk_23) begin
  adc_dat_r[2] <= {adc_dat_raw[2][14-1], ~adc_dat_raw[2][14-2:0]};
  adc_dat_r[3] <= {adc_dat_raw[3][14-1], ~adc_dat_raw[3][14-2:0]};

  adc_dat  [2] <= adc_dat_r[2];
  adc_dat  [3] <= adc_dat_r[3];
end

////////////////////////////////////////////////////////////////////////////////
//  House Keeping
////////////////////////////////////////////////////////////////////////////////

logic [  8-1: 0] exp_p_in , exp_n_in ;
logic [  8-1: 0] exp_p_out, exp_n_out;
logic [  8-1: 0] exp_p_dir, exp_n_dir;

red_pitaya_hk_4adc i_hk (
  // system signals
  .clk_i           (adc_clk_01 ),  // clock
  .rstn_i          (adc_rstn_01),  // reset - active low
  // LED
  .led_o           (led_o       ),  // LED output
  // idelay control
  .idly_rst_o      (idly_rst    ),
  .idly_ce_o       (idly_ce     ),
  .idly_inc_o      (idly_inc    ),
  .idly_cnt_i      ({idly_cnt[21],idly_cnt[14],idly_cnt[7],idly_cnt[0]}),

  .spi_cs_o        (spi_cs_o   ),
  .spi_clk_o       (spi_clk_o  ),
  .spi_mosi_o      (spi_mosi_o ),

  // global configuration
  .digital_loop    (digital_loop),
  .pll_sys_i       (adc_10mhz   ),    // system clock
  .pll_ref_i       (pll_ref_i   ),    // reference clock
  .pll_hi_o        (pll_hi_o    ),    // PLL high
  .pll_lo_o        (pll_lo_o    ),    // PLL low
  // Expansion connector
  .exp_p_dat_i     (exp_p_in ),  // input data
  .exp_p_dat_o     (exp_p_out),  // output data
  .exp_p_dir_o     (exp_p_dir),  // 1-output enable
  .exp_n_dat_i     (exp_n_in ),
  .exp_n_dat_o     (exp_n_out),
  .exp_n_dir_o     (exp_n_dir),
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
// Analog mixed signals (PDM analog outputs)
////////////////////////////////////////////////////////////////////////////////

logic [4-1:0] [8-1:0] pdm_cfg;

red_pitaya_ams i_ams (
  // power test
  .clk_i           (adc_clk_01 ),  // clock
  .rstn_i          (adc_rstn_01),  // reset - active low
  // PWM configuration
  .dac_a_o         (pdm_cfg[0]),
  .dac_b_o         (pdm_cfg[1]),
  .dac_c_o         (pdm_cfg[2]),
  .dac_d_o         (pdm_cfg[3]),
  // System bus
  .sys_addr        (sys[4].addr ),
  .sys_wdata       (sys[4].wdata),
  .sys_wen         (sys[4].wen  ),
  .sys_ren         (sys[4].ren  ),
  .sys_rdata       (sys[4].rdata),
  .sys_err         (sys[4].err  ),
  .sys_ack         (sys[4].ack  )
);

red_pitaya_pdm pdm (
  // system signals
  .clk   (adc_clk_01 ),
  .rstn  (adc_rstn_01),
  // configuration
  .cfg   (pdm_cfg),
  .ena      (1'b1),
  .rng      (8'd255),
  // PWM outputs
  .pdm (dac_pwm_o)
);

////////////////////////////////////////////////////////////////////////////////
// GPIO
////////////////////////////////////////////////////////////////////////////////

IOBUF i_iobufp [8-1:0] (.O(exp_p_in), .IO(exp_p_io), .I(exp_p_out), .T(~exp_p_dir) );
IOBUF i_iobufn [8-1:0] (.O(exp_n_in), .IO(exp_n_io), .I(exp_n_out), .T(~exp_n_dir) );

assign gpio.i[15: 8] = exp_p_in;
assign gpio.i[23:16] = exp_n_in;

////////////////////////////////////////////////////////////////////////////////
// oscilloscope CH0 and CH1
////////////////////////////////////////////////////////////////////////////////
wire [4-1:0] trig_ch_0_1;
wire [4-1:0] trig_ch_2_3;
logic        trig_asg_out;

red_pitaya_scope i_scope_0_1 (
  // ADC
  .adc_a_i       (adc_dat[0]  ),  // CH 1
  .adc_b_i       (adc_dat[1]  ),  // CH 2
  .adc_clk_i     (adc_clk_01  ),  // clock
  .adc_rstn_i    (adc_rstn_01 ),  // reset - active low
  .trig_ext_i    (gpio.i[8]   ),  // external trigger
  .trig_asg_i    (trig_asg_out),  // ASG trigger
  .trig_ch_o     (trig_ch_0_1 ),  // output trigger to ADC for other 2 channels
  .trig_ch_i     (trig_ch_2_3 ),  // input ADC trigger from other 2 channels
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
// oscilloscope CH2 and CH3
////////////////////////////////////////////////////////////////////////////////

sys_bus_sync i_sync (
  // system signals
  .bus_m (sys[2]    ),
  .bus_s (sys_adc_23)
);

red_pitaya_scope i_scope_2_3 (
  // ADC
  .adc_a_i       (adc_dat[2]  ),  // CH 1
  .adc_b_i       (adc_dat[3]  ),  // CH 2
  .adc_clk_i     (adc_clk_23  ),  // clock
  .adc_rstn_i    (adc_rstn_23 ),  // reset - active low
  .trig_ext_i    (gpio.i[8]   ),  // external trigger
  .trig_asg_i    (trig_asg_out),  // ASG trigger
  .trig_ch_o     (trig_ch_2_3 ),  // output trigger to ADC for other 2 channels
  .trig_ch_i     (trig_ch_0_1 ),  // input ADC trigger from other 2 channels
  // AXI2 master                 // AXI3 master
  .axi0_clk_o    (axi2_clk   ),  .axi1_clk_o    (axi3_clk   ),
  .axi0_rstn_o   (axi2_rstn  ),  .axi1_rstn_o   (axi3_rstn  ),
  .axi0_waddr_o  (axi2_waddr ),  .axi1_waddr_o  (axi3_waddr ),
  .axi0_wdata_o  (axi2_wdata ),  .axi1_wdata_o  (axi3_wdata ),
  .axi0_wsel_o   (axi2_wsel  ),  .axi1_wsel_o   (axi3_wsel  ),
  .axi0_wvalid_o (axi2_wvalid),  .axi1_wvalid_o (axi3_wvalid),
  .axi0_wlen_o   (axi2_wlen  ),  .axi1_wlen_o   (axi3_wlen  ),
  .axi0_wfixed_o (axi2_wfixed),  .axi1_wfixed_o (axi3_wfixed),
  .axi0_werr_i   (axi2_werr  ),  .axi1_werr_i   (axi3_werr  ),
  .axi0_wrdy_i   (axi2_wrdy  ),  .axi1_wrdy_i   (axi3_wrdy  ),
  // System bus
  .sys_addr      (sys_adc_23.addr ),
  .sys_wdata     (sys_adc_23.wdata),
  .sys_wen       (sys_adc_23.wen  ),
  .sys_ren       (sys_adc_23.ren  ),
  .sys_rdata     (sys_adc_23.rdata),
  .sys_err       (sys_adc_23.err  ),
  .sys_ack       (sys_adc_23.ack  )
);

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
  .par_clk_i       (  adc_clk_01                 ),  // data paralel clock
  .par_rstn_i      (  adc_rstn_01                ),  // reset - active low
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
  .sys_clk_i       (  adc_clk_01                 ),  // clock
  .sys_rstn_i      (  adc_rstn_01                ),  // reset - active low
  .sys_addr_i      (  sys[5].addr                ),
  .sys_sel_i       (                             ),
  .sys_wdata_i     (  sys[5].wdata               ),
  .sys_wen_i       (  sys[5].wen                 ),
  .sys_ren_i       (  sys[5].ren                 ),
  .sys_rdata_o     (  sys[5].rdata               ),
  .sys_err_o       (  sys[5].err                 ),
  .sys_ack_o       (  sys[5].ack                 )
);


endmodule: red_pitaya_top_4ADC
