/**
 * $Id: red_pitaya_daisy_test.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya daisy chain communication module. Testing module.
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
 * Testing serial connection module.
 *
 *
 *                          ----------< CFG
 *                          |
 *                  /----------\
 *   RX ----------> | COMPARE  | -----> STATUS
 *                  \----------/
 *                       ^
 *   PARALLEL            |
 *                       |
 *                  /-----------\
 *   TX <---------- | GENERATE  |
 *                  \-----------/
 *
 *
 * In order to test serial link over multiple board this module was made. Every
 * 32 cycles it sends semi-random numbers to transmitter.If retured value is the
 * same (non zero) then it increase successful counter. If value is different it 
 * increases error counter. Both counters can be reseted by SW.   
 *
 */




module red_pitaya_daisy_test
(
   // transmit ports
   input                 tx_clk_i        ,  //!< transmitter clock
   input                 tx_rstn_i       ,  //!< transmitter reset - active low
   input                 tx_rdy_i        ,  //!< transmitter ready
   output                tx_dv_o         ,  //!< transmitter data valid
   output     [ 16-1: 0] tx_dat_o        ,  //!< transmitter data

   // receive ports
   input                 rx_clk_i        ,  //!< receiver clock
   input                 rx_rstn_i       ,  //!< receiver reset - active low
   input                 rx_dv_i         ,  //!< receiver data valid
   input      [ 16-1: 0] rx_dat_i        ,  //!< receiver data

   input                 stat_clr_i      ,  //!< status clear
   output     [ 32-1: 0] stat_err_o      ,  //!< status error counter
   output     [ 32-1: 0] stat_dat_o         //!< status success counter
);





//---------------------------------------------------------------------------------
//
//  generate random numbers

wire [32-1: 0] rand_temp   ;
reg  [32-1: 0] rand_work   ;
reg  [32-1: 0] rand_dat    ;

assign rand_temp = rand_work ^ (32'h84C11DB6 & {32{rand_work[0]}}) ;  // x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1

always @(posedge tx_clk_i) begin
   if (tx_rstn_i == 1'b0) begin
      rand_work <= 32'h01010101 ; // starting seed
      rand_dat  <= 32'h0 ;
   end 
   else begin
      rand_work <= {rand_work[0], rand_temp[31:1]};

      rand_dat  <= rand_work ;
   end
end















//---------------------------------------------------------------------------------
//
//  Transmitter 

reg  [ 5-1: 0] tx_dv_cnt   ;
reg  [16-1: 0] tx_dat      ;

always @(posedge tx_clk_i) begin
   if (tx_rstn_i == 1'b0) begin
      tx_dv_cnt <=  5'h0 ;
      tx_dat    <= 16'h0 ;
   end
   else begin
      tx_dv_cnt <= tx_dv_cnt + 5'h1 ;

      if ( (tx_dv_cnt[4:2] == 3'h7) && tx_rdy_i ) // every 32 cycles (when ready)
         tx_dat <= rand_dat[15:0] ;

   end
end

assign tx_dv_o  = (tx_dv_cnt[4:2] == 3'h7) && tx_rdy_i;
assign tx_dat_o = rand_dat[15:0] ;





//---------------------------------------------------------------------------------
//
//  Receiver and verifier 

reg  [32-1: 0] rx_err_cnt  ;
reg  [32-1: 0] rx_dat_cnt  ;
reg            rx_dv       ;
reg  [16-1: 0] rx_dat      ;

always @(posedge rx_clk_i) begin
   if (rx_rstn_i == 1'b0) begin
      rx_err_cnt <= 32'h0 ;
      rx_dat_cnt <= 32'h0 ;
      rx_dv      <=  1'b0 ;
      rx_dat     <= 16'h0 ;
   end
   else begin
      rx_dv      <= rx_dv_i  ;
      rx_dat     <= rx_dat_i ;

      // counting errors
      if ( rx_dv && (rx_dat != tx_dat) && (rx_dat != 16'h0))
         rx_err_cnt <= rx_err_cnt + 32'h1 ;
      else if (stat_clr_i)
         rx_err_cnt <= 32'h0 ;

      // counting successfull transfer
      if ( rx_dv && (rx_dat == tx_dat) && (rx_dat != 16'h0))
         rx_dat_cnt <= rx_dat_cnt + 32'h1 ;
      else if (stat_clr_i)
         rx_dat_cnt <= 32'h0 ;

   end
end

assign stat_err_o = rx_err_cnt ;
assign stat_dat_o = rx_dat_cnt ;






endmodule

