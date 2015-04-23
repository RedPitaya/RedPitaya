/**
 * $Id: red_pitaya_daisy_tb.v 964 2014-01-24 12:58:17Z matej.oblak $
 *
 * @brief Red Pitaya daisy chain testbench.
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
 * Testbench to test daisy chain connection.
 *
 * Testing daisy connection using single daisy chain module connected via
 * loopback. Commands makes initial training and than goes into mode of 
 * sending and checking received random numbers. 
 * 
 */

`timescale 1ns / 1ps

module red_pitaya_daisy_tb(
);

glbl glbl ();

logic [  4-1: 0] daisy_p         ;
logic [  4-1: 0] daisy_n         ;
logic            par_clk         ;
logic            par_rstn        ;
logic            ser_clk         ;
logic            dly_clk         ;

logic            sys_clk         ;
logic            sys_rstn        ;
logic [ 32-1: 0] sys_addr        ;
logic [ 32-1: 0] sys_wdata       ;
logic [  4-1: 0] sys_sel         ;
logic            sys_wen         ;
logic            sys_ren         ;
logic [ 32-1: 0] sys_rdata       ;
logic            sys_err         ;
logic            sys_ack         ;

logic [ 32-1: 0] rdata;

logic [ 16-1: 0] test_cnt        ;
logic            test_val        ;
logic            test_rdy        ;

//---------------------------------------------------------------------------------
//
// signal generation

initial begin
   sys_clk  <= 1'b0 ;
   sys_rstn <= 1'b0 ;
   repeat(10) @(posedge sys_clk);
      sys_rstn <= 1'b1  ;
end

initial   sys_clk = 1'b0 ;
always #5 sys_clk = !sys_clk ;

initial begin
   par_rstn <= 1'b0  ;
   repeat(10) @(posedge par_clk);
      par_rstn <= 1'b1  ;
end

initial      par_clk = 1'b0;
always #4    par_clk = !par_clk;

initial      ser_clk = 1'b1;
always #2    ser_clk = !ser_clk; //2 for 250MHz

initial      dly_clk = 1'b1;
always #2.5  dly_clk = !dly_clk;

assign #0.2 daisy_p[3] = daisy_p[1] ;
assign #0.2 daisy_n[3] = daisy_n[1] ;
assign #0.2 daisy_p[2] = daisy_p[0] ;
assign #0.2 daisy_n[2] = daisy_n[0] ;

always @(posedge par_clk)
begin
  if (!par_rstn) begin
    test_cnt <= 16'h0;
    test_val <=  1'b0;
  end else begin
     test_val <= test_rdy;
  if (test_val)
     test_cnt <= test_cnt + 16'h1;
  end
end

// Commands on bus
initial begin
   wait (sys_rstn && par_rstn)
   repeat(100) @(posedge sys_clk); bus.write(32'h0, 32'h1      );        // Enable transmitter
   repeat(20)  @(posedge sys_clk); bus.write(32'h0, 32'h3      );        // Enable transmitter & receiver
   repeat(101) @(posedge sys_clk); bus.write(32'h4, 32'h3      );        // enable TX train
   repeat(10)  @(posedge sys_clk); bus.write(32'h8, 32'h1      );        // enable RX train
   repeat(500) @(posedge sys_clk); bus.read (32'hC, rdata      );        // Return read value
   repeat(20)  @(posedge sys_clk); bus.write(32'h8, 32'h0      );        // disable RX train
   repeat(20)  @(posedge sys_clk); bus.write(32'h4, {16'hF419, 16'h2});  // Custom value
   repeat(20)  @(posedge sys_clk); bus.write(32'h4, {16'hF419, 16'h5});  // Random valu
   repeat(20)  @(posedge sys_clk); bus.write(32'h10, 32'h1      );       // Clear error counter
   repeat(20)  @(posedge sys_clk); bus.write(32'h10, 32'h0      );       // Enable error counter
   repeat(404) @(posedge sys_clk); bus.write(32'h4, {16'h0, 16'h4});     // Sent back read value

   repeat(20000) @(posedge par_clk);
end

////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

sys_bus_model bus (
  // system signals
  .clk          (clk      ),
  .rstn         (rstn     ),
  // bus protocol signals
  .sys_addr     (sys_addr ),
  .sys_wdata    (sys_wdata),
  .sys_sel      (sys_sel  ),
  .sys_wen      (sys_wen  ),
  .sys_ren      (sys_ren  ),
  .sys_rdata    (sys_rdata),
  .sys_err      (sys_err  ),
  .sys_ack      (sys_ack  ) 
);

red_pitaya_daisy i_daisy (
   // SATA connector
  .daisy_p_o       ( daisy_p[1:0]  ),  //!< TX data and clock [1]-clock, [0]-data
  .daisy_n_o       ( daisy_n[1:0]  ),  //!< TX data and clock [1]-clock, [0]-data
  .daisy_p_i       ( daisy_p[3:2]  ),  //!< RX data and clock [1]-clock, [0]-data
  .daisy_n_i       ( daisy_n[3:2]  ),  //!< RX data and clock [1]-clock, [0]-data
   // Data
  .ser_clk_i       (  ser_clk    ),
  .dly_clk_i       (  dly_clk    ),
   // TX
  .par_clk_i       (  par_clk    ),  // clock
  .par_rstn_i      (  par_rstn   ),  // reset - active low
  .par_rdy_o       (  test_rdy   ),
  .par_dv_i        (  test_val   ),
  .par_dat_i       (  test_cnt   ),
   // RX
  .par_clk_o       (    ),  // receiver clock   !!! not in relation with par_clk_i !!!
  .par_rstn_o      (    ),
  .par_dv_o        (    ),
  .par_dat_o       (    ),
   // System bus
  .sys_clk_i       (  sys_clk       ),  // clock
  .sys_rstn_i      (  sys_rstn      ),  // reset - active low
  .sys_addr_i      (  sys_addr      ),  // address
  .sys_wdata_i     (  sys_wdata     ),  // write data
  .sys_sel_i       (  sys_sel       ),  // write byte select
  .sys_wen_i       (  sys_wen       ),  // write enable
  .sys_ren_i       (  sys_ren       ),  // read enable
  .sys_rdata_o     (  sys_rdata     ),  // read data
  .sys_err_o       (  sys_err       ),  // error indicator
  .sys_ack_o       (  sys_ack       )   // acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_daisy_tb.vcd");
  $dumpvars(0, red_pitaya_daisy_tb);
end

endmodule: red_pitaya_daisy_tb
