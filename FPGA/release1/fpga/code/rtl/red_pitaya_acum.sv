////////////////////////////////////////////////////////////////////////////////
// (c) Iztok Jeras  iztok.jeras@gmail.com
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module red_pitaya_acum #(
  // input stream parameters
  parameter IDW = 14,          // input data width
  // accumulation parameters
  parameter ACW = 32,          // accumulation counter width
  parameter AMS = 2**14,       // accumulation memory size
  parameter AAW = $clog2(AMS), // accumulation address width
  // output stream parameters
  parameter ODW = IDW+ACW      // output data width
)(
  // system signals
  input  logic           clk,  // clock
  input  logic           rst,  // reset (synchronous for now)
  input  logic           clr,  // clear (synchronous)
  // configuration
  input  logic [ACW-1:0] cfg_cnt,  // accumulation count
  // control
  input  logic           ctl_run,  // accumulation running status
  // status
  input  logic           sts_end,  // end of sequence
  output logic [ACW-1:0] sts_cnt,  // accumulation count
  // input data stream
//input  logic           sti_tkeep ,
  input  logic           sti_tlast ,
  input  logic [IDW-1:0] sti_tdata ,
  input  logic           sti_tvalid,
  output logic           sti_tready,
  // output data stream
//output logic           sto_tkeep ,
  output logic           sto_tlast ,
  output logic [ODW-1:0] sto_tdata ,
  output logic           sto_tvalid,
  input  logic           sto_tready,
  // memmory interface (read only)
  input  logic           bus_ren,
  input  logic [AAW-1:0] bus_adr,
  output logic [ODW-1:0] bus_rdt
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// stream transfer
logic           sti_trnsfr;
logic           sto_trnsfr;

// accumulation counter
logic [ACW-1:0] acu_cnt;  // accumulation counter
logic           acu_end;  // counter reached the end
logic           acu_beg;  // counter at the beginning

// buffered data/last
logic           buf_tlast;
logic [IDW-1:0] buf_tdata;

// memory access
logic           mem_vld;  // memory valid
logic           mem_end;  // memory size end
logic [AAW-1:0] mem_adr;  // address counter
logic [AAW-1:0] mem_nxt;  // address next
logic [ODW-1:0] mem [0:AMS-1];
logic           mem_wen, mem_ren;  // write/read enable
logic [AAW-1:0] mem_wad, mem_rad;  // write/read address
logic [ODW-1:0] mem_wdt, mem_rdt;  // write/read data

////////////////////////////////////////////////////////////////////////////////
// I/O stream
////////////////////////////////////////////////////////////////////////////////

// stream transfer
assign sti_trnsfr = sti_tvalid & sti_tready;
assign sto_trnsfr = sto_tvalid & sto_tready;

// accumulation counter
always_ff @ (posedge clk) //, posedge rst)
if (rst)                            acu_cnt <=           0;
else begin
  if (clr)                          acu_cnt <=           0;
  else if (sti_trnsfr & sti_tlast)  acu_cnt <= acu_end ? 0 : acu_cnt + 1;
end

assign sts_cnt = acu_cnt;
assign acu_end = (acu_cnt == cfg_cnt);
assign acu_beg = (acu_cnt == 0);

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
if (rst)    sto_tvalid <= 1'b0;
else begin
  if (clr)  sto_tvalid <= 1'b0;
  else      sto_tvalid <= mem_wen & acu_end;
end

// output stream data
always_ff @ (posedge clk)
if (mem_wen & acu_end)  sto_tdata <= mem_wdt;

// end status pulse
assign sts_end = sti_trnsfr & sti_tlast & acu_end;

////////////////////////////////////////////////////////////////////////////////
// memory access
////////////////////////////////////////////////////////////////////////////////

// write data
//assign mem_wdt = ODW'($signed(buf_tdata)) + $signed(acu_beg ? ODW'(0) : mem_rdt);
assign mem_wdt = $signed(buf_tdata) + $signed(acu_beg ? 0 : mem_rdt);

// read enable
assign mem_vld = sti_trnsfr;
// address end
assign mem_end = sti_tlast;

// address
always_ff @ (posedge clk) //, posedge rst)
if (rst)             mem_adr <=           0;
else begin
  if (clr)           mem_adr <=           0;
  else if (mem_vld)  mem_adr <= mem_end ? 0 : mem_nxt;
end

// address next
assign mem_nxt = mem_adr + 1;

// write enable
always_ff @ (posedge clk) //, posedge rst)
if (rst)  mem_wen <= 1'b0;
else begin
  if (clr)  mem_wen <= 1'b0;
  else      mem_wen <= mem_vld;
end

// write address, is a delayed read address
always_ff @ (posedge clk)
if (mem_vld)  mem_wad <= mem_adr;

// write access
always_ff @ (posedge clk)
if (mem_wen)  mem [mem_wad] <= mem_wdt;

// read access
always_ff @ (posedge clk)
mem_rdt <= mem [mem_rad];
//if (mem_ren)  mem_rdt <= mem [mem_rad];

// read enable mux between stream and read bus
assign mem_ren = ctl_run ? mem_vld : bus_ren;

// read address mux between stream and read bus
assign mem_rad = ctl_run ? mem_adr : bus_adr;

// read bus data
assign bus_rdt = mem_rdt;

endmodule: red_pitaya_acum
