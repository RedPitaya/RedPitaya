/**
 * $Id: red_pitaya_pid.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya MIMO PID controller.
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
 * Multiple input multiple output controller.
 *
 *
 *                 /-------\       /-----------\
 *   CHA -----+--> | PID11 | ------| SUM & SAT | ---> CHA
 *            |    \-------/       \-----------/
 *            |                            ^
 *            |    /-------\               |
 *            ---> | PID21 | ----------    |
 *                 \-------/           |   |
 *                                     |   |
 *  INPUT                              |   |         OUTPUT
 *                                     |   |
 *                 /-------\           |   |
 *            ---> | PID12 | --------------
 *            |    \-------/           |    
 *            |                        ˇ
 *            |    /-------\       /-----------\
 *   CHB -----+--> | PID22 | ------| SUM & SAT | ---> CHB
 *                 \-------/       \-----------/
 *
 *
 * MIMO controller is build from four equal submodules, each can have 
 * different settings.
 *
 * Each output is sum of two controllers with different input. That sum is also
 * saturated to protect from wrapping.
 * 
 */



module red_pitaya_pid
(
   // signals
   input                 clk_i           ,  //!< processing clock
   input                 rstn_i          ,  //!< processing reset - active low
   input      [ 14-1: 0] dat_a_i         ,  //!< input data CHA
   input      [ 14-1: 0] dat_b_i         ,  //!< input data CHB
   output     [ 14-1: 0] dat_a_o         ,  //!< output data CHA
   output     [ 14-1: 0] dat_b_o         ,  //!< output data CHB
  
   // system bus
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


localparam  PSR = 12         ;
localparam  ISR = 18         ;
localparam  DSR = 10         ;


//---------------------------------------------------------------------------------
//  PID 11

wire [ 14-1: 0] pid_11_out   ;
reg  [ 14-1: 0] set_11_sp    ;
reg  [ 14-1: 0] set_11_kp    ;
reg  [ 14-1: 0] set_11_ki    ;
reg  [ 14-1: 0] set_11_kd    ;
reg             set_11_irst  ;

red_pitaya_pid_block #(
  .PSR (  PSR   ),
  .ISR (  ISR   ),
  .DSR (  DSR   )      
)
i_pid11
(
   // data
  .clk_i        (  clk_i          ),  // clock
  .rstn_i       (  rstn_i         ),  // reset - active low
  .dat_i        (  dat_a_i        ),  // input data
  .dat_o        (  pid_11_out     ),  // output data

   // settings
  .set_sp_i     (  set_11_sp      ),  // set point
  .set_kp_i     (  set_11_kp      ),  // Kp
  .set_ki_i     (  set_11_ki      ),  // Ki
  .set_kd_i     (  set_11_kd      ),  // Kd
  .int_rst_i    (  set_11_irst    )   // integrator reset
);




//---------------------------------------------------------------------------------
//  PID 21

wire [ 14-1: 0] pid_21_out   ;
reg  [ 14-1: 0] set_21_sp    ;
reg  [ 14-1: 0] set_21_kp    ;
reg  [ 14-1: 0] set_21_ki    ;
reg  [ 14-1: 0] set_21_kd    ;
reg             set_21_irst  ;

red_pitaya_pid_block #(
  .PSR (  PSR   ),
  .ISR (  ISR   ),
  .DSR (  DSR   )      
)
i_pid21
(
   // data
  .clk_i        (  clk_i          ),  // clock
  .rstn_i       (  rstn_i         ),  // reset - active low
  .dat_i        (  dat_a_i        ),  // input data
  .dat_o        (  pid_21_out     ),  // output data

   // settings
  .set_sp_i     (  set_21_sp      ),  // set point
  .set_kp_i     (  set_21_kp      ),  // Kp
  .set_ki_i     (  set_21_ki      ),  // Ki
  .set_kd_i     (  set_21_kd      ),  // Kd
  .int_rst_i    (  set_21_irst    )   // integrator reset
);




//---------------------------------------------------------------------------------
//  PID 12

wire [ 14-1: 0] pid_12_out   ;
reg  [ 14-1: 0] set_12_sp    ;
reg  [ 14-1: 0] set_12_kp    ;
reg  [ 14-1: 0] set_12_ki    ;
reg  [ 14-1: 0] set_12_kd    ;
reg             set_12_irst  ;

red_pitaya_pid_block #(
  .PSR (  PSR   ),
  .ISR (  ISR   ),
  .DSR (  DSR   )      
)
i_pid12
(
   // data
  .clk_i        (  clk_i          ),  // clock
  .rstn_i       (  rstn_i         ),  // reset - active low
  .dat_i        (  dat_b_i        ),  // input data
  .dat_o        (  pid_12_out     ),  // output data

   // settings
  .set_sp_i     (  set_12_sp      ),  // set point
  .set_kp_i     (  set_12_kp      ),  // Kp
  .set_ki_i     (  set_12_ki      ),  // Ki
  .set_kd_i     (  set_12_kd      ),  // Kd
  .int_rst_i    (  set_12_irst    )   // integrator reset
);




//---------------------------------------------------------------------------------
//  PID 22

wire [ 14-1: 0] pid_22_out   ;
reg  [ 14-1: 0] set_22_sp    ;
reg  [ 14-1: 0] set_22_kp    ;
reg  [ 14-1: 0] set_22_ki    ;
reg  [ 14-1: 0] set_22_kd    ;
reg             set_22_irst  ;

