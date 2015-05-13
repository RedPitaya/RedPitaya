/**
 * $Id: red_pitaya_daisy.v 964 2014-01-24 12:58:17Z matej.oblak $
 *
 * @brief Red Pitaya daisy chain communication module.
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
 * Connection of multiple boards can be done with this module.
 *
 *
 *             /------\
 *             | SER  |
 *   RX -----> |  ->  | ------+---------> RX
 *             | PAR  |       |
 *             \------/       |
 *                         /------\
 *  SERIAL                 | TEST |     PARALLEL
 *                         \------/
 *             /------\       |
 *             | PAR  |       |
 *   TX <----- |  ->  | <-----+---------- TX
 *             | SER  |
 *             \------/
 *
 *
 * To communicate with other boards with some basic data transfer daisy chain 
 * module can be used. Connection is made via fast serial lines with separate
 * clock and data. Module consists from multiple submodules.
 *
 * TX submodule serialize parallel data, which can be selected with tx_cfg_sel
 * switch. There is option for user data, training or manual value, loopback...
 *
 * RX submodule de-serialize input data and when in training mode looks for
 * predefined value.
 *
 * Testing submodule creates random values which can be selected to be used by TX
 * module. Then after some time check received values and compares if they are
 * the same.
 * 
 */




module red_pitaya_daisy
(
   // SATA connector
   output     [  2-1: 0] daisy_p_o       ,  //!< TX data and clock [1]-clock, [0]-data
   output     [  2-1: 0] daisy_n_o       ,  //!< TX data and clock [1]-clock, [0]-data
   input      [  2-1: 0] daisy_p_i       ,  //!< RX data and clock [1]-clock, [0]-data
   input      [  2-1: 0] daisy_n_i       ,  //!< RX data and clock [1]-clock, [0]-data

   // Parallel data
   input                 ser_clk_i       ,  //!< high speed serial clock, used for TX
   input                 dly_clk_i       ,  //!< delay clock, used for IDELAY
   // TX port
   input                 par_clk_i       ,  //!< parallel TX clock - data clock, must be in ration with ser_clk_i
   input                 par_rstn_i      ,  //!< parallel TX reset - active low
   output                par_rdy_o       ,  //!< parallel TX data - ready to receive new
   input                 par_dv_i        ,  //!< parallel TX data valid
   input      [ 16-1: 0] par_dat_i       ,  //!< parallel TX data to send
   // RX port
   output                par_clk_o       ,  //!< parallel RX clock   !!! not in relation with TX par_clk_i !!!
   output                par_rstn_o      ,  //!< parallel RX reset - active low
   output                par_dv_o        ,  //!< parallel RX data valid
   output     [ 16-1: 0] par_dat_o       ,  //!< parallel RX data received

   output     [  8-1: 0] debug_o         ,  //!< some debug

   // System bus
   input                 sys_clk_i       ,  //!< bus clock
   input                 sys_rstn_i      ,  //!< bus reset - active low
   input      [ 32-1: 0] sys_addr_i      ,  //!< bus address
   input      [ 32-1: 0] sys_wdata_i     ,  //!< bus write data
   input      [  4-1: 0] sys_sel_i       ,  //!< bus write byte select
   input                 sys_wen_i       ,  //!< bus write enable
   input                 sys_ren_i       ,  //!< bus read enable
   output reg [ 32-1: 0] sys_rdata_o     ,  //!< bus read data
   output reg            sys_err_o       ,  //!< bus error indicator
   output reg            sys_ack_o          //!< bus acknowledge signal
);





// in[1] -> out[1] clock
// in[0] -> out[0] data

//---------------------------------------------------------------------------------
//
//  Transmiter

reg            cfg_tx_en     ;
reg  [32-1: 0] cfg_tx_sys    ;
reg            cfg_tx_sys_n  ;

wire           txs_clk       ;
wire           txs_dat       ;
wire           txp_rdy       ;
reg            txp_dv        ;
reg  [16-1: 0] txp_dat       ;


OBUFDS #(.IOSTANDARD ("DIFF_SSTL18_I"), .SLEW ("FAST")) i_OBUF_clk
(
  .O  ( daisy_p_o[1]  ),
  .OB ( daisy_n_o[1]  ),
  .I  ( txs_clk       )
);

OBUFDS #(.IOSTANDARD ("DIFF_SSTL18_I"), .SLEW ("FAST")) i_OBUF_dat
(
  .O  ( daisy_p_o[0]  ),
  .OB ( daisy_n_o[0]  ),
  .I  ( txs_dat       )
);



