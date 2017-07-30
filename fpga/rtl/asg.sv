////////////////////////////////////////////////////////////////////////////////
// Arbitrary signal generator. Holds table and FSM for one channel.
// Author: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// GENERAL DESCRIPTION:
//
// Arbitrary signal generator takes data stored in buffer and sends them to DAC.
//
//
//                /-----\
//   SW --------> | BUF | ---> output
//          |     \-----/
//          |        ^
//          |        |
//          |     /-----\
//          ----> |     |
//                | FSM | ---> trigger notification
//   trigger ---> |     |
//                \-----/
//
//
// Submodule for ASG which hold buffer data and control registers for one channel.
//
////////////////////////////////////////////////////////////////////////////////

module asg #(
  // data bus
  int unsigned DN = 1,
  type DT = logic [8-1:0],
  // continuous/periodic buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16,  // counter width fraction  (fixed point fraction)
  type CT = logic [CWM-1:-CWF],
  // burst counter parameters
  int unsigned CWR = 14,  // counter width repetitions
  int unsigned CWL = 32,  // counter width length
  int unsigned CWN = 16   // counter width number
)(
  // stream output
  axi4_stream_if.s       sto    ,
  // events input/output
  input  evn_pkg::evn_t  evn    ,  // input
  output evn_pkg::evn_t  evs    ,  // output
  // control/status trigger
  input  logic           ctl_trg,
  input  logic           cfg_tre,
  // events
  output logic           evo_per,  // period
  output logic           evo_lst,  // last
  // generator mode
  input  logic           cfg_mod,  // burst enable
  input  logic           cfg_inf,  // infinite
  // continuous/periodic configuration
  input  CT              cfg_siz,  // data table size
  input  CT              cfg_ste,  // pointer step    size
  input  CT              cfg_off,  // pointer initial offset (used to define phase)
  // burst configuration (burst mode)
  input  logic [CWR-1:0] cfg_bdr,  // burst data   repetitions
  input  logic [CWM-1:0] cfg_bdl,  // burst data   length
  input  logic [CWL-1:0] cfg_bpl,  // burst period length
  input  logic [CWN-1:0] cfg_bpn,  // burst period number
  // status
  output logic [CWL-1:0] sts_bpl,  // burst period length counter
  output logic [CWN-1:0] sts_bpn,  // burst period number counter
  // System bus
  sys_bus_if.s               bus
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// buffer
DT              buf_mem [0:2**CWM-1];
DT    [2  -1:0] buf_rdata;            // read data
logic [CWM-1:0] buf_raddr;            // read address
logic [2  -1:0] buf_adr_vld;          // valid (read data enable)
logic [2  -1:0] buf_adr_lst;          // last

// bus
// additional register added to optimize timing
DT              bus_rdata;   // read data
logic           bus_ren;     // read enable 

// backpressure (TODO: it is not implemented properly)
logic           sts_rdy;      // ready

////////////////////////////////////////////////////////////////////////////////
// table RAM CPU access
////////////////////////////////////////////////////////////////////////////////

logic bus_ena;
assign bus_ena = bus.wen | bus_ren;

// CPU read/write access
always_ff @(posedge bus.clk)
begin
  bus_rdata <= buf_mem [bus.addr[2+:CWM]];
  if (bus_ren)  bus.rdata <= bus_rdata;
  if (bus.wen)  buf_mem [bus.addr[2+:CWM]] <= bus.wdata;
end
// TODO: asymetric bus width is failing synthesis
//for (int unsigned i=0; i<2; i++) begin
//  if (bus_ena) begin
//                  bus.rdata [16*i+:16] <= buf_mem [{bus.addr[2+:CWM-1],i[0]}];
//    if (bus.wen)  buf_mem [{bus.addr[2+:CWM-1],i[0]}] <= bus.wdata [16*i+:16];
//  end
//end

always_ff @(posedge bus.clk)
bus_ren <= bus.ren;

// CPU control signals
always_ff @(posedge bus.clk)
if (~bus.rstn)  bus.ack <= 1'b0;
else            bus.ack <= bus_ena;

assign bus.err = 1'b0;

////////////////////////////////////////////////////////////////////////////////
// table RAM stream read
////////////////////////////////////////////////////////////////////////////////

// stream read data
always_ff @(posedge sto.ACLK)
begin
  buf_rdata[0] <= buf_mem[buf_raddr];
  if (buf_adr_vld[1])  buf_rdata[1] <= buf_rdata[0] ;
end

// stream read pointer
always_ff @(posedge sto.ACLK)
if (str_adr.TVALID & str_adr.TKEEP)  buf_raddr <= str_adr.TDATA;

// valid signal used to enable memory read access
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  buf_adr_vld <= 2'b0;
  buf_adr_lst <= 2'b0;
end else begin
  if (evn.rst) begin
    buf_adr_vld <= 2'b0;
    buf_adr_lst <= 2'b0;
  end else begin
    buf_adr_vld[1] <= buf_adr_vld[0];
    buf_adr_lst[1] <= buf_adr_lst[0];
    if (sts_rdy) begin
      buf_adr_vld[0] <= str_adr.TVALID;
      buf_adr_lst[0] <= str_adr.TLAST;
    end
  end
end

////////////////////////////////////////////////////////////////////////////////
// output stream
////////////////////////////////////////////////////////////////////////////////

assign evo_lst = buf_adr_lst[1];

// output data
assign sto.TDATA = buf_rdata[1];

// output keep/last
always_ff @(posedge sto.ACLK)
if (sts_rdy) begin
  sto.TKEEP <= '1;
  sto.TLAST <= buf_adr_lst[1];
end

// output valid
always_ff @(posedge sto.ACLK)
if (~sto.ARESETn) begin
  sto.TVALID <= 1'b0;
end else begin
  // synchronous clear
  if (evn.rst) begin
    sto.TVALID <= 1'b0;
  end else if (sts_rdy) begin
    sto.TVALID <= buf_adr_vld[1];
  end
end

assign sts_rdy = sto.TREADY | ~sto.TVALID;

////////////////////////////////////////////////////////////////////////////////
// mode switch and engine instances
////////////////////////////////////////////////////////////////////////////////

// event multiplexer
evn_pkg::evn_t evn_per, evs_per;
evn_pkg::evn_t evn_bst, evs_bst;

localparam int unsigned AN = DN;
localparam type AT = logic [CWM-1:0];

// streams
axi4_stream_if #(.DN (AN), .DT (AT)) str_adr       (.ACLK (sto.ACLK), .ARESETn (sto.ARESETn));
axi4_stream_if #(.DN (AN), .DT (AT)) str_mux [1:0] (.ACLK (sto.ACLK), .ARESETn (sto.ARESETn));

// event multiplexer
assign evs = cfg_mod ? evs_bst : evs_per;

assign evn_per = ~cfg_mod ? evn : '0;
assign evn_bst =  cfg_mod ? evn : '0;

assign str_adr.TREADY = sts_rdy;

// address stream multiplexer
axi4_stream_mux #(
  .SN (2),
  .DT (AT)
) mux_mode (
  // control
  .sel  (cfg_mod),
  // streams
  .sti  (str_mux),
  .sto  (str_adr)
);

