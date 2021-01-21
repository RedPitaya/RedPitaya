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
  //input  logic          trig_i ,
  input  logic          pll_ref_i,
  output logic          pll_hi_o,
  output logic          pll_lo_o,
  //input  logic [ 2-1:0] temp_prot_i ,
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
  input  logic [ 9-1:0] exp_p_io   ,
  output  logic [ 9-1:0] exp_n_io   ,
  // LED
  output logic [ 8-1:0] led_o
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
logic                 pll_adc_clk2d;
logic                 pll_adc_10mhz;
logic                 pll_pwm_clk;
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
logic                 adc_rstn2d;

logic [  8-1: 0] led_hk;

// stream bus type
localparam type SBA_T = logic signed [ 14-1:0];  // acquire
localparam type SBG_T = logic signed [ 14-1:0];  // generate
localparam type SBL_T = logic        [GDW-1:0];  // logic ananlyzer/generator

// analog input streams
axi4_stream_if #(         .DT (SBA_T)) str_adc [MNA-1:0] (.ACLK (adc_clk), .ARESETn (adc_rstn));  // ADC
// analog output streams
axi4_stream_if #(         .DT (SBG_T)) str_asg [MNG-1:0] (.ACLK (adc_clk), .ARESETn (adc_rstn));  // ASG
axi4_stream_if #(         .DT (SBG_T)) str_dac [MNG-1:0] (.ACLK (adc_clk), .ARESETn (adc_rstn));  // DAC
// digital input streams
axi4_stream_if #(.DN (2), .DT (SBL_T)) str_lgo           (.ACLK (adc_clk), .ARESETn (adc_rstn));  // LG

// DMA sterams RX/TX
axi4_stream_if #(         .DT (SBL_T)) str_drx   [3-1:0] (.ACLK (adc_clk), .ARESETn (adc_rstn));  // RX


// AXI4-Stream DMA RX/TX
axi4_stream_if #(.DN (2), .DT (logic [8-1:0])) axi_drx [4-1:0] (.ACLK (adc_clk), .ARESETn (adc_rstn));  // RX
axi4_stream_if #(.DN (2), .DT (logic [8-1:0])) axi_dtx [4-1:0] (.ACLK (adc_clk), .ARESETn (adc_rstn));  // TX

axi4_stream_if #(.DN (2), .DT (SBL_T))         exp_exe         (.ACLK (adc_clk), .ARESETn (adc_rstn));
axi4_stream_if #(.DN (2), .DT (SBL_T))         exp_exo         (.ACLK (adc_clk), .ARESETn (adc_rstn));
axi4_stream_if #(.DN (2), .DT (SBL_T))         exp_exi         (.ACLK (adc_clk), .ARESETn (adc_rstn));

// DAC signals
logic                    dac_clk_1x;
logic                    dac_clk_2x;
logic                    dac_clk_2p;
logic                    dac_rst;
logic [MNG-1:0] [14-1:0] dac_dat;

// multiplexer configuration
logic [MNG-1:0] mux_loop;
logic [MNG-1:0] mux_gen ;
logic           mux_lg  ;

// triggers
typedef struct packed {
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
sys_bus_if   ps_sys       (.clk (adc_clk), .rstn (adc_rstn));
sys_bus_if   sys [16-1:0] (.clk (adc_clk), .rstn (adc_rstn));

// GPIO interface
gpio_if #(.DW (24)) gpio ();

logic [GDW-1:0] exp_e;  // output enable
logic [GDW-1:0] exp_o;  // output
logic [GDW-1:0] exp_i;  // input

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

// TODO: reset is a mess
logic top_rst;
assign top_rst = ~frstn[0] | ~pll_locked;

// ADC reset (active low)
always_ff @(posedge adc_clk, posedge top_rst)
if (top_rst) adc_rstn <= 1'b0;
else         adc_rstn <= ~top_rst;

// ADC reset (active low)
always_ff @(posedge adc_clk2d, posedge top_rst)
if (top_rst) adc_rstn2d <= 1'b0;
else         adc_rstn2d <= ~top_rst;

// DAC reset (active high)
always_ff @(posedge dac_clk_1x, posedge top_rst)
if (top_rst) dac_rst  <= 1'b1;
else         dac_rst  <= top_rst;

// PDM reset (active low)
always_ff @(posedge pdm_clk, posedge top_rst)
if (top_rst) pdm_rstn <= 1'b0;
else         pdm_rstn <= ~top_rst;

assign led_o = led_hk;
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
  // interrupts
  .irq           (irq         ),
  // system read/write channel
  .bus           (ps_sys      ),
  // AXI streams
  .srx           (axi_drx     ),
  .stx           (axi_dtx     )
);