red_pitaya_daisy_tx i_tx
(
   // serial ports
  .ser_clk_i       (  ser_clk_i        ),
  .ser_clk_o       (  txs_clk          ),
  .ser_dat_o       (  txs_dat          ),

   // paralel ports
  .par_clk_i       (  par_clk_i        ),
  .par_rstn_i      (  cfg_tx_en        ),

  .par_rdy_o       (  txp_rdy          ),
  .par_dv_i        (  txp_dv           ),
  .par_dat_i       (  txp_dat          ) 
);









//---------------------------------------------------------------------------------
//
//  Reciever

reg            cfg_rx_en        ;
reg            cfg_rx_train     ;
wire           cfg_rx_trained   ;

wire           rxs_clk          ;
wire           rxs_dat          ;
wire           rxp_clk          ;
wire           rxp_rstn         ;
wire           rxp_dv           ;
wire [16-1: 0] rxp_dat          ;


IBUFGDS #(.IOSTANDARD ("DIFF_SSTL18_I")) i_IBUFGDS_clk
(
  .I  ( daisy_p_i[1]  ),
  .IB ( daisy_n_i[1]  ),
  .O  ( rxs_clk     )
);

IBUFDS #(.DIFF_TERM ("FALSE"), .IOSTANDARD ("DIFF_SSTL18_I")) i_IBUFDS_dat
(
  .I  ( daisy_p_i[0]  ),
  .IB ( daisy_n_i[0]  ),
  .O  ( rxs_dat       )
);



red_pitaya_daisy_rx i_rx
(
   // serial ports
  .ser_clk_i       (  rxs_clk            ),
  .ser_dat_i       (  rxs_dat            ),

   // paralel ports
  .cfg_en_i        (  cfg_rx_en          ),
  .cfg_train_i     (  cfg_rx_train       ),
  .cfg_trained_o   (  cfg_rx_trained     ),
  .dly_clk_i       (  dly_clk_i          ),

  .par_clk_o       (  rxp_clk            ), // rxp_clk is not the same as par_clk_i (its par_clk_i from transmitter)!!!
  .par_rstn_o      (  rxp_rstn           ),
  .par_dv_o        (  rxp_dv             ),
  .par_dat_o       (  rxp_dat            ) 
);









//---------------------------------------------------------------------------------
//
//  Testing module

reg            cfg_tst_clr       ;

wire           tst_dv            ;
wire [16-1: 0] tst_dat           ;
wire [32-1: 0] tst_err_cnt       ;
wire [32-1: 0] tst_dat_cnt       ;

red_pitaya_daisy_test i_test
(
   // transmit ports
  .tx_clk_i        (  par_clk_i         ),
  .tx_rstn_i       (  par_rstn_i        ),
  .tx_rdy_i        (  txp_rdy           ),
  .tx_dv_o         (  tst_dv            ),
  .tx_dat_o        (  tst_dat           ),

   // receive ports
  .rx_clk_i        (  rxp_clk           ),
  .rx_rstn_i       (  rxp_rstn          ),
  .rx_dv_i         (  rxp_dv            ),
  .rx_dat_i        (  rxp_dat           ),

  .stat_clr_i      (  cfg_tst_clr       ),
  .stat_err_o      (  tst_err_cnt       ),
  .stat_dat_o      (  tst_dat_cnt       )
);







//---------------------------------------------------------------------------------
//
//  Data selector

reg  [ 4-1: 0] tx_cfg_new   ;
reg  [ 3-1: 0] tx_cfg_sel   ;
reg  [16-1: 0] tx_cfg_dat   ; 

reg            rxp_dv_n     ;
reg  [16-1: 0] rxp_dat_n    ; 
reg  [ 4-1: 0] tx_rx_new    ;
reg            tx_rx_dv     ;
reg  [16-1: 0] tx_rx_dat    ; 

