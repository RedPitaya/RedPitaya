/**
 * $Id: red_pitaya_asg.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya arbitrary signal generator (ASG).
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
 * Arbitrary signal generator takes data stored in buffer and sends them to DAC.
 *
 *
 *                /-----\         /--------\
 *   SW --------> | BUF | ------> | kx + o | ---> DAC CHA
 *                \-----/         \--------/
 *                   ^
 *                   |
 *                /-----\
 *   SW --------> |     |
 *                | FSM | ------> trigger notification
 *   trigger ---> |     |
 *                \-----/
 *                   |
 *                   ˇ
 *                /-----\         /--------\
 *   SW --------> | BUF | ------> | kx + o | ---> DAC CHB
 *                \-----/         \--------/ 
 *
 *
 * Buffers are filed with SW. It also sets finite state machine which take control
 * over read pointer. All registers regarding reading from buffer has additional 
 * 16 bits used as decimal points. In this way we can make better ratio betwen 
 * clock cycle and frequency of output signal. 
 *
 * Finite state machine can be set for one time sequence or continously wrapping.
 * Starting trigger can come from outside, notification trigger used to synchronize
 * with other applications (scope) is also available. Both channels are independant.
 *
 * Output data is scaled with linear transmormation.
 * 
 */



module red_pitaya_asg
(
   // DAC
   output     [ 14-1: 0] dac_a_o         ,  //!< DAC data CHA
   output     [ 14-1: 0] dac_b_o         ,  //!< DAC data CHB
   input                 dac_clk_i       ,  //!< DAC clock
   input                 dac_rstn_i      ,  //!< DAC reset - active low
   input                 trig_a_i        ,  //!< starting trigger CHA
   input                 trig_b_i        ,  //!< starting trigger CHB
   output                trig_out_o      ,  //!< notification trigger

   // System bus
   input                 sys_clk_i       ,  //!< bus clock
   input                 sys_rstn_i      ,  //!< bus reset - active low
   input      [ 32-1: 0] sys_addr_i      ,  //!< bus address
   input      [ 32-1: 0] sys_wdata_i     ,  //!< bus write data
   input      [  4-1: 0] sys_sel_i       ,  //!< bus write byte select
   input                 sys_wen_i       ,  //!< bus write enable
   input                 sys_ren_i       ,  //!< bus read enable
   output     [ 32-1: 0] sys_rdata_o     ,  //!< bus read data
   output                sys_err_o       ,  //!< bus error indicator
   output                sys_ack_o          //!< bus acknowledge signal
);



wire [ 32-1: 0] addr         ;
wire [ 32-1: 0] wdata        ;
wire            wen          ;
wire            ren          ;
reg  [ 32-1: 0] rdata        ;
reg             err          ;
reg             ack          ;


//---------------------------------------------------------------------------------
//
// generating signal from DAC table 

localparam RSZ = 14 ;  // RAM size 2^RSZ

reg   [RSZ+15: 0] set_a_size       ;
reg   [RSZ+15: 0] set_a_step       ;
reg   [RSZ+15: 0] set_a_ofs        ;
reg               set_a_rst        ;
reg               set_a_once       ;
reg               set_a_wrap       ;
reg   [  14-1: 0] set_a_amp        ;
reg   [  14-1: 0] set_a_dc         ;
reg               set_a_zero       ;
reg               buf_a_we         ;
reg   [ RSZ-1: 0] buf_a_addr       ;
wire  [  14-1: 0] buf_a_rdata      ;
reg               trig_a_sw        ;
reg   [   3-1: 0] trig_a_src       ;
wire              trig_a_done      ;


reg   [RSZ+15: 0] set_b_size       ;
reg   [RSZ+15: 0] set_b_step       ;
reg   [RSZ+15: 0] set_b_ofs        ;
reg               set_b_rst        ;
reg               set_b_once       ;
reg               set_b_wrap       ;
reg   [  14-1: 0] set_b_amp        ;
reg   [  14-1: 0] set_b_dc         ;
reg               set_b_zero       ;
reg               buf_b_we         ;
reg   [ RSZ-1: 0] buf_b_addr       ;
wire  [  14-1: 0] buf_b_rdata      ;
reg               trig_b_sw        ;
reg   [   3-1: 0] trig_b_src       ;
wire              trig_b_done      ;