generate
for (genvar i=0; i<3; i++) begin: for_str

  // RX
//  for (genvar b=0; b<DN; b==b+DN) begin: for_byte_i
//  assign sai[i].TKEEP[DN*b+:DN] = {DN{sti[i].kep}};
//  end: for_byte_i
  assign axi_drx[i].TKEEP           = {2{str_drx[i].TKEEP}};
  assign axi_drx[i].TDATA           =    str_drx[i].TDATA  ;
  assign axi_drx[i].TLAST           =    str_drx[i].TLAST  ;
  assign axi_drx[i].TVALID          =    str_drx[i].TVALID ;
  // TODO: fix this timing issue somewhere else
  if (i==2)
  assign str_drx[i].TREADY          = 1'b1;
  else
  assign str_drx[i].TREADY          =    axi_drx[i].TREADY;

end: for_str
endgenerate


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

sys_bus_stub sys_bus_stub_2 (sys[2]);

////////////////////////////////////////////////////////////////////////////////
//  House Keeping
////////////////////////////////////////////////////////////////////////////////

logic [  9-1: 0] exp_p_in , exp_n_in ;
logic [  9-1: 0] exp_p_out, exp_n_out;
logic [  9-1: 0] exp_p_dir, exp_n_dir;
logic            exp_9_in,  exp_9_out, exp_9_dir;


wire [2-1:0] hk_spi_cs  ;
wire [2-1:0] hk_spi_clk ;
wire [2-1:0] hk_spi_i   ;
wire [2-1:0] hk_spi_o   ;
wire [2-1:0] hk_spi_t   ;


logic [2*7-1:0] idly_rst ;
logic [2*7-1:0] idly_ce  ;
logic [2*7-1:0] idly_inc ;
logic [2*7-1:0] [5-1:0] idly_cnt ;


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

assign adc_spi_csb  = hk_spi_cs[0];
assign adc_spi_clk  = hk_spi_clk[0];
assign hk_spi_i[0]  = adc_spi_sdio;
assign adc_spi_sdio = hk_spi_t[0] ? 1'bz : hk_spi_o[0] ;

assign dac_spi_csb  = hk_spi_cs[1];
assign dac_spi_clk  = hk_spi_clk[1];
assign hk_spi_i[1]  = dac_spi_sdio;
assign dac_spi_sdio = hk_spi_t[1] ? 1'bz : hk_spi_o[1] ;

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
// I/O and stream multiplexing
////////////////////////////////////////////////////////////////////////////////

muxctl muxctl (
  // global configuration
  .mux_loop  (mux_loop),
  .mux_gen   (mux_gen ),
  .mux_lg    (mux_lg  ),
   // System bus
  .bus       (sys[1])
);

////////////////////////////////////////////////////////////////////////////////
// GPIO ports
////////////////////////////////////////////////////////////////////////////////

assign gpio.i [23:8] = {exp_n_io[7:0], exp_p_io[7:0]};

////////////////////////////////////////////////////////////////////////////////
// extension connector
////////////////////////////////////////////////////////////////////////////////

// temporary solution for unit testing
// IOBUF has been split into separate IBUF & OBUF
// // output enable DDR
// ODDR #(
//   .IS_D1_INVERTED (1'b1), // IOBUF T input is buffer disable, so negation is needed somewhere
//   .IS_D2_INVERTED (1'b1), // IOBUF T input is buffer disable, so negation is needed somewhere
//   .DDR_CLK_EDGE ("SAME_EDGE")
// ) oddr_exp_e [GDW-1:0] (
//   .Q  (exp_e           ),
//   .C  (exp_exe.ACLK    ),
//   .CE (exp_exe.TVALID  ),
//   .D1 (exp_exe.TDATA[0]),  // TODO: add DDR support for LG here
//   .D2 (exp_exe.TDATA[1]),
//   .R  (~exp_exe.ARESETn ),
//   .S  (1'b0)
// );

