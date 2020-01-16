/**
 * $Id: red_pitaya_hk.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya house keeping.
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
 * House keeping module takes care of system identification.
 *
 *
 * This module takes care of system identification via DNA readout at startup and
 * ID register which user can define at compile time.
 *
 * Beside that it is currently also used to test expansion connector and for
 * driving LEDs.
 * 
 */

module red_pitaya_hk #(
  parameter DWL = 8, // data width for LED
  parameter DWE = 8, // data width for extension
  parameter [57-1:0] DNA = 57'h0823456789ABCDE
)(
  // system signals
  input                clk_i      ,  // clock
  input                rstn_i     ,  // reset - active low
  // LED
  output reg [DWL-1:0] led_o      ,  // LED output
  // idelay control
  output reg [2*7-1:0] idly_rst_o,
  output reg [2*7-1:0] idly_ce_o,
  output reg [2*7-1:0] idly_inc_o,
  input      [2*5-1:0] idly_cnt_i,
  // global configuration
  output reg           digital_loop,
  input                pll_sys_i,    // system clock
  input                pll_ref_i,    // reference clock
  output               pll_hi_o,     // PLL high
  output               pll_lo_o,     // PLL low
  //SPI
  output     [  2-1:0] spi_cs_o,
  output     [  2-1:0] spi_clk_o,
  input      [  2-1:0] spi_miso_i,
  output     [  2-1:0] spi_mosi_t,
  output     [  2-1:0] spi_mosi_o,
  // Expansion connector
  input      [DWE-1:0] exp_p_dat_i,  // exp. con. input data
  output reg [DWE-1:0] exp_p_dat_o,  // exp. con. output data
  output reg [DWE-1:0] exp_p_dir_o,  // exp. con. 1-output enable
  input      [DWE-1:0] exp_n_dat_i,  //
  output reg [DWE-1:0] exp_n_dat_o,  //
  output reg [DWE-1:0] exp_n_dir_o,  //
  // System bus
  input      [ 32-1:0] sys_addr   ,  // bus address
  input      [ 32-1:0] sys_wdata  ,  // bus write data
  input                sys_wen    ,  // bus write enable
  input                sys_ren    ,  // bus read enable
  output reg [ 32-1:0] sys_rdata  ,  // bus read data
  output reg           sys_err    ,  // bus error indicator
  output reg           sys_ack       // bus acknowledge signal
);

//---------------------------------------------------------------------------------
//
//  Read device DNA

wire           dna_dout ;
reg            dna_clk  ;
reg            dna_read ;
reg            dna_shift;
reg  [ 9-1: 0] dna_cnt  ;
reg  [57-1: 0] dna_value;
reg            dna_done ;

