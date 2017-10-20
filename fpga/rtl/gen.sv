////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya signal generator.
// Authors: Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

/**
 * GENERAL DESCRIPTION:
 *
 * Arbitrary signal generator takes data stored in buffer and sends them to DAC.
 *
 *           /-----\      /--------\
 *   SW ---> | BUF | ---> | kx + o | ---> DAC
 *           \-----/      \--------/ 
 *
 * Buffers are filed with SW. It also sets finite state machine which take control
 * over read pointer. All registers regarding reading from buffer has additional 
 * 16 bits used as decimal points. In this way we can make better ratio betwen 
 * clock cycle and frequency of output signal. 
 *
 * Finite state machine can be set for one time sequence or continously wrapping.
 * Starting trigger can come from outside, notification trigger used to synchronize
 * with other applications (scope) is also available. Both channels are independant.
 */

module gen #(
  // stream parameters
  int unsigned DN = 1,      // data number
  type DT = logic [8-1:0],  // data type
  // configuration parameters
  type DTM = DT,  // data type for multiplication
  type DTS = DT,  // data type for summation
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16,  // counter width fraction  (fixed point fraction)
  int unsigned CW  = CWM+CWF,
  // burst counter parameters
  int unsigned CWR = 14,  // counter width for burst data repeat
  int unsigned CWL = 32,  // counter width for burst length
  int unsigned CWN = 16,  // counter width for burst number
  // event parameters
  int unsigned ER  = 0,   // event reset
  int unsigned EN  = 1,   // event number
  int unsigned EL  = $clog2(EN),
  // trigger parameters
  int unsigned TN  = 1    // trigger number
)(
  // streams
  axi4_stream_if.s               sto,  // output
  // events input/output
  input  evn_pkg::evn_t [EN-1:0] evi,  // input
  output evn_pkg::evn_t          evo,  // output
  // triggers input/output
  input  logic          [TN-1:0] trg,  // input
  output logic                   tro,  // output
  // interrupt
  output logic                   irq,
  // system bus
  sys_bus_if.s                   bus    ,  // CPU access to memory mapped registers
  sys_bus_if.s                   bus_tbl   // CPU access to waveform table
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// event select
logic  [EL-1:0] cfg_evn;
// trigger mask
logic  [TN-1:0] cfg_trg;
logic           cfg_tre; // trigger repeat enable

// software events
evn_pkg::evn_t  evn;  // multiplexed input
evn_pkg::evn_t  evs;  // status

// trigger
logic           ctl_trg;
logic           sts_trg;

// generator mode
logic           cfg_mod;  // mode (0 - periodic, 1 - burst)
logic           cfg_inf;  // infinite burst
// continuous/periodic configuration
logic  [CW-1:0] cfg_siz;  // table size
logic  [CW-1:0] cfg_off;  // address initial offset (phase)
logic  [CW-1:0] cfg_ste;  // address increment step (frequency)
// burst configuration
logic [CWR-1:0] cfg_bdr;  // burst data   repetitions
logic [CWM-1:0] cfg_bdl;  // burst data   length
logic [CWL-1:0] cfg_bpl;  // burst period length
logic [CWN-1:0] cfg_bpn;  // burst period number
// status
logic [CWL-1:0] sts_bpl;  // burst period length counter
logic [CWN-1:0] sts_bpn;  // burst period number counter
// linear offset and gain
DTM             cfg_mul;
DTS             cfg_sum;
logic           cfg_ena;

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  bus.err <= 1'b0;
  bus.ack <= bus.wen | bus.ren;
end

localparam int unsigned BAW=7;

// write access
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  // event select
  cfg_evn <= ER;
  // trigger mask
  cfg_trg <= '0;
  cfg_tre <= '0;
  // state machine
  cfg_siz <= '0;
  cfg_off <= '0;
  cfg_ste <= '0;
  // burst mode
  cfg_mod <= '0;
  cfg_inf <= '0;
  cfg_bdr <= '0;
  cfg_bdl <= '0;
  cfg_bpl <= '0;
  cfg_bpn <= '0;
  // linear transform or logic analyzer output enable
  cfg_mul <= '0;
  cfg_sum <= '0;
end else begin
  if (bus.wen) begin
    // event select
    if (bus.addr[BAW-1:0]=='h04)  cfg_evn <= bus.wdata;
    // triger mask
    if (bus.addr[BAW-1:0]=='h08)  cfg_trg <= bus.wdata    ;
    if (bus.addr[BAW-1:0]=='h08)  cfg_tre <= bus.wdata[31];
    // generator mode
    if (bus.addr[BAW-1:0]=='h10)  cfg_mod <= bus.wdata[0];
    // continuous/periodic configuration
    if (bus.addr[BAW-1:0]=='h14)  cfg_siz <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h18)  cfg_off <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h1c)  cfg_ste <= bus.wdata;
    // burst configuration
    if (bus.addr[BAW-1:0]=='h20)  cfg_bdr <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h24)  cfg_bdl <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h28)  cfg_bpl <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h2c)  cfg_bpn <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h2c)  cfg_inf <= bus.wdata[31];
    // linear transformation and enable
    if (bus.addr[BAW-1:0]=='h40)  cfg_mul <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h44)  cfg_sum <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h48)  cfg_ena <= bus.wdata[0];
  end
end

// event outputs
always_ff @(posedge bus.clk)
if (~bus.rstn)  evo <= '0;
else            evo <= (bus.wen & (bus.addr[BAW-1:0]=='h00)) ? bus.wdata : '0;

// read access
always_ff @(posedge bus.clk)
casez (bus.addr[BAW-1:0])
  // control
  'h00: bus.rdata <= evs;
  // event select
  'h04: bus.rdata <= cfg_evn;
  // trigger mask
  'h08: bus.rdata <= (cfg_tre << 31) | cfg_trg;
  // generator mode
  'h10: bus.rdata <= cfg_mod;
  // continuous/periodic configuration
  'h14: bus.rdata <= cfg_siz;
  'h18: bus.rdata <= cfg_off;
  'h1c: bus.rdata <= cfg_ste;
  // burst configuration
  'h20: bus.rdata <= cfg_bdr;
  'h24: bus.rdata <= cfg_bdl;
  'h28: bus.rdata <= cfg_bpl;
  'h2c: bus.rdata <= (cfg_inf << 31) | cfg_bpn;
  // burst status
  'h30: bus.rdata <= sts_bpl;
  'h34: bus.rdata <= sts_bpn;
  // linear transformation and enable
  'h40: bus.rdata <= cfg_mul;
  'h44: bus.rdata <= cfg_sum;
  'h48: bus.rdata <= cfg_ena;
  // default is 'x for better optimization
  default: bus.rdata <= 'x;
endcase

////////////////////////////////////////////////////////////////////////////////
// generator core instance 
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge bus.clk)
if (~bus.rstn)  evn <= '0;
else            evn <= evi[cfg_evn];

assign ctl_trg = |(trg & cfg_trg);

// stream from generator
axi4_stream_if #(.DN (DN), .DT (DT)) stg (.ACLK (sto.ACLK), .ARESETn (sto.ARESETn));

asg #(
  .DN (DN),
  .DT (DT),
  // buffer parameters
  .CWM (CWM),
  .CWF (CWF),
  // burst counters
  .CWR (CWR),
  .CWL (CWL),
  .CWN (CWN)
) asg (
  // stream output
  .sto      (stg),
  // event control/status
  .evn      (evn),
  .evs      (evs),
  // trigger
  .ctl_trg  (ctl_trg),
  .cfg_tre  (cfg_tre),
  // events
  .evo_per  (tro),
  .evo_lst  (irq),
  // generator mode
  .cfg_mod  (cfg_mod),
  // continuous/periodic configuration
  .cfg_siz  (cfg_siz),
  .cfg_off  (cfg_off),
  .cfg_ste  (cfg_ste),
  // burst configuration
  .cfg_bdr  (cfg_bdr),
  .cfg_bdl  (cfg_bdl),
  .cfg_bpl  (cfg_bpl),
  .cfg_bpn  (cfg_bpn),
  .cfg_inf  (cfg_inf),
  // status
  .sts_bpl  (sts_bpl),
  .sts_bpn  (sts_bpn),
  // CPU buffer access
  .bus      (bus_tbl)
);

// TODO: this will be a continuous stream, data stream control needs rethinking

axi4_stream_if #(.DN (DN), .DT (DT)) stm (.ACLK (sto.ACLK), .ARESETn (sto.ARESETn));
axi4_stream_if #(.DN (DN), .DT (DT)) sta (.ACLK (sto.ACLK), .ARESETn (sto.ARESETn));

lin_mul #(
  .DN  (DN),
  .DTI (DT),
  .DTO (DT),
  .DTM (DTM)
) lin_mul (
  // stream input/output
  .sti       (stg),
  .sto       (stm),
  // configuration
  .cfg_mul   (cfg_mul)
);

lin_add #(
  .DN  (DN),
  .DTI (DT),
  .DTO (DT),
  .DTS (DTS)
) lin_add (
  // stream input/output
  .sti       (stm),
  .sto       (sta),
  // configuration
  .cfg_sum   (cfg_sum)
);

bin_and #(
  .DN (DN),
  .DT (DT)
) bin_and (
  // stream input/output
  .sti       (sta),
  .sto       (sto),
  // configuration
  .cfg_and   ({$bits(DT){cfg_ena}})
);

endmodule: gen
