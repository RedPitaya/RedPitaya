/**
 * $Id: red_pitaya_test.v 964 2014-01-24 12:58:17Z matej.oblak $
 *
 * @brief Red Pitaya power consumtion test.
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
 * Just simple module doing nothing usefull except consume power to help test 
 * power supplies. It also has generator of random signal which is usefull for
 * testing EM radiation of external signals.
 *
 * User has complete freedom to remove it from design.
 * 
 */




module red_pitaya_test
(
   // power test
   input                 clk_i           ,  //!< processing clock
   input                 rstn_i          ,  //!< processing reset - active low

   output     [ 32-1: 0] rand_o          ,

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


reg  [32-1: 0] rand_dat    ;
reg  [14-1: 0] pwr_out     ;

//---------------------------------------------------------------------------------
//
//  System bus connection

reg   [  1-1: 0] dat_en    ;
reg   [ 32-1: 0] a0_r      ;
reg   [ 32-1: 0] b0_r      ;
reg   [ 32-1: 0] a1_r      ;
reg   [ 32-1: 0] b1_r      ;

always @(posedge sys_clk_i) begin
   if (sys_rstn_i == 1'b0) begin
      dat_en   <=  1'b0        ;
      a0_r     <= 32'h01010101 ;
      b0_r     <= 32'h03030303 ;
      a1_r     <= 32'h01010101 ;
      b1_r     <= 32'h03030303 ;
   end
   else begin
      if (sys_wen_i) begin
         if (sys_addr_i[19:0]==16'h00)   dat_en <= sys_wdata_i[0] ;
         if (sys_addr_i[19:0]==16'h10)   a0_r   <= sys_wdata_i    ;
         if (sys_addr_i[19:0]==16'h14)   b0_r   <= sys_wdata_i    ;
         if (sys_addr_i[19:0]==16'h18)   a1_r   <= sys_wdata_i    ;
         if (sys_addr_i[19:0]==16'h1C)   b1_r   <= sys_wdata_i    ;
      end
   end
end





always @(*) begin
   sys_err_o <= 1'b0 ;

   casez (sys_addr_i[19:0])
     20'h00000 : begin sys_ack_o <= 1'b1;          sys_rdata_o <= {31'h0,dat_en}          ; end
     20'h00010 : begin sys_ack_o <= 1'b1;          sys_rdata_o <=  a0_r                   ; end
     20'h00014 : begin sys_ack_o <= 1'b1;          sys_rdata_o <=  b0_r                   ; end
     20'h00018 : begin sys_ack_o <= 1'b1;          sys_rdata_o <=  a1_r                   ; end
     20'h0001C : begin sys_ack_o <= 1'b1;          sys_rdata_o <=  b1_r                   ; end
     20'h00020 : begin sys_ack_o <= 1'b1;          sys_rdata_o <= {18'h0,pwr_out}         ; end
     20'h00024 : begin sys_ack_o <= 1'b1;          sys_rdata_o <=  rand_dat               ; end

       default : begin sys_ack_o <= 1'b1;          sys_rdata_o <=  32'h0                  ; end
   endcase
end






//---------------------------------------------------------------------------------
//  generate random numbers

wire           rand_wr     ;
wire [32-1: 0] rand_temp   ;
reg  [32-1: 0] rand_work   ;

assign rand_wr   = sys_wen_i && (sys_addr_i[19:0]==16'h18) ;
assign rand_temp = rand_work ^ (32'h84C11DB6 & {32{rand_work[0]}}) ;  // x32 + x26 + x23 + x22 + x16 + x12 + x11 + x10 + x8 + x7 + x5 + x4 + x2 + x + 1
assign rand_o    = rand_dat ;

always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      rand_work <= 32'h01010101 ;
      rand_dat  <= 32'h0 ;
   end 
   else begin
      if (rand_wr)
         rand_work <= a1_r ;
      else
         rand_work <= {rand_work[0], rand_temp[31:1]};


      rand_dat <= rand_work ;
   end
end









//---------------------------------------------------------------------------------
//  Power modules

localparam M = 2; //20
localparam K = 3; //33
localparam F = M*K;
localparam S = M;
genvar r;

reg [F*14+13: 0] fifo_aa ;
reg [F*14+13: 0] fifo_bb ;
reg [M*32+31: 0] mult_aa ;
reg [M*32+31: 0] mult_bb ;
reg [S*32+63: 0] sum_aa  ;
reg [S*32+63: 0] sum_bb  ;

always @(posedge clk_i) begin
   fifo_aa[13: 0] <= {14{dat_en}} & rand_dat[13: 0] ;
   fifo_bb[13: 0] <= {14{dat_en}} & rand_dat[29:16] ;

   sum_aa[ 31: 0] <= mult_aa[31:0] ;
   sum_bb[ 31: 0] <= mult_bb[31:0] ;
end

generate
  for (r = 0; r < F; r = r+1) begin : r_reg
     always @(posedge clk_i) begin
        fifo_aa[(r+1)*14+13: (r+1)*14] <= fifo_aa[r*14+13: r*14];
        fifo_bb[(r+1)*14+13: (r+1)*14] <= fifo_bb[r*14+13: r*14];
     end
  end
endgenerate


generate
  for (r = 0; r < M; r = r+1) begin : m_reg
     always @(posedge clk_i) begin
        mult_aa[r*32+31: r*32] <= $signed(fifo_aa[r*K*14+13: r*K*14]) * $signed(rand_dat[31:16]) + $signed(rand_dat[13:0]);
        mult_bb[r*32+31: r*32] <= $signed(fifo_bb[r*K*14+13: r*K*14]) * $signed(rand_dat[15:0])  + $signed(fifo_bb[13: 0]);
     end
  end
endgenerate

generate
  for (r = 0; r < S; r = r+1) begin : s_reg
     always @(posedge clk_i) begin
        sum_aa[(r+1)*32+31: (r+1)*32] <= $signed(sum_bb[r*32+31: r*32]) + $signed(mult_aa[r*32+31: r*32]);
        sum_bb[(r+1)*32+31: (r+1)*32] <= $signed(sum_aa[r*32+31: r*32]) + $signed(mult_bb[r*32+31: r*32]);
     end
  end
endgenerate



reg [31: 0] out_aa ;

always @(posedge clk_i) begin
  out_aa <= fifo_aa[F*14+13: F*14] + sum_aa[0*32+31: 0*32] + sum_aa[S*32+31: S*32] + sum_bb[S*32+31: S*32];
end

always @(posedge clk_i) begin
   casez(b1_r[7:0])
     8'h00 : pwr_out <= out_aa[13+18: 18] ;
     8'h01 : pwr_out <= out_aa[13+17: 17] ;
     8'h02 : pwr_out <= out_aa[13+16: 16] ;
     8'h04 : pwr_out <= out_aa[13+15: 15] ;
     8'h0A : pwr_out <= out_aa[13+14: 14] ;
     8'h10 : pwr_out <= out_aa[13+13: 13] ;
     8'h32 : pwr_out <= out_aa[13+12: 12] ;
     8'h33 : pwr_out <= out_aa[13+11: 11] ;
     8'h43 : pwr_out <= out_aa[13+10: 10] ;
     8'h4C : pwr_out <= out_aa[13+9 : 9 ] ;
   default : pwr_out <= out_aa[13+18: 18] ;
   endcase
end




















endmodule
