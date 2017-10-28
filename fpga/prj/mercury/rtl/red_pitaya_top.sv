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
  int unsigned MNO = 2,  // number of oscilloscope modules
  int unsigned MNG = 2   // number of generator    modules
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
  input  logic [MNO-1:0] [16-1:0] adc_dat_i,  // ADC data
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


// stream bus type
localparam type DTG = logic   signed [14-1:0];  // generate
localparam type DTO = logic   signed [16-1:0];  // acquire
localparam type DTL = logic unsigned [16-1:0];  // logic (generator/analyzer)
localparam type DTLG = struct packed {DTL e, o;};

// GPIO parameter
localparam int unsigned GDW = 8+8;

logic [4-1:0] fclk ; //[0]-125MHz, [1]-250MHz, [2]-50MHz, [3]-200MHz
logic [4-1:0] frstn;

// PLL signals
logic adc_clk_in;
logic pll_adc_clk;
logic pll_locked;

// ADC clock/reset
logic adc_clk;
logic adc_rstn;

// DAC signals
logic dac_clk_1x;
logic dac_clk_2x;
logic dac_clk_2p;
logic dac_rst;

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
  .clk_adc     (pll_adc_clk),  // ADC clock
  .clk_dac_1x  (dac_clk_1x ),  // DAC clock 125MHz
  .clk_dac_2x  (dac_clk_2x ),  // DAC clock 250MHz
  .clk_dac_2p  (dac_clk_2p ),  // DAC clock 250MHz -45DGR
  .clk_ser     (           ),  // fast serial clock
  .clk_pdm     (           ),  // PDM clock
  // status outputs
  .pll_locked  (pll_locked)
);

BUFG bufg_adc_clk    (.O (adc_clk   ), .I (pll_adc_clk   ));

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

////////////////////////////////////////////////////////////////////////////////
// ADC IO
////////////////////////////////////////////////////////////////////////////////

// ADC AXI4-Stream interface
axi4_stream_if #(.DT (DTO)) str_adc [MNO-1:0] (.ACLK (adc_clk), .ARESETn (adc_rstn));

generate
for (genvar i=0; i<MNO; i++) begin: for_adc
  DTO adc_raw;

  // IO block registers should be used here
  // lowest 2 bits reserved for 16bit ADC
  always_ff @(posedge adc_clk)
  adc_raw <= adc_dat_i[i];

  // transform into 2's complement (negative slope)
  assign str_adc[i].TDATA  = {adc_raw[$bits(DTO)-1], ~adc_raw[$bits(DTO)-2:0]};
  assign str_adc[i].TKEEP  = '1;
  assign str_adc[i].TLAST  = 1'b0;
  // TVALID is always active
  assign str_adc[i].TVALID = 1'b1;

end: for_adc
endgenerate

// generating ADC clock is disabled
assign adc_clk_o = 2'b10;
//ODDR i_adc_clk_p ( .Q(adc_clk_o[0]), .D1(1'b1), .D2(1'b0), .C(fclk[0]), .CE(1'b1), .R(1'b0), .S(1'b0));
//ODDR i_adc_clk_n ( .Q(adc_clk_o[1]), .D1(1'b0), .D2(1'b1), .C(fclk[0]), .CE(1'b1), .R(1'b0), .S(1'b0));

// ADC clock duty cycle stabilizer is enabled
assign adc_cdcs_o = 1'b1;

////////////////////////////////////////////////////////////////////////////////
// DAC IO
////////////////////////////////////////////////////////////////////////////////

// DAC AXI4-Stream interface
axi4_stream_if #(.DT (DTG)) str_dac [MNG-1:0] (.ACLK (adc_clk), .ARESETn (adc_rstn));

logic [MNG-1:0] [14-1:0] dac_raw;

generate
for (genvar i=0; i<MNG; i++) begin: for_dac
  // output registers + signed to unsigned (also to negative slope)
  always_ff @(posedge str_dac[i].ACLK)
  if (~str_dac[i].ARESETn) begin
      dac_raw[i] <= (1<<($bits(DTG)-1))-1;
  end else if (str_dac[i].TVALID & str_dac[i].TREADY & str_dac[i].TKEEP) begin
      dac_raw[i] <= {str_dac[i].TDATA[0][$bits(DTG)-1], ~str_dac[i].TDATA[0][$bits(DTG)-2:0]};
  end

  // TREADY is always active
  assign str_dac[i].TREADY = 1'b1;
end: for_dac
endgenerate

