////////////////////////////////////////////////////////////////////////////////
// Red Pitaya TOP module. It connects external pins and PS part with
// other application modules.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

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

  // ADC
  input  logic [MNA-1:0] [16-1:0] adc_dat_i,  // ADC data
  input  logic           [ 2-1:0] adc_clk_i,  // ADC clock {p,n}
  output logic           [ 2-1:0] adc_clk_o,  // optional ADC clock source (unused)
  output logic                    adc_cdcs_o, // ADC clock duty cycle stabilizer
  // DAC
  output logic [14-1:0] dac_dat_o  ,  // DAC combined data
  output logic          dac_wrt_o  ,  // DAC write
  output logic          dac_sel_o  ,  // DAC channel select
  output logic          dac_clk_o  ,  // DAC clock
  output logic          dac_rst_o  ,  // DAC reset
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
logic                 pll_dac_clk_1x;
logic                 pll_dac_clk_2x;
logic                 pll_dac_clk_2p;
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
logic                 adc_rstn;

// stream bus type
localparam type SBA_T = logic signed [14-1:0];  // acquire
localparam type SBG_T = logic signed [14-1:0];  // generate

SBA_T [MNA-1:0]          adc_dat;

// DAC signals
logic                    dac_clk_1x;
logic                    dac_clk_2x;
logic                    dac_clk_2p;
logic                    dac_rst;

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
sys_bus_if   ps_sys       (.clk  (adc_clk), .rstn    (adc_rstn));
sys_bus_if   sys [16-1:0] (.clk  (adc_clk), .rstn    (adc_rstn));

// GPIO interface
gpio_if #(.DW (24)) gpio ();

// SPI0
spi_if spi0 ();

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
  .clk_dac_1x  (pll_dac_clk_1x),  // DAC clock 125MHz
  .clk_dac_2x  (pll_dac_clk_2x),  // DAC clock 250MHz
  .clk_dac_2p  (pll_dac_clk_2p),  // DAC clock 250MHz -45DGR
  .clk_ser     (pll_ser_clk   ),  // fast serial clock
  .clk_pdm     (pll_pwm_clk   ),  // PWM clock
  // status outputs
  .pll_locked  (pll_locked)
);

BUFG bufg_adc_clk    (.O (adc_clk   ), .I (pll_adc_clk   ));
BUFG bufg_dac_clk_1x (.O (dac_clk_1x), .I (pll_dac_clk_1x));
BUFG bufg_dac_clk_2x (.O (dac_clk_2x), .I (pll_dac_clk_2x));
BUFG bufg_dac_clk_2p (.O (dac_clk_2p), .I (pll_dac_clk_2p));
BUFG bufg_ser_clk    (.O (ser_clk   ), .I (pll_ser_clk   ));
BUFG bufg_pwm_clk    (.O (pwm_clk   ), .I (pll_pwm_clk   ));

// ADC reset (active low)
always @(posedge adc_clk)
adc_rstn <=  frstn[0] &  pll_locked;

// DAC reset (active high)
always @(posedge dac_clk_1x)
dac_rst  <= ~frstn[0] | ~pll_locked;

// PWM reset (active low)
always @(posedge pwm_clk)
pwm_rstn <=  frstn[0] &  pll_locked;

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

////////////////////////////////////////////////////////////////////////////////
// system bus decoder & multiplexer (it breaks memory addresses into 8 regions)
////////////////////////////////////////////////////////////////////////////////

wire              sys_clk   = ps_sys.clk  ;
wire              sys_rstn  = ps_sys.rstn ;
wire  [  32-1: 0] sys_addr  = ps_sys.addr ;
wire  [  32-1: 0] sys_wdata = ps_sys.wdata;
wire  [8   -1: 0] sys_wen   ;
wire  [8   -1: 0] sys_ren   ;
wire  [8*32-1: 0] sys_rdata ;
wire  [8* 1-1: 0] sys_err   ;
wire  [8* 1-1: 0] sys_ack   ;
wire  [8   -1: 0] sys_cs    ;

assign sys_cs = 8'h01 << sys_addr[22:20];

assign sys_wen = sys_cs & {8{ps_sys.wen}};
assign sys_ren = sys_cs & {8{ps_sys.ren}};

assign ps_sys.rdata = sys_rdata[sys_addr[22:20]*32+:32];

assign ps_sys.err   = |(sys_cs & sys_err);
assign ps_sys.ack   = |(sys_cs & sys_ack);

