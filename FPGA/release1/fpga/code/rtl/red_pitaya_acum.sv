////////////////////////////////////////////////////////////////////////////////
// (c) Iztok Jeras  iztok.jeras@gmail.com
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module red_pitaya_acum #(
  // input stream parameters
  int unsigned IDW = 14,          // input data width
  int unsigned IDN = 1,           // input data number (paralel processing of multiple samples)
  // accumulation parameters
  int unsigned ACW = 32,          // accumulation counter width
  int unsigned AMS = 2*14,        // accumulation memory size
  int unsigned AAW = $clog2(AMS), // accumulation address width
  // output stream parameters
  int unsigned ODW = IDW+ACW,     // output data width
  int unsigned ODN = IDN          // output data number
)(
  // system signals
  input  logic                     clk,  // clock
  input  logic                     rst,  // reset (synchronous for now)
  input  logic                     clr,  // clear (synchronous)
  // configuration
  input  logic           [ACW-1:0] cfg_cnt,  // accumulation count
  input  logic           [AAW-1:0] cfg_len,  // buffer length
  // input data stream
//input  logic                     sti_tkeep ,
  input  logic                     sti_tlast ,
  input  logic [IDN-1:0] [IDW-1:0] sti_tdata ,
  input  logic                     sti_tvalid,
  output logic                     sti_tready,
  // output data stream
//output logic                     sto_tkeep ,
  output logic                     sto_tlast ,
  output logic [ODN-1:0] [ODW-1:0] sto_tdata ,
  output logic                     sto_tvalid,
  input  logic                     sto_tready
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// stream transfer
logic           sti_trnsfr;
logic           sto_trnsfr;

// accumulation counter
logic [ACW-1:0] acu_cnt;
logic           acu_end;  // counter reached the end
logic           acu_beg;  // counter at the beginning

// buffered data/last
logic                     buf_tlast;
logic [IDN-1:0] [IDW-1:0] buf_tdata;

// memory access
logic                     mem_end;  // memory size end
logic           [AAW-1:0] mem_nxt;  // address next
logic [ODN-1:0] [ODW-1:0] mem [0:AMS/IDN-1];
logic                     mem_wen, mem_ren;  // write/read enable
logic           [AAW-1:0] mem_wad, mem_rad;  // write/read address
logic [ODN-1:0] [ODW-1:0] mem_wdt, mem_rdt;  // write/read data

////////////////////////////////////////////////////////////////////////////////
// I/O stream
////////////////////////////////////////////////////////////////////////////////

// stream transfer
assign sti_trnsfr = sti_tvalid & sti_tready;
assign sto_trnsfr = sto_tvalid & sto_tready;

// accumulation counter
always_ff @ (posedge clk) //, posedge rst)
if (rst)                          acu_cnt <=           ACW'(0);
else if (sti_trnsfr & sti_tlast)  acu_cnt <= acu_end ? ACW'(0) : acu_cnt + ACW'(1);

assign acu_end = (acu_cnt == cfg_cnt);
assign acu_beg = (acu_cnt == ACW'(0));

// input data/last delayed
always_ff @ (posedge clk)
if (sti_trnsfr) begin
  buf_tlast <= sti_tlast;
  buf_tdata <= sti_tdata;
end

// input stream ready
assign sti_tready = sto_tready;

// output stream valid
always_ff @ (posedge clk) //, posedge rst)
if (rst)  sto_tvalid <= 1'b0;
else      sto_tvalid <= mem_wen & acu_end;

// output stream data
always_ff @ (posedge clk)
if (mem_wen & acu_end)  sto_tdata <= mem_wdt;

////////////////////////////////////////////////////////////////////////////////
// memory access
////////////////////////////////////////////////////////////////////////////////

// write data
assign mem_wdt = ODW'($signed(buf_tdata)) + $signed(acu_beg ? ODW'(0) : mem_rdt);

// read enable
assign mem_ren = sti_trnsfr;

// address
always_ff @ (posedge clk) //, posedge rst)
if (rst)           mem_rad <=           AAW'(0);
else if (mem_ren)  mem_rad <= mem_end ? AAW'(0) : mem_nxt;

assign mem_nxt = mem_rad + AAW'(1);
assign mem_end = mem_nxt == cfg_len;

// write enable
always_ff @ (posedge clk) //, posedge rst)
if (rst)  mem_wen <= 1'b0;
else      mem_wen <= mem_ren;

// write address, is a delayed read address
always_ff @ (posedge clk)
if (mem_ren)  mem_wad <= mem_rad;

// write access
always_ff @ (posedge clk)
if (mem_wen)  mem [mem_wad] <= mem_wdt;

// read access
always_ff @ (posedge clk)
if (mem_ren)  mem [mem_rad] <= mem_rdt;

endmodule: red_pitaya_acum
