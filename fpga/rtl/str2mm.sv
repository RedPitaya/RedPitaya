////////////////////////////////////////////////////////////////////////////////
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module str2mm #(
  type DT = logic signed [16-1:0],
  int unsigned DN = 1<<14
)(
   // ADC
   input  logic                 adc_clk_i ,  // ADC clock
   input  logic                 adc_rstn_i,  // ADC reset - active low
   input  logic signed [14-1:0] adc_a_i   ,  // ADC data CHA
   // System bus
   input  logic          sys_wen   ,  // bus write enable
   input  logic          sys_ren   ,  // bus read enable
   input  logic [32-1:0] sys_addr  ,  // bus saddress
   input  logic [32-1:0] sys_wdata ,  // bus write data
   output logic [32-1:0] sys_rdata ,  // bus read data
   output logic          sys_err   ,  // bus error indicator
   output logic          sys_ack      // bus acknowledge signal
);

localparam int unsigned AW = $clog2(DN);

// memory
DT             buf_dat [0:DN-1];
// write port
logic          buf_wen;
logic [AW-1:0] buf_wad;

////////////////////////////////////////////////////////////////////////////////
// write
////////////////////////////////////////////////////////////////////////////////

assign buf_wen = str.TVALID & str.TREADY;

assign str.TREADY = 1;

always @(posedge str.ACLK)
if (~str.ARESETn) begin
  buf_wad <= '0;
end else begin
  buf_wad <= buf_wad + buf_wen;
end

always @(posedge str.ACLK)
if (buf_wen)
   buf_dat [buf_wad] <= str.TDATA;

////////////////////////////////////////////////////////////////////////////////
// read
////////////////////////////////////////////////////////////////////////////////

always @(posedge str.ACLK)
if (bus.ren)
  bus.rdata <= buf_dat [bus.addr];

endmodule: str2mm
