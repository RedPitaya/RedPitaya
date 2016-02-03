////////////////////////////////////////////////////////////////////////////////
// Red Pitaya TOP module. It connects external pins and PS part with
// other application modules.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module red_pitaya_top #(
  // identification
  bit [40*8-1:0] GITH = '0,
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
  input  logic [MNA-1:0] [16-1:2] adc_dat_i,  // ADC data
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

logic [4-1:0] fclk ; //[0]-125MHz, [1]-250MHz, [2]-50MHz, [3]-200MHz
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
logic                    adc_clk;
logic                    adc_rstn;

// stream bus type
localparam type SBA_T = logic signed [14-1:0];  // acquire
localparam type SBG_T = logic signed [14-1:0];  // generate
localparam type SBL_T = logic        [16-1:0];  // logic ananlyzer/generator

// analog input streams
str_bus_if #(           .DAT_T (SBA_T)) str_adc [MNA-1:0] (.clk (adc_clk), .rstn (adc_rstn));  // ADC
str_bus_if #(           .DAT_T (SBA_T)) str_osc [MNA-1:0] (.clk (adc_clk), .rstn (adc_rstn));  // osciloscope
// analog output streams
str_bus_if #(           .DAT_T (SBG_T)) str_asg [MNG-1:0] (.clk (adc_clk), .rstn (adc_rstn));  // ASG
str_bus_if #(           .DAT_T (SBG_T)) str_dac [MNG-1:0] (.clk (adc_clk), .rstn (adc_rstn));  // DAC
// digital input streams
str_bus_if #(.DN (  2), .DAT_T (SBL_T)) str_lgo           (.clk (adc_clk), .rstn (adc_rstn));  // LG
str_bus_if #(           .DAT_T (SBL_T)) str_lai           (.clk (adc_clk), .rstn (adc_rstn));  // LA

// DMA sterams RX/TX
str_bus_if #(           .DAT_T (SBL_T)) str_drx   [4-1:0] (.clk (adc_clk), .rstn (adc_rstn));  // RX
str_bus_if #(           .DAT_T (SBL_T)) str_dtx   [4-1:0] (.clk (adc_clk), .rstn (adc_rstn));  // TX

// DAC signals
logic                    dac_clk_1x;
logic                    dac_clk_2x;
logic                    dac_clk_2p;
logic                    dac_rst;
logic [MNG-1:0] [14-1:0] dac_dat;

// calibration mul/sum type
localparam type CLM_T = logic signed [16-1:0];
localparam type CLS_T = logic signed [14-1:0];

// configuration
logic                 digital_loop;
// ADC calibration
CLM_T [MNA-1:0] adc_cfg_mul;  // gain
CLS_T [MNA-1:0] adc_cfg_sum;  // offset
// DAC calibration
CLM_T [MNG-1:0] dac_cfg_mul;  // gain
CLS_T [MNG-1:0] dac_cfg_sum;  // offset

// triggers
typedef struct packed {
  // GPIO
  logic   [2-1:0] gio_out;  // 2   - event    triggers from GPIO       {negedge, posedge}
  // analog generator
  logic [MNG-1:0] gen_out;  // event    triggers
  logic [MNG-1:0] gen_swo;  // software triggers
  // analog acquire
  logic [MNA-1:0] acq_out;  // event    triggers
  logic [MNA-1:0] acq_swo;  // software triggers
  // logic generator
  logic           lg_out;
  logic           lg_swo;
  // logic analyzer
  logic           la_out;
  logic           la_swo;
} trg_t;

trg_t trg;

// interrupts
typedef struct packed {
  // GPIO
  logic   [1-1:0] gio_out;  // 2   - event    triggers from GPIO       {negedge, posedge}
  // analog generator
  logic [MNG-1:0] gen_trg;  // event    triggers
  logic [MNG-1:0] gen_stp;  // software triggers
  // analog acquire
  logic [MNA-1:0] acq_trg;  // trigger
  logic [MNA-1:0] acq_stp;  // stop
  // logic generator
  logic           lg_trg;
  logic           lg_stp;
  // logic analyzer
  logic           la_trg;
  logic           la_stp;
} irq_t;

irq_t irq;

// system bus
sys_bus_if   ps_sys       (.clk  (adc_clk), .rstn    (adc_rstn));
sys_bus_if   sys [16-1:0] (.clk  (adc_clk), .rstn    (adc_rstn));

////////////////////////////////////////////////////////////////////////////////
// PLL (clock and reaset)
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
  // interrupts
  .irq           (irq         ),
  // system read/write channel
  .bus           (ps_sys      ),
  // AXI streams
  .sti           (str_drx     ),
  .sto           (str_dtx     )
);

