/*
* Copyright (c) 2015 Instrumentation Technologies
* All Rights Reserved.
*
* $Id: $
*/

/*
  Module for divide.
  Input values must be unsigned !!!
  Latency = (XDW /GRAIN) +2
*/


module divide 
#(
   parameter   XDW     = -1 , // mod(XDW, PIPE*GRAIN) == 0  !!!!!!!! x data width
   parameter   XDWW    = -1 , // ceil(log2(XDW)) x data width, width
/*
   with radix2 max number range must be less then 2^(YDW-1)-1    ---  (YDW-1:0)
   with radix4 max number range must be less then 2^(YDW-1)-1    ---  (YDW-1:0)
   with radix8 max number range must be less then 2^(YDW-2)-1    ---  (YDW-2:0)
*/
   parameter   YDW     = -1 , //y data width
   parameter   PIPE    = -1 , // how many parallel pipes (1 is minimal)
/*
  GRAIN=1 - radix 2
  GRAIN=2 - radix 4
  GRAIN=3 - radix 8
*/
   parameter   GRAIN   = -1 ,

   parameter   RST_ACT_LVL = -1 , //positive or negative reset
/*
  calculated parameters
*/
   parameter   PIPE_CNT    = XDW/ GRAIN/ PIPE
)
(
   input                  clk_i            ,
   input                  rst_i            ,

   // Both input values must be unsigned !!!
   input      [ XDW-1: 0] x_i              ,  // numerator (dividend)
   input      [ YDW-1: 0] y_i              ,  // denominator (divisor)
   input                  dv_i             ,

   output reg [ XDW-1: 0] q_o              ,  // quotient
   output reg             dv_o     
);


reg   [   XDW-1: 0] reg_X   [ 0: PIPE-1] ;
reg   [   XDW-1: 0] reg_Q   [ 0: PIPE-1] ;
reg   [   YDW  : 0] R       [ 0: PIPE-1] ;
wire  [   YDW  : 0] R_w     [ 0: PIPE-1] ;
wire  [ GRAIN-1: 0] q_bits  [ 0: PIPE-1] ; 
wire  [ GRAIN-1: 0] x_bits  [ 0: PIPE-1] ;
reg   [  XDWW-1: 0] cnt     [ 0: PIPE-1] ;

reg   [   YDW  : 0] reg_Y   [ 0: PIPE-1] ;
reg   [   YDW+1: 0] reg_3Y  [ 0: PIPE-1] ;
reg   [   YDW+2: 0] reg_5Y  [ 0: PIPE-1] ;
reg   [   YDW+2: 0] reg_7Y  [ 0: PIPE-1] ;
reg                 pipe_dv [ 0: PIPE-1] ;




