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
 *                            \-------/     
 *
 * Inside analog module, ADC data is translated from unsigned neg-slope into
 * two's complement. Similar is done on DAC data.
 *
 * Scope module stores data from ADC into RAM, arbitrary signal generator (ASG)
 * sends data from RAM to DAC. MIMO PID uses ADC ADC as input and DAC as its output.
 *
 * Daisy chain connects with other boards with fast serial link. Data which is
 * send and received is at the moment undefined. This is left for the user.
 * 
 */

module red_pitaya_top #(
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
  output logic                    adc_cdcs_o ,  // ADC clock duty cycle stabilizer
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
  output logic [ 8-1:0] led_o       
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

logic [  4-1: 0] fclk               ; //[0]-125MHz, [1]-250MHz, [2]-50MHz, [3]-200MHz
logic [  4-1: 0] frstn              ;

logic [ 32-1: 0] ps_sys_addr        ;
logic [ 32-1: 0] ps_sys_wdata       ;
logic [  4-1: 0] ps_sys_sel         ;
logic            ps_sys_wen         ;
logic            ps_sys_ren         ;
logic [ 32-1: 0] ps_sys_rdata       ;
logic            ps_sys_err         ;
logic            ps_sys_ack         ;

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
logic                           adc_clk;
logic                           adc_rstn;
// ADC signals
logic signed [MNA-1:0] [14-1:0] adc_dat;
logic        [MNA-1:0]          adc_vld;
logic        [MNA-1:0]          adc_rdy;
// acquire signals
logic signed [MNA-1:0] [14-1:0] acq_dat;
logic        [MNA-1:0]          acq_lst;
logic        [MNA-1:0]          acq_vld;
logic        [MNA-1:0]          acq_rdy;
// DAC signals
logic                           dac_clk_1x;
logic                           dac_clk_2x;
logic                           dac_clk_2p;
logic                           dac_rst;
logic        [MNG-1:0] [14-1:0] dac_dat;
logic signed [MNG-1:0] [14-1:0] dac_dat_cal;
// ASG
logic signed [MNG-1:0] [14-1:0] asg_dat;
// PID
logic signed [MNG-1:0] [14-1:0] pid_dat;

localparam int unsigned DWM = 16;
localparam int unsigned DWS = 14;

// configuration
logic                 digital_loop;
// ADC calibration
logic signed [MNA-1:0] [DWM-1:0] adc_cfg_mul;  // gain
logic signed [MNA-1:0] [DWS-1:0] adc_cfg_sum;  // offset
// DAC calibration
logic signed [MNG-1:0] [DWM-1:0] dac_cfg_mul;  // gain
logic signed [MNG-1:0] [DWS-1:0] dac_cfg_sum;  // offset

// triggers
struct packed { 
  // generator
  logic [MNG  -1:0] gen_out;  // MNA*2 - event    triggers from acquire    {negedge, posedge}
  logic [MNG  -1:0] gen_swo;  // MNA   - software triggers from acquire
  // acquire
  logic [MNA*2-1:0] acq_out;  // MNG   - event    triggers from generators
  logic [MNA  -1:0] acq_swo;  // MNG   - software triggers from generators
  // GPIO
  logic [    2-1:0] gio_out;  // 2     - event    triggers from GPIO       {negedge, posedge}
} trg; 


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
  .clk           (adc_clk     ),
  .rstn          (adc_rstn    ),
  .fclk_clk_o    (fclk        ),
  .fclk_rstn_o   (frstn       ),
  // ADC analog inputs
  .vinp_i        (vinp_i      ),
  .vinn_i        (vinn_i      ),
   // system read/write channel
  .sys_addr      (ps_sys_addr ),
  .sys_wdata     (ps_sys_wdata),
  .sys_sel       (ps_sys_sel  ),
  .sys_wen       (ps_sys_wen  ),
  .sys_ren       (ps_sys_ren  ),
  .sys_rdata     (ps_sys_rdata),
  .sys_err       (ps_sys_err  ),
  .sys_ack       (ps_sys_ack  ),
  // AXI masters
  // TODO, handle this bitsize change elsewhere
  .axi1_tdata  (16'(acq_dat[1]<<<2)),  .axi0_tdata  (16'(acq_dat[0]<<<2)),
  .axi1_tlast  (acq_lst[1]),  .axi0_tlast  (acq_lst[0]),
  .axi1_tvalid (acq_vld[1]),  .axi0_tvalid (acq_vld[0]),
  .axi1_tready (acq_rdy[1]),  .axi0_tready (acq_rdy[0])
);

