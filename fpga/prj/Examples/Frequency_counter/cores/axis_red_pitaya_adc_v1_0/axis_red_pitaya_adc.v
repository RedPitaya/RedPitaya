
`timescale 1 ns / 1 ps

module axis_red_pitaya_adc #
(
  parameter integer ADC_DATA_WIDTH = 14,
  parameter integer AXIS_TDATA_WIDTH = 32
)
(
  // System signals
  output wire                        adc_clk,

  // ADC signals
  output wire                        adc_csn,
  input  wire                        adc_clk_p,
  input  wire                        adc_clk_n,
  input  wire [ADC_DATA_WIDTH-1:0]   adc_dat_a,
  input  wire [ADC_DATA_WIDTH-1:0]   adc_dat_b,

  // Master side
  output wire                        m_axis_tvalid,
  output wire [AXIS_TDATA_WIDTH-1:0] m_axis_tdata
);
  localparam PADDING_WIDTH = AXIS_TDATA_WIDTH/2 - ADC_DATA_WIDTH;

  reg  [ADC_DATA_WIDTH-1:0] int_dat_a_reg;
  reg  [ADC_DATA_WIDTH-1:0] int_dat_b_reg;
  wire                      int_clk0;
  wire 						int_clk;

  IBUFGDS adc_clk_inst0 (.I(adc_clk_p), .IB(adc_clk_n), .O(int_clk0));
  BUFG adc_clk_inst (.I(int_clk0), .O(int_clk));

  always @(posedge int_clk)
  begin
    int_dat_a_reg <= adc_dat_a;
    int_dat_b_reg <= adc_dat_b;
  end

  assign adc_clk = int_clk;

  assign adc_csn = 1'b1;

  assign m_axis_tvalid = 1'b1;

  assign m_axis_tdata = {
    {(PADDING_WIDTH+1){int_dat_b_reg[ADC_DATA_WIDTH-1]}}, ~int_dat_b_reg[ADC_DATA_WIDTH-2:0],
    {(PADDING_WIDTH+1){int_dat_a_reg[ADC_DATA_WIDTH-1]}}, ~int_dat_a_reg[ADC_DATA_WIDTH-2:0]};

endmodule
