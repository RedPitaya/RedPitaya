/**
 * $Id: red_pitaya_asg_tb.v 1271 2014-02-25 12:32:34Z matej.oblak $
 *
 * @brief Red Pitaya arbitrary signal generator testbench.
 *
 * @Author Matej Oblak
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */



/**
 * GENERAL DESCRIPTION:
 *
 * Testbench for arbitrary signal generator module.
 *
 * This testbench writes values into RAM table and sets desired configuration.
 * 
 */



`timescale 1ns / 1ps

module red_pitaya_asg_tb(
);



wire  [ 14-1: 0] dac_a           ;
wire  [ 14-1: 0] dac_b           ;
reg              dac_clk         ;
reg              dac_rstn        ;

reg              sys_clk         ;
reg              sys_rstn        ;
wire  [ 32-1: 0] sys_addr        ;
wire  [ 32-1: 0] sys_wdata       ;
wire  [  4-1: 0] sys_sel         ;
wire             sys_wen         ;
wire             sys_ren         ;
wire  [ 32-1: 0] sys_rdata       ;
wire             sys_err         ;
wire             sys_ack         ;

reg              trig            ;


sys_bus_model i_bus
(
  .sys_clk_i      (  sys_clk      ),
  .sys_rstn_i     (  sys_rstn     ),
  .sys_addr_o     (  sys_addr     ),
  .sys_wdata_o    (  sys_wdata    ),
  .sys_sel_o      (  sys_sel      ),
  .sys_wen_o      (  sys_wen      ),
  .sys_ren_o      (  sys_ren      ),
  .sys_rdata_i    (  sys_rdata    ),
  .sys_err_i      (  sys_err      ),
  .sys_ack_i      (  sys_ack      ) 
);



red_pitaya_asg i_asg
(
  // ADC
  .dac_a_o         (  dac_a         ),  // CH 1
  .dac_b_o         (  dac_b         ),  // CH 2
  .dac_clk_i       (  dac_clk       ),  // clock
  .dac_rstn_i      (  dac_rstn      ),  // reset - active low
  .trig_a_i        (  trig          ),
  .trig_b_i        (  trig          ),

   // System bus
  .sys_clk_i       (  sys_clk       ),  // clock
  .sys_rstn_i      (  sys_rstn      ),  // reset - active low
  .sys_addr_i      (  sys_addr      ),  // address
  .sys_wdata_i     (  sys_wdata     ),  // write data
  .sys_sel_i       (  sys_sel       ),  // write byte select
  .sys_wen_i       (  sys_wen       ),  // write enable
  .sys_ren_i       (  sys_ren       ),  // read enable
  .sys_rdata_o     (  sys_rdata     ),  // read data
  .sys_err_o       (  sys_err       ),  // error indicator
  .sys_ack_o       (  sys_ack       )   // acknowledge signal
);





//---------------------------------------------------------------------------------
//
// signal generation

initial begin
   sys_clk  <= 1'b0 ;
   sys_rstn <= 1'b0 ;
   repeat(10) @(posedge sys_clk);
      sys_rstn <= 1'b1  ;
end

always begin
   #5  sys_clk <= !sys_clk ;
end



initial begin
   dac_clk  <= 1'b0  ;
   dac_rstn <= 1'b0  ;
   repeat(10) @(posedge dac_clk);
      dac_rstn <= 1'b1  ;
end

always begin
   #4  dac_clk <= !dac_clk ;
end

always begin
          trig <= 1'b0 ;
   repeat(100000) @(posedge dac_clk);
      trig <= 1'b1 ;
   repeat(1200) @(posedge dac_clk);
      trig <= 1'b0 ;
end




reg [9-1: 0] ch0_set;
reg [9-1: 0] ch1_set;
integer k;

initial begin
   wait (sys_rstn && dac_rstn)
   repeat(10) @(posedge sys_clk);
      //CH0 DAC data
      i_bus.bus_write(32'h10000, 32'd3     );  // write table
      i_bus.bus_write(32'h10004, 32'd30    );  // write table
      i_bus.bus_write(32'h10008, 32'd8000  );  // write table
      i_bus.bus_write(32'h1000C,-32'd4     );  // write table
      i_bus.bus_write(32'h10010,-32'd40    );  // write table
      i_bus.bus_write(32'h10014,-32'd8000  );  // write table
      i_bus.bus_write(32'h10018,-32'd2000  );  // write table
      i_bus.bus_write(32'h1001c, 32'd250   );  // write table

      //CH0 DAC settings
      i_bus.bus_write(32'h00004,{2'h0,-14'd500, 2'h0, 14'h2F00}  );  // DC offset, amplitude
      i_bus.bus_write(32'h00008,{2'h0, 14'd7, 16'd0}             );  // table size
      i_bus.bus_write(32'h0000C,{2'h0, 14'h1, 16'h0}             );  // reset offset
      i_bus.bus_write(32'h00010,{2'h0, 14'h2, 16'h0}             );  // table step
      i_bus.bus_write(32'h00018,{16'h0, 16'd0}                   );  // number of cycles
      i_bus.bus_write(32'h0001C,{16'h0, 16'd0}                   );  // number of repetitions
      i_bus.bus_write(32'h00020,{32'd3}                          );  // number of 1us delay between repetitions

      ch0_set = {1'b0 ,1'b0, 1'b0, 1'b0, 1'b1,    1'b0, 3'h2} ;  // set_rgate, set_zero, set_rst, set_once(NA), set_wrap, 1'b0, trig_src



      //CH1 DAC data
    k = 0;
    while (k<8000) begin
      i_bus.bus_write(32'h20000 + (k*4), k   );  // write table
      k= k + 1 ;
    end

      //CH1 DAC settings
      i_bus.bus_write(32'h00024,{2'h0, 14'd0, 2'h0, 14'h2000}    );  // DC offset, amplitude
      i_bus.bus_write(32'h00028,{2'h0, 14'd7999, 16'd0}          );  // table size
      i_bus.bus_write(32'h0002C,{2'h0, 14'h5, 16'h0}             );  // reset offset
      i_bus.bus_write(32'h00030,{2'h0, 14'h9, 16'h0}             );  // table step
      i_bus.bus_write(32'h00038,{16'h0, 16'd0}                   );  // number of cycles
      i_bus.bus_write(32'h0003C,{16'h0, 16'd5}                   );  // number of repetitions
      i_bus.bus_write(32'h00040,{32'd10}                         );  // number of 1us delay between repetitions

      ch1_set = {1'b0, 1'b0, 1'b0, 1'b1, 1'b1,    1'b0, 3'h1} ;  // set_rgate, set_zero, set_rst, set_once(NA), set_wrap, 1'b0, trig_src

      i_bus.bus_write(32'h00000,{8'h0, ch1_set,  8'h0, ch0_set}  ); // write configuration


   repeat(2000) @(posedge dac_clk);


      ch1_set = {1'b0, 1'b0, 1'b1, 1'b1,    1'b0, 3'h1} ;  // set_a_zero, set_a_rst, set_a_once, set_a_wrap, 1'b0, trig_src

      i_bus.bus_write(32'h00000,{7'h0, ch1_set,  7'h0, ch0_set}  ); // write configuration



   repeat(200) @(posedge dac_clk);

      //CH1 table data readback
    k = 0;
    while (k<80) begin
      i_bus.bus_read(32'h20000 + (k*4));  // read table
      k= k + 1 ;
    end


      //CH1 table data readback
    k = 0;
    while (k<20) begin
      i_bus.bus_read(32'h00014);  // read read pointer
      i_bus.bus_read(32'h00034);  // read read pointer
      k= k + 1 ;
      repeat(1737) @(posedge dac_clk);
    end



   repeat(20000) @(posedge dac_clk);

end




endmodule
