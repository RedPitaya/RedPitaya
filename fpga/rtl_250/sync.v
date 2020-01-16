
/*
* Copyright (c) 2011 Instrumentation Technologies
* All Rights Reserved.
*
* $Id: $
*/

/*
    Synchronizes data from source to destination clock domain
    supports also pulse synchronization.
*/

module sync
#(
  parameter       DW                 =  1    ,   // width parameter of Input and Output signals
  parameter       PULSE              =  0        // pulse mode parameter for pulse transition (pus in - puls out)
)
(
  input                 sclk_i       ,
  input                 srstn_i      ,
  input                 dclk_i       ,
  input                 drstn_i      ,

  input       [DW-1: 0] src_i        ,
  output reg  [DW-1: 0] dst_o
);


//======================================================================
// source side
//======================================================================
wire  [DW-1: 0] src_in         ;
reg   [DW-1: 0] src_in_prev    ;

reg   [DW-1: 0] src_to_dst     ;

assign src_in = src_i ;

always @(posedge sclk_i) begin
  if (srstn_i == 1'b0) begin
    src_in_prev <=  {(DW){1'b0}} ;
    src_to_dst  <=  {(DW){1'b0}} ;
  end else begin
    src_in_prev <= src_in ;

    if ( !PULSE ) begin
      src_to_dst <= src_in ;
    end else begin
      if (src_in & (~src_in_prev))
        src_to_dst <= src_to_dst ^ (src_in ^ src_in_prev);
    end
  end
end




//======================================================================
// destination side
//======================================================================
reg   [DW-1: 0] dst_in_csff    ;
reg   [DW-1: 0] dst_in         ;
reg   [DW-1: 0] dst_in_prev    ;


always @(posedge dclk_i) begin
  if (drstn_i == 1'b0) begin
    dst_in_csff <=  {(DW){1'b0}} ;
    dst_in      <=  {(DW){1'b0}} ;
    dst_in_prev <=  {(DW){1'b0}} ;
    dst_o       <=  {(DW){1'b0}} ;
  end else begin
    dst_in_csff <= src_to_dst  ;
    dst_in      <= dst_in_csff ;
    dst_in_prev <= dst_in      ;

    if ( !PULSE ) begin
      dst_o <= dst_in_csff ;
    end else begin
      dst_o <= dst_in ^ dst_in_prev ;
    end
  end
end


endmodule