// DDR outputs
// TODO set parameter #(.DDR_CLK_EDGE ("SAME_EDGE"))
ODDR oddr_dac_clk          (.Q(dac_clk_o), .D1(1'b0      ), .D2(1'b1      ), .C(dac_clk_2p), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_wrt          (.Q(dac_wrt_o), .D1(1'b0      ), .D2(1'b1      ), .C(dac_clk_2x), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_sel          (.Q(dac_sel_o), .D1(1'b0      ), .D2(1'b1      ), .C(dac_clk_1x), .CE(1'b1), .R(dac_rst), .S(1'b0));
ODDR oddr_dac_rst          (.Q(dac_rst_o), .D1(dac_rst   ), .D2(dac_rst   ), .C(dac_clk_1x), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_dat [14-1:0] (.Q(dac_dat_o), .D1(dac_raw[0]), .D2(dac_raw[1]), .C(dac_clk_1x), .CE(1'b1), .R(dac_rst), .S(1'b0));

////////////////////////////////////////////////////////////////////////////////
// Daisy dummy code
////////////////////////////////////////////////////////////////////////////////

assign daisy_p_o = 1'bz;
assign daisy_n_o = 1'bz;

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// software events
top_pkg::evn_t evn;
// trigger events
top_pkg::trg_t trg;
// interrupts
top_pkg::irq_t irq;

// system bus
sys_bus_if   ps_sys       (.clk (adc_clk), .rstn (adc_rstn));
sys_bus_if   sys [32-1:0] (.clk (adc_clk), .rstn (adc_rstn));

// GPIO interface
gpio_if #(.DW (24)) gpio ();

// RX DMA streaming interfaces
axi4_stream_if #(.DT (DTO)) srx_osc [MNO-1:0] (.ACLK (adc_clk), .ARESETn (adc_rstn));
axi4_stream_if #(.DT (DTL)) srx_la            (.ACLK (adc_clk), .ARESETn (adc_rstn));

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
  .gpio          (gpio),
  // interrupt
  .irq           (irq),
  // DMA stream input
  .srx_osc       (srx_osc),
  .srx_la        (srx_la ),
  // system read/write channel
  .bus           (ps_sys     )
);

////////////////////////////////////////////////////////////////////////////////
// system bus decoder & multiplexer
// it breaks memory addresses into 32 regions of 64kB each
////////////////////////////////////////////////////////////////////////////////

sys_bus_interconnect #(
  .SN (32),
  .SW (16)
) sys_bus_interconnect (
  .bus_m (ps_sys),
  .bus_s (sys)
);

// silence unused busses
generate
for (genvar i=17; i<32; i++) begin: for_sys_2
  sys_bus_stub sys_bus_stub_2 (sys[i]);
end: for_sys_2
endgenerate

////////////////////////////////////////////////////////////////////////////////
// identification
////////////////////////////////////////////////////////////////////////////////

id #(.GITH (GITH)) id (
  // System bus
  .bus (sys[0])
);

////////////////////////////////////////////////////////////////////////////////
// management
////////////////////////////////////////////////////////////////////////////////

// GPIO mode
DTL exp_iom;
logic [2-1:0] mgmt_loop;

mgmt #(.GW ($bits(DTL))) mgmt (
  // GPIO mode
  .cfg_iom  (exp_iom),
  // loop mux control
  .cfg_loop (mgmt_loop),
  // System bus
  .bus      (sys[1])
);

////////////////////////////////////////////////////////////////////////////////
// LED
////////////////////////////////////////////////////////////////////////////////

IOBUF iobuf_led [8-1:0] (.O(gpio.i[7:0]), .IO(led_o), .I(gpio.o[7:0]), .T(gpio.t[7:0]));
//IOBUF iobuf_exp [16-1:0] (.O(gpio.i[23:8]), .IO({exp_n_io, exp_p_io}), .I(gpio.o[23:8]), .T(gpio.t[23:8]));

////////////////////////////////////////////////////////////////////////////////
// GPIO mux
////////////////////////////////////////////////////////////////////////////////

DTL exp_i;
DTL exp_o;
DTL exp_t;

// multiplexing GPIO signals from PS with logic generator
assign gpio.i[23:8] = exp_i;
// GPIO mode is multiplexing between (0 - PS GPIO, 1 - LG)
assign exp_o =   ~exp_iom &  gpio.o[23:8] | exp_iom & str_lg.TDATA[0].o ;
assign exp_t = ~(~exp_iom & ~gpio.t[23:8] | exp_iom & str_lg.TDATA[0].e);

// TODO connect logic generator
assign str_lg.TREADY = 1'b1;