assign exp_exe.TREADY = 1'b1;

// // output DDR
// ODDR #(
//   .DDR_CLK_EDGE ("SAME_EDGE")
// ) oddr_exp_o [GDW-1:0] (
//   .Q  (exp_o           ),
//   .C  (exp_exo.ACLK    ),
//   .CE (exp_exo.TVALID  ),
//   .D1 (exp_exo.TDATA[0]),
//   .D2 (exp_exo.TDATA[1]),  // TODO: add DDR support for LG here
//   .R  (~exp_exo.ARESETn ),
//   .S  (1'b0            )
// );
assign exp_n_io = exp_exo.TDATA[0];
assign exp_exo.TREADY = 1'b1;

// // input DDR
// IDDR #(
//   .DDR_CLK_EDGE ("SAME_EDGE_PIPELINED")
// ) iddr_exp_i [GDW-1:0] (
//   .Q1 (exp_exi.TDATA[0]),
//   .Q2 (exp_exi.TDATA[1]),  // TODO: add DDR support for LA here
//   .C  (exp_exi.ACLK    ),
//   .CE (exp_exi.TREADY  ),
//   .R  (~exp_exi.ARESETn ),
//   .S  (1'b0            ),
//   .D  (exp_i           )
// );
assign exp_exi.TDATA[0] = exp_p_io ;
assign exp_exi.TDATA[1] = exp_p_io ;
assign exp_exi.TVALID = 1'b1;
assign exp_exi.TKEEP  = '1;
assign exp_exi.TLAST  = 1'b0;

