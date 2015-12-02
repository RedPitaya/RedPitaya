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
  // data parameters
  int unsigned DWO = 14,  // RAM data width
  int unsigned DWM = 16,  // data width for multiplier (gain)
  int unsigned DWS = DWO, // data width for summation (offset)
  // buffer parameters
  int unsigned CWM = 14,  // counter width magnitude (fixed point integer)
  int unsigned CWF = 16,  // counter width fraction  (fixed point fraction)
  // trigger parameters
  int unsigned TWA =  4,          // external trigger array  width
  int unsigned TWS = $clog2(TWA)  // external trigger select width
)(
  input  logic                  clk ,  // DAC clock
  input  logic                  rstn,  // DAC reset - active low
  // stream output
  output logic signed [DWO-1:0] sto_dat,  // data
  output logic                  sto_vld,  // valid
  input  logic                  sto_rdy,  // ready
  // triggers
  input  logic        [TWA-1:0] trg_ext,  // external input
  output logic                  trg_swo,  // output from software
  output logic                  trg_out,  // output from engine
  // System bus
  sys_bus_if.s                  bus
);

////////////////////////////////////////////////////////////////////////////////
// read/write access to buffer
////////////////////////////////////////////////////////////////////////////////

logic                  bus_ena  ;
logic                  bus_wen  ;
logic        [CWM-1:0] bus_addr ;
logic signed [DWO-1:0] bus_wdata;
logic signed [DWO-1:0] bus_rdata;
logic          [3-1:0] bus_dly ;
logic                  bus_ack ;

always_ff @(posedge clk)
if ((bus.wen | bus.ren) & bus.addr[CWM+2]) begin
  bus_ena   <= 1'b1;
  bus_wen   <= bus.wen;
  bus_addr  <= bus.addr[CWM+2+1:2];
  bus_wdata <= bus.wdata;
end else begin
  bus_ena   <= 1'b0;
end

always_ff @(posedge clk)
if (~rstn) begin
  bus_dly <= '0;
  bus_ack <= '0;
end else begin
  bus_dly <= {bus_dly[3-2:0], bus.ren};
  bus_ack <=  bus_dly[3-1] || bus.wen ;
end

////////////////////////////////////////////////////////////////////////////////
//  System bus connection
////////////////////////////////////////////////////////////////////////////////

logic               ctl_rst ;
// configuration
logic     [TWS-1:0] cfg_tsel;  // trigger select

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

always_ff @(posedge clk)
if (~rstn) begin
  bus.err <= 1'b0;
  bus.ack <= 1'b0;
end else begin
  bus.err <= 1'b0;
  if (~bus.addr[CWM]) begin
    bus.ack <= bus_en;
  end else begin
    bus.ack <= bus_ack;
  end
end

// write access
always_ff @(posedge clk)
if (rstn == 1'b0) begin
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
    if (bus.addr[6-1:0]==6'h04)  cfg_tsel <= bus.wdata[    TWS-1:0];
    if (bus.addr[6-1:0]==6'h04)  cfg_bena <= bus.wdata[    TWS+0  ];
    if (bus.addr[6-1:0]==6'h04)  cfg_binf <= bus.wdata[    TWS+1  ];
    if (bus.addr[6-1:0]==6'h08)  cfg_size <= bus.wdata[CWM+CWF-1:0];
    if (bus.addr[6-1:0]==6'h0c)  cfg_offs <= bus.wdata[CWM+CWF-1:0];
    if (bus.addr[6-1:0]==6'h10)  cfg_step <= bus.wdata[CWM+CWF-1:0];
    // burst mode
    if (bus.addr[6-1:0]==6'h18)  cfg_bcyc <= bus.wdata[     16-1:0];
    if (bus.addr[6-1:0]==6'h1c)  cfg_bdly <= bus.wdata[     32-1:0];
    if (bus.addr[6-1:0]==6'h20)  cfg_bnum <= bus.wdata[     16-1:0];
    // linear transformation
    if (bus.addr[6-1:0]==6'h24)  cfg_lmul <= bus.wdata[    DWM-1:0];
    if (bus.addr[6-1:0]==6'h28)  cfg_lsum <= bus.wdata[    DWS-1:0];
  end
end

// control signals
assign ctl_rst = bus.wen & (bus.addr[19:0]==20'h00) & bus.wdata[0];  // reset
assign trg_swo = bus.wen & (bus.addr[19:0]==20'h00) & bus.wdata[1];  // trigger

// read access
always_ff @(posedge clk)
if (~bus.addr[CWM+2]) begin
  casez (bus.addr[19:0])
    // configuration
    6'h04 : bus.rdata <= {{32-2  -TWS{1'b0}}, cfg_binf
                                            , cfg_bena
                                            , cfg_tsel};
    6'h08 : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_size};
    6'h0c : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_offs};
    6'h10 : bus.rdata <= {{32-CWM-CWF{1'b0}}, cfg_step};
    // burst mode
    6'h18 : bus.rdata <= {{32-     16{1'b0}}, cfg_bcyc};
    6'h1c : bus.rdata <=                      cfg_bdly ;
    6'h20 : bus.rdata <= {{32-     16{1'b0}}, cfg_bnum};
    // linear transformation (should be properly sign extended)
    6'h24 : bus.rdata <= cfg_lmul;
    6'h28 : bus.rdata <= cfg_lsum;
  endcase
end else begin
            bus.rdata <= {{32-    DWO{1'b0}}, bus_rdata};
end

////////////////////////////////////////////////////////////////////////////////
// trigger multiplexer
////////////////////////////////////////////////////////////////////////////////

logic trg_mux;
assign trg_mux = trg_ext [cfg_tsel];

////////////////////////////////////////////////////////////////////////////////
// generator core instance 
////////////////////////////////////////////////////////////////////////////////

// stream from generator
logic signed [DWO-1:0] stg_dat;  // data
logic                  stg_vld;  // valid
logic                  stg_rdy;  // ready

asg #(
  .DWO (DWO),
  .CWM (CWM),
  .CWF (CWF)
) asg (
  // system signals
  .clk       (clk      ),
  .rstn      (rstn     ),
  // DAC
  .sto_dat   (stg_dat  ),
  .sto_vld   (stg_vld  ),
  .sto_rdy   (stg_rdy  ),
  // trigger
  .trg_i     (trg_mux  ),
  .trg_o     (trg_out  ),
  // CPU buffer access
  .bus_ena   (bus_ena  ),
  .bus_wen   (bus_wen  ),
  .bus_addr  (bus_addr ),
  .bus_wdata (bus_wdata),
  .bus_rdata (bus_rdata),
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
  .cfg_bnum  (cfg_bnum )
);

// TODO: this will be a continuous stream, data stream control needs rethinking

linear #(
  .DWI (DWO),
  .DWO (DWO),
  .DWM (DWM)
) linear (
  // system signals
  .clk       (clk      ),
  .rstn      (rstn     ),
  // input stream
  .sti_dat   (stg_dat),
  .sti_vld   (1'b1),
  .sti_rdy   (stg_rdy),
  // output stream
  .sto_dat   (sto_dat),
  .sto_vld   (sto_vld),
  .sto_rdy   (sto_rdy),
  // configuration
  .cfg_mul   (cfg_lmul),
  .cfg_sum   (cfg_lsum)
);

endmodule: asg_top
