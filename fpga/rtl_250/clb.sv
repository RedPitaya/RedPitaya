////////////////////////////////////////////////////////////////////////////////
// Module: calibration
// Authors: Iztok Jeras <iztok.jeras@redpitaya.com>
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

module clb #(
  // stream parameters
  int unsigned DN = 1,  // data number
  type DTG = logic signed [14-1:0], // generator    data type
  type DTO = logic signed [16-1:0], // oscilloscope data type
  // module numbers
  int unsigned MNG = 2,  // number of generator    modules
  int unsigned MNO = 2   // number of oscilloscope modules
)(
  // generator (DAC) streams
  axi4_stream_if.d str_gen [MNG-1:0],
  axi4_stream_if.s str_dac [MNG-1:0],
  // oscilloscope (ADC) streams
  axi4_stream_if.d str_adc [MNO-1:0],
  axi4_stream_if.s str_osc [MNO-1:0],
  // system bus
  sys_bus_if.s     bus
);

localparam int unsigned AW = $clog2((MNO+MNG)*2)+2;

////////////////////////////////////////////////////////////////////////////////
// local signals
////////////////////////////////////////////////////////////////////////////////

// linear offset and gain
DTG [MNG-1:0] cfg_dac_mul;
DTG [MNG-1:0] cfg_dac_sum;
DTO [MNO-1:0] cfg_adc_mul;
DTO [MNO-1:0] cfg_adc_sum;

////////////////////////////////////////////////////////////////////////////////
// calibration
////////////////////////////////////////////////////////////////////////////////

// DAC
generate
for (genvar i=0; i<MNO; i++) begin: for_dac

  axi4_stream_if #(.DN (DN), .DT (DTG)) str_tmp (.ACLK (str_dac[0].ACLK), .ARESETn (str_dac[0].ARESETn));

  lin_mul #(
    .DN  (DN),
    .DTI (DTG),
    .DTO (DTG),
    .DTM (DTG)
  ) lin_mul (
    // stream input/output
    .sti       (str_gen[i]),
    .sto       (str_tmp),
    // configuration
    .cfg_mul   (cfg_dac_mul[i])
  );

  lin_add #(
    .DN  (DN),
    .DTI (DTG),
    .DTO (DTG),
    .DTS (DTG)
  ) lin_add (
    // stream input/output
    .sti       (str_tmp),
    .sto       (str_dac[i]),
    // configuration
    .cfg_sum   (cfg_dac_sum[i])
  );

end: for_dac
endgenerate

// ADC
generate
for (genvar i=0; i<MNO; i++) begin: for_adc

  axi4_stream_if #(.DN (DN), .DT (DTO)) str_tmp (.ACLK (str_adc[0].ACLK), .ARESETn (str_adc[0].ARESETn));

  lin_add #(
    .DN  (DN),
    .DTI (DTO),
    .DTO (DTO),
    .DTS (DTO)
  ) lin_add (
    // stream input/output
    .sti       (str_adc[i]),
    .sto       (str_tmp),
    // configuration
    .cfg_sum   (cfg_adc_sum[i])
  );

  lin_mul #(
    .DN  (DN),
    .DTI (DTO),
    .DTO (DTO),
    .DTM (DTO)
  ) lin_mul (
    // stream input/output
    .sti       (str_tmp),
    .sto       (str_osc[i]),
    // configuration
    .cfg_mul   (cfg_adc_mul[i])
  );

end: for_adc
endgenerate

///////////////////////////////////////////////////////////////////////////////a
// system bus connection
////////////////////////////////////////////////////////////////////////////////

// write access
always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  for (int unsigned i=0; i<MNO; i++) begin
    cfg_adc_mul[i] <= 1 << ($bits(DTO)-2);
    cfg_adc_sum[i] <= '0;
  end
  for (int unsigned i=0; i<MNG; i++) begin
    cfg_dac_mul[i] <= 1 << ($bits(DTG)-2);
    cfg_dac_sum[i] <= '0;
  end
end else begin
  if (bus.wen) begin
    for (int unsigned i=0; i<MNG; i++) begin
      if (bus.addr[AW-1:2]==      (2*i+0))  cfg_dac_mul[i] <= bus.wdata;
      if (bus.addr[AW-1:2]==      (2*i+1))  cfg_dac_sum[i] <= bus.wdata;
    end
    for (int unsigned i=0; i<MNO; i++) begin
      if (bus.addr[AW-1:2]==2*MNG+(2*i+0))  cfg_adc_mul[i] <= bus.wdata;
      if (bus.addr[AW-1:2]==2*MNG+(2*i+1))  cfg_adc_sum[i] <= bus.wdata;
    end
  end
end

// control signals
logic sys_en;
assign sys_en = bus.wen | bus.ren;

always_ff @(posedge bus.clk)
if (!bus.rstn) begin
  bus.err <= 1'b1;
  bus.ack <= 1'b0;
end else begin
  bus.err <= 1'b0;
  bus.ack <= sys_en;
end

// read access
always_ff @(posedge bus.clk)
if (bus.ren) begin
  case (bus.addr[AW-1])
    1'b0: begin
      case (bus.addr[2])
        1'b0:  bus.rdata <= cfg_dac_mul[bus.addr[AW-2:3]];
        1'b1:  bus.rdata <= cfg_dac_sum[bus.addr[AW-2:3]];
      endcase
    end
    1'b1: begin
      case (bus.addr[2])
        1'b0:  bus.rdata <= cfg_adc_mul[bus.addr[AW-2:3]];
        1'b1:  bus.rdata <= cfg_adc_sum[bus.addr[AW-2:3]];
      endcase
    end
  endcase
end

endmodule: clb