// first and last pipe
always @(posedge clk_i) begin
   if (rst_i == RST_ACT_LVL) begin
      dv_o       <= 1'b0 ;
      pipe_dv[0] <= 1'b0 ;
   end
   else begin

      if (dv_i) begin   //  -- initialization
         pipe_dv[0]  <= 1'b1                           ;
         cnt[0]      <= 'h1                            ;
         R[0]        <= { YDW+1{1'b0}}                 ;
         reg_X[0]    <= x_i                            ;
         reg_Y[0]    <= {1'b0, y_i}                    ;   // Y
         reg_3Y[0]   <= {y_i,1'b0 } + y_i              ;   // 2Y+Y
         reg_5Y[0]   <= {y_i,2'b00} + y_i              ;   // 4Y+Y
         reg_7Y[0]   <= {y_i,2'b00} + {y_i,1'b0} + y_i ;   // 4Y+2Y+Y
      end
      else if (pipe_dv[0]) begin
         // shifting bits
         R[0]     <= R_w[0]   ;           
         reg_Q[0] <= { reg_Q[0][ XDW-1-GRAIN: 0], q_bits[0][ GRAIN-1: 0]      };
         reg_X[0] <= { reg_X[0][ XDW-1-GRAIN: 0], reg_X[0][ XDW-1: XDW-GRAIN] };
         // how many cicles?
         cnt[0] <= cnt[0] + 1;
         if (cnt[0] == (PIPE_CNT)) begin
            pipe_dv[0]  <= 1'b0;
         end 
      end


      // result available 
      if (pipe_dv[PIPE-1] && (cnt[PIPE-1] == (PIPE_CNT)) ) begin
         q_o    <= ~{reg_Q[PIPE-1][XDW-1-GRAIN : 0], q_bits[PIPE-1][GRAIN-1: 0] };
         dv_o   <= 1'b1 ;
      end
      else begin
         dv_o   <= 1'b0 ;
      end

   end
end


// pipes logic
genvar i ;
generate for (i=1; i<PIPE; i=i+1) begin: pipe_loop
   always @(posedge clk_i) begin
      if (rst_i == RST_ACT_LVL) begin
         pipe_dv[i]  <= 1'b0 ;
         cnt[i]      <= 1'b0 ;
      end
      else begin

         if (cnt[i-1] == (PIPE_CNT)) begin   //  -- finished previus step
            pipe_dv[i]  <= 1'b1         ;
            cnt[i]      <= 'h1          ;
            R[i]        <= R_w[i-1]     ;
            reg_Q[i]    <= { reg_Q[i-1][XDW-1-GRAIN: 0], q_bits[i-1][GRAIN-1: 0]      };
            reg_X[i]    <= { reg_X[i-1][XDW-1-GRAIN: 0], reg_X[i-1][XDW-1: XDW-GRAIN] };
            reg_Y[i]    <= reg_Y[i-1]   ;   // Y
            reg_3Y[i]   <= reg_3Y[i-1]  ;   // 2Y+Y
            reg_5Y[i]   <= reg_5Y[i-1]  ;   // 4Y+Y   
            reg_7Y[i]   <= reg_7Y[i-1]  ;   // 4Y+2Y+Y
         end
         else if (pipe_dv[i]) begin
            // shifting bits
            R[i]     <= R_w[i]   ;            
            reg_Q[i] <= { reg_Q[i][XDW-1-GRAIN: 0], q_bits[i][GRAIN-1: 0]      };
            reg_X[i] <= { reg_X[i][XDW-1-GRAIN: 0], reg_X[i][XDW-1: XDW-GRAIN] };

            // how many cicles?
            cnt[i] <= cnt[i] + 1;
            if (cnt[i] == (PIPE_CNT)) begin
               pipe_dv[i]  <= 1'b0;
            end 

         end

      end

   end
end
endgenerate


// pipe instances
generate for (i=0; i<PIPE; i=i+1) begin: inst_loop

   assign x_bits[i] = reg_X[i][XDW-1: XDW-GRAIN] ;

   if (GRAIN == 1) begin: radix2_inst_blk
      div_add_sub_rad2 #( .XDW( XDW ), .YDW( YDW )
      )
      i_div_add_sub
      (
         .r_w_i            ( R[i]             ),  //   reminder - working
         .y_1y_i           ( reg_Y[i]         ),  //   denominator
         .x_bits_i         ( x_bits[i]        ),  //   numerator bits
         .q_bits_o         ( q_bits[i]        ),  //   quotient bits
         .r_n_o            ( R_w[i]           )   //   reminder - next
      );
   end

   if (GRAIN == 2) begin: radix4_inst_blk
      div_add_sub_rad4 #( .XDW( XDW ), .YDW( YDW )
      )
      i_div_add_sub
      (
         .r_w_i            ( R[i]             ),  //   reminder - working
         .y_1y_i           ( reg_Y[i]         ),  //   denominator
         .y_3y_i           ( reg_3Y[i]        ),  //   denominator * 3
         .x_bits_i         ( x_bits[i]        ),  //   numerator bits
         .q_bits_o         ( q_bits[i]        ),  //   quotient bits
         .r_n_o            ( R_w[i]           )   //   reminder - next
      );
   end

   if (GRAIN == 3) begin: radix8_inst_blk
      div_add_sub_rad8 #( .XDW( XDW ), .YDW( YDW )
      )
      i_div_add_sub
      (
         .r_w_i            ( R[i]             ),  //   reminder - working
         .y_1y_i           ( reg_Y[i]         ),  //   denominator
         .y_3y_i           ( reg_3Y[i]        ),  //   denominator * 3
         .y_5y_i           ( reg_5Y[i]        ),  //   denominator * 5
         .y_7y_i           ( reg_7Y[i]        ),  //   denominator * 7
         .x_bits_i         ( x_bits[i]        ),  //   numerator bits
         .q_bits_o         ( q_bits[i]        ),  //   quotient bits
         .r_n_o            ( R_w[i]           )   //   reminder - next
      );
   end

end
endgenerate


endmodule  //divide_top
















/***************************************************************************/
/**/
/**/// SUBMODULE ADD-SUB WITH RADIX 2
/**/
/**************************************************************************/


module div_add_sub_rad2 #(parameter XDW=-1, YDW=-1, GRAIN=1 )
(
   input      [    YDW: 0] r_w_i            ,  //    reminder - working
   input      [    YDW: 0] y_1y_i           ,  //    denominator
   input      [GRAIN-1: 0] x_bits_i         ,  //    numerator bits
   output reg [GRAIN-1: 0] q_bits_o         ,  //    quotient bits
   output reg [    YDW: 0] r_n_o               //    reminder - next
);


wire                   r_sign  ;
wire [  YDW+1: 0]  w_2r    ; 
reg  [  YDW+1: 0]  w_2r_y  ; 


assign  r_sign = r_w_i[YDW];  // reminder sign

assign  w_2r   = { {1{r_w_i[YDW]}}, r_w_i[YDW-1: 0], x_bits_i[GRAIN-1]  } ;  // working reminder 2R


always @(*) begin
   if(r_sign) begin
      w_2r_y  <= w_2r + y_1y_i ;
   end
   else begin
      w_2r_y  <= w_2r - y_1y_i ;
   end
end


// computing mux
always @(*) begin
   r_n_o    <= w_2r_y[YDW:0]  ;
   q_bits_o <= w_2r_y[YDW+1]  ;
end



endmodule //div_add_sub_rad2






/***************************************************************************/
/**/
/**/// SUBMODULE ADD-SUB WITH RADIX 4
/**/
/**************************************************************************/


module div_add_sub_rad4 #(parameter XDW=-1, YDW=-1, GRAIN=2 )
(
   input      [    YDW: 0] r_w_i            ,  //    reminder - working
   input      [    YDW: 0] y_1y_i           ,  //    denominator
   input      [  YDW+1: 0] y_3y_i           ,  //    denominator * 3
   input      [GRAIN-1: 0] x_bits_i         ,  //    numerator bits
   output reg [GRAIN-1: 0] q_bits_o         ,  //    quotient bits
   output reg [    YDW: 0] r_n_o               //    reminder - next
);


wire                   r_sign  ;
wire [  YDW+1: 0]  w_2r    ; 
reg  [  YDW+1: 0]  w_2r_y  ; 
wire [  YDW+2: 0]  w_4r    ;
reg  [  YDW+2: 0]  w_4r_y, w_4r_3y ;


assign  r_sign = r_w_i[YDW];  // reminder sign

assign  w_2r   = { {1{r_w_i[YDW]}}, r_w_i[YDW-1: 0], x_bits_i[GRAIN-1]              } ;  // working reminder 2R
assign  w_4r   = { {2{r_w_i[YDW]}}, r_w_i[YDW-2: 0], x_bits_i[GRAIN-1: GRAIN-2] } ;  // working reminder 4R



always @(*) begin
   if(r_sign) begin
      w_2r_y  <= w_2r + y_1y_i ;
      w_4r_y  <= w_4r + y_1y_i ;
      w_4r_3y <= w_4r + y_3y_i ;
   end
   else begin
      w_2r_y  <= w_2r - y_1y_i ;
      w_4r_y  <= w_4r - y_1y_i ;
      w_4r_3y <= w_4r - y_3y_i ;
   end
end




// computing mux
always @(*) begin

   case ({r_sign,w_2r_y[YDW+1]})
      2'b11: begin
                r_n_o    <=  w_4r_3y[YDW:0]         ;
                q_bits_o <= {w_2r_y[YDW+1], w_4r_3y[YDW+2]}  ;
             end
      2'b10: begin
                r_n_o    <=  w_4r_y[YDW:0]          ;
                q_bits_o <= {w_2r_y[YDW+1], w_4r_y[YDW+2]}   ;
             end
      2'b01: begin
                r_n_o    <=  w_4r_y[YDW:0]          ;
                q_bits_o <= {w_2r_y[YDW+1], w_4r_y[YDW+2]}   ;
             end
      2'b00: begin
                r_n_o    <=  w_4r_3y[YDW:0]         ;
                q_bits_o <= {w_2r_y[YDW+1], w_4r_3y[YDW+2]}  ;
             end
   endcase

end


endmodule //div_add_sub_rad4




















/***************************************************************************/
/**/
/**/// SUBMODULE ADD-SUB WITH RADIX 8
/**/
/**************************************************************************/



module div_add_sub_rad8 #(parameter XDW=-1, YDW=-1, GRAIN=3 )
(
   input      [    YDW: 0] r_w_i            ,  //   reminder - working
   input      [    YDW: 0] y_1y_i           ,  //   denominator
   input      [  YDW+1: 0] y_3y_i           ,  //   denominator * 3
   input      [  YDW+2: 0] y_5y_i           ,  //   denominator * 5
   input      [  YDW+2: 0] y_7y_i           ,  //   denominator * 7
   input      [GRAIN-1: 0] x_bits_i         ,  //   numerator bits
   output reg [GRAIN-1: 0] q_bits_o         ,  //   quotient bits
   output reg [    YDW: 0] r_n_o               //   reminder - next
);


wire                   r_sign  ;
wire [  YDW+1: 0]  w_2r    ;
wire [  YDW+2: 0]  w_4r    ; 
wire [  YDW+3: 0]  w_8r    ;
reg  [  YDW+1: 0]  w_2r_y                            ;
reg  [  YDW+2: 0]  w_4r_y, w_4r_3y                   ;
reg  [  YDW+3: 0]  w_8r_y, w_8r_3y, w_8r_5y, w_8r_7y ;

assign  r_sign = r_w_i[YDW];  // reminder sign

assign  w_2r   = { {1{r_w_i[YDW]}}, r_w_i[YDW-1: 0], x_bits_i[GRAIN-1]              } ;  // working reminder 2R
assign  w_4r   = { {2{r_w_i[YDW]}}, r_w_i[YDW-2: 0], x_bits_i[GRAIN-1: GRAIN-2] } ;  // working reminder 4R
assign  w_8r   = { {3{r_w_i[YDW]}}, r_w_i[YDW-3: 0], x_bits_i[GRAIN-1: GRAIN-3] } ;  // working reminder 8R

always @(*) begin
   if(r_sign) begin
      w_2r_y  <= w_2r + y_1y_i ;
      w_4r_y  <= w_4r + y_1y_i ;
      w_4r_3y <= w_4r + y_3y_i ;
      w_8r_y  <= w_8r + y_1y_i ;
      w_8r_3y <= w_8r + y_3y_i ;
      w_8r_5y <= w_8r + y_5y_i ;
      w_8r_7y <= w_8r + y_7y_i ;
   end
   else begin
      w_2r_y  <= w_2r - y_1y_i ;
      w_4r_y  <= w_4r - y_1y_i ;
      w_4r_3y <= w_4r - y_3y_i ;
      w_8r_y  <= w_8r - y_1y_i ;
      w_8r_3y <= w_8r - y_3y_i ;
      w_8r_5y <= w_8r - y_5y_i ;
      w_8r_7y <= w_8r - y_7y_i ;
   end
end



// computing mux
always @(*) begin

   casex ({r_sign,w_2r_y[YDW+1],w_4r_y[YDW+2],w_4r_3y[YDW+2]})
      4'b11x1: begin
                 r_n_o    <=  w_8r_7y[YDW:0]                                          ;
                 q_bits_o <= {w_2r_y[YDW+1], w_4r_3y[YDW+2], w_8r_7y[YDW+3]}  ;
              end
      4'b11x0: begin
                 r_n_o    <=  w_8r_5y[YDW:0]                                          ;
                 q_bits_o <= {w_2r_y[YDW+1], w_4r_3y[YDW+2], w_8r_5y[YDW+3]}  ;
              end
      4'b101x: begin
                 r_n_o    <=  w_8r_3y[YDW:0]                                          ;
                 q_bits_o <= {w_2r_y[YDW+1], w_4r_y[YDW+2],  w_8r_3y[YDW+3]}  ;
              end
      4'b100x: begin
                 r_n_o    <=  w_8r_y[YDW:0]                                           ;
                 q_bits_o <= {w_2r_y[YDW+1], w_4r_y[YDW+2],  w_8r_y[YDW+3]}   ;
              end

      4'b011x: begin
                 r_n_o    <=  w_8r_y[YDW:0]                                           ;
                 q_bits_o <= {w_2r_y[YDW+1], w_4r_y[YDW+2],  w_8r_y[YDW+3]}   ;
              end
      4'b010x: begin
                 r_n_o    <=  w_8r_3y[YDW:0]                                          ;
                 q_bits_o <= {w_2r_y[YDW+1], w_4r_y[YDW+2],  w_8r_3y[YDW+3]}  ;
              end
      4'b00x1: begin
                 r_n_o    <=  w_8r_5y[YDW:0]                                          ;
                 q_bits_o <= {w_2r_y[YDW+1], w_4r_3y[YDW+2], w_8r_5y[YDW+3]}  ;
              end
      4'b00x0: begin
                 r_n_o    <=  w_8r_7y[YDW:0]                                          ;
                 q_bits_o <= {w_2r_y[YDW+1], w_4r_3y[YDW+2], w_8r_7y[YDW+3]}  ;
              end
   endcase

end


endmodule //div_add_sub_rad8