// IO buffer with output enable
// TODO: this is hardcoded, since it somehow did not work before, simulation was fine, but synthesis might have a problem
// IOBUF iobuf_exp [GDW-1:0] (.O (exp_i), .IO({exp_n_io, exp_p_io}), .I(exp_o), .T({8'h00, 8'hff}));
//IOBUF iobuf_exp [GDW-1:0] (.O (exp_i), .IO({exp_n_io, exp_p_io}), .I(exp_o), .T(exp_e));

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<MNA; i++) begin: for_adc

  // local variables
  logic signed [14-1:0] adc_dat_raw;

  // IO block registers should be used here
  // lowest 2 bits reserved for 16bit ADC
  always_ff @(posedge adc_clk)
  adc_dat_raw <= {adc_dat_in[i][14-1:2] , 4'h0};

  // digital loopback multiplexer
  assign str_adc[i].TVALID = 1'b1;
  assign str_adc[i].TDATA  = mux_loop[i] ? str_dac[i].TDATA : adc_dat_raw;
  assign str_adc[i].TKEEP  = mux_loop[i] ? str_dac[i].TKEEP : '1;
  assign str_adc[i].TLAST  = mux_loop[i] ? str_dac[i].TLAST : 1'b0;

end: for_adc
endgenerate

////////////////////////////////////////////////////////////////////////////////
// DAC IO
////////////////////////////////////////////////////////////////////////////////

// driving DAC reset
always @(posedge adc_clk)
  dac_reset_o <= !adc_rstn;

generate
for (genvar i=0; i<MNA; i++) begin: for_dac

  axi4_stream_if #(.DN (1), .DT (SBG_T)) str_gen [2-1:0] (.ACLK (adc_clk), .ARESETn (adc_rstn));  // ASG

  axi4_stream_pas pas_0 (
    .ena (1'b1),
    .sti (str_asg[i]),
    .sto (str_gen[0])
  );

`ifdef ENABLE_GEN_DMA
  // TODO this conversion is not actually implemented yet
  axi4_stream_pas #(.DNI (2), .DNO (1)) pas_1 (
    .ena (1'b1),
    .sti (axi_dtx[0+i]),
    .sto (str_gen[1])
  );
`else
  // toward DMA
  assign axi_dtx[0+i].TREADY = 1'b0;
  // toward DAC
  assign str_gen[1].TVALID = 1'b0;
`endif

  axi4_stream_mux #(.DN (1), .DT (SBG_T)) mux_gen (
    .sel (mux_gen[i]),
    .sti (str_gen),
    .sto (str_dac[i])
  );

  // output registers + signed to unsigned (also to negative slope)
  //assign dac_dat_o[i] = str_dac[i].TDATA;
  assign str_dac[i].TREADY = 1'b1;

end: for_dac
endgenerate

assign dac_dat_o[0] = str_dac[0].TDATA;
assign dac_dat_o[1] = str_dac[1].TDATA;


// DDR outputs
// TODO set parameter #(.DDR_CLK_EDGE ("SAME_EDGE"))
//ODDR oddr_dac_sel          (.Q(dac_sel_o), .D1(1'b1      ), .D2(1'b0      ), .C(dac_clk_1x), .CE(1'b1), .R(dac_rst), .S(1'b0));
//ODDR oddr_dac_rst          (.Q(dac_rst_o), .D1(dac_rst   ), .D2(dac_rst   ), .C(dac_clk_1x), .CE(1'b1), .R(1'b0   ), .S(1'b0));///////////
//ODDR oddr_dac_dat [14-1:0] (.Q(dac_dat_o), .D1(dac_dat[0]), .D2(dac_dat[1]), .C(dac_clk_1x), .CE(1'b1), .R(dac_rst), .S(1'b0));

////////////////////////////////////////////////////////////////////////////////
// ASG (arbitrary signal generators)
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<MNG; i++) begin: for_gen

old_asg_top #(
  .DT  (SBG_T),
  .DTM (logic signed [$bits(SBG_T)+2-1:0]),
  .DTS (SBG_T),
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

`ifdef ENABLE_OSC

scope_top #(
  .TN ($bits(trg)),
  .CW (32)
) scope (
  // streams
  .sti       (str_adc[0+i]),
  .sto       (str_drx[0+i]),
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

`else

// this code is unused, for now just silence everything

assign str_adc[0+i].TREADY = 1'b1;

assign str_drx[0+i].TVALID = '0;
assign str_drx[0+i].TDATA  = '0;
assign str_drx[0+i].TKEEP  = '0;
assign str_drx[0+i].TLAST  = '0;

assign trg.acq_swo[i] = '0;
assign trg.acq_out[i] = '0;

assign irq.acq_trg[i] = '0;
assign irq.acq_stp[i] = '0;

`endif

end: for_acq
endgenerate


////////////////////////////////////////////////////////////////////////////////
// LG (logic generator)
////////////////////////////////////////////////////////////////////////////////

old_asg_top #(
  .EN_LIN (0),
  .DT (SBL_T),
  .TN (12)
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

assign exp_exe.TDATA  = {2{str_lgo.TDATA[1]}};
assign exp_exe.TKEEP  =    str_lgo.TKEEP   ;
assign exp_exe.TLAST  =    str_lgo.TLAST   ;
assign exp_exe.TVALID =    str_lgo.TVALID  ;

assign exp_exo.TDATA  = {2{str_lgo.TDATA[0]}};
assign exp_exo.TKEEP  =    str_lgo.TKEEP   ;
assign exp_exo.TLAST  =    str_lgo.TLAST   ;
assign exp_exo.TVALID =    str_lgo.TVALID  ;

assign str_lgo.TREADY = exp_exo.TREADY;

// TODO: for now just a loopback
// this is an attempt to minimize the related DMA

assign axi_dtx[2].TREADY = 1'b1;

////////////////////////////////////////////////////////////////////////////////
// LA (logic analyzer)
////////////////////////////////////////////////////////////////////////////////

old_la_top #(
  .DT (SBL_T),
  .TN (12),
  .CW (32)
) la (
  // streams
  .sti       (exp_exi),
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

axi4_stream_pas loopback (
  .ena (1'b1),
  .sti (axi_dtx[3]),
  .sto (axi_drx[3])
);

endmodule: red_pitaya_top