////////////////////////////////////////////////////////////////////////////////
// system bus decoder & multiplexer (it breaks memory addresses into 8 regions)
////////////////////////////////////////////////////////////////////////////////

logic        [32-1:0] sys_addr  = ps_sys_addr ;
logic        [32-1:0] sys_wdata = ps_sys_wdata;
logic        [ 4-1:0] sys_sel   = ps_sys_sel  ;
logic [8-1:0]         sys_wen   ;
logic [8-1:0]         sys_ren   ;
logic [8-1:0][32-1:0] sys_rdata ;
logic [8-1:0]         sys_err   ;
logic [8-1:0]         sys_ack   ;
logic [8-1:0]         sys_cs    ;

assign sys_cs = 8'h01 << sys_addr[22:20];

assign sys_wen = sys_cs & {8{ps_sys_wen}};
assign sys_ren = sys_cs & {8{ps_sys_ren}};

assign ps_sys_rdata = sys_rdata[sys_addr[22:20]];
assign ps_sys_err   = sys_err  [sys_addr[22:20]];
assign ps_sys_ack   = sys_ack  [sys_addr[22:20]];

////////////////////////////////////////////////////////////////////////////////
// Housekeeping
////////////////////////////////////////////////////////////////////////////////

logic [8-1:0] exp_p_i , exp_n_i ;
logic [8-1:0] exp_p_o , exp_n_o ;
logic [8-1:0] exp_p_oe, exp_n_oe;

red_pitaya_hk hk (
  // system signals
  .clk           (adc_clk ),
  .rstn          (adc_rstn),
  // LED
  .led_o         (led_o),
  // global configuration
  .digital_loop  (digital_loop),
  // Expansion connector
  .exp_p_i       (exp_p_i ),
  .exp_p_o       (exp_p_o ),
  .exp_p_oe      (exp_p_oe),
  .exp_n_i       (exp_n_i ),
  .exp_n_o       (exp_n_o ),
  .exp_n_oe      (exp_n_oe),
   // System bus
  .sys_addr      (sys_addr    ),
  .sys_wdata     (sys_wdata   ),
  .sys_sel       (sys_sel     ),
  .sys_wen       (sys_wen  [0]),
  .sys_ren       (sys_ren  [0]),
  .sys_rdata     (sys_rdata[0]),
  .sys_err       (sys_err  [0]),
  .sys_ack       (sys_ack  [0]) 
);

IOBUF i_iobufp [8-1:0] (.O(exp_p_i), .IO(exp_p_io), .I(exp_p_o), .T(~exp_p_oe));
IOBUF i_iobufn [8-1:0] (.O(exp_n_i), .IO(exp_n_io), .I(exp_n_o), .T(~exp_n_oe));

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
  .d_i  (exp_p_i[0]),
  .d_o  (),
  .d_p  (trg.gio_out[0]),
  .d_n  (trg.gio_out[1])
);

////////////////////////////////////////////////////////////////////////////////
// Calibration
////////////////////////////////////////////////////////////////////////////////