// unused system bus slave ports

assign sys_rdata[5*32+:32] = 32'h0; 
assign sys_err  [5       ] =  1'b0;
assign sys_ack  [5       ] =  1'b1;

assign sys_rdata[6*32+:32] = 32'h0; 
assign sys_err  [6       ] =  1'b0;
assign sys_ack  [6       ] =  1'b1;

assign sys_rdata[7*32+:32] = 32'h0; 
assign sys_err  [7       ] =  1'b0;
assign sys_ack  [7       ] =  1'b1;

////////////////////////////////////////////////////////////////////////////////
// Analog mixed signals (PDM analog outputs)
////////////////////////////////////////////////////////////////////////////////

logic [4-1:0] [24-1:0] pwm_cfg;

red_pitaya_ams i_ams (
   // power test
  .clk_i           (  adc_clk       ),  // clock
  .rstn_i          (  adc_rstn      ),  // reset - active low
  // PWM configuration
  .dac_a_o         (  pwm_cfg[0]    ),
  .dac_b_o         (  pwm_cfg[1]    ),
  .dac_c_o         (  pwm_cfg[2]    ),
  .dac_d_o         (  pwm_cfg[3]    ),
   // System bus
  .sys_addr        (  sys_addr   ),  // address
  .sys_wdata       (  sys_wdata  ),  // write data
  .sys_wen         (  sys_wen[4] ),  // write enable
  .sys_ren         (  sys_ren[4] ),  // read enable
  .sys_rdata       (  sys_rdata[ 4*32+31: 4*32]  ),  // read data
  .sys_err         (  sys_err[4] ),  // error indicator
  .sys_ack         (  sys_ack[4] )   // acknowledge signal
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
// Daisy dummy code
////////////////////////////////////////////////////////////////////////////////

assign daisy_p_o = 1'bz;
assign daisy_n_o = 1'bz;

////////////////////////////////////////////////////////////////////////////////
// ADC IO
////////////////////////////////////////////////////////////////////////////////

// generating ADC clock is disabled
assign adc_clk_o = 2'b10;
//ODDR i_adc_clk_p ( .Q(adc_clk_o[0]), .D1(1'b1), .D2(1'b0), .C(fclk[0]), .CE(1'b1), .R(1'b0), .S(1'b0));
//ODDR i_adc_clk_n ( .Q(adc_clk_o[1]), .D1(1'b0), .D2(1'b1), .C(fclk[0]), .CE(1'b1), .R(1'b0), .S(1'b0));

// ADC clock duty cycle stabilizer is enabled
assign adc_cdcs_o = 1'b1 ;

logic [2-1:0] [14-1:0] adc_dat_raw;

// IO block registers should be used here
// lowest 2 bits reserved for 16bit ADC
always @(posedge adc_clk)
begin
  adc_dat_raw[0] <= adc_dat_i[0][16-1:2];
  adc_dat_raw[1] <= adc_dat_i[1][16-1:2];
end
    
// transform into 2's complement (negative slope)
assign adc_dat[0] = digital_loop ? dac_a : {adc_dat_raw[0][14-1], ~adc_dat_raw[0][14-2:0]};
assign adc_dat[1] = digital_loop ? dac_b : {adc_dat_raw[1][14-1], ~adc_dat_raw[1][14-2:0]};

////////////////////////////////////////////////////////////////////////////////
// DAC IO
////////////////////////////////////////////////////////////////////////////////

// Sumation of ASG and PID signal perform saturation before sending to DAC 
assign dac_a_sum = asg_dat[0] + pid_dat[0];
assign dac_b_sum = asg_dat[1] + pid_dat[1];

// saturation
assign dac_a = (^dac_a_sum[15-1:15-2]) ? {dac_a_sum[15-1], {13{~dac_a_sum[15-1]}}} : dac_a_sum[14-1:0];
assign dac_b = (^dac_b_sum[15-1:15-2]) ? {dac_b_sum[15-1], {13{~dac_b_sum[15-1]}}} : dac_b_sum[14-1:0];

// output registers + signed to unsigned (also to negative slope)
always @(posedge dac_clk_1x)
begin
  dac_dat_a <= {dac_a[14-1], ~dac_a[14-2:0]};
  dac_dat_b <= {dac_b[14-1], ~dac_b[14-2:0]};
end

// DDR outputs
ODDR oddr_dac_clk          (.Q(dac_clk_o), .D1(1'b0     ), .D2(1'b1     ), .C(dac_clk_2p), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_wrt          (.Q(dac_wrt_o), .D1(1'b0     ), .D2(1'b1     ), .C(dac_clk_2x), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_sel          (.Q(dac_sel_o), .D1(1'b1     ), .D2(1'b0     ), .C(dac_clk_1x), .CE(1'b1), .R(dac_rst), .S(1'b0));
ODDR oddr_dac_rst          (.Q(dac_rst_o), .D1(dac_rst  ), .D2(dac_rst  ), .C(dac_clk_1x), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_dat [14-1:0] (.Q(dac_dat_o), .D1(dac_dat_b), .D2(dac_dat_a), .C(dac_clk_1x), .CE(1'b1), .R(dac_rst), .S(1'b0));

////////////////////////////////////////////////////////////////////////////////
//  House Keeping
////////////////////////////////////////////////////////////////////////////////

red_pitaya_id i_hk (
  // system signals
  .clk_i           (  adc_clk       ),  // clock
  .rstn_i          (  adc_rstn      ),  // reset - active low
  // global configuration
  .digital_loop    (  digital_loop  ),
   // System bus
  .sys_addr        (  sys_addr   ),  // address
  .sys_wdata       (  sys_wdata  ),  // write data
  .sys_wen         (  sys_wen[0] ),  // write enable
  .sys_ren         (  sys_ren[0] ),  // read enable
  .sys_rdata       (  sys_rdata[ 0*32+31: 0*32]  ),  // read data
  .sys_err         (  sys_err[0] ),  // error indicator
  .sys_ack         (  sys_ack[0] )   // acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
// LED
////////////////////////////////////////////////////////////////////////////////

IOBUF iobuf_led [8-1:0] (.O (gpio.i[7:0]), .IO(led_o), .I(gpio.o[7:0]), .T(gpio.t[7:0]));

////////////////////////////////////////////////////////////////////////////////
// TFT ports
////////////////////////////////////////////////////////////////////////////////

logic [GDW-1:0] iomux_i;
logic [GDW-1:0] iomux_o;
logic [GDW-1:0] iomux_t;

IOBUF iobuf_exp [GDW-1:0] (.O (iomux_i), .IO({exp_n_io, exp_p_io}), .I(iomux_o), .T(iomux_t));

assign spi0.sck_i   = iomux_i [0+1];
assign spi0.io_i[0] = iomux_i [8+1];
assign spi0.io_i[1] = iomux_i [0+2];
assign spi0.ss_i    = iomux_i [8+2];

//      N              P                   
assign {gpio.i [08+1], gpio.i [08+0]} = {iomux_i [8+0], iomux_i [0+0]};
assign {gpio.i [08+3], gpio.i [08+2]} = {iomux_i [8+1], iomux_i [0+1]};
assign {gpio.i [08+5], gpio.i [08+4]} = {iomux_i [8+2], iomux_i [0+2]};
assign {gpio.i [08+7], gpio.i [08+6]} = {iomux_i [8+3], iomux_i [0+3]};
assign {gpio.i [16+1], gpio.i [16+0]} = {iomux_i [8+4], iomux_i [0+4]};
assign {gpio.i [16+3], gpio.i [16+2]} = {iomux_i [8+5], iomux_i [0+5]};
assign {gpio.i [16+5], gpio.i [16+4]} = {iomux_i [8+6], iomux_i [0+6]};
assign {gpio.i [16+7], gpio.i [16+6]} = {iomux_i [8+7], iomux_i [0+7]};

//      N              P                   
assign {iomux_o [8+0], iomux_o [0+0]} = {gpio.o [08+1], gpio.o [08+0]};
assign {iomux_o [8+1], iomux_o [0+1]} = {spi0.io_o[0] , spi0.sck_o   };
assign {iomux_o [8+2], iomux_o [0+2]} = {spi0.ss_o    , spi0.io_o[1] };
assign {iomux_o [8+3], iomux_o [0+3]} = {gpio.o [08+7], spi0.ss1_o   };
assign {iomux_o [8+4], iomux_o [0+4]} = {gpio.o [16+1], gpio.o [16+0]};
assign {iomux_o [8+5], iomux_o [0+5]} = {gpio.o [16+3], gpio.o [16+2]};
assign {iomux_o [8+6], iomux_o [0+6]} = {gpio.o [16+5], gpio.o [16+4]};
assign {iomux_o [8+7], iomux_o [0+7]} = {gpio.o [16+7], gpio.o [16+6]};

//      N              P                   
assign {iomux_t [8+0], iomux_t [0+0]} = {gpio.t [08+1], gpio.t [08+0]};
assign {iomux_t [8+1], iomux_t [0+1]} = {spi0.io_t[0] , spi0.sck_t   };
assign {iomux_t [8+2], iomux_t [0+2]} = {spi0.ss_t    , spi0.io_t[1] };
assign {iomux_t [8+3], iomux_t [0+3]} = {gpio.t [08+7], 1'b0         };
assign {iomux_t [8+4], iomux_t [0+4]} = {gpio.t [16+1], gpio.t [16+0]};
assign {iomux_t [8+5], iomux_t [0+5]} = {gpio.t [16+3], gpio.t [16+2]};
assign {iomux_t [8+6], iomux_t [0+6]} = {gpio.t [16+5], gpio.t [16+4]};
assign {iomux_t [8+7], iomux_t [0+7]} = {gpio.t [16+7], gpio.t [16+6]};

////////////////////////////////////////////////////////////////////////////////
// oscilloscope
////////////////////////////////////////////////////////////////////////////////

logic trig_asg_out;

red_pitaya_scope i_scope (
  // ADC
  .adc_a_i         (  adc_dat[0]    ),  // CH 1
  .adc_b_i         (  adc_dat[1]    ),  // CH 2
  .adc_clk_i       (  adc_clk       ),  // clock
  .adc_rstn_i      (  adc_rstn      ),  // reset - active low
  .trig_ext_i      (  gpio.i[8]     ),  // external trigger
  .trig_asg_i      (  trig_asg_out  ),  // ASG trigger
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
  .sys_addr        (  sys_addr   ),  // address
  .sys_wdata       (  sys_wdata  ),  // write data
  .sys_wen         (  sys_wen[1] ),  // write enable
  .sys_ren         (  sys_ren[1] ),  // read enable
  .sys_rdata       (  sys_rdata[ 1*32+31: 1*32]  ),  // read data
  .sys_err         (  sys_err[1] ),  // error indicator
  .sys_ack         (  sys_ack[1] )   // acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
//  DAC arbitrary signal generator
////////////////////////////////////////////////////////////////////////////////


red_pitaya_asg i_asg (
   // DAC
  .dac_a_o         (  asg_dat[0]    ),  // CH 1
  .dac_b_o         (  asg_dat[1]    ),  // CH 2
  .dac_clk_i       (  adc_clk       ),  // clock
  .dac_rstn_i      (  adc_rstn      ),  // reset - active low
  .trig_a_i        (  gpio.i[8]     ),
  .trig_b_i        (  gpio.i[8]     ),
  .trig_out_o      (  trig_asg_out  ),
  // System bus
  .sys_addr        (  sys_addr      ),  // address
  .sys_wdata       (  sys_wdata     ),  // write data
  .sys_wen         (  sys_wen[2]    ),  // write enable
  .sys_ren         (  sys_ren[2]    ),  // read enable
  .sys_rdata       (  sys_rdata[ 2*32+31: 2*32]  ),  // read data
  .sys_err         (  sys_err[2]    ),  // error indicator
  .sys_ack         (  sys_ack[2]    )   // acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
//  MIMO PID controller
////////////////////////////////////////////////////////////////////////////////

red_pitaya_pid i_pid (
   // signals
  .clk_i           (  adc_clk       ),  // clock
  .rstn_i          (  adc_rstn      ),  // reset - active low
  .dat_a_i         (  adc_dat[0]    ),  // in 1
  .dat_b_i         (  adc_dat[1]    ),  // in 2
  .dat_a_o         (  pid_dat[0]    ),  // out 1
  .dat_b_o         (  pid_dat[1]    ),  // out 2
  // System bus
  .sys_addr        (  sys_addr      ),  // address
  .sys_wdata       (  sys_wdata     ),  // write data
  .sys_wen         (  sys_wen[3]    ),  // write enable
  .sys_ren         (  sys_ren[3]    ),  // read enable
  .sys_rdata       (  sys_rdata[ 3*32+31: 3*32]  ),  // read data
  .sys_err         (  sys_err[3]    ),  // error indicator
  .sys_ack         (  sys_ack[3]    )   // acknowledge signal
);

endmodule: red_pitaya_top
