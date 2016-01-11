/**
 * $Id: axi_slave.v 961 2014-01-21 11:40:39Z matej.oblak $
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



/**
 * GENERAL DESCRIPTION:
 *
 * AXI slave used also for simple bus master.
 *
 *
 *                     /------\
 *   WR ADDRESS  ----> |  WR  |
 *   WR DATA     ----> |      | ----------- 
 *   WR RESPONSE <---- |  CH  |            |
 *                     \------/       /--------\
 *                                    | SIMPLE | ---> WR/RD ADDRRESS
 *   AXI                              |        | ---> WR DATA
 *                                    |   RP   | <--- RD DATA
 *                                    |  BUS   | <--- ACKNOWLEDGE
 *                     /------\       \--------/
 *   RD ADDRESS  ----> |  RD  |            |
 *   RD DATA     <---- |  CH  | -----------
 *                     \------/
 *
 *
 * Because AXI bus is quite complex simplier bus was created.
 * 
 * It combines write and read channel, where write has bigger priority. Command
 * is then send forward to red pitaya bus. When wite or read acknowledge is
 * received AXI response is created and new AXI is accepted.
 *
 * To prevent AXI lockups because no response is received, this slave creates its
 * own after 32 cycles (ack_cnt).
 * 
 */

module axi_slave #(
  int unsigned AXI_DW =  64        , // data width (8,16,...,1024)
  int unsigned AXI_AW =  32        , // address width
  int unsigned AXI_IW =   8        , // ID width
  int unsigned AXI_SW = AXI_DW >> 3  // sobe width - 1 bit for every data byte
)(
  // AXI MM
  axi_bus_if.s axi,
  // system read/write channel
  sys_bus_if.m bus
);

//---------------------------------------------------------------------------------
//  AXI slave Module
//---------------------------------------------------------------------------------

logic                ack      ;
logic [      6-1: 0] ack_cnt  ;

logic                rd_do    ;
logic [ AXI_IW-1: 0] rd_arid  ;
logic [ AXI_AW-1: 0] rd_araddr;
logic                rd_error ;
logic                rd_errorw;

logic                wr_do    ;
logic [ AXI_IW-1: 0] wr_awid  ;
logic [ AXI_AW-1: 0] wr_awaddr;
logic [ AXI_IW-1: 0] wr_wid   ;
logic [ AXI_DW-1: 0] wr_wdata ;
logic                wr_error ;
logic                wr_errorw;

assign wr_errorw = (axi.AWLEN != 4'h0) || (axi.AWSIZE != 3'b010); // error if write burst and more/less than 4B transfer
assign rd_errorw = (axi.ARLEN != 4'h0) || (axi.ARSIZE != 3'b010); // error if read burst and more/less than 4B transfer

always_ff @(posedge axi.ACLK)
if (~axi.ARESETn) begin
   rd_do    <= 1'b0 ;
   rd_error <= 1'b0 ;
end else begin
   if (axi.ARVALID && !rd_do && !axi.AWVALID && !wr_do) // accept just one read request - write has priority
      rd_do  <= 1'b1 ;
   else if (axi.RREADY && rd_do && ack)
      rd_do  <= 1'b0 ;

   if (axi.ARVALID && axi.ARREADY) begin // latch ID and address
      rd_arid   <= axi.ARID   ;
      rd_araddr <= axi.ARADDR ;
      rd_error  <= rd_errorw    ;
   end
end

always_ff @(posedge axi.ACLK)
if (~axi.ARESETn) begin
   wr_do    <= 1'b0 ;
   wr_error <= 1'b0 ;
end else begin
   if (axi.AWVALID && !wr_do && !rd_do) // accept just one write request - if idle
      wr_do  <= 1'b1 ;
   else if (axi.BREADY && wr_do && ack)
      wr_do  <= 1'b0 ;
   if (axi.AWVALID && axi.AWREADY) begin // latch ID and address
      wr_awid   <= axi.AWID  ;
      wr_awaddr <= axi.AWADDR;
      wr_error  <= wr_errorw ;
   end
   if (axi.WVALID && wr_do) begin // latch ID and write data
      wr_wid    <= axi.WID  ;
      wr_wdata  <= axi.WDATA;
   end
end

assign axi.AWREADY = !wr_do && !rd_do;
assign axi.WREADY  = (wr_do && axi.WVALID) || (wr_errorw && axi.WVALID);
assign axi.BID     = wr_awid;

assign axi.ARREADY = !rd_do && !wr_do && !axi.AWVALID;
assign axi.RID     = rd_arid                         ;

always_ff @(posedge axi.ACLK)
if (~axi.ARESETn) begin
   axi.BVALID  <= 1'b0 ;
   axi.BRESP   <= 2'h0 ;
   axi.RLAST   <= 1'b0 ;
   axi.RVALID  <= 1'b0 ;
   axi.RRESP   <= 2'h0 ;
end else begin
   axi.BVALID  <= wr_do && ack  ;
   axi.BRESP   <= {(wr_error || ack_cnt[5]),1'b0} ;  // 2'b10 SLVERR    2'b00 OK
   axi.RLAST   <= rd_do && ack  ;
   axi.RVALID  <= rd_do && ack  ;
   axi.RRESP   <= {(rd_error || ack_cnt[5]),1'b0} ;  // 2'b10 SLVERR    2'b00 OK
   axi.RDATA   <= bus.rdata;
end

// acknowledge protection
always_ff @(posedge axi.ACLK)
if (~axi.ARESETn) begin
   ack_cnt   <= 6'h0 ;
end else begin
   if ((axi.ARVALID && axi.ARREADY) || (axi.AWVALID && axi.AWREADY))  // rd || wr request
      ack_cnt <= 6'h1 ;
   else if (ack)
      ack_cnt <= 6'h0 ;
   else if (|ack_cnt)
      ack_cnt <= ack_cnt + 6'h1 ;
end

assign ack = bus.ack || ack_cnt[5] || (rd_do && rd_errorw) || (wr_do && wr_errorw); // bus acknowledge or timeout or error

//------------------------------------------
//  Simple slave interface

always_ff @(posedge axi.ACLK)
if (~axi.ARESETn) begin
   bus.wen <= 1'b0 ;
   bus.ren <= 1'b0 ;
end else begin
   bus.wen <= wr_do && axi.WVALID && !wr_errorw;
   bus.ren <= axi.ARVALID && axi.ARREADY && !rd_errorw;
end

assign bus.addr  = rd_do ? rd_araddr : wr_awaddr  ;
assign bus.wdata = wr_wdata                       ;

endmodule: axi_slave
