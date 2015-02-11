module red_pitaya_acum #(
  // input stream parameters
  int unsigned IDW = 14,         // input data width
  int unsigned IDN = 1,          // input data number (paralel processing of multiple samples)
  // accumulation parameters
  int unsigned ACW = 32,         // accumulation counter width
  int unsigned AMS = 2*14,       // accumulation memory size
  int unsigned AAW = $clog2(MS), // accumulation address width
  // output stream parameters
  int unsigned ODW = IDW+ACW     // output data width
  int unsigned ODW = IDN+ACW     // output data number
)(
  // system signals
  input  logic                     clk,
  input  logic                     rst,
  // configuration
  input  logic            [CW-1:0] cfg_cnt,  // accumulation count
  input  logic            [AW-1:0] cfg_len,  // buffer length
  // control
  // input data stream
  input  logic                     sti_tkeep ,
  input  logic                     sti_tlast ,
  input  logic [IDN-1:0] [IDW-1:0] sti_tdata ,
  input  logic                     sti_tvalid,
  output logic                     sti_tready,
  // output data stream
  input  logic                     sto_tkeep ,
  input  logic                     sto_tlast ,
  input  logic [ODN-1:0] [ODW-1:0] sto_tdata ,
  input  logic                     sto_tvalid,
  output logic                     sto_tready,
);

logic [ODN-1:0] [ODW-1:0] mem [0:AMS/IDN-1];



endmodule: red_pitaya_acum