asg_per #(
  .AN (AN),
  .AT (AT),
  // buffer parameters
  .CWM (CWM),
  .CWF (CWF)
) asg_per (
  // stream output
  .sto      (str_mux[0]),
  // event control/status
  .evn      (evn_per),
  .evs      (evs_per),
  // trigger
  .ctl_trg  (ctl_trg),
  .cfg_tre  (cfg_tre),
  // configuration
  .cfg_siz  (cfg_siz),
  .cfg_off  (cfg_off),
  .cfg_ste  (cfg_ste)
);

asg_bst #(
  .AN (AN),
  .AT (AT),
  // burst counters
  .CWR (CWR),
  .CWL (CWL),
  .CWN (CWN)
) asg_bst (
  // stream output
  .sto      (str_mux[1]),
  // event control/status
  .evn      (evn_bst),
  .evs      (evs_bst),
  // trigger
  .ctl_trg  (ctl_trg),
  .cfg_tre  (cfg_tre),
  // events
  .evn_per  (evo_per),
  // generator mode
  .cfg_inf  (cfg_inf),
  // configuration
  .cfg_bdr  (cfg_bdr),
  .cfg_bdl  (cfg_bdl),
  .cfg_bpl  (cfg_bpl),
  .cfg_bpn  (cfg_bpn),
  // status
  .sts_bpl  (sts_bpl),
  .sts_bpn  (sts_bpn)
);

endmodule: asg