red_pitaya_asg_ch  #(.RSZ ( RSZ ))  i_cha
(
   // DAC
  .dac_o           (  dac_a_o          ),  // dac data output
  .dac_clk_i       (  dac_clk_i        ),  // dac clock
  .dac_rstn_i      (  dac_rstn_i       ),  // dac reset - active low

   // trigger
  .trig_sw_i       (  trig_a_sw        ),  // software trigger
  .trig_ext_i      (  trig_a_i         ),  // external trigger
  .trig_src_i      (  trig_a_src       ),  // trigger source selector
  .trig_done_o     (  trig_a_done      ),  // trigger event

   // buffer ctrl
  .buf_we_i        (  buf_a_we         ),  // buffer buffer write
  .buf_addr_i      (  buf_a_addr       ),  // buffer address
  .buf_wdata_i     (  wdata[14-1:0]    ),  // buffer write data
  .buf_rdata_o     (  buf_a_rdata      ),  // buffer read data

   // configuration
  .set_size_i      (  set_a_size       ),  // set table data size
  .set_step_i      (  set_a_step       ),  // set pointer step
  .set_ofs_i       (  set_a_ofs        ),  // set reset offset
  .set_rst_i       (  set_a_rst        ),  // set FMS to reset
  .set_once_i      (  set_a_once       ),  // set only once
  .set_wrap_i      (  set_a_wrap       ),  // set wrap pointer
  .set_amp_i       (  set_a_amp        ),  // set amplitude scale
  .set_dc_i        (  set_a_dc         ),  // set output offset
  .set_zero_i      (  set_a_zero       )   // set output to zero
);


red_pitaya_asg_ch  #(.RSZ ( RSZ ))  i_chb
(
   // DAC
  .dac_o           (  dac_b_o          ),  // dac data output
  .dac_clk_i       (  dac_clk_i        ),  // dac clock
  .dac_rstn_i      (  dac_rstn_i       ),  // dac reset - active low

   // trigger
  .trig_sw_i       (  trig_b_sw        ),  // software trigger
  .trig_ext_i      (  trig_b_i         ),  // external trigger
  .trig_src_i      (  trig_b_src       ),  // trigger source selector
  .trig_done_o     (  trig_b_done      ),  // trigger event

   // buffer ctrl
  .buf_we_i        (  buf_b_we         ),  // buffer buffer write
  .buf_addr_i      (  buf_b_addr       ),  // buffer address
  .buf_wdata_i     (  wdata[14-1:0]    ),  // buffer write data
  .buf_rdata_o     (  buf_b_rdata      ),  // buffer read data

   // configuration
  .set_size_i      (  set_b_size       ),  // set table data size
  .set_step_i      (  set_b_step       ),  // set pointer step
  .set_ofs_i       (  set_b_ofs        ),  // set reset offset
  .set_rst_i       (  set_b_rst        ),  // set FMS to reset
  .set_once_i      (  set_b_once       ),  // set only once
  .set_wrap_i      (  set_b_wrap       ),  // set wrap pointer
  .set_amp_i       (  set_b_amp        ),  // set amplitude scale
  .set_dc_i        (  set_b_dc         ),  // set output offset
  .set_zero_i      (  set_b_zero       )   // set output to zero
);