always @(posedge clk_i)
if (rstn_i == 1'b0) begin
  dna_clk   <=  1'b0;
  dna_read  <=  1'b0;
  dna_shift <=  1'b0;
  dna_cnt   <=  9'd0;
  dna_value <= 57'd0;
  dna_done  <=  1'b0;
end else begin
  if (!dna_done)
    dna_cnt <= dna_cnt + 1'd1;

  dna_clk <= dna_cnt[2] ;
  dna_read  <= (dna_cnt < 9'd10);
  dna_shift <= (dna_cnt > 9'd18);

  if ((dna_cnt[2:0]==3'h0) && !dna_done)
    dna_value <= {dna_value[57-2:0], dna_dout};

  if (dna_cnt > 9'd465)
    dna_done <= 1'b1;
end

// parameter specifies a sample 57-bit DNA value for simulation
DNA_PORT #(.SIM_DNA_VALUE (DNA)) i_DNA (
  .DOUT  ( dna_dout   ), // 1-bit output: DNA output data.
  .CLK   ( dna_clk    ), // 1-bit input: Clock input.
  .DIN   ( 1'b0       ), // 1-bit input: User data input pin.
  .READ  ( dna_read   ), // 1-bit input: Active high load DNA, active low read input.
  .SHIFT ( dna_shift  )  // 1-bit input: Active high shift enable input.
);

//---------------------------------------------------------------------------------
//
//  Design identification

wire [32-1: 0] id_value;

assign id_value[31: 4] = 28'h0; // reserved
assign id_value[ 3: 0] =  4'h2; // board type  2 - 250MHz
                                //             1 - release 1

//---------------------------------------------------------------------------------
//
//  Simple FF PLL 

// detect RF clock
reg  [16-1:0] pll_ref_cnt = 16'h0;
reg  [ 3-1:0] pll_sys_syc  ;
reg  [21-1:0] pll_sys_cnt  ;
reg           pll_sys_val  ;
reg           pll_cfg_en   ;
wire [32-1:0] pll_cfg_rd   ;

always @(posedge pll_ref_i) begin
  pll_ref_cnt <= pll_ref_cnt + 16'h1;
end

always @(posedge clk_i) 
if (rstn_i == 1'b0) begin
  pll_sys_syc <=  3'h0 ;
  pll_sys_cnt <= 21'h100000;
  pll_sys_val <=  1'b0 ;
end else begin
  pll_sys_syc <= {pll_sys_syc[3-2:0], pll_ref_cnt[14-1]} ;

  if (pll_sys_syc[3-1] ^ pll_sys_syc[3-2])
    pll_sys_cnt <= 21'h1;
  else if (!pll_sys_cnt[21-1])
    pll_sys_cnt <= pll_sys_cnt + 21'h1;

  // pll_sys_clk must be around 102400 (125000000/(10000000/2^13))
  if (pll_sys_syc[3-1] ^ pll_sys_syc[3-2])
    pll_sys_val <= (pll_sys_cnt > 102385) && (pll_sys_cnt < 102415) ;
  else if (pll_sys_cnt[21-1])
    pll_sys_val <= 1'b0 ;
end


reg  pll_ff_sys ;
reg  pll_ff_ref ;
wire pll_ff_rst ;
wire pll_ff_lck ;

// FF PLL functionality
always @(posedge pll_sys_i or negedge pll_ff_rst) begin
  if (!pll_ff_rst)  pll_ff_sys <= 1'b0 ;
  else              pll_ff_sys <= 1'b1 ;
end
always @(posedge pll_ref_i or negedge pll_ff_rst) begin
  if (!pll_ff_rst)  pll_ff_ref <= 1'b0 ;
  else              pll_ff_ref <= 1'b1 ;
end
assign pll_ff_rst = !(pll_ff_sys && pll_ff_ref) ;
assign pll_ff_lck = (!pll_ff_sys && !pll_ff_ref);

assign pll_lo_o = !pll_ff_sys && ( pll_sys_val &&  pll_cfg_en);
assign pll_hi_o =  pll_ff_ref || (!pll_sys_val || !pll_cfg_en);


// filter out PLL lock status
reg  [21-1:0] pll_lck_lcnt  ;
reg  [21-1:0] pll_lck_hcnt  ;
reg  [ 4-1:0] pll_lck_sts  ;

always @(posedge clk_i) 
if (rstn_i == 1'b0) begin
  pll_lck_lcnt <= 21'h0 ;
  pll_lck_hcnt <= 21'h0 ;
  pll_lck_sts <=   2'b0 ;
end else begin

  if (pll_sys_syc[3-1] ^ pll_sys_syc[3-2])
    pll_lck_lcnt <= 21'h1;
  else if (pll_lo_o)
    pll_lck_lcnt <= pll_lck_lcnt + 21'h1;

  if (pll_sys_syc[3-1] ^ pll_sys_syc[3-2])
    pll_lck_hcnt <= 21'h1;
  else if (!pll_hi_o)
    pll_lck_hcnt <= pll_lck_hcnt + 21'h1;


  // pll_lck_cnt threshold 70% of whole period
  if (pll_sys_syc[3-1] ^ pll_sys_syc[3-2])
    pll_lck_sts[0] <= (pll_lck_lcnt > 21'd80000) && (pll_lck_hcnt > 21'd80000);

  pll_lck_sts[1] <= pll_lck_sts[0] && pll_sys_val;

  if (pll_sys_syc[3-1] ^ pll_sys_syc[3-2])
    pll_lck_sts[3:2] <= {(pll_lck_lcnt > 21'd80000), (pll_lck_hcnt > 21'd80000)};
end

assign pll_cfg_rd = {{32-14{1'h0}}, pll_lck_sts[3:2], 3'h0,pll_lck_sts[1], 3'h0,pll_sys_val, 3'h0,pll_cfg_en};






//---------------------------------------------------------------------------------
//
//  SPI

reg  [  1: 0] spi_do         ;
wire [  1: 0] spi_bsy        ;
reg  [ 15: 0] spi_wr_h[1:0]  ;
reg  [ 15: 0] spi_wr_l[1:0]  ;
wire [ 15: 0] spi_rd_l[1:0]  ;

spi_master i_spi_adc
(
    // SPI ports
  .spi_cs_o           (spi_cs_o[0]),
  .spi_clk_o          (spi_clk_o[0]),
  .spi_miso_i         (spi_miso_i[0]),
  .spi_mosi_t         (spi_mosi_t[0]),
  .spi_mosi_o         (spi_mosi_o[0]),

    // settings & status
  .clk_i              (clk_i),
  .rst_i              (rstn_i),

  .spi_start_i        (spi_do[0]),

  .dat_wr_h_i         (spi_wr_h[0]),  // data to write high part
  .dat_wr_l_i         (spi_wr_l[0]),  // data to write low part
  .dat_rd_l_o         (spi_rd_l[0]),  // data readed on low part

  .cfg_rw_i           (spi_wr_h[0][15]),  // config - 1-read 0-write
  .cfg_cs_act_i       (1'b1),  // config - active cs - ONLY ONE CS CAN BE ACTIVE FOR CORRECT READING !!
  .cfg_h_lng_i        (5'd16),  // config - h part length
  .cfg_l_lng_i        (5'd8),  // config - l part length
  .cfg_clk_presc_i    (8'd255),  // config - clk_i/presc -> spi_clk_o
  .cfg_clk_wr_edg_i   (1'b1),  // config - sent data on clock: 1-falling edge 0-rising edge
  .cfg_clk_rd_edg_i   (1'b1),  // config - read data on clock: 1-rising edge 0-falling edge
  .cfg_clk_idle_i     (1'b1),  // config - clock leven on idle
  .sts_spi_busy_o     (spi_bsy[0])   // status - spi state machine busy
);

spi_master i_spi_dac
(
    // SPI ports
  .spi_cs_o           (spi_cs_o[1]),
  .spi_clk_o          (spi_clk_o[1]),
  .spi_miso_i         (spi_miso_i[1]),
  .spi_mosi_t         (spi_mosi_t[1]),
  .spi_mosi_o         (spi_mosi_o[1]),

    // settings & status
  .clk_i              (clk_i),
  .rst_i              (rstn_i),

  .spi_start_i        (spi_do[1]),

  .dat_wr_h_i         (spi_wr_h[1]),  // data to write high part
  .dat_wr_l_i         (spi_wr_l[1]),  // data to write low part
  .dat_rd_l_o         (spi_rd_l[1]),  // data readed on low part

  .cfg_rw_i           (spi_wr_h[1][7]),  // config - 1-read 0-write
  .cfg_cs_act_i       (1'b1),  // config - active cs - ONLY ONE CS CAN BE ACTIVE FOR CORRECT READING !!
  .cfg_h_lng_i        (5'd8),  // config - h part length
  .cfg_l_lng_i        (5'd8),  // config - l part length
  .cfg_clk_presc_i    (8'd255),  // config - clk_i/presc -> spi_clk_o
  .cfg_clk_wr_edg_i   (1'b1),  // config - sent data on clock: 1-falling edge 0-rising edge
  .cfg_clk_rd_edg_i   (1'b1),  // config - read data on clock: 1-rising edge 0-falling edge
  .cfg_clk_idle_i     (1'b1),  // config - clock leven on idle
  .sts_spi_busy_o     (spi_bsy[1])   // status - spi state machine busy
);




//---------------------------------------------------------------------------------
//
//  System bus connection

always @(posedge clk_i)
if (rstn_i == 1'b0) begin
  digital_loop <= 1'b0 ;
  led_o        <= {DWL{1'b0}};
  exp_p_dat_o  <= {DWE{1'b0}};
  exp_p_dir_o  <= {DWE{1'b0}};
  exp_n_dat_o  <= {DWE{1'b0}};
  exp_n_dir_o  <= {DWE{1'b0}};
  pll_cfg_en   <= 1'b1 ;
end else if (sys_wen) begin
  if (sys_addr[19:0]==20'h0c)   digital_loop <= sys_wdata[0];

  if (sys_addr[19:0]==20'h10)   exp_p_dir_o  <= sys_wdata[DWE-1:0];
  if (sys_addr[19:0]==20'h14)   exp_n_dir_o  <= sys_wdata[DWE-1:0];
  if (sys_addr[19:0]==20'h18)   exp_p_dat_o  <= sys_wdata[DWE-1:0];
  if (sys_addr[19:0]==20'h1C)   exp_n_dat_o  <= sys_wdata[DWE-1:0];

  if (sys_addr[19:0]==20'h30)   led_o        <= sys_wdata[DWL-1:0];

  if (sys_addr[19:0]==20'h40)   pll_cfg_en   <= sys_wdata[0];

  if (sys_addr[19:0]==20'h50)   spi_wr_h[0]  <= sys_wdata[ 16-1:0];
  if (sys_addr[19:0]==20'h54)   spi_wr_l[0]  <= sys_wdata[ 16-1:0];
  if (sys_addr[19:0]==20'h60)   spi_wr_h[1]  <= sys_wdata[ 16-1:0];
  if (sys_addr[19:0]==20'h64)   spi_wr_l[1]  <= sys_wdata[ 16-1:0];
end


always @(posedge clk_i)
begin
  idly_rst_o[  6: 0] <= sys_wdata[ 7-1: 0] & {7{ ((sys_addr[19:0]==20'h44) & sys_wen) || (rstn_i == 1'b0) }};
  idly_rst_o[ 13: 7] <= sys_wdata[15-1: 8] & {7{ ((sys_addr[19:0]==20'h44) & sys_wen) || (rstn_i == 1'b0) }};

  idly_ce_o[   6: 0] <= sys_wdata[ 7-1: 0] & {7{ ((sys_addr[19:0]==20'h48) & sys_wen) }};
  idly_inc_o[  6: 0] <= sys_wdata[15-1: 8] & {7{ ((sys_addr[19:0]==20'h48) & sys_wen) }};
  idly_ce_o[  13: 7] <= sys_wdata[23-1:16] & {7{ ((sys_addr[19:0]==20'h4C) & sys_wen) }};
  idly_inc_o[ 13: 7] <= sys_wdata[31-1:24] & {7{ ((sys_addr[19:0]==20'h4C) & sys_wen) }};

  spi_do[0]    <= (sys_addr[19:0]==20'h54) & sys_wen;
  spi_do[1]    <= (sys_addr[19:0]==20'h64) & sys_wen;
end



wire sys_en;
assign sys_en = sys_wen | sys_ren;

always @(posedge clk_i)
if (rstn_i == 1'b0) begin
  sys_err <= 1'b0;
  sys_ack <= 1'b0;
end else begin
  sys_err <= 1'b0;

  casez (sys_addr[19:0])
    20'h00000: begin sys_ack <= sys_en;  sys_rdata <= {                id_value          }; end
    20'h00004: begin sys_ack <= sys_en;  sys_rdata <= {                dna_value[32-1: 0]}; end
    20'h00008: begin sys_ack <= sys_en;  sys_rdata <= {{64- 57{1'b0}}, dna_value[57-1:32]}; end
    20'h0000c: begin sys_ack <= sys_en;  sys_rdata <= {{32-  1{1'b0}}, digital_loop      }; end

    20'h00010: begin sys_ack <= sys_en;  sys_rdata <= {{32-DWE{1'b0}}, exp_p_dir_o}       ; end
    20'h00014: begin sys_ack <= sys_en;  sys_rdata <= {{32-DWE{1'b0}}, exp_n_dir_o}       ; end
    20'h00018: begin sys_ack <= sys_en;  sys_rdata <= {{32-DWE{1'b0}}, exp_p_dat_o}       ; end
    20'h0001C: begin sys_ack <= sys_en;  sys_rdata <= {{32-DWE{1'b0}}, exp_n_dat_o}       ; end
    20'h00020: begin sys_ack <= sys_en;  sys_rdata <= {{32-DWE{1'b0}}, exp_p_dat_i}       ; end
    20'h00024: begin sys_ack <= sys_en;  sys_rdata <= {{32-DWE{1'b0}}, exp_n_dat_i}       ; end

    20'h00030: begin sys_ack <= sys_en;  sys_rdata <= {{32-DWL{1'b0}}, led_o}             ; end

    20'h00040: begin sys_ack <= sys_en;  sys_rdata <= {                pll_cfg_rd}        ; end

    20'h00048: begin sys_ack <= sys_en;  sys_rdata <= {{32-  5{1'b0}}, idly_cnt_i[4:0]}   ; end
    20'h0004C: begin sys_ack <= sys_en;  sys_rdata <= {{32-  5{1'b0}}, idly_cnt_i[9:5]}   ; end


    20'h00050: begin sys_ack <= sys_en;  sys_rdata <= {16'h0,spi_wr_h[0]}                 ; end
    20'h00054: begin sys_ack <= sys_en;  sys_rdata <= {16'h0,spi_wr_l[0]}                 ; end
    20'h00058: begin sys_ack <= sys_en;  sys_rdata <= {15'h0,spi_bsy[0],  spi_rd_l[0]}    ; end

    20'h00060: begin sys_ack <= sys_en;  sys_rdata <= {16'h0,spi_wr_h[1]}                 ; end
    20'h00064: begin sys_ack <= sys_en;  sys_rdata <= {16'h0,spi_wr_l[1]}                 ; end
    20'h00068: begin sys_ack <= sys_en;  sys_rdata <= {15'h0,spi_bsy[1],  spi_rd_l[1]}    ; end

      default: begin sys_ack <= sys_en;  sys_rdata <=  32'h0                              ; end
  endcase
end

endmodule
