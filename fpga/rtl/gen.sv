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
  type DTC = logic,
  type DTT = evn_pkg::evt_t,
  type DTE = evn_pkg::evd_t
)(
  // stream output
  axi4_stream_if.s      sto,
  // events input/output
  input  DTE            evi,  // input
  output evn_pkg::evs_t evo,  // output
  // interrupt
  output logic          irq,
  // system bus
  sys_bus_if.s          bus    ,  // CPU access to memory mapped registers
  sys_bus_if.s          bus_tbl   // CPU access to waveform table
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// event select masks
DTC             cfg_rst;  // software reset
DTC             cfg_str;  // software start
DTC             cfg_stp;  // software stop
DTC             cfg_swt;  // software trigger
DTT             cfg_trg;  // trigger

// interrupt enable/status/clear
logic   [2-1:0] irq_ena;  // enable
logic   [2-1:0] irq_sts;  // status

// control
logic           ctl_rst;
// control/status start
logic           ctl_str;
logic           sts_str;
// control/status stop
logic           ctl_stp;
logic           sts_stp;
// control/status trigger
logic           ctl_trg;
logic           sts_trg;

// generator mode
logic           cfg_ben;  // burst enable
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
  // interrupt enable
  irq_ena <= '0;
  // event masks
  cfg_trg <= '0;
  cfg_rst <= '0;
  cfg_str <= '0;
  cfg_stp <= '0;
  cfg_swt <= '0;
  // state machine
  cfg_siz <= '0;
  cfg_off <= '0;
  cfg_ste <= '0;
  // burst mode
  cfg_ben <= '0;
  cfg_inf <= '0;
  cfg_bdl <= '0;
  cfg_bpn <= '0;
  cfg_bpl <= '0;
  // linear transform or logic analyzer output enable
  cfg_mul <= '0;
  cfg_sum <= '0;
end else begin
  if (bus.wen) begin
    // trigger mask
    if (bus.addr[BAW-1:0]=='h04)  cfg_trg <= bus.wdata;
    // interrupt enable (status/clear are elsewhere)
    if (bus.addr[BAW-1:0]=='h08)  irq_ena <= bus.wdata[2-1:0];
    // event masks
    if (bus.addr[BAW-1:0]=='h10)  cfg_rst <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h14)  cfg_str <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h18)  cfg_stp <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h1c)  cfg_swt <= bus.wdata;
    // generator mode
    if (bus.addr[BAW-1:0]=='h20)  cfg_ben <= bus.wdata[0];
    if (bus.addr[BAW-1:0]=='h20)  cfg_inf <= bus.wdata[1];
    // continuous/periodic configuration
    if (bus.addr[BAW-1:0]=='h24)  cfg_siz <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h28)  cfg_off <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h2c)  cfg_ste <= bus.wdata;
    // burst configuration
    if (bus.addr[BAW-1:0]=='h30)  cfg_bdr <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h34)  cfg_bdl <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h38)  cfg_bpl <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h3c)  cfg_bpn <= bus.wdata;
    // linear transformation and enable
    if (bus.addr[BAW-1:0]=='h50)  cfg_mul <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h54)  cfg_sum <= bus.wdata;
    if (bus.addr[BAW-1:0]=='h58)  cfg_ena <= bus.wdata[0];
  end
end

// control signals
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  evo.rst <= 1'b0;
  evo.str <= 1'b0;
  evo.stp <= 1'b0;
  evo.swt <= 1'b0;
end else begin
  if (bus.wen & (bus.addr[BAW-1:0]=='h00)) begin
    evo.rst <= bus.wdata[0];  // reset
    evo.str <= bus.wdata[1];  // start
    evo.stp <= bus.wdata[2];  // stop
    evo.swt <= bus.wdata[3];  // trigger
  end else begin
    evo.rst <= 1'b0;
    evo.str <= 1'b0;
    evo.stp <= 1'b0;
    evo.swt <= 1'b0;
  end
end

// read access
always_ff @(posedge bus.clk)
casez (bus.addr[BAW-1:0])
  // control
  'h00: bus.rdata <= {sts_trg, sts_stp, sts_str, 1'b0};
  'h04: bus.rdata <= cfg_trg;
  // interrupts enable/status/clear
  'h08: bus.rdata <= irq_ena;
  'h0c: bus.rdata <= irq_sts;
  // event masks
  'h10: bus.rdata <= cfg_rst;
  'h14: bus.rdata <= cfg_str;
  'h18: bus.rdata <= cfg_stp;
  'h1c: bus.rdata <= cfg_swt;
  // generator mode
  'h20: bus.rdata <= {cfg_inf, cfg_ben};
  // continuous/periodic configuration
  'h24: bus.rdata <= cfg_siz;
  'h28: bus.rdata <= cfg_off;
  'h2c: bus.rdata <= cfg_ste;
  // burst configuration
  'h30: bus.rdata <= cfg_bdr;
  'h34: bus.rdata <= cfg_bdl;
  'h38: bus.rdata <= cfg_bpl;
  'h3c: bus.rdata <= cfg_bpn;
  // burst status
  'h40: bus.rdata <= sts_bpl;
  'h44: bus.rdata <= sts_bpn;
  // linear transformation and enable
  'h50: bus.rdata <= cfg_mul;
  'h54: bus.rdata <= cfg_sum;
  'h58: bus.rdata <= cfg_ena;
  // default is 'x for better optimization
  default: bus.rdata <= 'x;
endcase

// interrupt status/clear
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  irq_sts <= '0;
end else begin
  if (ctl_rst) begin
    irq_sts <= '0;
  end else if (bus.wen & (bus.addr[BAW-1:0]=='h0c)) begin
    // interrupt clear
    irq_sts <= irq_sts & ~bus.wdata[3-1:0];
  end else begin
    // interrupt set
    irq_sts <= irq_sts | {evo.lst, evo.trg} & irq_ena;
  end
end

// interrupt output
always_ff @(posedge bus.clk)
if (~bus.rstn)  irq <= '0;
else            irq <= |irq_sts;

////////////////////////////////////////////////////////////////////////////////
// generator core instance 
////////////////////////////////////////////////////////////////////////////////

assign ctl_rst = |(evi.rst & cfg_rst);
assign ctl_str = |(evi.str & cfg_str);
assign ctl_stp = |(evi.stp & cfg_stp);
assign ctl_trg = |(evi.swt & cfg_swt)
               | |(evi.trg & cfg_trg);

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
  // control
  .ctl_rst  (ctl_rst),
  // control/status start
  .ctl_str  (ctl_str),
  .sts_str  (sts_str),
  // control/status stop
  .ctl_stp  (ctl_stp),
  .sts_stp  (sts_stp),
  // control/status trigger
  .ctl_trg  (ctl_trg),
  .sts_trg  (sts_trg),
  // events
  .evn_per  (evo.trg),
  .evn_lst  (evo.lst),
  // generator mode
  .cfg_ben  (cfg_ben),
  .cfg_inf  (cfg_inf),
  // continuous/periodic configuration
  .cfg_siz  (cfg_siz),
  .cfg_off  (cfg_off),
  .cfg_ste  (cfg_ste),
  // burst configuration
  .cfg_bdr  (cfg_bdr),
  .cfg_bdl  (cfg_bdl),
  .cfg_bpl  (cfg_bpl),
  .cfg_bpn  (cfg_bpn),
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
