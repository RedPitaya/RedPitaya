
`timescale 1 ns / 1 ps

module axis_red_pitaya_dac #
(
  parameter integer DAC_DATA_WIDTH = 14,
  parameter integer AXIS_TDATA_WIDTH = 32
)
(
  // PLL signals
  input  wire                        aclk,
  input  wire                        ddr_clk,
  input  wire                        locked,

  // DAC signals
  output wire                        dac_clk,
  output wire                        dac_rst,
  output wire                        dac_sel,
  output wire                        dac_wrt,
  output wire [DAC_DATA_WIDTH-1:0]   dac_dat,

  // Slave side
  output wire                        s_axis_tready,
  input  wire [AXIS_TDATA_WIDTH-1:0] s_axis_tdata,
  input  wire                        s_axis_tvalid
);

  reg [DAC_DATA_WIDTH-1:0] int_dat_a_reg;
  reg [DAC_DATA_WIDTH-1:0] int_dat_b_reg;
  reg int_rst_reg;

  wire [DAC_DATA_WIDTH-1:0] int_dat_a_wire;
  wire [DAC_DATA_WIDTH-1:0] int_dat_b_wire;

  assign int_dat_a_wire = s_axis_tdata[DAC_DATA_WIDTH-1:0];
  assign int_dat_b_wire = s_axis_tdata[AXIS_TDATA_WIDTH/2+DAC_DATA_WIDTH-1:AXIS_TDATA_WIDTH/2];

  genvar j;

  always @(posedge aclk)
  begin
    if(~locked | ~s_axis_tvalid)
    begin
      int_dat_a_reg <= {(DAC_DATA_WIDTH){1'b0}};
      int_dat_b_reg <= {(DAC_DATA_WIDTH){1'b0}};
    end
    else
    begin
      int_dat_a_reg <= {int_dat_a_wire[DAC_DATA_WIDTH-1], ~int_dat_a_wire[DAC_DATA_WIDTH-2:0]};
      int_dat_b_reg <= {int_dat_b_wire[DAC_DATA_WIDTH-1], ~int_dat_b_wire[DAC_DATA_WIDTH-2:0]};
    end
    int_rst_reg <= ~locked | ~s_axis_tvalid;
  end

  ODDR ODDR_rst(.Q(dac_rst), .D1(int_rst_reg), .D2(int_rst_reg), .C(aclk), .CE(1'b1), .R(1'b0), .S(1'b0));
  ODDR ODDR_sel(.Q(dac_sel), .D1(1'b0), .D2(1'b1), .C(aclk), .CE(1'b1), .R(1'b0), .S(1'b0));
  ODDR ODDR_wrt(.Q(dac_wrt), .D1(1'b0), .D2(1'b1), .C(ddr_clk), .CE(1'b1), .R(1'b0), .S(1'b0));
  ODDR ODDR_clk(.Q(dac_clk), .D1(1'b0), .D2(1'b1), .C(ddr_clk), .CE(1'b1), .R(1'b0), .S(1'b0));

  generate
    for(j = 0; j < DAC_DATA_WIDTH; j = j + 1)
    begin : DAC_DAT
      ODDR ODDR_inst(
        .Q(dac_dat[j]),
        .D1(int_dat_a_reg[j]),
        .D2(int_dat_b_reg[j]),
        .C(aclk),
        .CE(1'b1),
        .R(1'b0),
        .S(1'b0)
      );
    end
  endgenerate

  assign s_axis_tready = 1'b1;

endmodule