red_pitaya_calib calib (
  // system signals
  .clk           (adc_clk ),
  .rstn          (adc_rstn),
  // ADC calibration
  .adc_cfg_mul   (adc_cfg_mul),
  .adc_cfg_sum   (adc_cfg_sum),
  // DAC calibration
  .dac_cfg_mul   (dac_cfg_mul),
  .dac_cfg_sum   (dac_cfg_sum),
   // System bus
  .sys_addr      (sys_addr    ),
  .sys_wdata     (sys_wdata   ),
  .sys_sel       (sys_sel     ),
  .sys_wen       (sys_wen  [1]),
  .sys_ren       (sys_ren  [1]),
  .sys_rdata     (sys_rdata[1]),
  .sys_err       (sys_err  [1]),
  .sys_ack       (sys_ack  [1]) 
);

////////////////////////////////////////////////////////////////////////////////
// Analog mixed signals (PDM analog outputs)
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned PDM_CHN = 4;
localparam int unsigned PDM_DWC = 8;

logic [PDM_CHN-1:0] [PDM_DWC-1:0] pdm_cfg;

red_pitaya_ams #(
  .DWC (PDM_DWC),
  .CHN (PDM_CHN)
) ams (
  // system signals
  .clk        (adc_clk ),
  .rstn       (adc_rstn),
  // PDM configuration
  .pdm_cfg    (pdm_cfg),
  // system bus
  .sys_addr   (sys_addr    ),
  .sys_wdata  (sys_wdata   ),
  .sys_sel    (sys_sel     ),
  .sys_wen    (sys_wen  [2]),
  .sys_ren    (sys_ren  [2]),
  .sys_rdata  (sys_rdata[2]),
  .sys_err    (sys_err  [2]),
  .sys_ack    (sys_ack  [2])
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
// Daisy dummy code
////////////////////////////////////////////////////////////////////////////////

assign daisy_p_o = 1'bz;
assign daisy_n_o = 1'bz;

////////////////////////////////////////////////////////////////////////////////
//  MIMO PID controller
////////////////////////////////////////////////////////////////////////////////

red_pitaya_pid #(
  .CNI (MNA),
  .CNO (MNG)
) pid (
  // system signals
  .clk        (adc_clk ),
  .rstn       (adc_rstn),
  // signals
  .dat_i      (adc_dat),
  .dat_o      (pid_dat),
  // System bus
  .sys_addr   (sys_addr    ),
  .sys_wdata  (sys_wdata   ),
  .sys_sel    (sys_sel     ),
  .sys_wen    (sys_wen  [3]),
  .sys_ren    (sys_ren  [3]),
  .sys_rdata  (sys_rdata[3]),
  .sys_err    (sys_err  [3]),
  .sys_ack    (sys_ack  [3])
);

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
  logic signed [14-1:0] adc_dat_mux;

  // IO block registers should be used here
  // lowest 2 bits reserved for 16bit ADC
  always_ff @(posedge adc_clk)
  adc_dat_raw <= {adc_dat_i[i][16-1], ~adc_dat_i[i][16-2:2]};
    
  // transform into 2's complement (negative slope)
  assign adc_dat_mux[0] = digital_loop ? dac_dat_cal[i] : adc_dat_raw;

  linear #(
    .DWI  (14),
    .DWO  (14),
    .DWM  (16)
  ) linear_adc (
    // system signals
    .clk      (adc_clk ),
    .rstn     (adc_rstn),
    // input stream
    .sti_dat  (adc_dat_mux),
    .sti_vld  (1'b1),
    .sti_rdy  (),
    // output stream
    .sto_dat  (adc_dat[i]),
    .sto_vld  (adc_vld[i]),
    .sto_rdy  (adc_rdy[i]),
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

  logic signed [15-1:0] dac_dat_sum;
  logic signed [14-1:0] dac_dat_sat;

  // Sumation of ASG and PID signal perform saturation before sending to DAC 
  // TODO: there should be a proper metod to disable PID
  assign dac_dat_sum = asg_dat[i]; // + pid_dat[0];

  // saturation
  assign dac_dat_sat = (^dac_dat_sum[15-1:15-2]) ? {dac_dat_sum[15-1], {13{~dac_dat_sum[15-1]}}} : dac_dat_sum[14-1:0];

  linear #(
    .DWI  (14),
    .DWO  (14),
    .DWM  (16)
  ) linear_dac (
    // system signals
    .clk      (adc_clk ),
    .rstn     (adc_rstn),
    // input stream
    .sti_dat  (dac_dat_sat),
    .sti_vld  (1'b1),
    .sti_rdy  (),
    // output stream
    .sto_dat  (dac_dat_cal[i]),
    .sto_vld  (),
    .sto_rdy  (1'b1),
    // configuration
    .cfg_mul  (dac_cfg_mul[i]),
    .cfg_sum  (dac_cfg_sum[i])
  );

  // output registers + signed to unsigned (also to negative slope)
  assign dac_dat[i] = {dac_dat_cal[i][14-1], ~dac_dat_cal[i][14-2:0]};

end: for_dac
endgenerate

// DDR outputs
ODDR oddr_dac_clk          (.Q(dac_clk_o), .D1(1'b0      ), .D2(1'b1      ), .C(dac_clk_2p), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_wrt          (.Q(dac_wrt_o), .D1(1'b0      ), .D2(1'b1      ), .C(dac_clk_2x), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_sel          (.Q(dac_sel_o), .D1(1'b1      ), .D2(1'b0      ), .C(dac_clk_1x), .CE(1'b1), .R(dac_rst), .S(1'b0));
ODDR oddr_dac_rst          (.Q(dac_rst_o), .D1(dac_rst   ), .D2(dac_rst   ), .C(dac_clk_1x), .CE(1'b1), .R(1'b0   ), .S(1'b0));
ODDR oddr_dac_dat [14-1:0] (.Q(dac_dat_o), .D1(dac_dat[0]), .D2(dac_dat[1]), .C(dac_clk_1x), .CE(1'b1), .R(dac_rst), .S(1'b0));

////////////////////////////////////////////////////////////////////////////////
// ASG (arbitrary signal generators)
////////////////////////////////////////////////////////////////////////////////

asg_top #(
  .TWA ($bits(trg))
) asg [MNG-1:0] (
  // system signals
  .clk       (adc_clk ),
  .rstn      (adc_rstn),
  // stream output
  .sto_dat   (asg_dat),
  .sto_vld   (),
  .sto_rdy   (1'b1),
  // triggers
  .trg_ext   (trg),
  .trg_swo   (trg.gen_swo),
  .trg_out   (trg.gen_out),
  // System bus
  .sys_sel   (sys_sel       ),
  .sys_wen   (sys_wen  [5:4]),
  .sys_ren   (sys_ren  [5:4]),
  .sys_addr  (sys_addr      ),
  .sys_wdata (sys_wdata     ),
  .sys_rdata (sys_rdata[5:4]),
  .sys_err   (sys_err  [5:4]),
  .sys_ack   (sys_ack  [5:4])
);

////////////////////////////////////////////////////////////////////////////////
//  Oscilloscope application
////////////////////////////////////////////////////////////////////////////////

scope_top #(
  .TWA ($bits(trg))
) scope [MNA-1:0] (
  // system signals
  .clk           (adc_clk ),
  .rstn          (adc_rstn),
  // stream input
  .sti_dat       (adc_dat),
  .sti_vld       (adc_vld),
  .sti_rdy       (adc_rdy),
  // stream_output
  .sto_dat       (acq_dat),
  .sto_lst       (acq_lst),
  .sto_vld       (acq_vld),
  .sto_rdy       (acq_rdy),
  // triggers
  .trg_ext       (trg),
  .trg_swo       (trg.acq_swo),
  .trg_out       (trg.acq_out),
 // System bus
  .sys_sel       (sys_sel       ),
  .sys_wen       (sys_wen  [7:6]),
  .sys_ren       (sys_ren  [7:6]),
  .sys_addr      (sys_addr      ),
  .sys_wdata     (sys_wdata     ),
  .sys_rdata     (sys_rdata[7:6]),
  .sys_err       (sys_err  [7:6]),
  .sys_ack       (sys_ack  [7:6])
);

endmodule: red_pitaya_top