// logic analyzer
assign str_la.TLAST  = 1'b0;
assign str_la.TKEEP  = '1;
assign str_la.TDATA  = exp_i;
assign str_la.TVALID = 1'b1;

////////////////////////////////////////////////////////////////////////////////
// GPIO DDR
////////////////////////////////////////////////////////////////////////////////

DTL exp_ddr_i;
DTL exp_ddr_o;
DTL exp_ddr_t;

// output enable DDR
ODDR #(
//  .IS_D1_INVERTED (1'b1), // IOBUF T input is buffer disable, so negation is needed somewhere
//  .IS_D2_INVERTED (1'b1), // IOBUF T input is buffer disable, so negation is needed somewhere
  .DDR_CLK_EDGE ("SAME_EDGE")
) oddr_exp_t [GDW-1:0] (
  .Q  (exp_ddr_t       ),
  .C  (str_lg.ACLK     ),
  .CE (1'b1            ),
  .D1 ( exp_t          ),  // exp_exo.TDATA[0]
  .D2 ( exp_t          ),  // exp_exo.TDATA[1]
  .R  (~str_lg.ARESETn ),
  .S  (1'b0)
);

// output DDR
ODDR #(
  .DDR_CLK_EDGE ("SAME_EDGE")
) oddr_exp_o [GDW-1:0] (
  .Q  ( exp_ddr_o      ),
  .C  ( str_lg.ACLK    ),
  .CE (1'b1            ),
  .D1 ( exp_o          ),  // exp_exo.TDATA[0]
  .D2 ( exp_o          ),  // exp_exo.TDATA[1]
  .R  (~str_lg.ARESETn ),
  .S  (1'b0            )
);

// input DDR
IDDR #(
  .DDR_CLK_EDGE ("SAME_EDGE_PIPELINED")
) iddr_exp_i [GDW-1:0] (
  .Q1 (                ),  //exp_exi.TDATA[0]
  .Q2 ( exp_i          ),  //exp_exi.TDATA[1]
  .C  ( str_lg.ACLK    ),
  .CE ( 1'b1           ),
  .R  (~str_lg.ARESETn ),
  .S  (1'b0            ),
  .D  (exp_ddr_i       )
);

IOBUF iobuf_exp [16-1:0] (
  .O  (exp_ddr_i),
  .IO ({exp_n_io, exp_p_io}),
  .I  (exp_ddr_o),
  .T  (exp_ddr_t)
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
  .val      (pdm_cfg),
  .bus      (sys[2])
);

pdm #(
  .DWC (PDM_DWC),
  .CHN (PDM_CHN)
) pdm (
  // system signals
  .clk      (adc_clk ),
  .rstn     (adc_rstn),
  .cke      (1'b1),
  // configuration
  .ena      (1'b1),
  .rng      (8'd255),
  // input stream
  .str_dat  (pdm_cfg),
  .str_vld  (1'b1   ),
  .str_rdy  (       ),
  // PDM outputs
  .pdm      (dac_pwm_o)
);

////////////////////////////////////////////////////////////////////////////////
// calibration
////////////////////////////////////////////////////////////////////////////////

// ADC(osc)/DAC(gen) AXI4-Stream interfaces
axi4_stream_if #(.DT (DTG))  str_gen_clb [MNG-1:0] (.ACLK (str_dac[0].ACLK), .ARESETn (str_dac[0].ARESETn));
axi4_stream_if #(.DT (DTG))  str_gen     [MNG-1:0] (.ACLK (str_dac[0].ACLK), .ARESETn (str_dac[0].ARESETn));
axi4_stream_if #(.DT (DTO))  str_osc     [MNO-1:0] (.ACLK (str_adc[0].ACLK), .ARESETn (str_adc[0].ARESETn));
axi4_stream_if #(.DT (DTO))  str_osc_clb [MNO-1:0] (.ACLK (str_adc[0].ACLK), .ARESETn (str_adc[0].ARESETn));
axi4_stream_if #(.DT (DTLG)) str_lg_mux            (.ACLK (str_dac[0].ACLK), .ARESETn (str_dac[0].ARESETn));
axi4_stream_if #(.DT (DTLG)) str_lg                (.ACLK (str_dac[0].ACLK), .ARESETn (str_dac[0].ARESETn));
axi4_stream_if #(.DT (DTL))  str_la                (.ACLK (str_adc[0].ACLK), .ARESETn (str_adc[0].ARESETn));
axi4_stream_if #(.DT (DTL))  str_la_mux            (.ACLK (str_adc[0].ACLK), .ARESETn (str_adc[0].ARESETn));

clb #(
  .MNG (MNG),
  .MNO (MNO),
  .DTG (DTG),
  .DTO (DTO)
) clb (
  // generator (DAC) streams
  .str_gen  (str_gen_clb),
  .str_dac  (str_dac),
  // oscilloscope (ADC) streams
  .str_adc  (str_adc),
  .str_osc  (str_osc_clb),
  // system bus
  .bus      (sys[3])
);

////////////////////////////////////////////////////////////////////////////////
// DAC -> ADC loop
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<MNG; i++) begin: for_gen_osc_loop

  axi4_stream_if #(.DT (DTG))  str_gen_mux [1:0] (.ACLK (str_dac[0].ACLK), .ARESETn (str_dac[0].ARESETn));
  axi4_stream_if #(.DT (DTO))  str_osc_mux [1:0] (.ACLK (str_adc[0].ACLK), .ARESETn (str_adc[0].ARESETn));

  // generator demultiplexer
  axi4_stream_demux #(
    .SN (2),
    .DT (DTG)
  ) mux_gen (
    // control
    .sel  (mgmt_loop[i]),
    // streams
    .sti  (str_gen[i] ),
    .sto  (str_gen_mux)
  );

  axi4_stream_pas pas_gen (
    // control
    .ena  (1'b1),
    // streams
    .sti  (str_gen_mux[0]),
    .sto  (str_gen_clb[i])
  );

  // bit shift for from 14bit gen to 16bit osc
  assign str_gen_mux[1].TREADY = str_osc_mux[1].TREADY;
  assign str_osc_mux[1].TVALID = str_gen_mux[1].TVALID;
  assign str_osc_mux[1].TKEEP  = str_gen_mux[1].TKEEP ;
  assign str_osc_mux[1].TLAST  = str_gen_mux[1].TLAST ;
  for (genvar j=0; j<1; j++) begin: for_gen_osc_loop_dat
  assign str_osc_mux[1].TDATA[j] = str_gen_mux[1].TDATA[j] <<< 2;
  end: for_gen_osc_loop_dat

//  axi4_stream_pas pas_osc (
//    // control
//    .ena  (1'b1),
//    // streams
//    .sti  (str_osc_clb[i]),
//    .sto  (str_osc_mux[0])
//  );

  // bit shift for from 14bit gen to 16bit osc
  assign str_osc_clb[i].TREADY = str_osc_mux[0].TREADY;
  assign str_osc_mux[0].TVALID = str_osc_clb[i].TVALID;
  assign str_osc_mux[0].TKEEP  = str_osc_clb[i].TKEEP ;
  assign str_osc_mux[0].TLAST  = str_osc_clb[i].TLAST ;
  assign str_osc_mux[0].TDATA  = str_osc_clb[i].TDATA ;

  // oscilloscope multiplexer
  axi4_stream_mux #(
    .SN (2),
    .DT (DTO)
  ) mux_osc (
    // control
    .sel  (mgmt_loop[i]),
    // streams
    .sti  (str_osc_mux),
    .sto  (str_osc[i] )
  );

end: for_gen_osc_loop
endgenerate

////////////////////////////////////////////////////////////////////////////////
//  DAC arbitrary signal generator
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<MNG; i++) begin: for_gen

  gen #(
    .DN  (1),
    .DT  (DTG),
    .ER  (0+i),
    .EN  (top_pkg::MNS),
    .TN  ($bits(trg))
  ) gen (
    // stream output
    .sto      (str_gen[i]),
    // software events
    .evi      (evn),
    .evo      (evn.gen[i]),
    // trigger events
    .trg      (trg),
    .tro      (trg.gen[i]),
    // interrupts
    .irq      (irq.gen[i]),
    // System bus
    .bus      (sys[4+2*i+0]),
    .bus_tbl  (sys[4+2*i+1])
  );

end: for_gen
endgenerate

////////////////////////////////////////////////////////////////////////////////
// oscilloscope
////////////////////////////////////////////////////////////////////////////////

generate
for (genvar i=0; i<MNO; i++) begin: for_osc

  axi4_stream_if #(.DT (DTO)) str (.ACLK (str_osc[i].ACLK), .ARESETn (str_osc[i].ARESETn));

  logic ctl_rst;

  osc #(
    .DN  (1),
    .DT  (DTO),
    .ER  (2+i),
    .EN  (top_pkg::MNS),
    .TN  ($bits(trg))
  ) osc (
    // streams
    .sti      (str_osc[i]),
    .sto      (str),
    // software events
    .evi      (evn),
    .evo      (evn.osc[i]),
    // trigger events
    .trg      (trg),
    .tro      (trg.osc[i]),
    // reset output
    .ctl_rst  (ctl_rst),
    // interrupts
    .irq      (irq.osc[i]),
    // System bus
    .bus      (sys[8+2*i+0])
  );

  // TODO: when DMA starts functioning properly, this module should be removed
  str2mm #(
    .DT  (DTO),
    .DL  (1<<14)
  ) str2mm (
    .ctl_rst  (ctl_rst),
    .str      (str),
    .bus      (sys[8+2*i+1])
  );

  assign srx_osc[i].TLAST  = str.TLAST ;
  assign srx_osc[i].TKEEP  = str.TKEEP ;
  assign srx_osc[i].TDATA  = str.TDATA ;
  assign srx_osc[i].TVALID = str.TVALID;
  // TREADY is ignored