////////////////////////////////////////////////////////////////////////////////
// system bus decoder & multiplexer (it breaks memory addresses into 8 regions)
////////////////////////////////////////////////////////////////////////////////

sys_bus_interconnect #(
  .SN (16),
  .SW (18)
) sys_bus_interconnect (
  .bus_m (ps_sys),
  .bus_s (sys)
);

// silence unused busses
generate
for (genvar i=13; i<16; i++) begin: for_sys
  sys_bus_stub sys_bus_stub_13_16 (sys[i]);
end: for_sys
endgenerate

////////////////////////////////////////////////////////////////////////////////
// Current time stamp
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned TW = 64;

logic [TW-1:0] cts;

cts cts_i (
  // system signals
  .clk  (adc_clk),
  .rstn (adc_rstn),
  // counter
  .cts  (cts)
);

////////////////////////////////////////////////////////////////////////////////
// Housekeeping
////////////////////////////////////////////////////////////////////////////////

id #(
  .GITH (GITH)
) id (
  .bus (sys[0])
);

////////////////////////////////////////////////////////////////////////////////
// I/O and stream multiplexing
////////////////////////////////////////////////////////////////////////////////

muxctl muxctl (
  // global configuration
  .digital_loop  (digital_loop),
  .gpio_mux      (),
   // System bus
  .bus           (sys[1])
);

////////////////////////////////////////////////////////////////////////////////
// GPIO
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned GDW = 8+8;

logic [GDW-1:0] gpio_e;  // output enable
logic [GDW-1:0] gpio_o;  // output
logic [GDW-1:0] gpio_i;  // input

gpio #(.DW (GDW)) gpio (
  // expansion connector
  .gpio_e  (gpio_e),
  .gpio_o  (gpio_o),
  .gpio_i  (gpio_i),
  // interrupt
  .irq     (irq.gio_out),
  // system bus
  .bus     (sys[2])
);

////////////////////////////////////////////////////////////////////////////////
// extension connector
////////////////////////////////////////////////////////////////////////////////

// TODO: there should be a multiplexer between GPIO and LG/LA
//IOBUF i_iobufp [8-1:0] (.O(exp_p_i), .IO(exp_p_io), .I(exp_p_o), .T(~exp_p_e));
//IOBUF i_iobufn [8-1:0] (.O(exp_n_i), .IO(exp_n_io), .I(exp_n_o), .T(~exp_n_e));
//IOBUF iobuf_exp [GDW-1:0] (.O(gpio_i), .IO({exp_n_io, exp_p_io}), .I(gpio_o), .T(~gpio_e));

logic [GDW-1:0] exp_e;  // output enable
logic [GDW-1:0] exp_o;  // output
logic [GDW-1:0] exp_i;  // input

// IO buffer with output enable
IOBUF iobuf_exp [GDW-1:0] (.O (exp_i), .IO({exp_n_io, exp_p_io}), .I(exp_o), .T(exp_e));

