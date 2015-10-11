////////////////////////////////////////////////////////////////////////////////
// Stream edge (positive/negative) detection
// Authors: Matej Oblak, Iztok Jeras
// (c) Red Pitaya  http://www.redpitaya.com
////////////////////////////////////////////////////////////////////////////////

module scope_edge #(
  // stream parameters
  parameter DWI = 14,  // data width for input
)(
  // system signals
  input  logic                  clk ,  // clock
  input  logic                  rstn,  // reset - active low
  // stream monitor
  input  logic signed [DWI-1:0] sti_dat,  // data
  input  logic                  sti_vld,  // valid
  input  logic                  sti_rdy,  // ready
  // configuration
  input  logic signed [DWI-1:0] cfg_tresh ;
  input  logic signed [DWI-1:0] cfg_treshp;
  input  logic signed [DWI-1:0] cfg_treshm;
  input  logic signed [DWI-1:0] cfg_hyst  ;
  // triggers
  input  logic                  trg_ext,  // external input
  output logic                  trg_out,  // output
);

logic [  2-1: 0] adc_scht_p;
logic [  2-1: 0] adc_scht_n;

always @(posedge clk)
if (rstn == 1'b0) begin
  adc_scht_ap  <=  2'h0 ;
  adc_scht_an  <=  2'h0 ;
  adc_trg_ap  <=  1'b0 ;
  adc_trg_an  <=  1'b0 ;
end else begin
  cfg_treshp <= cfg_tresh + cfg_hyst ; // calculate positive
  cfg_treshm <= cfg_tresh - cfg_hyst ; // and negative treshold

  if (adc_dv) begin
         if (sti_dat >= $signed(cfg_tresh ))  adc_scht_p[0] <= 1'b1 ;  // treshold reached
    else if (sti_dat <  $signed(cfg_treshm))  adc_scht_p[0] <= 1'b0 ;  // wait until it goes under hysteresis
         if (sti_dat <= $signed(cfg_tresh ))  adc_scht_n[0] <= 1'b1 ;  // treshold reached
    else if (sti_dat >  $signed(cfg_treshp))  adc_scht_n[0] <= 1'b0 ;  // wait until it goes over hysteresis
  end

  adc_scht_ap[1] <= adc_scht_ap[0] ;
  adc_scht_an[1] <= adc_scht_an[0] ;

  adc_trg_ap <= adc_scht_ap[0] && !adc_scht_ap[1] ; // make 1 cyc pulse 
  adc_trg_an <= adc_scht_an[0] && !adc_scht_an[1] ;
end

endmodule: scope_edge
