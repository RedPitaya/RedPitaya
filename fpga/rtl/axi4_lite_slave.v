/**
 * $Id: axi4_lite_slave.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya symplified AXI slave.
 *
 * @Author Matej Oblak
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/Verilog
 * for more details on the language used herein.
 */
module axi4_lite_slave #(
  parameter           AXI_DW         =  32           , // data width (8,16,...,1024)
  parameter           AXI_AW         =  32           , // address width
  parameter           AXI_SW         = AXI_DW >> 3     // strobe width - 1 bit for every data byte
)(
  // global signals
  input                     axi_clk_i      ,  //!< AXI global clock
  input                     axi_rstn_i     ,  //!< AXI global reset
  // axi write address channel
  input      [ AXI_AW-1: 0] axi_awaddr_i   ,  //!< AXI write address
  input      [      3-1: 0] axi_awprot_i   ,  //!< AXI write protection type
  input                     axi_awvalid_i  ,  //!< AXI write address valid
  output                    axi_awready_o  ,  //!< AXI write ready
  // axi write data channel
  input      [ AXI_DW-1: 0] axi_wdata_i    ,  //!< AXI write data
  input      [ AXI_SW-1: 0] axi_wstrb_i    ,  //!< AXI write strobes
  input                     axi_wvalid_i   ,  //!< AXI write valid
  output                    axi_wready_o   ,  //!< AXI write ready
  // axi write response channel
  output reg [      2-1: 0] axi_bresp_o    ,  //!< AXI write response
  output reg                axi_bvalid_o   ,  //!< AXI write response valid
  input                     axi_bready_i   ,  //!< AXI write response ready
  // axi read address channel
  input      [ AXI_AW-1: 0] axi_araddr_i   ,  //!< AXI read address
  input      [      3-1: 0] axi_arprot_i   ,  //!< AXI read protection type
  input                     axi_arvalid_i  ,  //!< AXI read address valid
  output                    axi_arready_o  ,  //!< AXI read address ready
  // axi read data channel
  output reg [ AXI_DW-1: 0] axi_rdata_o    ,  //!< AXI read data
  output reg [      2-1: 0] axi_rresp_o    ,  //!< AXI read response
  output reg                axi_rvalid_o   ,  //!< AXI read response valid
  input                     axi_rready_i   ,  //!< AXI read response ready
  // RP system read/write channel
  output     [ AXI_AW-1: 0] sys_addr_o     ,  //!< system bus read/write address.
  output     [ AXI_DW-1: 0] sys_wdata_o    ,  //!< system bus write data.
  output reg                sys_wen_o      ,  //!< system bus write enable.
  output reg                sys_ren_o      ,  //!< system bus read enable.
  input      [ AXI_DW-1: 0] sys_rdata_i    ,  //!< system bus read data.
  input                     sys_err_i      ,  //!< system bus error indicator.
  input                     sys_ack_i         //!< system bus acknowledge signal.
);

//---------------------------------------------------------------------------------
//  AXI slave Module
//---------------------------------------------------------------------------------

wire                 ack         ;
reg   [      6-1: 0] ack_cnt     ;

reg                  rd_do       ;
reg   [ AXI_AW-1: 0] rd_araddr   ;

reg                  wr_do       ;
reg   [ AXI_AW-1: 0] wr_awaddr   ;
reg   [ AXI_DW-1: 0] wr_wdata    ;


always @(posedge axi_clk_i)
if (axi_rstn_i == 1'b0) begin
   rd_do    <= 1'b0 ;
end else begin
   if (axi_arvalid_i && !rd_do && !axi_awvalid_i && !wr_do) // accept just one read request - write has priority
      rd_do  <= 1'b1 ;
   else if (axi_rready_i && rd_do && ack)
      rd_do  <= 1'b0 ;

   if (axi_arvalid_i && axi_arready_o) begin // latch ID and address
      rd_araddr <= axi_araddr_i ;
   end
end

always @(posedge axi_clk_i)
if (axi_rstn_i == 1'b0) begin
   wr_do    <= 1'b0 ;
end else begin
   if (axi_awvalid_i && !wr_do && !rd_do) // accept just one write request - if idle
      wr_do  <= 1'b1 ;
   else if (axi_bready_i && wr_do && ack)
      wr_do  <= 1'b0 ;
   if (axi_awvalid_i && axi_awready_o) begin // latch ID and address
      wr_awaddr <= axi_awaddr_i ;
   end
   if (axi_wvalid_i && wr_do) begin // latch ID and write data
      wr_wdata  <= axi_wdata_i  ;
   end
end

assign axi_awready_o = !wr_do && !rd_do                      ;
assign axi_wready_o  = (wr_do && axi_wvalid_i);

assign axi_arready_o = !rd_do && !wr_do && !axi_awvalid_i     ;

always @(posedge axi_clk_i)
if (axi_rstn_i == 1'b0) begin
   axi_bvalid_o  <= 1'b0 ;
   axi_bresp_o   <= 2'h0 ;
   axi_rvalid_o  <= 1'b0 ;
   axi_rresp_o   <= 2'h0 ;
end else begin
   axi_bvalid_o  <= wr_do && ack  ;
   axi_bresp_o   <= {(ack_cnt[5]),1'b0} ;  // 2'b10 SLVERR    2'b00 OK
   axi_rvalid_o  <= rd_do && ack  ;
   axi_rresp_o   <= {(ack_cnt[5]),1'b0} ;  // 2'b10 SLVERR    2'b00 OK
   axi_rdata_o   <= sys_rdata_i   ;
end

// acknowledge protection
always @(posedge axi_clk_i)
if (axi_rstn_i == 1'b0) begin
   ack_cnt   <= 6'h0 ;
end else begin
   if ((axi_arvalid_i && axi_arready_o) || (axi_awvalid_i && axi_awready_o))  // rd || wr request
      ack_cnt <= 6'h1 ;
   else if (ack)
      ack_cnt <= 6'h0 ;
   else if (|ack_cnt)
      ack_cnt <= ack_cnt + 6'h1 ;
end

assign ack = sys_ack_i || ack_cnt[5]; // bus acknowledge or timeout or error

//------------------------------------------
//  Simple slave interface

always @(posedge axi_clk_i)
if (axi_rstn_i == 1'b0) begin
   sys_wen_o  <= 1'b0 ;
   sys_ren_o  <= 1'b0 ;
end else begin
   sys_wen_o  <= wr_do && axi_wvalid_i;
   sys_ren_o  <= axi_arvalid_i && axi_arready_o;
end

assign sys_addr_o  = rd_do ? rd_araddr : wr_awaddr  ;
assign sys_wdata_o = wr_wdata                       ;

endmodule