red_pitaya_pid_block #(
  .PSR (  PSR   ),
  .ISR (  ISR   ),
  .DSR (  DSR   )      
)
i_pid22
(
   // data
  .clk_i        (  clk_i          ),  // clock
  .rstn_i       (  rstn_i         ),  // reset - active low
  .dat_i        (  dat_b_i        ),  // input data
  .dat_o        (  pid_22_out     ),  // output data

   // settings
  .set_sp_i     (  set_22_sp      ),  // set point
  .set_kp_i     (  set_22_kp      ),  // Kp
  .set_ki_i     (  set_22_ki      ),  // Ki
  .set_kd_i     (  set_22_kd      ),  // Kd
  .int_rst_i    (  set_22_irst    )   // integrator reset
);





//---------------------------------------------------------------------------------
//  Sum and saturation

wire [ 15-1: 0] out_1_sum   ;
reg  [ 14-1: 0] out_1_sat   ;
wire [ 15-1: 0] out_2_sum   ;
reg  [ 14-1: 0] out_2_sat   ;

assign out_1_sum = $signed(pid_11_out) + $signed(pid_12_out);
assign out_2_sum = $signed(pid_22_out) + $signed(pid_21_out);

always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      out_1_sat <= 14'd0 ;
      out_2_sat <= 14'd0 ;
   end
   else begin
      if (out_1_sum[15-1:15-2]==2'b01) // postitive sat
         out_1_sat <= 14'h1FFF ;
      else if (out_1_sum[15-1:15-2]==2'b10) // negative sat
         out_1_sat <= 14'h2000 ;
      else
         out_1_sat <= out_1_sum[14-1:0] ;

      if (out_2_sum[15-1:15-2]==2'b01) // postitive sat
         out_2_sat <= 14'h1FFF ;
      else if (out_2_sum[15-1:15-2]==2'b10) // negative sat
         out_2_sat <= 14'h2000 ;
      else
         out_2_sat <= out_2_sum[14-1:0] ;
   end
end


assign dat_a_o = out_1_sat ;
assign dat_b_o = out_2_sat ;














//---------------------------------------------------------------------------------
//
//  System bus connection


always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      set_11_sp    <= 14'd0 ;
      set_11_kp    <= 14'd0 ;
      set_11_ki    <= 14'd0 ;
      set_11_kd    <= 14'd0 ;
      set_11_irst  <=  1'b1 ;
      set_12_sp    <= 14'd0 ;
      set_12_kp    <= 14'd0 ;
      set_12_ki    <= 14'd0 ;
      set_12_kd    <= 14'd0 ;
      set_12_irst  <=  1'b1 ;
      set_21_sp    <= 14'd0 ;
      set_21_kp    <= 14'd0 ;
      set_21_ki    <= 14'd0 ;
      set_21_kd    <= 14'd0 ;
      set_21_irst  <=  1'b1 ;
      set_22_sp    <= 14'd0 ;
      set_22_kp    <= 14'd0 ;
      set_22_ki    <= 14'd0 ;
      set_22_kd    <= 14'd0 ;
      set_22_irst  <=  1'b1 ;

   end
   else begin
      if (wen) begin
         if (addr[19:0]==16'h0)    {set_22_irst,set_21_irst,set_12_irst,set_11_irst} <= wdata[ 4-1:0] ;

         if (addr[19:0]==16'h10)    set_11_sp  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h14)    set_11_kp  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h18)    set_11_ki  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h1C)    set_11_kd  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h20)    set_12_sp  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h24)    set_12_kp  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h28)    set_12_ki  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h2C)    set_12_kd  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h30)    set_21_sp  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h34)    set_21_kp  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h38)    set_21_ki  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h3C)    set_21_kd  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h40)    set_22_sp  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h44)    set_22_kp  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h48)    set_22_ki  <= wdata[14-1:0] ;
         if (addr[19:0]==16'h4C)    set_22_kd  <= wdata[14-1:0] ;
      end
   end
end





always @(*) begin
   err <= 1'b0 ;

   casez (addr[19:0])
      20'h00 : begin ack <= 1'b1;          rdata <= {{32- 4{1'b0}}, set_22_irst,set_21_irst,set_12_irst,set_11_irst}       ; end 

      20'h10 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_11_sp}          ; end 
      20'h14 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_11_kp}          ; end 
      20'h18 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_11_ki}          ; end 
      20'h1C : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_11_kd}          ; end 

      20'h20 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_12_sp}          ; end 
      20'h24 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_12_kp}          ; end 
      20'h28 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_12_ki}          ; end 
      20'h2C : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_12_kd}          ; end 

      20'h30 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_21_sp}          ; end 
      20'h34 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_21_kp}          ; end 
      20'h38 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_21_ki}          ; end 
      20'h3C : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_21_kd}          ; end 

      20'h40 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_22_sp}          ; end 
      20'h44 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_22_kp}          ; end 
      20'h48 : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_22_ki}          ; end 
      20'h4C : begin ack <= 1'b1;          rdata <= {{32-14{1'b0}}, set_22_kd}          ; end 

     default : begin ack <= 1'b1;          rdata <=  32'h0                              ; end
   endcase
end






// bridge between processing and sys clock
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

   .clk_i         (  clk_i          ),
   .rstn_i        (  rstn_i         ),
   .addr_o        (  addr           ),
   .wdata_o       (  wdata          ),
   .wen_o         (  wen            ),
   .ren_o         (  ren            ),
   .rdata_i       (  rdata          ),
   .err_i         (  err            ),
   .ack_i         (  ack            )
);






endmodule