// input DDR
IDDR #(
  .DDR_CLK_EDGE ("SAME_EDGE_PIPELINED")
) iddr_exp_i [GDW-1:0] (
  .Q1 (str_lai.dat),
  .Q2 (),  // TODO: add DDR support for LA here
  .C  (adc_clk),
  .CE (1'b1),
  .D  (exp_i),
  .R  (1'b0),
  .S  (1'b0)
);

assign str_lai.vld = 1'b1;
assign str_lai.kep = '1;
assign str_lai.lst = 1'b0;

// output DDR
ODDR #(
  .DDR_CLK_EDGE ("SAME_EDGE")
) oddr_exp_o [GDW-1:0] (
  .Q  (exp_o),
  .C  (adc_clk),
  .CE (1'b1),
  .D1 (str_lgo.dat[0]),
  .D2 (str_lgo.dat[0]),  // TODO: add DDR support for LG here
  .R  (1'b0),
  .S  (1'b0)
);

assign str_lgo.rdy = 1'b1;

// output enable DDR
ODDR #(
  .IS_D1_INVERTED (1'b1), // IOBUF T input is buffer disable, so negation is needed somewhere
  .IS_D2_INVERTED (1'b1), // IOBUF T input is buffer disable, so negation is needed somewhere
  .DDR_CLK_EDGE ("SAME_EDGE")
) oddr_exp_e [GDW-1:0] (
  .Q  (exp_e),
  .C  (adc_clk),
  .CE (1'b1),
  .D1 (str_lgo.dat[1]),  // TODO: add DDR support for LG here
  .D2 (str_lgo.dat[1]),
  .R  (1'b0),
  .S  (1'b0)
);

////////////////////////////////////////////////////////////////////////////////
// debounce
////////////////////////////////////////////////////////////////////////////////

debounce #(
  .CW (20),
  .DI (1'b0)
) debounce (
  // system signals
  .clk  (adc_clk ),
  .rstn (adc_rstn),
  // configuration
  .ena  (1'b1),
  .len  (20'd62500),  // 0.5ms
  // input stream
  .d_i  (exp_i[0]),
  .d_o  (),
  .d_p  (trg.gio_out[0]),
  .d_n  (trg.gio_out[1])
);

////////////////////////////////////////////////////////////////////////////////
// LED
////////////////////////////////////////////////////////////////////////////////

gpio #(.DW (8)) led (
  // expansion connector
  .gpio_e  (     ),
  .gpio_o  (led_o),
  .gpio_i  (led_o),
  // interrupts
  .irq     (),
  // system bus
  .bus     (sys[3])
);

//IOBUF iobuf_led [GDW-1:0] (.O(gpio_i), .IO(led_o), .I(gpio_o), .T(~gpio_e));

////////////////////////////////////////////////////////////////////////////////
// Calibration
////////////////////////////////////////////////////////////////////////////////

red_pitaya_calib calib (
  // ADC calibration
  .adc_cfg_mul   (adc_cfg_mul),
  .adc_cfg_sum   (adc_cfg_sum),
  // DAC calibration
  .dac_cfg_mul   (dac_cfg_mul),
  .dac_cfg_sum   (dac_cfg_sum),
  // System bus
  .bus           (sys[4])
);

////////////////////////////////////////////////////////////////////////////////
// Analog mixed signals (PDM analog outputs)
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned PDM_CHN = 4;
localparam int unsigned PDM_DWC = 8;
localparam type PDM_T = logic [PDM_DWC-1:0];

PDM_T [PDM_CHN-1:0]  pdm_cfg;

sys_reg_array_o #(
  .RT (PDM_T  ),
  .RN (PDM_CHN)
) regset_pdm (
  .val       (pdm_cfg),
  .bus       (sys[5])
);

pdm #(
  .DWC (PDM_DWC),
  .CHN (PDM_CHN)
) pdm (
  // system signals
  .clk      (pdm_clk ),
  .rstn     (pdm_rstn),
  .cke      (1'b1),
  // configuration
  .ena      (1'b1),
  .rng      (8'd255),
  // input stream
  .str_dat  (pdm_cfg),
  .str_vld  (1'b1   ),
  .str_rdy  (       ),
  // PWM outputs
  .pdm      (dac_pwm_o)
);

////////////////////////////////////////////////////////////////////////////////
// PWM
////////////////////////////////////////////////////////////////////////////////

`ifdef ENABLE_PWM

localparam int unsigned PWM_CHN = 4;
localparam int unsigned PWM_DWC = 8;
localparam type PWM_T = logic [PWM_DWC-1:0];

PWM_T [PWM_CHN-1:0] pwm_cfg;

sys_reg_array_o #(
  .RT (PWM_T  ),
  .RN (PWM_CHN)
) regset_pwm (
  .val       (pwm_cfg),
  .bus       (sys[6])
);

pwm #(
  .DWC (PWM_DWC),
  .CHN (PWM_CHN)
) pwm (
  // system signals
  .clk      (pdm_clk ),
  .rstn     (pdm_rstn),
  .cke      (1'b1),
  // configuration
  .ena      (1'b1),
  .rng      (8'd255),
  // input stream
  .str_dat  (pwm_cfg),
  .str_vld  (1'b1   ),
  .str_rdy  (       ),
  // PWM outputs
  .pwm      ()
);

`else

sys_bus_stub sys_bus_stub_6 (sys[6]);

`endif // ENABLE_PWM

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

// ADC clock duty cycle stabilizer is enabled
assign adc_cdcs_o = 1'b1 ;

generate
for (genvar i=0; i<MNA; i++) begin: for_adc

  // local variables
  logic signed [14-1:0] adc_dat_raw;

  // IO block registers should be used here
  // lowest 2 bits reserved for 16bit ADC
  always_ff @(posedge adc_clk)
  adc_dat_raw <= {adc_dat_i[i][16-1], ~adc_dat_i[i][16-2:2]};

  // digital loopback multiplexer
  assign str_adc[i].vld = 1'b1;
  assign str_adc[i].dat = digital_loop ? str_dac[i].dat : adc_dat_raw;

  linear #(
    .DTI  (SBA_T),
    .DTO  (SBA_T),
    .DWM  (16)
  ) linear_adc (
    // stream input/output
    .sti      (str_adc[i]),
    .sto      (str_osc[i]),
    // configuration
    .cfg_mul  (adc_cfg_mul[i]),
    .cfg_sum  (adc_cfg_sum[i])
  );

end: for_adc
endgenerate

////////////////////////////////////////////////////////////////////////////////
// DAC IO
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<MNA; i++) begin: for_dac

  linear #(
    .DTI  (SBG_T),
    .DTO  (SBG_T),
    .DWM  (16)
  ) linear_dac (
    // stream input/output
    .sti      (str_asg[i]),
    .sto      (str_dac[i]),
    // configuration
    .cfg_mul  (dac_cfg_mul[i]),
    .cfg_sum  (dac_cfg_sum[i])
  );

  // output registers + signed to unsigned (also to negative slope)
  assign dac_dat[i] = {str_dac[i].dat[0][14-1], ~str_dac[i].dat[0][14-2:0]};
  assign str_dac[i].rdy = 1'b1;

end: for_dac
endgenerate

// DDR outputs
// TODO set parameter #(.DDR_CLK_EDGE ("SAME_EDGE"))
ODDR oddr_dac_clk          (.Q(dac_clk_o), .D1(1'b0      ), .D2(1'b1      ), .C(dac_clk_2p), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_wrt          (.Q(dac_wrt_o), .D1(1'b0      ), .D2(1'b1      ), .C(dac_clk_2x), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_sel          (.Q(dac_sel_o), .D1(1'b1      ), .D2(1'b0      ), .C(dac_clk_1x), .CE(1'b1), .R(dac_rst), .S(1'b0));
ODDR oddr_dac_rst          (.Q(dac_rst_o), .D1(dac_rst   ), .D2(dac_rst   ), .C(dac_clk_1x), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_dat [14-1:0] (.Q(dac_dat_o), .D1(dac_dat[0]), .D2(dac_dat[1]), .C(dac_clk_1x), .CE(1'b1), .R(dac_rst), .S(1'b0));

////////////////////////////////////////////////////////////////////////////////
// ASG (arbitrary signal generators)
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<MNG; i++) begin: for_gen

asg_top #(
  .DAT_T (SBG_T),
  .DAT_M (logic signed [$bits(SBG_T)+2-1:0]),
  .DAT_S (SBG_T),
  .TN ($bits(trg))
) asg (
  // stream output
  .sto       (str_asg[i]),
  // triggers
  .trg_ext   (trg),
  .trg_swo   (trg.gen_swo[i]),
  .trg_out   (trg.gen_out[i]),
  // interrupts
  .irq_trg   (irq.gen_trg[i]),
  .irq_stp   (irq.gen_stp[i]),
  // System bus
  .bus       (sys[7+i])
);

end: for_gen
endgenerate

////////////////////////////////////////////////////////////////////////////////
// oscilloscope
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<MNA; i++) begin: for_acq

scope_top #(
  .TN ($bits(trg))
) scope (
  // streams
  .sti       (str_osc[i]),
  .sto       (str_drx[i]),
  // current time stamp
  .cts       (cts),
  // triggers
  .trg_ext   (trg),
  .trg_swo   (trg.acq_swo[i]),
  .trg_out   (trg.acq_out[i]),
  // interrupts
  .irq_trg   (irq.acq_trg[i]),
  .irq_stp   (irq.acq_stp[i]),
  // System bus
  .bus       (sys[9+i])
);

end: for_acq
endgenerate

////////////////////////////////////////////////////////////////////////////////
// LG (logic generator)
////////////////////////////////////////////////////////////////////////////////

asg_top #(
  .EN_LIN (0),
  .DAT_T (SBL_T),
  .TN ($bits(trg))
) lg (
  // stream output
  .sto       (str_lgo),
  // triggers
  .trg_ext   (trg),
  .trg_swo   (trg.lg_swo),
  .trg_out   (trg.lg_out),
  // interrupts
  .irq_trg   (irq.lg_trg),
  .irq_stp   (irq.lg_stp),
  // System bus
  .bus       (sys[11])
);

////////////////////////////////////////////////////////////////////////////////
// LA (logic analyzer)
////////////////////////////////////////////////////////////////////////////////

la_top #(
  .DAT_T (SBL_T),
  .TN ($bits(trg))
) la (
  // streams
  .sti       (str_lai),
  .sto       (str_drx[2]),
  // current time stamp
  .cts       (cts),
  // triggers
  .trg_ext   (trg),
  .trg_swo   (trg.la_swo),
  .trg_out   (trg.la_out),
  // interrupts
  .irq_trg   (irq.la_trg),
  .irq_stp   (irq.la_stp),
  // System bus
  .bus       (sys[12])
);

////////////////////////////////////////////////////////////////////////////////
// on demand HW processor
////////////////////////////////////////////////////////////////////////////////

// TODO: for now just a loopback
// this is an attempt to minimize the related DMA

str_pas on_demand (
  .ena (1'b1),
  .sti (str_dtx[3]),
  .sto (str_drx[3])
);

endmodule: red_pitaya_top
