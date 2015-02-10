/**
 * $Id: red_pitaya_daisy_tb.v 964 2014-01-24 12:58:17Z matej.oblak $
 *
 * @brief Red Pitaya daisy chain testbench.
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
 * Testbench to test daisy chain connection.
 *
 * Testing daisy connection using single daisy chain module connected via
 * loopback. Commands makes initial training and than goes into mode of 
 * sending and checking received random numbers. 
 * 
 */






`timescale 1ns / 1ps

module red_pitaya_daisy_tb(
);



wire  [  4-1: 0] daisy_p         ;
wire  [  4-1: 0] daisy_n         ;
reg              par_clk         ;
reg              par_rstn        ;
reg              ser_clk         ;
reg              dly_clk         ;

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


reg   [ 16-1: 0] test_cnt        ;
reg              test_val        ;
wire             test_rdy        ;



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



red_pitaya_daisy i_daisy
(
   // SATA connector
  .daisy_p_o       ( daisy_p[1:0]  ),  //!< TX data and clock [1]-clock, [0]-data
  .daisy_n_o       ( daisy_n[1:0]  ),  //!< TX data and clock [1]-clock, [0]-data
  .daisy_p_i       ( daisy_p[3:2]  ),  //!< RX data and clock [1]-clock, [0]-data
  .daisy_n_i       ( daisy_n[3:2]  ),  //!< RX data and clock [1]-clock, [0]-data

   // Data
  .ser_clk_i       (  ser_clk    ),
  .dly_clk_i       (  dly_clk    ),
   // TX
  .par_clk_i       (  par_clk    ),  // clock
  .par_rstn_i      (  par_rstn   ),  // reset - active low
  .par_rdy_o       (  test_rdy   ),
  .par_dv_i        (  test_val   ),
  .par_dat_i       (  test_cnt   ),

   // RX
  .par_clk_o       (    ),  // receiver clock   !!! not in relation with par_clk_i !!!
  .par_rstn_o      (    ),
  .par_dv_o        (    ),
  .par_dat_o       (    ),


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
   par_clk  <= 1'b0  ;
   par_rstn <= 1'b0  ;
   repeat(10) @(posedge par_clk);
      par_rstn <= 1'b1  ;
end
always begin
   #4  par_clk <= !par_clk ;
end



initial begin
   ser_clk  <= 1'b1  ;
end
always begin
   #2  ser_clk <= !ser_clk ; //2 for 250MHz
end



initial begin
   dly_clk  <= 1'b1  ;
end
always begin
   #2.5  dly_clk <= !dly_clk ;
end



assign #0.2 daisy_p[3] = daisy_p[1] ;
assign #0.2 daisy_n[3] = daisy_n[1] ;
assign #0.2 daisy_p[2] = daisy_p[0] ;
assign #0.2 daisy_n[2] = daisy_n[0] ;



always @(posedge par_clk) begin
   if (!par_rstn) begin
      test_cnt <= 16'h0  ;
      test_val <=  1'b0  ;
   end
   else begin
      test_val <= test_rdy ;

      if (test_val)
         test_cnt <= test_cnt + 16'h1  ;

   end
end













// Commands on bus

initial begin
   wait (sys_rstn && par_rstn)
   repeat(100) @(posedge sys_clk);
      //Enable transmitter
      i_bus.bus_write(32'h0, 32'h1      );

   repeat(20) @(posedge sys_clk);
      //Enable transmitter & receiver
      i_bus.bus_write(32'h0, 32'h3      );


   repeat(101) @(posedge sys_clk);
      //enable TX train
      i_bus.bus_write(32'h4, 32'h3      );

   repeat(10) @(posedge sys_clk);
      //enable RX train
      i_bus.bus_write(32'h8, 32'h1      );


   repeat(500) @(posedge sys_clk);
      //Return read value
      i_bus.bus_read(32'hC);

   repeat(20) @(posedge sys_clk);
      //disable RX train
      i_bus.bus_write(32'h8, 32'h0      );

   repeat(20) @(posedge sys_clk);
      //Custom value
      i_bus.bus_write(32'h4, {16'hF419, 16'h2}   );

   repeat(20) @(posedge sys_clk);
      //Random value
      i_bus.bus_write(32'h4, {16'hF419, 16'h5}   );

   repeat(20) @(posedge sys_clk);
      //Clear error counter
      i_bus.bus_write(32'h10, 32'h1      );
   repeat(20) @(posedge sys_clk);
      //Enable error counter
      i_bus.bus_write(32'h10, 32'h0      );

   repeat(404) @(posedge sys_clk);
      //Sent back read value
      i_bus.bus_write(32'h4, {16'h0, 16'h4}      );




   repeat(20000) @(posedge par_clk);

end




endmodule
