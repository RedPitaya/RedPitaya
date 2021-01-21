

`timescale 1ns / 1ps

module top_tc ();


default clocking cb @ (posedge top_tb.clk);
endclocking: cb

task test_hk (
  int unsigned offset,
  int unsigned led=0 
);
  int unsigned dat;
  // test registers
  axi_read_osc1(offset+'h0, dat); //ID
  axi_read_osc1(offset+'h4, dat); //DNA
  axi_read_osc1(offset+'h8, dat); //DNA
  axi_write(offset+'h30, led); // LED
  axi_read_osc1(offset+'h30, dat); // LED

  ##1000;
  axi_write(offset+'h40, 1); // enable PLL
  ##1000;
  axi_read_osc1(offset+'h40, dat); // PLL status


  axi_write(offset+'h44, 32'hFFFF); // reset IDELAY

  //ADC SPI
  axi_write(offset+'h50, 32'h8016); // SPI offset
  axi_write(offset+'h54, 32'h20); // SPI offset
  ##1000;
  axi_read_osc1(offset+'h58, dat); // SPI offset

  //DAC SPI
  axi_write(offset+'h60, 32'h83); // SPI offset
  axi_write(offset+'h64, 32'h54); // SPI offset
  ##1000;
  axi_read_osc1(offset+'h68, dat); // SPI offset


  for (int k=0; k<4; k++) begin
  ##100;
   $display("%m - Increment IDELAY @%0t.", $time) ;
   axi_write(offset+'h48, 32'hFFFF); // increment IDELAY A
   //axi_write(offset+'h4C, 32'hFFFF); // increment IDELAY B
  end
  axi_write(offset+'h44, 32'hFFFF); // reset IDELAY

endtask: test_hk



////////////////////////////////////////////////////////////////////////////////
// Testing osciloscope
////////////////////////////////////////////////////////////////////////////////

task test_osc(
  int unsigned offset,
  int unsigned evnt_in
);
   int unsigned dat;

  ##100;
  // configure
  // DMA control address (80) controls ack signals for buffers. for breakdown see rp_dma_s2mm_ctrl


  axi_write(offset+'h0,   'd4);  // start
  axi_write(offset+'h04,   evnt_in);  // osc1 events
  axi_write(offset+'h10,  'h2000);  // pre trigger samples
  axi_write(offset+'h14,  'h6000);  // pre trigger samples
  axi_write(offset+'h20,  'd30);  // LOW LEVEL TRIG
  axi_write(offset+'h24,  'd50);  // HI LEVEL TRIG
  ##5;
  axi_write(offset+'h28, 'd0);  // TRIG EDGE
  ##5
  axi_write(offset+'h38, 'd0);  // decimation enable
  axi_write(offset+'h3C, 'd1);  // decimation enable
  axi_write(offset+'h30, 'h18);  // decimation factor
  axi_write(offset+'h34, 'd0);  // decimation shift
  axi_write(offset+'h0,   'd2);  // start
  axi_write(offset+'h08,   'd4);  // start

  ##5;
  axi_write(offset+'h64, 'h1000);  // buffer 1 address
  ##5;
  axi_write(offset+'h68, 'h2000);  // buffer 2 address
  ##5;
  axi_write(offset+'h6C, 'd30000);  // buffer 1 address
  ##5;
  axi_write(offset+'h70, 'd40000);  // buffer 2 address
  ##5;
  axi_write(offset+'h74, 'h0);  // calibration offset
  ##5;
  axi_write(offset+'h78, 'h8000);  // calibration gain
  ##5;
  axi_write(offset+'h58, 'h400);  // buffer size - must be greater than axi burst size (128)
  ##5;
      axi_write(offset+'h50, 'h212);  // streaming DMA
      axi_write(offset+'h50, 'h2);  // streaming DMA
      axi_write(offset+'h50, 'h1);  // streaming DMA
        axi_write(offset+'h0,   'd0);  // start
  axi_write(offset+'h0,   'd1);  // start
  axi_write(offset+'h0,   'd2);  // start


  //axi_write(offset+'h00, 4'b1000);  // trigger

  int_ack(offset);
    axi_write(offset+'h74, 'hFFFFF800);  // calibration offset

  int_ack_del(offset);
  int_ack(offset);
  int_ack_del(offset);
  int_ack(offset);
  int_ack_del(offset);
  int_ack(offset);
  int_ack_del(offset);
  int_ack(offset);
  int_ack_del(offset);
  int_ack(offset);
  int_ack_del(offset);
  int_ack(offset);
  int_ack_del(offset);
  int_ack(offset);
  int_ack_del(offset);

  /*
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);
  int_ack(offset);*/
  
  
  /*##1300;
  axi_write(offset+'d80,   'd4);  // BUF1 ACK
  ##1300;
  axi_write(offset+'d80,   'd8);  // BUF2 ACK
  ##1300;
  axi_write(offset+'d80,   'd4);  // BUF1 ACK
  ##1300;
  axi_write(offset+'d80,   'd8);  // BUF2 ACK*/
##10000;
endtask: test_osc

task buf_ack(
  int unsigned offset
);
  int unsigned dat;
  //axi_read_osc1(offset+'d80, dat);
  ##10;

  // configure
  // DMA control address (80) controls ack signals for buffers. for breakdown see rp_dma_s2mm_ctrl

  do begin
    axi_read_osc1(offset+'d84, dat);
    ##5;
  end while (dat[0] != 1'b1); // BUF 1 is full
  ##1000;
  axi_write(offset+'d80, 'd4);  // BUF1 ACK

  do begin
    axi_read_osc1(offset+'d84, dat);
        ##5;
  end while (dat[1] != 1'b1); // BUF 2 is full
  ##2000;
  axi_write(offset+'d80,   'd8);  // BUF2 ACK 

endtask: buf_ack

task int_ack(
  int unsigned offset
);
  int unsigned dat;
  ##2000;

  // configure
  // DMA control address (80) controls ack signals for buffers. for breakdown see rp_dma_s2mm_ctrl
  do begin
    ##5;
  end while (top_tb.red_pitaya_top_sim.system_wrapper_i.system_i.processing_system7_0.IRQ_F2P[1] != 1'b1); // BUF 1 is full
  ##5;
  axi_write(offset+'h50, 'd2);  // INTR ACK
  ##500;
  axi_write(offset+'h50, 'h4);  // BUF1 ACK

  do begin
        ##5;
  end while (top_tb.red_pitaya_top_sim.system_wrapper_i.system_i.processing_system7_0.IRQ_F2P[1] != 1'b1); // BUF 2 is full
  ##5;
  axi_write(offset+'h50, 'd2);  // INTR ACK
  ##500;
  axi_write(offset+'h50,   'h8);  // BUF2 ACK 

endtask: int_ack

task int_ack_del(
  int unsigned offset
);
  int unsigned dat;
  axi_read_osc1(offset+'h50, dat);
  ##10;

  // configure
  // DMA control address (80) controls ack signals for buffers. for breakdown see rp_dma_s2mm_ctrl

  do begin
    ##5;
  end while (top_tb.red_pitaya_top_sim.system_wrapper_i.system_i.processing_system7_0.IRQ_F2P[1] != 1'b1); // BUF 1 is full
  ##5;
  axi_write(offset+'h50, 'd2);  // INTR ACK
  ##500;
  axi_write(offset+'h50, 'h4);  // BUF1 ACK

  do begin
        ##5;
  end while (top_tb.red_pitaya_top_sim.system_wrapper_i.system_i.processing_system7_0.IRQ_F2P[1] != 1'b1); // BUF 2 is full
  ##5;
  axi_write(offset+'h50, 'd2);  // INTR ACK
  ##500;
  axi_write(offset+'h50, 'h8);  // BUF2 ACK


endtask: int_ack_del
////////////////////////////////////////////////////////////////////////////////
// Testing arbitrary signal generator
////////////////////////////////////////////////////////////////////////////////

task test_asg(
  int unsigned offset,
  int unsigned buffer,
  int unsigned sh = 0
);

reg [9-1: 0] ch0_set;
reg [9-1: 0] ch1_set;
logic        [ 32-1: 0] rdata;
logic signed [ 32-1: 0] rdata_blk [];

  ##10;

  // CH0 DAC data
  axi_write(offset+32'h10000, 32'd3     );  // write table
  axi_write(offset+32'h10004, 32'd30    );  // write table
  axi_write(offset+32'h10008, 32'd8000  );  // write table
  axi_write(offset+32'h1000C,-32'd4     );  // write table
  axi_write(offset+32'h10010,-32'd40    );  // write table
  axi_write(offset+32'h10014,-32'd8000  );  // write table
  axi_write(offset+32'h10018,-32'd2000  );  // write table
  axi_write(offset+32'h1001c, 32'd250   );  // write table

  // CH0 DAC settings
  axi_write(offset+32'h00004,{2'h0,-14'd500, 2'h0, 14'h2F00}  );  // DC offset, amplitude
  axi_write(offset+32'h00008,{2'h0, 14'd7, 16'hffff}          );  // table size
  axi_write(offset+32'h0000C,{2'h0, 14'h1, 16'h0}             );  // reset offset
  axi_write(offset+32'h00010,{2'h0, 14'h2, 16'h0}             );  // table step
  axi_write(offset+32'h00018,{16'h0, 16'd0}                   );  // number of cycles
  axi_write(offset+32'h0001C,{16'h0, 16'd0}                   );  // number of repetitions
  axi_write(offset+32'h00020,{32'd3}                          );  // number of 1us delay between repetitions

  ch0_set = {1'b0 ,1'b0, 1'b0, 1'b0, 1'b1,    1'b0, 3'h2} ;  // set_rgate, set_zero, set_rst, set_once(NA), set_wrap, 1'b0, trig_src

  // CH1 DAC data
  for (int k=0; k<8000; k++) begin
    axi_write(offset+32'h20000 + (k*4), k);  // write table
  end

  // CH1 DAC settings
  axi_write(offset+32'h00024,{2'h0, 14'd0, 2'h0, 14'h2000}    );  // DC offset, amplitude
  axi_write(offset+32'h00028,{2'h0, 14'd7999, 16'hffff}       );  // table size
  axi_write(offset+32'h0002C,{2'h0, 14'h5, 16'h0}             );  // reset offset
  axi_write(offset+32'h00030,{2'h0, 14'h9, 16'h0}             );  // table step
  axi_write(offset+32'h00038,{16'h0, 16'd0}                   );  // number of cycles
  axi_write(offset+32'h0003C,{16'h0, 16'd5}                   );  // number of repetitions
  axi_write(offset+32'h00040,{32'd10}                         );  // number of 1us delay between repetitions

  ch1_set = {1'b0, 1'b0, 1'b0, 1'b1, 1'b1,    1'b0, 3'h1} ;  // set_rgate, set_zero, set_rst, set_once(NA), set_wrap, 1'b0, trig_src

  axi_write(offset+32'h00000,{8'h0, ch1_set,  8'h0, ch0_set}  ); // write configuration

  ##2000;

  ch1_set = {1'b0, 1'b0, 1'b1, 1'b1,    1'b0, 3'h1} ;  // set_a_zero, set_a_rst, set_a_once, set_a_wrap, 1'b0, trig_src

  axi_write(offset+32'h00000,{7'h0, ch1_set,  7'h0, ch0_set}  ); // write configuration

  ##200;

  // CH1 table data readback
  rdata_blk = new [80];
  for (int k=0; k<80; k++) begin
    axi_read_osc1(offset+32'h20000 + (k*4), rdata_blk [k]);  // read table
  end

  // CH1 table data readback
  for (int k=0; k<20; k++) begin
    axi_read_osc1(offset+32'h00014, rdata);  // read read pointer
    axi_read_osc1(offset+32'h00034, rdata);  // read read pointer
    ##1737;
  end

  ##20000;

endtask: test_asg





////////////////////////////////////////////////////////////////////////////////
// Testing SATA
////////////////////////////////////////////////////////////////////////////////

task test_sata(
  int unsigned offset,
  int unsigned sh = 0
);
logic        [ 32-1: 0] rdata;
  ##10;

  // configure
  ##100; axi_write(offset+'h0, 32'h1      );        // Enable transmitter
  ##20;  axi_write(offset+'h0, 32'h3      );        // Enable transmitter & receiver
  ##101; axi_write(offset+'h4, 32'h3      );        // enable TX train
  ##10;  axi_write(offset+'h8, 32'h1      );        // enable RX train
  ##1500; axi_read_osc1 (offset+'hC, rdata      );        // Return read value
  ##20;  axi_write(offset+'h8, 32'h0      );        // disable RX train
  ##20;  axi_write(offset+'h4, {16'hF419, 16'h2});  // Custom value
  ##20;  axi_write(offset+'h4, {16'hF419, 16'h5});  // Random valu
  ##20;  axi_write(offset+'h10, 32'h1      );       // Clear error counter
  ##20;  axi_write(offset+'h10, 32'h0      );       // Enable error counter
  ##404; axi_write(offset+'h4, {16'h0, 16'h4});     // Sent back read value

  ##1000;

endtask: test_sata


































////////////////////////////////////////////////////////////////////////////////
// AXI4 read/write tasks
////////////////////////////////////////////////////////////////////////////////

task axi_read_osc1 (
  input  logic [32-1:0] adr,
  output logic [32-1:0] dat
);
  top_tb.axi_bm_reg.ReadTransaction (
    .ARDelay (0),  .ar ('{
                          id    : 0,
                          addr  : adr,
                          region: 0,
                          len   : 0,
                          size  : 3'b010,
                          burst : 0,
                          lock  : 0,
                          cache : 0,
                          prot  : 0,
                          qos   : 0
                         }),
     .RDelay (0),   .rdat (dat)
  );
endtask: axi_read_osc1

/*task axi_read_osc2 (
  input  logic [32-1:0] adr,
  output logic [32-1:0] dat
);
  top_tb.axi_bm_osc2.ReadTransaction (
    .ARDelay (0),  .ar ('{
                          id    : 0,
                          addr  : adr,
                          region: 0,
                          len   : 0,
                          size  : 3'b010,
                          burst : 0,
                          lock  : 0,
                          cache : 0,
                          prot  : 0,
                          qos   : 0
                         }),
     .RDelay (0),   .rdat (dat)
  );
endtask: axi_read_osc2*/


task axi_write (
  input  logic [32-1:0] adr,
  input  logic [32-1:0] dat
);
  int b;
  top_tb.axi_bm_reg.WriteTransaction (
    .AWDelay (0),  .aw ('{
                          id    : 0,
                          addr  : adr,
                          region: 0,
                          len   : 0,
                          size  : 3'b010,
                          burst : 0,
                          lock  : 0,
                          cache : 0,
                          prot  : 0,
                          qos   : 0
                         }),
     .WDelay (0),   .w ('{
                          id    : 0,
                          data  : dat,
                          strb  : '1,
                          last  : 1
                         }),
     .BDelay (0),   .b (b)
  );
endtask: axi_write






endmodule
