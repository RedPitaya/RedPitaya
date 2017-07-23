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
  // PDM DAC
  output logic [ 4-1:0] dac_pwm_o  ,  // 1-bit PDM DAC
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

logic [4-1:0] fclk ;  // {200MHz, 166MHz, 142MHz, 125MHz}
logic [4-1:0] frstn;

// PLL signals
logic                 adc_clk_in;
logic                 pll_adc_clk;
logic                 pll_dac_clk_1x;
logic                 pll_dac_clk_2x;
logic                 pll_dac_clk_2p;
logic                 pll_ser_clk;
logic                 pll_pdm_clk;
logic                 pll_locked;
// fast serial signals
logic                 ser_clk ;
// PDM clock and reset
logic                 pdm_clk ;
logic                 pdm_rstn;

// ADC clock/reset
logic                 adc_clk;
logic                 adc_rstn;

// stream bus type
localparam type SBA_T = logic signed [ 14-1:0];  // acquire
localparam type SBG_T = logic signed [ 14-1:0];  // generate
localparam type SBL_T = logic        [GDW-1:0];  // logic ananlyzer/generator

// DAC signals
logic                    dac_clk_1x;
logic                    dac_clk_2x;
logic                    dac_clk_2p;
logic                    dac_rst;
logic [MNG-1:0] [14-1:0] dac_dat;

// calibration mul/sum type
localparam type CLM_T = logic signed [16-1:0];
localparam type CLS_T = logic signed [14-1:0];

// multiplexer configuration
logic [MNG-1:0] mux_loop;
logic [MNG-1:0] mux_gen ;
logic           mux_lg  ;
// ADC calibration
CLM_T [MNA-1:0] adc_cfg_mul;  // gain
CLS_T [MNA-1:0] adc_cfg_sum;  // offset
// DAC calibration
CLM_T [MNG-1:0] dac_cfg_mul;  // gain
CLS_T [MNG-1:0] dac_cfg_sum;  // offset

// system bus
logic bus_ACLK;
logic bus_ARESETn;
axi4_lite_if bus (.ACLK (bus_ACLK), .ARESETn (bus_ARESETn));

// GPIO interface
gpio_if #(.DW (24)) gpio ();
gpio_if #(.DW (24)) gpio_dummy ();

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
  .clk_pdm     (pll_pdm_clk   ),  // PDM clock
  // status outputs
  .pll_locked  (pll_locked)
);

BUFG bufg_adc_clk    (.O (adc_clk   ), .I (pll_adc_clk   ));
BUFG bufg_dac_clk_1x (.O (dac_clk_1x), .I (pll_dac_clk_1x));
BUFG bufg_dac_clk_2x (.O (dac_clk_2x), .I (pll_dac_clk_2x));
BUFG bufg_dac_clk_2p (.O (dac_clk_2p), .I (pll_dac_clk_2p));
BUFG bufg_ser_clk    (.O (ser_clk   ), .I (pll_ser_clk   ));
BUFG bufg_pdm_clk    (.O (pdm_clk   ), .I (pll_pdm_clk   ));

// TODO: reset is a mess
logic top_rst;
assign top_rst = ~frstn[0] | ~pll_locked;

// ADC reset (active low)
always_ff @(posedge adc_clk, posedge top_rst)
if (top_rst) adc_rstn <= 1'b0;
else         adc_rstn <= ~top_rst;

// DAC reset (active high)
always_ff @(posedge dac_clk_1x, posedge top_rst)
if (top_rst) dac_rst  <= 1'b1;
else         dac_rst  <= top_rst;

// PDM reset (active low)
always_ff @(posedge pdm_clk, posedge top_rst)
if (top_rst) pdm_rstn <= 1'b0;
else         pdm_rstn <= ~top_rst;

// system bus clock and reset
assign bus_ACLK = adc_clk;
assign bus_ARESETn = adc_rstn;

////////////////////////////////////////////////////////////////////////////////
//  Connections to PS
////////////////////////////////////////////////////////////////////////////////

red_pitaya_ps ps (
  .FIXED_IO_mio       (FIXED_IO_mio     ),
  .FIXED_IO_ps_clk    (FIXED_IO_ps_clk  ),
  .FIXED_IO_ps_porb   (FIXED_IO_ps_porb ),
  .FIXED_IO_ps_srstb  (FIXED_IO_ps_srstb),
  .FIXED_IO_ddr_vrn   (FIXED_IO_ddr_vrn ),
  .FIXED_IO_ddr_vrp   (FIXED_IO_ddr_vrp ),
  // DDR
  .DDR_addr      (DDR_addr   ),
  .DDR_ba        (DDR_ba     ),
  .DDR_cas_n     (DDR_cas_n  ),
  .DDR_ck_n      (DDR_ck_n   ),
  .DDR_ck_p      (DDR_ck_p   ),
  .DDR_cke       (DDR_cke    ),
  .DDR_cs_n      (DDR_cs_n   ),
  .DDR_dm        (DDR_dm     ),
  .DDR_dq        (DDR_dq     ),
  .DDR_dqs_n     (DDR_dqs_n  ),
  .DDR_dqs_p     (DDR_dqs_p  ),
  .DDR_odt       (DDR_odt    ),
  .DDR_ras_n     (DDR_ras_n  ),
  .DDR_reset_n   (DDR_reset_n),
  .DDR_we_n      (DDR_we_n   ),
  // system signals
  .fclk_clk_o    (fclk       ),
  .fclk_rstn_o   (frstn      ),
  // ADC analog inputs
  .vinp_i        (vinp_i     ),
  .vinn_i        (vinn_i     ),
  // GPIO
  .gpio          (gpio_dummy),
  // system read/write channel
  .bus           (bus)
);

axi4lite_gpio #(
  .DW (32)
) axi4lite_gpio (
  .gpio_o (gpio.o),
  .gpio_t (gpio.t),
  .gpio_i (gpio.i),
  // bus
  .bus (bus)
);

////////////////////////////////////////////////////////////////////////////////
// LED and GPIO
////////////////////////////////////////////////////////////////////////////////

IOBUF iobuf_led   [8-1:0] (.O(gpio.i[ 7: 0]), .IO(led_o)   , .I(gpio.o[ 7: 0]), .T(gpio.t[ 7: 0]));
IOBUF iobuf_exp_p [8-1:0] (.O(gpio.i[15: 8]), .IO(exp_p_io), .I(gpio.o[15: 8]), .T(gpio.t[15: 8]));
IOBUF iobuf_exp_n [8-1:0] (.O(gpio.i[23:16]), .IO(exp_n_io), .I(gpio.o[23:16]), .T(gpio.t[23:16]));

////////////////////////////////////////////////////////////////////////////////
// Daisy dummy code
////////////////////////////////////////////////////////////////////////////////

assign daisy_p_o = 1'bz;
assign daisy_n_o = 1'bz;

endmodule: red_pitaya_top