always @(posedge dac_clk_i) begin
   buf_a_we   <= wen && (addr[19:RSZ+2] == 'h1);
   buf_b_we   <= wen && (addr[19:RSZ+2] == 'h2);
   buf_a_addr <= addr[RSZ+1:2] ;  // address timing violation
   buf_b_addr <= addr[RSZ+1:2] ;  // can change only synchronous to write clock
end

assign trig_out_o = trig_a_done ;

//---------------------------------------------------------------------------------
//
//  System bus connection

reg  [3-1: 0] ren_dly ;
reg           ack_dly ;

always @(posedge dac_clk_i) begin
   if (dac_rstn_i == 1'b0) begin
      trig_a_sw  <=  1'b0    ;
      trig_a_src <=  3'h0    ;
      set_a_amp  <= 14'h2000 ;
      set_a_dc   <= 14'h0    ;
      set_a_zero <=  1'b0    ;
      set_a_rst  <=  1'b0    ;
      set_a_once <=  1'b0    ;
      set_a_wrap <=  1'b0    ;
      set_a_size <= {RSZ+16{1'b1}} ;
      set_a_ofs  <= {RSZ+16{1'b0}} ;
      set_a_step <={{RSZ+15{1'b0}},1'b0} ;
      trig_b_sw  <=  1'b0    ;
      trig_b_src <=  3'h0    ;
      set_b_amp  <= 14'h2000 ;
      set_b_dc   <= 14'h0    ;
      set_b_zero <=  1'b0    ;
      set_b_rst  <=  1'b0    ;
      set_b_once <=  1'b0    ;
      set_b_wrap <=  1'b0    ;
      set_b_size <= {RSZ+16{1'b1}} ;
      set_b_ofs  <= {RSZ+16{1'b0}} ;
      set_b_step <={{RSZ+15{1'b0}},1'b0} ;
      ren_dly    <=  3'h0    ;
      ack_dly    <=  1'b0    ;
   end
   else begin

      trig_a_sw  <= wen && (addr[19:0]==20'h0) && wdata[0]  ;
      if (wen && (addr[19:0]==20'h0))
         trig_a_src <= wdata[2:0] ;

      trig_b_sw  <= wen && (addr[19:0]==20'h0) && wdata[16]  ;
      if (wen && (addr[19:0]==20'h0))
         trig_b_src <= wdata[19:16] ;



      if (wen) begin
         if (addr[19:0]==20'h0)   {set_a_zero, set_a_rst, set_a_once, set_a_wrap} <= wdata[ 7: 4] ;
         if (addr[19:0]==20'h0)   {set_b_zero, set_b_rst, set_b_once, set_b_wrap} <= wdata[23:20] ;

         if (addr[19:0]==20'h4)   set_a_amp  <= wdata[  0+13: 0] ;
         if (addr[19:0]==20'h4)   set_a_dc   <= wdata[ 16+13:16] ;
         if (addr[19:0]==20'h8)   set_a_size <= wdata[RSZ+15: 0] ;
         if (addr[19:0]==20'hC)   set_a_ofs  <= wdata[RSZ+15: 0] ;
         if (addr[19:0]==20'h10)  set_a_step <= wdata[RSZ+15: 0] ;

         if (addr[19:0]==20'h24)  set_b_amp  <= wdata[  0+13: 0] ;
         if (addr[19:0]==20'h24)  set_b_dc   <= wdata[ 16+13:16] ;
         if (addr[19:0]==20'h28)  set_b_size <= wdata[RSZ+15: 0] ;
         if (addr[19:0]==20'h2C)  set_b_ofs  <= wdata[RSZ+15: 0] ;
         if (addr[19:0]==20'h30)  set_b_step <= wdata[RSZ+15: 0] ;
      end

      ren_dly <= {ren_dly[3-2:0], ren};
      ack_dly <=  ren_dly[3-1] || wen ;
   end
end

wire [32-1: 0] r0_rd = {8'h0,set_b_zero,set_b_rst,set_b_once,set_b_wrap, 1'b0,trig_b_src,
                        8'h0,set_a_zero,set_a_rst,set_a_once,set_a_wrap, 1'b0,trig_a_src };


always @(*) begin
   err <= 1'b0 ;

   casez (addr[19:0])
     20'h00000 : begin ack <= 1'b1;          rdata <= r0_rd                              ; end

     20'h00004 : begin ack <= 1'b1;          rdata <= {2'h0, set_a_dc, 2'h0, set_a_amp}  ; end
     20'h00008 : begin ack <= 1'b1;          rdata <= {{32-RSZ-16{1'b0}},set_a_size}     ; end
     20'h0000C : begin ack <= 1'b1;          rdata <= {{32-RSZ-16{1'b0}},set_a_ofs}      ; end
     20'h00010 : begin ack <= 1'b1;          rdata <= {{32-RSZ-16{1'b0}},set_a_step}     ; end

     20'h00024 : begin ack <= 1'b1;          rdata <= {2'h0, set_b_dc, 2'h0, set_b_amp}  ; end
     20'h00028 : begin ack <= 1'b1;          rdata <= {{32-RSZ-16{1'b0}},set_b_size}     ; end
     20'h0002C : begin ack <= 1'b1;          rdata <= {{32-RSZ-16{1'b0}},set_b_ofs}      ; end
     20'h00030 : begin ack <= 1'b1;          rdata <= {{32-RSZ-16{1'b0}},set_b_step}     ; end

     20'h1zzzz : begin ack <= ack_dly;       rdata <= {{32-14{1'b0}},buf_a_rdata}        ; end
     20'h2zzzz : begin ack <= ack_dly;       rdata <= {{32-14{1'b0}},buf_b_rdata}        ; end

       default : begin ack <= 1'b1;          rdata <=  32'h0                             ; end
   endcase
end




// bridge between DAC and sys clock
bus_clk_bridge i_bridge
(
   .sys_clk_i     (  sys_clk_i      ),
   .sys_rstn_i    (  sys_rstn_i     ),
   .sys_addr_i    (  sys_addr_i     ),
   .sys_wdata_i   (  sys_wdata_i    ),
   .sys_sel_i     (  sys_sel_i      ),
   .sys_wen_i     (  sys_wen_i      ),
   .sys_ren_i     (  sys_ren_i      ),
   .sys_rdata_o   (  sys_rdata_o    ),
   .sys_err_o     (  sys_err_o      ),
   .sys_ack_o     (  sys_ack_o      ),

   .clk_i         (  dac_clk_i      ),
   .rstn_i        (  dac_rstn_i     ),
   .addr_o        (  addr           ),
   .wdata_o       (  wdata          ),
   .wen_o         (  wen            ),
   .ren_o         (  ren            ),
   .rdata_i       (  rdata          ),
   .err_i         (  err            ),
   .ack_i         (  ack            )
);







endmodule

