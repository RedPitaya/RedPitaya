////////////////////////////////////////////////////////////////////////////////
// Red Pitaya oscilloscope application, used for capturing ADC data into BRAMs,
// which can be later read by SW.
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module acq #(
  int unsigned TW = 8,  // time width
  int unsigned DW = 8,  // data width
  int unsigned CW = 8   // counter width
)(
  // stream input/output
  str_bus_if.d  sti,
  str_bus_if.s  sto,
  // control
  input  logic          ctl_rst,
  // current time stamp
  input  logic [TW-1:0] ctl_cts,
  input  logic [TW-1:0] sts_cts,
  // delay configuration/status
  input  logic [CW-1:0] cfg_dly,  // requested delay value
  output logic [CW-1:0] sts_dly,  // delay counter status
  // acquire control/status
  input  logic          ctl_acq,
  output logic          sts_acq,
  // trigger control/status
  input  logic          ctl_trg,
  output logic          sts_trg
);

////////////////////////////////////////////////////////////////////////////////
// input stream transfer
////////////////////////////////////////////////////////////////////////////////

logic sti_trn;

assign sti_trn = sti.vld & sti.rdy;

////////////////////////////////////////////////////////////////////////////////
// aquire and trigger status handler
////////////////////////////////////////////////////////////////////////////////

always @(posedge sti.clk)
if (~sti.rstn) begin
  sts_dly <= 1'b0;
  sts_acq <= 1'b0;
  sts_trg <= 1'b0;
end else begin
  if (ctl_rst) begin
    sts_dly <= 1'b0;
    sts_acq <= 1'b0;
    sts_trg <= 1'b0;
  end else begin
    // scquire status
    if (ctl_acq) begin
      sts_acq <= 1'b1;
    end else if (sts_trg & ~|sts_dly) begin
      sts_acq <= 1'b0;
    end
    // trigger status and delay counter
    if (~sts_trg & ctl_trg & sts_acq) begin
      sts_trg <= 1'b1;
      sts_dly <= cfg_dly;
    end else if (sts_trg) begin
      if (~|sts_dly) begin
        sts_trg <= 1'b0;
      end else begin
        sts_dly <= sts_dly - sti_trn;
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
  sto.lst <= sts_acq & sti.lst & ~|sts_dly;
end

// output data
always @(posedge sti.clk)
if (sts_acq) begin
  sto.dat <= sti.dat;
end else begin
  sto.dat <= '0;
end

endmodule: acq
