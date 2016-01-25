////////////////////////////////////////////////////////////////////////////////
// Red Pitaya oscilloscope application, used for capturing ADC data into BRAMs,
// which can be later read by SW.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module acq #(
  int unsigned TW = 32,  // time width
  int unsigned CW = 32   // counter width
)(
  // stream input/output
  str_bus_if.d  sti,
  str_bus_if.s  sto,
  // current time stamp
  input  logic [TW-1:0] cts,
  // control
  input  logic          ctl_rst,
  // configuration (mode)
  input  logic          cfg_con,  // continuous
  input  logic          cfg_aut,  // automatic
  // configuration/status pre trigger
  input  logic [CW-1:0] cfg_pre,
  output logic [CW-1:0] sts_pre,
  // configuration/status post trigger
  input  logic [CW-1:0] cfg_pst,
  output logic [CW-1:0] sts_pst,
  // control/status/timestamp acquire
  input  logic          ctl_acq,  // acquire start
  output logic          sts_acq,
  output logic [TW-1:0] cts_acq,
  // control/status/timestamp trigger
  input  logic          ctl_trg,
  output logic          sts_trg,
  output logic [TW-1:0] cts_trg,
  // control/status/timestamp stop
  input  logic          ctl_stp,  // acquire stop
  output logic [TW-1:0] cts_stp
);

////////////////////////////////////////////////////////////////////////////////
// input stream transfer
////////////////////////////////////////////////////////////////////////////////

logic sti_trn;

assign sti_trn = sti.vld & sti.rdy;

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

logic sts_stp;
assign sts_stp = sts_acq & ( ctl_stp
               | (sts_trg & ~|sts_pst & ~cfg_con)
               | (sti_trn & sti.lst) );

always @(posedge sti.clk)
if (~sti.rstn) begin
  // status pre/post trigger
  sts_pre <= '0;
  sts_pst <= '0;
  // control/status/timestamp acquire
  sts_acq <= 1'b0;
  cts_acq <= '0;
  // control/status/timestamp trigger
  sts_trg <= 1'b0;
  cts_trg <= '0;
end else begin
  if (ctl_rst) begin
    // status pre/post trigger
    sts_pre <= '0;
    sts_pst <= '0;
    // control/status/timestamp acquire
    sts_acq <= 1'b0;
    cts_acq <= '0;
    // control/status/timestamp trigger
    sts_trg <= 1'b0;
    cts_trg <= '0;
  end else begin
    // acquire
    if (sts_stp) begin
      sts_acq <= 1'b0;
      cts_stp <= cts;
    end else if (ctl_acq) begin
      sts_acq <= 1'b1;
      cts_acq <= cts;
      sts_trg <= cfg_aut;
      sts_pre <= '0;
      sts_pst <= '0;
    end
    // trigger
    if (sts_acq) begin
      if (~sts_trg) begin
        sts_pre <= sts_pre + sti_trn; // TODO: add out of range
        if (ctl_trg) begin
          sts_trg <= 1'b1;
          cts_trg <= cts;
        end
      end else begin
        sts_pst <= sts_pst + sti_trn; // TODO: add out of range
      end
    end
  end
end

////////////////////////////////////////////////////////////////////////////////
// output stream
////////////////////////////////////////////////////////////////////////////////

assign sti.rdy = sto.rdy | ~sto.vld;

// output valid
always @(posedge sti.clk)
if (~sti.rstn) begin
  sto.vld <= 1'b0;
  sto.lst <= 1'b0;
end else begin
  sto.vld <= sts_acq & sti.vld;
  sto.lst <= sts_acq & sti.lst & ~|sts_pst;
end

// output data
always @(posedge sti.clk)
if (sts_acq) begin
  sto.dat <= sti.dat;
  sto.kep <= sti.kep; // TODO
end

endmodule: acq
