`timescale 1ns / 1ps

module tb_osc_calib;

reg clk_125;

reg signed [15:0] s_axis_tdata;

initial
begin
  clk_125 = 1;
  forever begin
    #(8.0/2.0) clk_125 = ~clk_125; 
  end
end

////////////////////////////////////////////////////////////
// Name : Calibration
// 
////////////////////////////////////////////////////////////

osc_calib U_osc_calib(
  .clk              (clk_125),                                             
  .s_axis_tdata     (s_axis_tdata),            
  .s_axis_tvalid    (1'b1),           
  .s_axis_tready    (),                                           
  .m_axis_tdata     (),            
  .m_axis_tvalid    (),           
  .m_axis_tready    (),                                         
  .cfg_calib_offset (-16'd1),        
  .cfg_calib_gain   (16'd1));    
  
initial
begin
  s_axis_tdata = -32768;
end      

endmodule