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
  input  logic [AAW-1:0] cfg_len,  // accumulation stream length
  // control
  input  logic           ctl_run,  // accumulation start pulse
  // status
  output logic           sts_run,  // end of sequence
  output logic [ACW-1:0] sts_cnt,  // accumulation count
  // input data stream
//input  logic           sti_tkeep ,
  input  logic           sti_t_trig,
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
  output logic [ODW-1:0] bus_rdt,
  output logic           bus_vld
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// stream transfer
logic           acu_valid;
logic           sti_trnsfr;
logic           sto_trnsfr;

// additional input stream signals
logic           sti_tlast ;
logic [AAW-1:0] sti_cnt;

logic           sts_end;  // end of sequence

// accumulation counter
logic [ACW-1:0] acu_cnt;  // accumulation counter
logic           acu_end;  // counter reached the end
logic           acu_lst;  // counter reached the end (registered version or last)
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
logic           mem_ten, mem_wen, mem_ren;  // tmp/write/read enable
logic [AAW-1:0] mem_tad, mem_wad, mem_rad;  // tmp/write/read address
logic [ODW-1:0] mem_tdt, mem_wdt, mem_rdt;  // tmp/write/read data
logic           mem_tcr;                    // addition tmp carry
logic           mem_tsg;                    // addition tmp sign
logic [ODW-1:IDW] mem_tts;

////////////////////////////////////////////////////////////////////////////////
// Input stream
////////////////////////////////////////////////////////////////////////////////

// stream transfer
assign acu_tvalid = sti_tvalid & sts_run & (sti_t_trig | (|sti_cnt));
assign sti_trnsfr = acu_tvalid & sti_tready;
assign sto_trnsfr = sto_tvalid & sto_tready;

// run status
always @(posedge clk) //, posedge rst)
if (rst)             sts_run <= 1'b0;
else begin
  if      (ctl_run)  sts_run <= 1'b1;
  else if (sts_end)  sts_run <= 1'b0;
end

// buffer length counter
always @(posedge clk) //, posedge rst)
if (rst)              sti_cnt <= 0;
else if (sti_trnsfr)  sti_cnt <= sti_tlast ? 0 : sti_cnt + 1;

assign sti_tlast = sti_cnt == cfg_len;

// accumulation counter
always_ff @ (posedge clk) //, posedge rst)
if (rst)                            acu_cnt <=           0;
else begin
  if (clr)                          acu_cnt <=           0;
  else if (sti_trnsfr & sti_tlast)  acu_cnt <= acu_end ? 0 : acu_cnt + 1;
end

assign sts_cnt = acu_cnt;
assign acu_end = (acu_cnt == cfg_cnt);

always_ff @ (posedge clk) //, posedge rst)
if (rst)    acu_beg <= 1'b0;
else begin
  if (clr)  acu_beg <= 1'b0;
  else      acu_beg <= (acu_cnt == 0);
end

// input data/last delayed
always_ff @ (posedge clk)
if (sti_trnsfr) begin
  buf_tlast <= sti_tlast;
  buf_tdata <= sti_tdata;
end

// end status pulse
assign sts_end = sti_trnsfr & sti_tlast & acu_end;

// input stream ready
assign sti_tready = sto_tready;

////////////////////////////////////////////////////////////////////////////////
// Output stream
////////////////////////////////////////////////////////////////////////////////

// output stream valid
always_ff @ (posedge clk) //, posedge rst)
if (rst)    sto_tvalid <= 1'b0;
else begin
  if (clr)  sto_tvalid <= 1'b0;
  else      sto_tvalid <= mem_wen & acu_lst;
end

// output stream data
always_ff @ (posedge clk)
if (mem_wen & acu_lst)  sto_tdata <= mem_wdt;

////////////////////////////////////////////////////////////////////////////////
// memory access
////////////////////////////////////////////////////////////////////////////////

// write data
//assign mem_wdt = ODW'($signed(buf_tdata)) + $signed(acu_beg ? ODW'(0) : mem_rdt);
always_ff @ (posedge clk)
begin
            mem_tdt [ODW-1:IDW]  <= (acu_beg ? 0 : mem_rdt [ODW-1:IDW])            ;
  {mem_tcr, mem_tdt [IDW-1:  0]} <= (acu_beg ? 0 : mem_rdt [IDW-1:  0]) + buf_tdata;
   mem_tsg                       <= buf_tdata[IDW-1];
end

assign mem_tts = $signed(mem_tsg) + $signed({1'b0, mem_tcr});

assign      mem_wdt [ODW-1:IDW]   = mem_tdt [ODW-1:IDW] + mem_tts;
assign      mem_wdt [IDW-1:  0]   = mem_tdt [IDW-1:  0]          ;

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
if (rst) begin
  mem_ten <= 1'b0;
  mem_wen <= 1'b0;
end else begin
  if (clr)  mem_ten <= 1'b0;
  else      mem_ten <= mem_vld;
  if (clr)  mem_wen <= 1'b0;
  else      mem_wen <= mem_ten;
end

// write address, is a delayed read address
always_ff @ (posedge clk)
begin
  if (mem_vld)  mem_tad <= mem_adr;
  if (mem_ten)  mem_wad <= mem_tad;
end

// write access
always_ff @ (posedge clk)
if (mem_wen)  mem [mem_wad] <= mem_wdt;

// read access
always_ff @ (posedge clk)
mem_rdt <= mem [mem_rad];
//if (mem_ren)  mem_rdt <= mem [mem_rad];

// read enable mux between stream and read bus
assign mem_ren = sts_run ? mem_vld : bus_ren;

// read address mux between stream and read bus
assign mem_rad = sts_run ? mem_adr : bus_adr;

// read bus data
assign bus_rdt = mem_rdt;

always_ff @ (posedge clk) //, posedge rst)
if (rst)  bus_vld <= 1'b0;
else      bus_vld <= bus_ren;

endmodule: red_pitaya_acum
