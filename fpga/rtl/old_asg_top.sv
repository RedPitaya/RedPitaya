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

module old_asg_top #(
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
  // trigger parameters
  int unsigned TN  =  4   // external trigger array  width
)(
  // stream output
  axi4_stream_if.s       sto,
  // triggers
  input  logic  [TN-1:0] trg_ext,  // external input
  output logic           trg_swo,  // output from software
  output logic           trg_out,  // output from engine
  // interrupts
  output logic           irq_trg,  // trigger
  output logic           irq_stp,  // stop
  // System bus
  sys_bus_if.s           bus
);

////////////////////////////////////////////////////////////////////////////////
// read/write access to buffer
////////////////////////////////////////////////////////////////////////////////

// TODO: the generic bus decoder should be used instead
sys_bus_if #(.DW ($bits(DT)), .AW (CWM)) bus_buf (.clk (bus.clk), .rstn (bus.rstn));

assign bus_buf.ren   = bus.ren & bus.addr[CWM+2];
assign bus_buf.wen   = bus.wen & bus.addr[CWM+2];
assign bus_buf.addr  = bus.addr[2+:CWM];
assign bus_buf.wdata = bus.wdata;

always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  if (~bus.addr[CWM+2]) begin
    bus.err <= 1'b0;
    bus.ack <= bus.wen | bus.ren;
  end else begin
    bus.err <= bus_buf.err;
    bus.ack <= bus_buf.ack;
  end
end

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

logic               ctl_rst ;
// configuration
logic      [TN-1:0] cfg_trg;  // trigger mask

logic [CWM+CWF-1:0] cfg_siz;  // table size
logic [CWM+CWF-1:0] cfg_stp;  // address increment step (frequency)
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
always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  // configuration
  cfg_trg <= '0;
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
  cfg_mul <= EN_LIN ? 1 << ($bits(DTM)-2) : '0;
  cfg_sum <= '0;
end else begin
  if (bus.wen & ~bus.addr[CWM+2]) begin
    // trigger configuration
    if (bus.addr[BAW-1:0]=='h04)  cfg_trg <= bus.wdata[     TN-1:0];
    // buffer configuration
    if (bus.addr[BAW-1:0]=='h10)  cfg_siz <= bus.wdata[CWM+CWF-1:0];
    if (bus.addr[BAW-1:0]=='h14)  cfg_off <= bus.wdata[CWM+CWF-1:0];
    if (bus.addr[BAW-1:0]=='h18)  cfg_stp <= bus.wdata[CWM+CWF-1:0];
    // burst mode
    if (bus.addr[BAW-1:0]=='h20)  cfg_ben <= bus.wdata[          0];
    if (bus.addr[BAW-1:0]=='h20)  cfg_inf <= bus.wdata[          1];
    if (bus.addr[BAW-1:0]=='h24)  cfg_bdl <= bus.wdata[    CWM-1:0];
    if (bus.addr[BAW-1:0]=='h28)  cfg_bln <= bus.wdata[     32-1:0];
    if (bus.addr[BAW-1:0]=='h2c)  cfg_bnm <= bus.wdata[     16-1:0];
    // linear transformation
    if (bus.addr[BAW-1:0]=='h38)  cfg_mul <= DTM'(bus.wdata);
    if (bus.addr[BAW-1:0]=='h3c)  cfg_sum <= DTS'(bus.wdata);
  end
end

// control signals
assign ctl_rst = bus.wen & ~bus.addr[CWM+2] & (bus.addr[BAW:0]=='h00) & bus.wdata[0];  // reset
assign trg_swo = bus.wen & ~bus.addr[CWM+2] & (bus.addr[BAW:0]=='h00) & bus.wdata[1];  // trigger

// read access
always_ff @(posedge bus.clk)
if (~bus.addr[CWM+2]) begin
  casez (bus.addr[BAW-1:0])
    'h00 : bus.rdata <= {{32-      4{1'b0}},~sts_run, sts_run, 2'b00};
    // trigger configuration
    'h04 : bus.rdata <= {{32-     TN{1'b0}}, cfg_trg};
    // buffer configuration
    'h10 : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_siz};
    'h14 : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_off};
    'h18 : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_stp};
    // burst mode
    'h20 : bus.rdata <= {{32-      2{1'b0}}, cfg_inf
                                           , cfg_ben};
    'h24 : bus.rdata <= {{32-    CWM{1'b0}}, cfg_bdl};
    'h28 : bus.rdata <=                      cfg_bln ;
    'h2c : bus.rdata <= {{32-     16{1'b0}}, cfg_bnm};
    // status
    'h30 : bus.rdata <= 32'(sts_bln);
    'h34 : bus.rdata <= 32'(sts_bnm);
    // linear transformation (should be properly sign extended)
    'h38 : bus.rdata <= cfg_mul;
    'h3c : bus.rdata <= cfg_sum;

    default : bus.rdata <= '0;
  endcase
end else begin
    if (EN_LIN)  bus.rdata <= $signed(bus_buf.rdata);
    else         bus.rdata <=         bus_buf.rdata ;
end

////////////////////////////////////////////////////////////////////////////////
// generator core instance 
////////////////////////////////////////////////////////////////////////////////

// stream from generator
axi4_stream_if #(.DN (DN), .DT (DT)) stg (.ACLK (sto.ACLK), .ARESETn (sto.ARESETn));

old_asg #(
  .TN (TN),
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
  .sto       (stg    ),
  // trigger
  .trg_i     (trg_ext),
  .trg_o     (trg_out),
  // interrupts
  .irq_trg   (irq_trg),
  .irq_stp   (irq_stp),
  // control
  .ctl_rst   (ctl_rst),
  // configuration
  .cfg_trg   (cfg_trg),
  .cfg_siz   (cfg_siz),
  .cfg_stp   (cfg_stp),
  .cfg_off   (cfg_off),
  // configuration (burst mode)
  .cfg_ben   (cfg_ben),
  .cfg_inf   (cfg_inf),
  .cfg_bdl   (cfg_bdl),
  .cfg_bln   (cfg_bln),
  .cfg_bnm   (cfg_bnm),
  // status
  .sts_bln   (sts_bln),
  .sts_bnm   (sts_bnm),
  .sts_run   (sts_run),
  // CPU buffer access
  .bus       (bus_buf)
);

// TODO: this will be a continuous stream, data stream control needs rethinking

generate
if (EN_LIN) begin: en_lin

  axi4_stream_if #(.DN (DN), .DT (DT)) str (.ACLK (sto.ACLK), .ARESETn (sto.ARESETn));

  lin_mul #(
    .DN  (DN),
    .DTI (DT),
    .DTO (DT),
    .DTM (logic signed [16-1:0])
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
    .DTS (DT)
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

endmodule: old_asg_top
