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
  // data path
  int unsigned DN = 1,
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
  // triggers
  input  logic  [EW-1:0] evn_ext,  // external input
  // events
  output logic           evn_str,  // start
  output logic           evn_stp,  // stop
  output logic           evn_trg,  // trigger
  output logic           evn_per,  // period
  output logic           evn_lst,  // last
  // System bus
  sys_bus_if.s           bus_reg,  // CPU access to memory mapped registers
  sys_bus_if.s           bus_tbl   // CPU access to waveform table
);

////////////////////////////////////////////////////////////////////////////////
// read/write access to buffer
////////////////////////////////////////////////////////////////////////////////

always_ff @(posedge bus_reg.clk)
if (~bus_reg.rstn) begin
  bus_reg.err <= 1'b0;
  bus_reg.ack <= 1'b0;
end else begin
  bus_reg.err <= 1'b0;
  bus_reg.ack <= bus_reg.wen | bus_reg.ren;
end

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

// event masks
logic      [EW-1:0] cfg_str;  // start
logic      [EW-1:0] cfg_stp;  // stop
logic      [EW-1:0] cfg_trg;  // trigger
// event control signals
logic               ctl_rst;
logic               ctl_str;
logic               ctl_stp;
logic               ctl_trg;

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

localparam int unsigned BAW=6;

// write access
always_ff @(posedge bus_reg.clk)
if (~bus_reg.rstn) begin
  // event configuration
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
  if (bus_reg.wen) begin
    // event configuration
    if (bus_reg.addr[BAW-1:0]=='h04)  cfg_str <= bus_reg.wdata[     EW-1:0];
    if (bus_reg.addr[BAW-1:0]=='h04)  cfg_stp <= bus_reg.wdata[     EW-1:0];
    if (bus_reg.addr[BAW-1:0]=='h04)  cfg_trg <= bus_reg.wdata[     EW-1:0];
    // buffer configuration
    if (bus_reg.addr[BAW-1:0]=='h10)  cfg_siz <= bus_reg.wdata[CWM+CWF-1:0];
    if (bus_reg.addr[BAW-1:0]=='h14)  cfg_off <= bus_reg.wdata[CWM+CWF-1:0];
    if (bus_reg.addr[BAW-1:0]=='h18)  cfg_ste <= bus_reg.wdata[CWM+CWF-1:0];
    // burst mode
    if (bus_reg.addr[BAW-1:0]=='h20)  cfg_ben <= bus_reg.wdata[          0];
    if (bus_reg.addr[BAW-1:0]=='h20)  cfg_inf <= bus_reg.wdata[          1];
    if (bus_reg.addr[BAW-1:0]=='h24)  cfg_bdl <= bus_reg.wdata[    CWM-1:0];
    if (bus_reg.addr[BAW-1:0]=='h28)  cfg_bln <= bus_reg.wdata[     32-1:0];
    if (bus_reg.addr[BAW-1:0]=='h2c)  cfg_bnm <= bus_reg.wdata[     16-1:0];
    // linear transformation
    if (bus_reg.addr[BAW-1:0]=='h38)  cfg_mul <= DTM'(bus_reg.wdata);
    if (bus_reg.addr[BAW-1:0]=='h3c)  cfg_sum <= DTS'(bus_reg.wdata);
  end
end

// control signals
always_ff @(posedge bus_reg.clk)
if (~bus_reg.rstn) begin
  ctl_rst <= 1'b0;
  evn_str <= 1'b0;
  evn_stp <= 1'b0;
  evn_trg <= 1'b0;
end else begin
  if (bus_reg.wen & (bus_reg.addr[BAW-1:0]=='h00)) begin
    ctl_rst <= bus_reg.wdata[0];  // reset
    evn_str <= bus_reg.wdata[1];  // start
    evn_stp <= bus_reg.wdata[1];  // stop
    evn_trg <= bus_reg.wdata[1];  // trigger
  end else begin
    ctl_rst <= 1'b0;
    evn_str <= 1'b0;
    evn_stp <= 1'b0;
    evn_trg <= 1'b0;
  end
end

// read access
always_ff @(posedge bus_reg.clk)
casez (bus_reg.addr[BAW-1:0])
  'h00 : bus_reg.rdata <= {{32-      3{1'b0}},~sts_run, sts_run, 1'b0};
  // event configuration
  'h04 : bus_reg.rdata <= {{32-     EW{1'b0}}, cfg_str};
  'h04 : bus_reg.rdata <= {{32-     EW{1'b0}}, cfg_stp};
  'h04 : bus_reg.rdata <= {{32-     EW{1'b0}}, cfg_trg};
  // buffer configuration
  'h10 : bus_reg.rdata <= {{32-CWM-CWF{1'b0}}, cfg_siz};
  'h14 : bus_reg.rdata <= {{32-CWM-CWF{1'b0}}, cfg_off};
  'h18 : bus_reg.rdata <= {{32-CWM-CWF{1'b0}}, cfg_ste};
  // burst mode
  'h20 : bus_reg.rdata <= {{32-      2{1'b0}}, cfg_inf
                                             , cfg_ben};
  'h24 : bus_reg.rdata <= {{32-    CWM{1'b0}}, cfg_bdl};
  'h28 : bus_reg.rdata <=                      cfg_bln ;
  'h2c : bus_reg.rdata <= {{32-     16{1'b0}}, cfg_bnm};
  // status
  'h30 : bus_reg.rdata <= 32'(sts_bln);
  'h34 : bus_reg.rdata <= 32'(sts_bnm);
  // linear transformation (should be properly sign extended)
  'h38 : bus_reg.rdata <= cfg_mul;
  'h3c : bus_reg.rdata <= cfg_sum;

  default : bus_reg.rdata <= '0;
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
  .sto      (stg    ),
  // control
  .ctl_rst  (ctl_rst),
  .ctl_trg  (ctl_trg),
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
