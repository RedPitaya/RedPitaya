/*
* Copyright (c) 2015 Instrumentation Technologies
* All Rights Reserved.
*
* $Id: $
*/


// synopsys translate_off
`timescale 1ns / 1ps
// synopsys translate_on

module axi_wr_fifo #(
  parameter   DW  =  64      , // data width (8,16,...,1024)
  parameter   AW  =  32      , // address width
  parameter   FW  =   5      , // address width of FIFO pointers
  parameter   SW  = DW >> 3    // strobe width - 1 bit for every data byte
)
(
   // global signals
   input                  axi_clk_i          , // global clock
   input                  axi_rstn_i         , // global reset

   // Connection to AXI master
   output reg  [ AW-1: 0] axi_waddr_o        , // write address
   output reg  [ DW-1: 0] axi_wdata_o        , // write data
   output reg  [ SW-1: 0] axi_wsel_o         , // write byte select
   output reg             axi_wvalid_o       , // write data valid
   output reg  [  4-1: 0] axi_wlen_o         , // write burst length
   output reg             axi_wfixed_o       , // write burst type (fixed / incremental)
   input                  axi_werr_i         , // write error
   input                  axi_wrdy_i         , // write ready

   // data and configuration
   input       [ DW-1: 0] wr_data_i          , // write data
   input                  wr_val_i           , // write data valid
   input       [ AW-1: 0] ctrl_start_addr_i  , // range start address
   input       [ AW-1: 0] ctrl_stop_addr_i   , // range stop address
   input       [  4-1: 0] ctrl_trig_size_i   , // trigger level
   input                  ctrl_wrap_i        , // start from begining when reached stop
   input                  ctrl_clr_i         , // clear / flush
   output reg             stat_overflow_o    , // overflow indicator
   output      [ AW-1: 0] stat_cur_addr_o    , // current address
   output reg             stat_write_data_o    // write data indicator
);



//---------------------------------------------------------------------------------
//
// Write address channel


reg  [ FW-1: 0] wr_pt              ;
reg  [ FW-1: 0] rd_pt              ;
reg  [ FW  : 0] fill_lvl           ;
reg  [ DW-1: 0] fifo[(1<<FW)-1:0]  ;
reg             data_in_reg        ;
reg             clear              ;
reg  [  4-1: 0] dat_cnt            ;
reg  [ AW  : 0] next_address       ;
reg             fifo_flush         ;
reg  [ AW-1: 0] sys_start_addr_r   ;
reg  [ AW-1: 0] sys_stop_addr_r    ;
reg  [  4-1: 0] sys_trig_size_r    ;

wire push = wr_val_i && !fill_lvl[FW] ;
wire pop ;
wire new_burst ;

// overflow detection & indication
always @ (posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      stat_overflow_o <= 'h0 ;
   end
   else begin
      stat_overflow_o <= fill_lvl[FW] && wr_val_i;
   end
end

reg clear_do ;
always @ (posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      clear    <= 1'h1 ;
      clear_do <= 1'b0 ;
   end
   else begin 
      if (ctrl_clr_i)
         clear_do <= 1'b1 ;
      else if (clear)
         clear_do <= 1'b0 ;

      clear <= clear_do && !axi_wvalid_o && !new_burst;
   end
end



always @ (posedge axi_clk_i)
begin
   if (clear) begin
      wr_pt <= 4'h0 ;
      rd_pt <= 4'h0 ;
   end
   else begin
      if (push) begin
         fifo[wr_pt] <= wr_data_i                  ;
         wr_pt       <= wr_pt + {{FW-1{1'b0}},1'b1} ;
      end

      if (pop) begin
         axi_wdata_o <= fifo[rd_pt]                 ;
         rd_pt       <= rd_pt + {{FW-1{1'b0}},1'b1} ;
      end
   end
end



always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      data_in_reg <= 'h0 ;
   end
   else begin
      if (pop)
         data_in_reg <= 1'b1 ;
      else if ((axi_wrdy_i && axi_wvalid_o) || clear)
         data_in_reg <= 1'b0 ;
   end
end


wire fifo_flush_cond = |fill_lvl && !wr_val_i && !dat_cnt[3:1];

always @(posedge axi_clk_i)
begin
   if (clear) begin
      fill_lvl   <= {FW+1{1'h0}} ;
      fifo_flush <= 1'h0 ;
   end
   else begin
      if (push && !pop)
         fill_lvl <= fill_lvl + {{FW{1'b0}}, 1'h1} ;
      else if(!push && pop)
         fill_lvl <= fill_lvl - {{FW{1'b0}}, 1'h1} ;

      if (fifo_flush_cond)
         fifo_flush <= 1'b1 ;
      else if (axi_wrdy_i)
         fifo_flush <= 1'b0 ;
   end
end










wire [8   :0] next_end_address   = next_address[10:3] + {3'h0,fill_lvl} ; // to where we have data
wire [AW  :0] next_stop_address  = {1'b0,sys_stop_addr_r[AW-1:3]} - next_address[AW:3] - {{AW-FW+1{1'h0}},fill_lvl} ;

// select which boundary condition is more restricting - 0x0 - 4k boundary is more restricting; 0x1 end address is more restricting
wire [AW+1:0] boundary_condition = {1'b0,next_address[AW:3]} + {{AW-FW+1{1'h0}},fill_lvl} - {1'b0,sys_stop_addr_r[AW-1:3]} ;

// select which boundary condition is more restricting, next transmission would cross the 4k address boundary (64-bit access) or stop address
wire [2   :0] boundary_cross     = {boundary_condition[AW+1], next_end_address[8],next_stop_address[AW]} ;

// prevents data to be trapped in output register
reg  single_burst    ;
reg  single_burst_r  ;
wire single_burst_posedge = !single_burst_r && single_burst;

always @(posedge axi_clk_i)
begin
   if (clear) begin
      single_burst   <= 'h0 ;
      single_burst_r <= 'h0 ;
   end
   else begin
      single_burst   <= (!fill_lvl && !fifo_flush && !dat_cnt && data_in_reg) ;
      single_burst_r <= single_burst ;
   end
end


assign new_burst = (((fifo_flush && axi_wrdy_i) || (fill_lvl >= {{FW-4{1'b0}},sys_trig_size_r})) && !dat_cnt && |fill_lvl 
                 || single_burst_posedge)
                 && !clear_do;

always @(posedge axi_clk_i)
begin
   if (clear) begin
      dat_cnt      <= 4'h0 ;
      axi_wsel_o   <= {SW{1'b1}} ;
      axi_wfixed_o <= 1'b0 ;
      axi_wlen_o   <= 4'h0 ;
   end
   else begin
      if (new_burst && (next_address <= {1'b0,sys_stop_addr_r})) begin
         if (boundary_cross[1:0] || fill_lvl[FW:4]) begin
            if (fill_lvl[FW:4] && !boundary_cross[1:0]) begin  //enough space to stop address  --!boundary_cross[1:0]
               dat_cnt    <= 4'hF ;
               axi_wlen_o <= 4'hF ;
            end
            else begin
               // select which boundary condition is more restricting 
               // 0x0 - 4k boundary is more restricting
               // 0x1 - end address is more restricting
               if (boundary_cross[2]) begin
                  dat_cnt    <= 4'hF - next_address[6:3];
                  axi_wlen_o <= 4'hF - next_address[6:3];
               end
               else begin
                  dat_cnt    <= sys_stop_addr_r[6:3] - next_address[6:3];
                  axi_wlen_o <= sys_stop_addr_r[6:3] - next_address[6:3];
               end
            end
         end
         else begin
            if (fifo_flush || fifo_flush_cond) begin
               dat_cnt    <= fill_lvl[3:0] - 4'h1 ;
               axi_wlen_o <= fill_lvl[3:0] - 4'h1 ;
            end
            else begin
               dat_cnt    <= fill_lvl[3:0] ;
               axi_wlen_o <= fill_lvl[3:0] ;
            end
         end
      end
      else if (axi_wrdy_i && axi_wvalid_o && dat_cnt) begin
         dat_cnt    <= dat_cnt    - 4'h1;
         axi_wlen_o <= axi_wlen_o - 4'h1;
      end
   end
end


wire [4-1: 0] aaaa = 4'hF - next_address[6:3];
wire [4-1: 0] bbbb = sys_stop_addr_r[6:3] - next_address[6:3];
wire [4-1: 0] cccc = fill_lvl[3:0] - 4'h1;
wire [4-1: 0] dddd = fill_lvl[3:0];




assign pop =  (!data_in_reg && fill_lvl) || ((|dat_cnt || (new_burst && axi_wvalid_o)) 
            && axi_wrdy_i && axi_wvalid_o && fill_lvl) ;

always @(posedge axi_clk_i)
begin
   if (clear) begin
      axi_wvalid_o     <= 1'h0                     ;
      axi_waddr_o      <= ctrl_start_addr_i        ;
      next_address     <= {1'b0,ctrl_start_addr_i} ;
      sys_start_addr_r <= ctrl_start_addr_i        ;
      sys_stop_addr_r  <= ctrl_stop_addr_i         ;
      sys_trig_size_r  <= ctrl_trig_size_i         ;
   end
   else begin
      if ((next_address <= {1'b0,sys_stop_addr_r}) && // still in address rage
          ( (new_burst && axi_wrdy_i) || (|dat_cnt && axi_wrdy_i && fill_lvl) ) ) begin  //new burst || still data in package
         axi_wvalid_o <= 1'h1 ;
         next_address <= next_address + DW/8  ; // in bytes
         axi_waddr_o  <= next_address[AW-1:0] ;
      end
      else if (ctrl_wrap_i && new_burst && (axi_waddr_o==sys_stop_addr_r)) begin //wrap around
         axi_wvalid_o <= 1'h1 ;
         next_address <= {1'b0,sys_start_addr_r} + DW/8  ; // in bytes
         axi_waddr_o  <= sys_start_addr_r ;
      end
      else if (axi_wrdy_i) begin
         axi_wvalid_o <= 1'h0 ;
      end
   end
end


// write data indication
always @(posedge axi_clk_i)
begin
   if (!axi_rstn_i) begin
      stat_write_data_o <= 'h0 ;
   end
   else begin
      stat_write_data_o <= (next_address <= {1'b0,sys_stop_addr_r}) ; // address in range
   end
end

assign stat_cur_addr_o = next_address ; // current address



endmodule // axi_wr_fifo
