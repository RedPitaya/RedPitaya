////////////////////////////////////////////////////////////////////////////////
// Red Pitaya arbitrary signal generator (ASG).
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

/**
 * GENERAL DESCRIPTION:
 *
 * Arbitrary signal generator takes data stored in buffer and sends them to DAC.
 *
 *                /-----\         /--------\
 *   SW --------> | BUF | ------> | kx + o | ---> DAC CHB
 *                \-----/         \--------/ 
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

module asg_top #(
  // functionality enable
  bit EN_LIN = 1,
  // stream parameters
  int unsigned DN = 1,  // data number
  type DT = logic [8-1:0],
  // configuration parameters
  type DTM = DT,  // data type for multiplication
  type DTS = DT,  // data type for summation
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16,  // counter width fraction  (fixed point fraction)
  // burst counter parameters
  int unsigned CWL = 32,  // counter width length
  int unsigned CWN = 16,  // counter width number
  // event parameters
  int unsigned EW  =  4   // external trigger array  width
)(
  // stream output
  axi4_stream_if.s       sto,
  // external events
  input  logic  [EW-1:0] evn_ext,
  // event sources
  output logic           evn_str,  // software start
  output logic           evn_stp,  // software stop
  output logic           evn_trg,  // software trigger
  output logic           evn_per,  // period
  output logic           evn_lst,  // last
  // System bus
  sys_bus_if.s           bus    ,  // CPU access to memory mapped registers
  sys_bus_if.s           bus_tbl   // CPU access to waveform table
);

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// event select masks
logic  [EW-1:0] cfg_str;  // start
logic  [EW-1:0] cfg_stp;  // stop
logic  [EW-1:0] cfg_trg;  // trigger

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

// configuration
logic [CWM+CWF-1:0] cfg_siz;  // table size
logic [CWM+CWF-1:0] cfg_ste;  // address increment step (frequency)
logic [CWM+CWF-1:0] cfg_off;  // address initial offset (phase)
// burst mode configuraton
logic               cfg_ben;  // burst enable
logic               cfg_inf;  // infinite burst
logic     [CWM-1:0] cfg_bdl;  // burst data length
logic     [ 32-1:0] cfg_bln;  // burst idle length
logic     [ 16-1:0] cfg_bnm;  // burst repetitions
// status
logic     [CWL-1:0] sts_bln;  // burst length counter
logic     [CWN-1:0] sts_bnm;  // burst number counter
logic               sts_run;  // running status
// linear offset and gain
DTM                 cfg_mul;
DTS                 cfg_sum;

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

localparam int unsigned BAW=6;

// write access
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  // event masks
  cfg_str <= '0;
  cfg_stp <= '0;
  cfg_trg <= '0;
  // state machine
  cfg_siz <= '0;
  cfg_off <= '0;
  cfg_stp <= '0;
  // burst mode
  cfg_ben <= '0;
  cfg_inf <= '0;
  cfg_bdl <= '0;
  cfg_bnm <= '0;
  cfg_bln <= '0;
  // linear transform or logic analyzer output enable
  cfg_mul <= '0;
  cfg_sum <= '0;
end else begin
  if (bus.wen) begin
    // event masks
    if (bus.addr[BAW-1:0]=='h10)  cfg_str <= bus.wdata[EW-1:0];
    if (bus.addr[BAW-1:0]=='h14)  cfg_stp <= bus.wdata[EW-1:0];
    if (bus.addr[BAW-1:0]=='h18)  cfg_trg <= bus.wdata[EW-1:0];
    // buffer configuration
    if (bus.addr[BAW-1:0]=='h20)  cfg_siz <= bus.wdata[CWM+CWF-1:0];
    if (bus.addr[BAW-1:0]=='h24)  cfg_off <= bus.wdata[CWM+CWF-1:0];
    if (bus.addr[BAW-1:0]=='h28)  cfg_ste <= bus.wdata[CWM+CWF-1:0];
    // burst mode
    if (bus.addr[BAW-1:0]=='h30)  cfg_ben <= bus.wdata[          0];
    if (bus.addr[BAW-1:0]=='h30)  cfg_inf <= bus.wdata[          1];
    if (bus.addr[BAW-1:0]=='h34)  cfg_bdl <= bus.wdata[    CWM-1:0];
    if (bus.addr[BAW-1:0]=='h38)  cfg_bln <= bus.wdata[     32-1:0];
    if (bus.addr[BAW-1:0]=='h3c)  cfg_bnm <= bus.wdata[     16-1:0];
    // linear transformation
    if (bus.addr[BAW-1:0]=='h48)  cfg_mul <= DTM'(bus.wdata);
    if (bus.addr[BAW-1:0]=='h4c)  cfg_sum <= DTS'(bus.wdata);
  end
end

// control signals
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  ctl_rst <= 1'b0;
  evn_str <= 1'b0;
  evn_stp <= 1'b0;
  evn_trg <= 1'b0;
end else begin
  if (bus.wen & (bus.addr[BAW-1:0]=='h00)) begin
    ctl_rst <= bus.wdata[0];  // reset
    evn_str <= bus.wdata[1];  // start
    evn_stp <= bus.wdata[2];  // stop
    evn_trg <= bus.wdata[3];  // trigger
  end else begin
    ctl_rst <= 1'b0;
    evn_str <= 1'b0;
    evn_stp <= 1'b0;
    evn_trg <= 1'b0;
  end
end

// read access
always_ff @(posedge bus.clk)
casez (bus.addr[BAW-1:0])
  // control
  'h00 : bus.rdata <= {{32-  4{1'b0}}, sts_trg, sts_stp, sts_str, 1'b0};
  // event masks
  'h10 : bus.rdata <= {{32- EW{1'b0}}, cfg_str};
  'h14 : bus.rdata <= {{32- EW{1'b0}}, cfg_stp};
  'h18 : bus.rdata <= {{32- EW{1'b0}}, cfg_trg};
  // buffer configuration
  'h20 : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_siz};
  'h24 : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_off};
  'h28 : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_ste};
  // burst mode
  'h30 : bus.rdata <= {{32-      2{1'b0}}, cfg_inf
                                             , cfg_ben};
  'h34 : bus.rdata <= {{32-    CWM{1'b0}}, cfg_bdl};
  'h38 : bus.rdata <=                      cfg_bln ;
  'h3c : bus.rdata <= {{32-     16{1'b0}}, cfg_bnm};
  // status
  'h40 : bus.rdata <= 32'(sts_bln);
  'h44 : bus.rdata <= 32'(sts_bnm);
  // linear transformation (should be properly sign extended)
  'h48 : bus.rdata <= cfg_mul;
  'h4c : bus.rdata <= cfg_sum;
  // default is 'x for better optimization
  default : bus.rdata <= 'x;
endcase

////////////////////////////////////////////////////////////////////////////////
// generator core instance 
////////////////////////////////////////////////////////////////////////////////

assign ctl_str = evn_ext & cfg_str;
assign ctl_stp = evn_ext & cfg_stp;
assign ctl_trg = evn_ext & cfg_trg;

// stream from generator
axi4_stream_if #(.DN (DN), .DT (DT)) stg (.ACLK (sto.ACLK), .ARESETn (sto.ARESETn));

asg #(
  .DN (DN),
  .DT (DT),
  // buffer parameters
  .CWM (CWM),
  .CWF (CWF),
  // burst counters
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
  .evn_per  (evn_per),
  .evn_lst  (evn_lst),
  // configuration
  .cfg_siz  (cfg_siz),
  .cfg_off  (cfg_off),
  .cfg_ste  (cfg_ste),
  // configuration (burst mode)
  .cfg_ben  (cfg_ben),
  .cfg_inf  (cfg_inf),
  .cfg_bdl  (cfg_bdl),
  .cfg_bln  (cfg_bln),
  .cfg_bnm  (cfg_bnm),
  // status
  .sts_bln  (sts_bln),
  .sts_bnm  (sts_bnm),
  .sts_run  (sts_run),
  // CPU buffer access
  .bus      (bus_tbl)
);

// TODO: this will be a continuous stream, data stream control needs rethinking

generate
if (EN_LIN) begin: en_lin

  axi4_stream_if #(.DN (DN), .DT (DT)) str (.ACLK (sto.ACLK), .ARESETn (sto.ARESETn));

  lin_mul #(
    .DN  (DN),
    .DTI (DT),
    .DTO (DT),
    .DTM (DTM)
  ) lin_mul (
    // stream input/output
    .sti       (stg),
    .sto       (str),
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
    .sti       (str),
    .sto       (sto),
    // configuration
    .cfg_sum   (cfg_sum)
  );

end else begin

  assign sto.TVALID = stg.TVALID;
  assign sto.TKEEP  = stg.TKEEP ;
  assign sto.TLAST  = stg.TLAST ;
  assign stg.TREADY = sto.TREADY;

  assign sto.TDATA  = '{cfg_mul & (~cfg_sum | cfg_sum & ~stg.TDATA),  // output enable (optional open colector)
                                                         stg.TDATA};  // output

end
endgenerate

endmodule: asg_top