end: for_osc
endgenerate

////////////////////////////////////////////////////////////////////////////////
// logic generator
////////////////////////////////////////////////////////////////////////////////

localparam EN_LG = 1;

generate
if (EN_LG) begin: if_lg

  lg #(
    .DN  (1),
    .DT  (DTL),
    .ER  (4),
    .EN  (top_pkg::MNS),
    .TN  ($bits(trg))
  ) lg (
    // stream output
    .sto      (str_lg),
    // software events
    .evi      (evn),
    .evo      (evn.lg),
    // trigger events
    .trg      (trg),
    .tro      (trg.lg),
    // interrupts
    .irq      (irq.lg),
    // System bus
    .bus      (sys[12+0]),
    .bus_tbl  (sys[12+1])
  );

end else begin

  sys_bus_stub sys_bus_stub_12 (sys[12]);
  sys_bus_stub sys_bus_stub_13 (sys[13]);

  assign str_lg.TLAST  = '0;
  assign str_lg.TKEEP  = '0;
  assign str_lg.TDATA  = '0;
  assign str_lg.TVALID = '0;
  // TREADY is ignored

  assign evn.lg = '0;
  assign irq.lg = 1'b0;

end
endgenerate

////////////////////////////////////////////////////////////////////////////////
// logic analyzer
////////////////////////////////////////////////////////////////////////////////

