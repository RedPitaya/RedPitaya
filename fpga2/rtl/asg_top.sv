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
  type DAT_T = logic [8-1:0],
  // data parameters
  int unsigned DWO = 14,  // RAM data width
  int unsigned DWM = 16,  // data width for multiplier (gain)
  int unsigned DWS = DWO, // data width for summation (offset)
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

sys_bus_if #(.DW (DWO), .AW (CWM)) bus_buf (.clk (clk), .rstn (rstn));

assign bus_buf.ren   = bus.ren & bus.addr[CWM+2];
assign bus_buf.wen   = bus.wen & bus.addr[CWM+2];
assign bus_buf.addr  = bus.addr[2+:CWM];
assign bus_buf.wdata = bus.wdata;

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

logic               ctl_rst ;
// configuration
logic      [TN-1:0] cfg_tsel;  // trigger select

logic [CWM+CWF-1:0] cfg_size;  // table size
logic [CWM+CWF-1:0] cfg_step;  // address increment step (frequency)
logic [CWM+CWF-1:0] cfg_offs;  // address initial offset (phase)
// burst mode configuraton
logic               cfg_bena;  // burst enable
logic               cfg_binf;  // infinite burst
logic     [ 16-1:0] cfg_bcyc;  // number of data cycles
logic     [ 32-1:0] cfg_bdly;  // number of delay cycles
logic     [ 16-1:0] cfg_bnum;  // number of repetitions
// linear offset and gain
logic signed [DWM-1:0] cfg_lmul;
logic signed [DWS-1:0] cfg_lsum;

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
  cfg_tsel <= '0;
  cfg_size <= '0;
  cfg_offs <= '0;
  cfg_step <= '0;
  // burst mode
  cfg_bena <= '0;
  cfg_binf <= '0;
  cfg_bcyc <= '0;
  cfg_bnum <= '0;
  cfg_bdly <= '0;
  // cinear transform
  cfg_lmul <= 1 << (DWM-2);
  cfg_lsum <= '0;
end else begin
  if (bus.wen & ~bus.addr[CWM+2]) begin
    // configuration
    if (bus.addr[BAW-1:0]=='h04)  cfg_tsel <= bus.wdata[     TN-1:0];
    if (bus.addr[BAW-1:0]=='h04)  cfg_bena <= bus.wdata[     TN+0  ];
    if (bus.addr[BAW-1:0]=='h04)  cfg_binf <= bus.wdata[     TN+1  ];
    if (bus.addr[BAW-1:0]=='h08)  cfg_size <= bus.wdata[CWM+CWF-1:0];
    if (bus.addr[BAW-1:0]=='h0c)  cfg_offs <= bus.wdata[CWM+CWF-1:0];
    if (bus.addr[BAW-1:0]=='h10)  cfg_step <= bus.wdata[CWM+CWF-1:0];
    // burst mode
    if (bus.addr[BAW-1:0]=='h18)  cfg_bcyc <= bus.wdata[     16-1:0];
    if (bus.addr[BAW-1:0]=='h1c)  cfg_bdly <= bus.wdata[     32-1:0];
    if (bus.addr[BAW-1:0]=='h20)  cfg_bnum <= bus.wdata[     16-1:0];
    // linear transformation
    if (bus.addr[BAW-1:0]=='h24)  cfg_lmul <= bus.wdata[    DWM-1:0];
    if (bus.addr[BAW-1:0]=='h28)  cfg_lsum <= bus.wdata[    DWS-1:0];
  end
end

// control signals
assign ctl_rst = bus.wen & ~bus.addr[CWM+2] & (bus.addr[BAW:0]==20'h00) & bus.wdata[0];  // reset
assign trg_swo = bus.wen & ~bus.addr[CWM+2] & (bus.addr[BAW:0]==20'h00) & bus.wdata[1];  // trigger

// read access
always_ff @(posedge bus.clk)
if (~bus.addr[CWM+2]) begin
  casez (bus.addr[BAW-1:0])
    // configuration
    'h04 : bus.rdata <= {{32-2  - TN{1'b0}}, cfg_binf
                                           , cfg_bena
                                           , cfg_tsel};
    'h08 : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_size};
    'h0c : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_offs};
    'h10 : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_step};
    // burst mode
    'h18 : bus.rdata <= {{32-     16{1'b0}}, cfg_bcyc};
    'h1c : bus.rdata <=                      cfg_bdly ;
    'h20 : bus.rdata <= {{32-     16{1'b0}}, cfg_bnum};
    // linear transformation (should be properly sign extended)
    'h24 : bus.rdata <= cfg_lmul;
    'h28 : bus.rdata <= cfg_lsum;

    default : bus.rdata <= '0;
  endcase
end else begin
           bus.rdata <= bus_buf.rdata;
end

////////////////////////////////////////////////////////////////////////////////
// trigger multiplexer
////////////////////////////////////////////////////////////////////////////////

logic trg_mux;
assign trg_mux = |(trg_ext & cfg_tsel);

////////////////////////////////////////////////////////////////////////////////
// generator core instance 
////////////////////////////////////////////////////////////////////////////////

// stream from generator
str_bus_if #(.DAT_T (DAT_T)) stg (.clk (sto.clk), .rstn (sto.rstn));

asg #(
  .DAT_T (DAT_T),
  .CWM (CWM),
  .CWF (CWF)
) asg (
  // stream output
  .sto       (stg      ),
  // trigger
  .trg_i     (trg_mux  ),
  .trg_o     (trg_out  ),
  // control
  .ctl_rst   (ctl_rst  ),
  // configuration
  .cfg_size  (cfg_size ),
  .cfg_step  (cfg_step ),
  .cfg_offs  (cfg_offs ),
  // configuration (burst mode)
  .cfg_bena  (cfg_bena ),
  .cfg_binf  (cfg_binf ),
  .cfg_bcyc  (cfg_bcyc ),
  .cfg_bdly  (cfg_bdly ),
  .cfg_bnum  (cfg_bnum ),
  // CPU buffer access
  .bus       (bus_buf  )
);

// TODO: this will be a continuous stream, data stream control needs rethinking

linear #(
  .DWI (DWO),
  .DWO (DWO),
  .DWM (DWM)
) linear (
  // stream input/output
  .sti       (stg),
  .sto       (sto),
  // configuration
  .cfg_mul   (cfg_lmul),
  .cfg_sum   (cfg_lsum)
);

endmodule: asg_top
