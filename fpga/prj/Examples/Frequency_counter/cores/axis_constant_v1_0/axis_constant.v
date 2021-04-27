
`timescale 1 ns / 1 ps

module axis_constant #
(
  parameter integer AXIS_TDATA_WIDTH = 32
)
(
  // System signals
  input  wire                        aclk,

  input  wire [AXIS_TDATA_WIDTH-1:0] cfg_data,

  // Master side
  output wire [AXIS_TDATA_WIDTH-1:0] m_axis_tdata,
  output wire                        m_axis_tvalid
);

  assign m_axis_tdata = cfg_data;
  assign m_axis_tvalid = 1'b1;

endmodule