localparam EN_LA = 1;

generate
if (EN_LA) begin: if_la

  axi4_stream_if #(.DT (DTO)) str_tmp (.ACLK (str_la.ACLK), .ARESETn (str_la.ARESETn));

  logic ctl_rst;

  la #(
    .DN  (1),
    .DT  (DTL),
    .ER  (5),
    .EN  (top_pkg::MNS),
    .TN  ($bits(trg))
  ) la (
    // streams
    .sti      (str_la),
    .sto      (str_tmp),
    // software events
    .evi      (evn),
    .evo      (evn.la),
    // trigger events
    .trg      (trg),
    .tro      (trg.la),
    // interrupts
    .irq      (irq.la),
    // reset output
    .ctl_rst  (ctl_rst),
    // System bus
    .bus      (sys[14+0])
  );

  // TODO: when DMA starts functioning properly, this module should be removed
  str2mm #(
    .DT  (DTL),
    .DL  (1<<12)
  ) str2mm (
    .ctl_rst  (ctl_rst),
    .str      (str_tmp),
    .bus      (sys[14+1])
  );

  assign srx_la.TLAST  = str_tmp.TLAST ;
  assign srx_la.TKEEP  = str_tmp.TKEEP ;
  assign srx_la.TDATA  = str_tmp.TDATA ;
  assign srx_la.TVALID = str_tmp.TVALID;
  // TREADY is ignored

end else begin

  sys_bus_stub sys_bus_stub_14 (sys[14]);
  sys_bus_stub sys_bus_stub_15 (sys[15]);

  assign srx_la.TLAST  = '0;
  assign srx_la.TKEEP  = '0;
  assign srx_la.TDATA  = '0;
  assign srx_la.TVALID = '0;
  // TREADY is ignored

  assign evn.la = '0;
  assign irq.la = 1'b0;

end
endgenerate

////////////////////////////////////////////////////////////////////////////////
// complex trigger
////////////////////////////////////////////////////////////////////////////////

ctrg #(
  .ER  (6),
  .EN  (top_pkg::MNS),
  .TN  ($bits(trg))
) ctrg (
  // software events
  .evi      (evn),
  .evo      (evn.ctrg),
  // trigger events
  .trg      (trg),
  .tro      (trg.ctrg),
  // System bus
  .bus      (sys[16])
);

endmodule: red_pitaya_top