always @(posedge par_clk_i) begin
   if (par_rstn_i == 1'b0) begin
      tx_cfg_new <=  4'h0 ;
      tx_cfg_sel <=  3'h0 ;
      tx_cfg_dat <= 16'h0 ;
      tx_rx_new  <=  4'h0 ;
      tx_rx_dv   <=  1'b0 ;
      tx_rx_dat  <= 16'h0 ; 
   end
   else begin
      // sync custom data
      tx_cfg_new <= {tx_cfg_new[4-2:0], cfg_tx_sys_n};
      if (tx_cfg_new[4-2] ^ tx_cfg_new[4-1]) begin
         tx_cfg_sel <= cfg_tx_sys[ 3-1: 0] ;
         tx_cfg_dat <= cfg_tx_sys[32-1:16] ; 
      end

      // sync received data
      tx_rx_new <= {tx_rx_new[4-2:0], rxp_dv_n};
      if (tx_rx_new[4-2] ^ tx_rx_new[4-1]) begin
         tx_rx_dv  <= 1'b1      ;
         tx_rx_dat <= rxp_dat_n ; 
      end
      else if (txp_rdy)
         tx_rx_dv  <= 1'b0 ;

   end
end

// output data selector
always @(*) begin
   case (tx_cfg_sel)
      3'h0 : begin txp_dat <= 16'h0         ;   txp_dv <= 1'b0         ; end
      3'h1 : begin txp_dat <= par_dat_i     ;   txp_dv <= par_dv_i     ; end  // working data
      3'h2 : begin txp_dat <= tx_cfg_dat    ;   txp_dv <= txp_rdy      ; end  // manual value
      3'h3 : begin txp_dat <= 16'h00FF      ;   txp_dv <= txp_rdy      ; end  // training data
      3'h4 : begin txp_dat <= tx_rx_dat     ;   txp_dv <= tx_rx_dv     ; end  // send back received data
      3'h5 : begin txp_dat <= tst_dat       ;   txp_dv <= tst_dv       ; end  // random testing data
   endcase
end




// latch received data if not zero
always @(posedge rxp_clk) begin
   if (rxp_rstn == 1'b0 ) begin
      rxp_dv_n  <=  1'b0 ;
      rxp_dat_n <= 16'h0 ; 
   end
   else begin
      if (rxp_dv && (rxp_dat != 16'h0) ) begin
         rxp_dv_n  <= !rxp_dv_n ;
         rxp_dat_n <=  rxp_dat  ;
      end
   end
end


// output assignments
assign par_clk_o  = rxp_clk   ;
assign par_rstn_o = rxp_rstn  ;
assign par_rdy_o  = txp_rdy && (tx_cfg_sel == 3'h1) ;
assign par_dv_o   = rxp_dv    ;
assign par_dat_o  = rxp_dat   ;
















//---------------------------------------------------------------------------------
//
//  System bus connection

wire sys_ack = sys_wen_i || sys_ren_i ;

always @(posedge sys_clk_i) begin
   if (sys_rstn_i == 1'b0) begin
      cfg_tx_en       <=  1'b0 ;
      cfg_tx_sys      <= 32'h0 ;
      cfg_tx_sys_n    <=  1'b0 ;
      cfg_rx_en       <=  1'b0 ;
      cfg_rx_train    <=  1'b0 ;
      cfg_tst_clr     <=  1'b0 ;
   end
   else begin
      if (sys_wen_i) begin
         if (sys_addr_i[19:0]==20'h00)           cfg_tx_en        <= sys_wdata_i[  0] ;
         if (sys_addr_i[19:0]==20'h00)           cfg_rx_en        <= sys_wdata_i[  1] ;

         if (sys_addr_i[19:0]==20'h04)     begin cfg_tx_sys       <= sys_wdata_i      ;     cfg_tx_sys_n <= !cfg_tx_sys_n;    end
         if (sys_addr_i[19:0]==20'h08)           cfg_rx_train     <= sys_wdata_i[  0] ;
         if (sys_addr_i[19:0]==20'h10)           cfg_tst_clr      <= sys_wdata_i[  0] ;
      end
   end
end





always @(posedge sys_clk_i) begin
   sys_err_o <= 1'b0 ;

   casez (sys_addr_i[19:0])
     20'h00000 : begin sys_ack_o <= sys_ack;       sys_rdata_o <= { {32-2{1'b0}}, cfg_rx_en, cfg_tx_en }                     ; end

     20'h00004 : begin sys_ack_o <= sys_ack;       sys_rdata_o <= { cfg_tx_sys[32-1:16], 12'h0, cfg_tx_sys[3-1:0] }          ; end
     20'h00008 : begin sys_ack_o <= sys_ack;       sys_rdata_o <= { 27'h0, cfg_rx_trained, 3'h0, cfg_rx_train }              ; end
     20'h0000C : begin sys_ack_o <= sys_ack;       sys_rdata_o <= { rxp_dat_n, rxp_dat }                                     ; end

     20'h00010 : begin sys_ack_o <= sys_ack;       sys_rdata_o <= { {32-1{1'b0}}, cfg_tst_clr }                              ; end
     20'h00014 : begin sys_ack_o <= sys_ack;       sys_rdata_o <= { tst_err_cnt }                                            ; end
     20'h00018 : begin sys_ack_o <= sys_ack;       sys_rdata_o <= { tst_dat_cnt }                                            ; end

       default : begin sys_ack_o <= 1'b1;          sys_rdata_o <=   32'h0                                                    ; end
   endcase
end







//---------------------------------------------------------------------------------
//
//  Debug connections

reg  [32-1: 0] dd_par_cnt     ;

always @(posedge rxp_clk)   begin if (rxp_rstn == 1'b0 ) dd_par_cnt <= 32'h0; else dd_par_cnt <= dd_par_cnt + 32'h1; end

assign debug_o = {dd_par_cnt[26], 1'b0, cfg_rx_trained, cfg_rx_train, 1'b0, 1'b0, cfg_rx_en, cfg_tx_en};




endmodule

