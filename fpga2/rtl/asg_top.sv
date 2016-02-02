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
  type DAT_T = logic [8-1:0],
  type DAT_M = DAT_T,
  type DAT_S = DAT_T,
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16,  // counter width fraction  (fixed point fraction)
  // trigger parameters
  int unsigned TN  =  4   // external trigger array  width
)(
  // stream output
  str_bus_if.s           sto,
  // triggers
  input  logic  [TN-1:0] trg_ext,  // external input
  output logic           trg_swo,  // output from software
  output logic           trg_out,  // output from engine
  // System bus
  sys_bus_if.s           bus
);

////////////////////////////////////////////////////////////////////////////////
// read/write access to buffer
////////////////////////////////////////////////////////////////////////////////

// TODO: the generic bus decoder should be used instead
sys_bus_if #(.DW ($bits(DAT_T)), .AW (CWM)) bus_buf (.clk (bus.clk), .rstn (bus.rstn));

assign bus_buf.ren   = bus.ren & bus.addr[CWM+2];
assign bus_buf.wen   = bus.wen & bus.addr[CWM+2];
assign bus_buf.addr  = bus.addr[2+:CWM];
assign bus_buf.wdata = bus.wdata;

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
// linear offset and gain
DAT_M               cfg_mul;
DAT_S               cfg_sum;

// control signals
logic  bus_en;
assign bus_en = bus.wen | bus.ren;

always_ff @(posedge bus.clk)
if (~bus.rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  if (~bus.addr[CWM+2]) begin
    bus.err <= 1'b0;
    bus.ack <= bus_en;
  end else begin
    bus.err <= bus_buf.err;
    bus.ack <= bus_buf.ack;
  end
end

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
  cfg_mul <= EN_LIN ? 1 << ($bits(DAT_M)-2) : '0;
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
    if (bus.addr[BAW-1:0]=='h30)  cfg_mul <= DAT_M'(bus.wdata);
    if (bus.addr[BAW-1:0]=='h34)  cfg_sum <= DAT_S'(bus.wdata);
  end
end

// control signals
assign ctl_rst = bus.wen & ~bus.addr[CWM+2] & (bus.addr[BAW:0]==20'h00) & bus.wdata[0];  // reset
assign trg_swo = bus.wen & ~bus.addr[CWM+2] & (bus.addr[BAW:0]==20'h00) & bus.wdata[1];  // trigger

// read access
always_ff @(posedge bus.clk)
if (~bus.addr[CWM+2]) begin
  casez (bus.addr[BAW-1:0])
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
    // linear transformation (should be properly sign extended)
    'h30 : bus.rdata <= cfg_mul;
    'h34 : bus.rdata <= cfg_sum;

    default : bus.rdata <= '0;
  endcase
end else begin
           bus.rdata <= bus_buf.rdata;
end

////////////////////////////////////////////////////////////////////////////////
// generator core instance 
////////////////////////////////////////////////////////////////////////////////

// stream from generator
str_bus_if #(.DN (DN), .DAT_T (DAT_T)) stg (.clk (sto.clk), .rstn (sto.rstn));

asg #(
  .TN    (TN),
  .DN    (DN),
  .DAT_T (DAT_T),
  .CWM (CWM),
  .CWF (CWF)
) asg (
  // stream output
  .sto       (stg    ),
  // trigger
  .trg_i     (trg_ext),
  .trg_o     (trg_out),
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
  // CPU buffer access
  .bus       (bus_buf)
);

// TODO: this will be a continuous stream, data stream control needs rethinking

generate
if (EN_LIN) begin: en_lin

  linear #(
    .DN  (DN),
    .DTI (DAT_T),
    .DTO (DAT_T),
    .DWM ($bits(DAT_M))
  ) linear (
    // stream input/output
    .sti       (stg),
    .sto       (sto),
    // configuration
    .cfg_mul   (cfg_mul),
    .cfg_sum   (cfg_sum)
  );

end else begin

  assign sto.vld = stg.vld;
  assign sto.kep = stg.kep;
  assign sto.lst = stg.lst;
  assign stg.rdy = sto.rdy;

  assign sto.dat = '{cfg_mul & (~cfg_sum | cfg_sum & ~stg.dat),  // output enable (optional open colector)
                                                      stg.dat};  // output

end
endgenerate

endmodule: asg_top
