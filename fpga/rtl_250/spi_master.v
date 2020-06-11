/*
* Copyright (c) 2010 Instrumentation Technologies
* All Rights Reserved.
*
* $Id: $
*/


module spi_master
#(
    parameter       RST_ACT_LVL    = 0   ,
    parameter       NUM_OF_CS      = 1
)
(
    // SPI ports
    output reg [NUM_OF_CS-1: 0] spi_cs_o           ,
    output reg                  spi_clk_o          ,
    input      [NUM_OF_CS-1: 0] spi_miso_i         ,
    output reg                  spi_mosi_t         ,
    output reg                  spi_mosi_o         ,

    // settings & status
    input                       clk_i              ,
    input                       rst_i              ,

    input                       spi_start_i        ,

    input      [         15: 0] dat_wr_h_i         ,  // data to write high part
    input      [         15: 0] dat_wr_l_i         ,  // data to write low part
    output reg [         15: 0] dat_rd_l_o         ,  // data readed on low part

    input                       cfg_rw_i           ,  // config - 1-read 0-write
    input      [NUM_OF_CS-1: 0] cfg_cs_act_i       ,  // config - active cs - ONLY ONE CS CAN BE ACTIVE FOR CORRECT READING !!
    input      [          4: 0] cfg_h_lng_i        ,  // config - h part length
    input      [          4: 0] cfg_l_lng_i        ,  // config - l part length
    input      [          7: 0] cfg_clk_presc_i    ,  // config - clk_i/presc -> spi_clk_o
    input                       cfg_clk_wr_edg_i   ,  // config - sent data on clock: 1-falling edge 0-rising edge
    input                       cfg_clk_rd_edg_i   ,  // config - read data on clock: 1-rising edge 0-falling edge
    input                       cfg_clk_idle_i     ,  // config - clock leven on idle
    output                      sts_spi_busy_o        // status - spi state machine busy
);




reg  spi_busy     ;
assign  sts_spi_busy_o  =  spi_busy     ;

// select mosi data input ONLY ONE CS CAN BE ACTIVE FOR CORRECT READING !!
wire spi_miso_in = |(spi_miso_i & cfg_cs_act_i)   ;
wire spi_start   = spi_start_i && !sts_spi_busy_o ;


//=========================================================================
// SPI master logic
//=========================================================================

reg                   spi_clk_en          ;
reg  [          8: 0] spi_clk_cnt         ;
reg                   spi_posedge         ;
reg                   spi_negedge         ;
reg                   spi_clk             ;
reg  [          4: 0] spi_h_word_lng_reg  ;
reg  [          4: 0] spi_l_word_lng_reg  ;

reg  [         15: 0] spi_h_word_reg      ;
reg  [         15: 0] spi_l_word_reg      ;
reg  [         15: 0] spi_l_word_rd_reg   ;
reg                   spi_rw_reg          ; 
reg  [NUM_OF_CS-1: 0] spi_css_reg         ;


wire    my_tx_edge  = cfg_clk_wr_edg_i ? spi_negedge : spi_posedge    ;
wire    dev_rx_edge = cfg_clk_wr_edg_i ? spi_posedge : spi_negedge    ;
wire    my_rx_edge  = cfg_clk_rd_edg_i ? spi_posedge : spi_negedge    ;

wire    [ 3: 0] l_word_bit_num  = spi_l_word_lng_reg - 1'b1 ;
wire    [ 3: 0] h_word_bit_num  = spi_h_word_lng_reg - 1'b1 ;

wire    spi_clk_run = spi_clk_en && (|spi_h_word_lng_reg || |spi_l_word_lng_reg || (spi_clk != cfg_clk_idle_i) || !spi_clk_cnt[8])    ;

always @(posedge clk_i)
begin
    if (rst_i == RST_ACT_LVL) begin
        spi_clk         <=  1'b0              ;
        spi_posedge     <=  1'b0              ;
        spi_negedge     <=  1'b0              ;
        spi_clk_en      <=  1'b0              ;
        spi_busy        <=  1'b0              ;
        spi_cs_o        <= {NUM_OF_CS{1'b1}}  ;
        dat_rd_l_o      <= {16{1'b0}}         ;
    end
    else begin
        if (spi_start)
            spi_clk_en  <= 1'b1 ;
        else
            spi_clk_en  <= spi_clk_run  ;
                    
        spi_busy    <= spi_start || spi_clk_en  ;

        if (!spi_clk_run)
            spi_clk <= cfg_clk_idle_i ;
        else if (spi_clk_cnt[8])
            spi_clk <= !spi_clk ;

        spi_posedge <= (spi_start &&  cfg_clk_idle_i && !cfg_clk_wr_edg_i) || (spi_clk_en && !spi_clk && spi_clk_cnt[8])   ;
        spi_negedge <= (spi_start && !cfg_clk_idle_i &&  cfg_clk_wr_edg_i) || (spi_clk_en &&  spi_clk && spi_clk_cnt[8])   ;

        if (my_tx_edge) begin
            if (|spi_h_word_lng_reg || |spi_l_word_lng_reg[4:1])
                spi_cs_o  <= spi_css_reg  ;
            else if (spi_l_word_lng_reg[0] && (!spi_rw_reg || !my_rx_edge))
                spi_cs_o  <= spi_css_reg  ;
            else
                spi_cs_o  <= {NUM_OF_CS{1'b1}}  ;
        end

        if (spi_busy && !spi_clk_en && spi_rw_reg)
            dat_rd_l_o  <= spi_l_word_rd_reg    ;
    end
end

always @(posedge clk_i)
begin
    if (!spi_busy)
        spi_h_word_lng_reg  <= cfg_h_lng_i   ;
    else if (|spi_h_word_lng_reg && dev_rx_edge)
        spi_h_word_lng_reg  <= spi_h_word_lng_reg -1'b1 ;

    if (!spi_busy)
        spi_l_word_lng_reg  <= cfg_l_lng_i   ;
    else if (~|spi_h_word_lng_reg && |spi_l_word_lng_reg && (spi_rw_reg ? my_rx_edge : dev_rx_edge))
        spi_l_word_lng_reg  <= spi_l_word_lng_reg -1'b1 ;

    if (!spi_busy) begin
        spi_h_word_reg  <= dat_wr_h_i    ;
        spi_l_word_reg  <= dat_wr_l_i    ;
        spi_rw_reg      <= cfg_rw_i      ;
        spi_css_reg     <= ~cfg_cs_act_i ;
    end        

    if (!spi_clk_run || spi_clk_cnt[8])
        spi_clk_cnt <= {1'b0, cfg_clk_presc_i}    ;
    else
        spi_clk_cnt <= spi_clk_cnt + {9{1'b1}}  ;

    if (my_tx_edge)
        spi_mosi_o  <= |spi_h_word_lng_reg ? spi_h_word_reg[h_word_bit_num] : spi_l_word_reg[l_word_bit_num]    ;

    spi_clk_o   <= spi_clk  ;

    if (!spi_busy)
        spi_l_word_rd_reg   <= {16{1'b0}}   ;
    else if (my_rx_edge && |spi_l_word_lng_reg && ~|spi_h_word_lng_reg)
        spi_l_word_rd_reg   <= {spi_l_word_rd_reg[14: 0], spi_miso_in}   ;
end


always @(posedge clk_i)
begin
  if (rst_i == RST_ACT_LVL) begin
    spi_mosi_t <= 1'b0 ;
  end
  else begin
    spi_mosi_t <= spi_rw_reg && !(|spi_h_word_lng_reg) ;
  end
end


endmodule
