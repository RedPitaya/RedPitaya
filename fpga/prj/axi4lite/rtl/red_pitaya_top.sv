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

// GPIO interface
gpio_if #(.DW (24)) gpio ();
gpio_if #(.DW (24)) gpio_dummy ();

logic gpio_irq;

////////////////////////////////////////////////////////////////////////////////
//  Connections to PS
////////////////////////////////////////////////////////////////////////////////

// system bus
axi4_lite_if bus (.ACLK (adc_clk), .ARESETn (adc_rstn));

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
  // interrupt
  .irq           (gpio_irq),

  // system read/write channel
  .bus           (bus)
);

////////////////////////////////////////////////////////////////////////////////
// LED and GPIO
////////////////////////////////////////////////////////////////////////////////

axi4lite_gpio #(
  .DW (32)
) axi4lite_gpio (
  .gpio_o (gpio.o),
  .gpio_t (gpio.t),
  .gpio_i (gpio.i),
  // interrupt
  .irq (gpio_irq),
  // bus
  .bus (bus)
);

IOBUF iobuf_led   [8-1:0] (.O(gpio.i[ 7: 0]), .IO(led_o)   , .I(gpio.o[ 7: 0]), .T(gpio.t[ 7: 0]));
IOBUF iobuf_exp_p [8-1:0] (.O(gpio.i[15: 8]), .IO(exp_p_io), .I(gpio.o[15: 8]), .T(gpio.t[15: 8]));
IOBUF iobuf_exp_n [8-1:0] (.O(gpio.i[23:16]), .IO(exp_n_io), .I(gpio.o[23:16]), .T(gpio.t[23:16]));

endmodule: red_pitaya_top
