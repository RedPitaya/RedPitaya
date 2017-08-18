////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya top FPGA module
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module top_tb #(
  // time period
  realtime  TP = 8.0ns,  // 125MHz
  // DUT configuration
  int unsigned DAC_DW = 14, // ADC data width
  int unsigned RSZ = 14  // RAM size is 2**RSZ
);

////////////////////////////////////////////////////////////////////////////////
// IO port signals
////////////////////////////////////////////////////////////////////////////////

// PS connections
wire  [54-1:0] FIXED_IO_mio     ;
wire           FIXED_IO_ps_clk  ;
wire           FIXED_IO_ps_porb ;
wire           FIXED_IO_ps_srstb;
wire           FIXED_IO_ddr_vrn ;
wire           FIXED_IO_ddr_vrp ;
// DDR
wire  [15-1:0] DDR_addr   ;
wire  [ 3-1:0] DDR_ba     ;
wire           DDR_cas_n  ;
wire           DDR_ck_n   ;
wire           DDR_ck_p   ;
wire           DDR_cke    ;
wire           DDR_cs_n   ;
wire  [ 4-1:0] DDR_dm     ;
wire  [32-1:0] DDR_dq     ;
wire  [ 4-1:0] DDR_dqs_n  ;
wire  [ 4-1:0] DDR_dqs_p  ;
wire           DDR_odt    ;
wire           DDR_ras_n  ;
wire           DDR_reset_n;
wire           DDR_we_n   ;

// ADC
logic [2-1:0] [16-1:0] adc_dat;
logic         [ 2-1:0] adc_clk;
logic         [ 2-1:0] adc_clk_o;   // optional ADC clock source
logic                  adc_cdcs_o;  // ADC clock duty cycle stabilizer
// DAC
logic         [14-1:0] dac_dat;     // DAC combined data
logic                  dac_wrt;     // DAC write
logic                  dac_sel;     // DAC channel select
logic                  dac_clk;     // DAC clock
logic                  dac_rst;     // DAC reset
// PDM DAC
logic         [ 4-1:0] dac_pwm;     // 1-bit PDM DAC
// XADC
logic         [ 5-1:0] vinp;        // voltages p
logic         [ 5-1:0] vinn;        // voltages n
// Expansion connector
wire          [ 8-1:0] exp_p_io;
wire          [ 8-1:0] exp_n_io;
// Expansion output data/enable
logic         [ 8-1:0] exp_p_od, exp_p_oe;
logic         [ 8-1:0] exp_n_od, exp_n_oe;
// LED
wire          [ 8-1:0] led;

glbl glbl();

////////////////////////////////////////////////////////////////////////////////
// DAC signal generation
////////////////////////////////////////////////////////////////////////////////

logic               clk ;
logic               rstn;

logic               trig;

// DAC clock
initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

// clocking 
default clocking cb @ (posedge clk);
  input  rstn;
  input  exp_p_od, exp_p_oe;
  input  exp_n_od, exp_n_oe;
endclocking: cb

// DAC reset
initial begin
  rstn = 1'b0;
  ##4;
  rstn = 1'b1;
end

// ADC cycle counter
int unsigned cyc=0;
always_ff @ (posedge clk)
cyc <= cyc+1;

always begin
  trig <= 1'b0;
  ##100000;
  trig <= 1'b1;
  ##1200;
  trig <= 1'b0;
end

////////////////////////////////////////////////////////////////////////////////
// initializtion
////////////////////////////////////////////////////////////////////////////////

initial begin
  exp_p_od = '0;
  exp_n_od = '0;
  exp_p_oe = '0;
  exp_n_oe = '0;
end

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  ##6000;
  $display("ERROR: timeout!");
  $finish();
end

initial begin
  ##100;
//   test_id                 (32'h40000000);
//   test_mgmt               (32'h40010000, '1, '0);
//   test_gen_periodic       (32'h40040000, 32'h40050000, 0);
//   test_gen_burst          (32'h40040000, 32'h40050000, 0);
//   test_gen_bst_ext_trig (32'h40040000, 32'h40050000, 0);
//   test_gen_per_ext_trig (32'h40040000, 32'h40050000, 0);
  test_lg_burst           (32'h400c0000, 32'h400d0000, 4);
//   test_la_trg         (32'h400e0000, 32'h400f0000, 5);
//   //  ##16;
//   test_osc                (32'h40080000, 32'h40090000, 2);
//   test_loopback           ();
//   test_clb (32'h40030000);
//   test_la (32'h40300000);
//   test_la_automatic (32'h40300000);
  ##16;
  $finish();
end

////////////////////////////////////////////////////////////////////////////////
// AXI4 read/write tasks
////////////////////////////////////////////////////////////////////////////////

task axi_read (
  input  logic [32-1:0] adr,
  output logic [32-1:0] dat
);
  top_tb.top.ps.system_i.axi_bus_model.ReadTransaction (
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
endtask: axi_read

task axi_write (
  input  logic [32-1:0] adr,
  input  logic [32-1:0] dat
);
  int b;
  top_tb.top.ps.system_i.axi_bus_model.WriteTransaction (
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

////////////////////////////////////////////////////////////////////////////////
// signal generation
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned DWM = 14;
localparam int unsigned CWM = 14;
localparam int unsigned CWF = 16;

//int buf_len = 2**CWM;
int buf_len = 'hff+1;
real freq  = 10_000; // 10kHz
real phase = 0; // DEG

task test_loopback ;
  logic signed [ 32-1: 0] rdata_blk;
  logic        [ 32-1: 0] sts_pre;
  logic        [ 32-1: 0] sts_pst;
  int                     ptr;
  static int              siz = 32;  
 
  ##10;
  // gen
  axi_write('h40040000+'h04 , 'h00000000 ); // SW event source select
  // osc
  axi_write('h40080000+'h40 , 'h00007d93 ); // AA coeficient
  axi_write('h40080000+'h44 , 'h000437c7 ); // BB coeficient
  axi_write('h40080000+'h48 , 'h00d9999a ); // KK coeficient
  axi_write('h40080000+'h4c , 'h00002666 ); // PP coeficient
  axi_write('h40080000+'h04 , 'h00000002 ); // SW event source select
  axi_write('h40080000+'h08 , 'h00000004 ); // HW trigger mask
  // gen data
  axi_write('h40050000+'h00 , 'h0000A00  ); // DAT0
  axi_write('h40050000+'h04 , 'h0000A00  ); // DAT1
  axi_write('h40050000+'h08 , 'h0000A00  ); // DAT2
  axi_write('h40050000+'h0c , 'h0000A00  ); // DAT3
  axi_write('h40050000+'h10 , 'h0000A00  ); // DAT4
  axi_write('h40050000+'h14 , 'h0000A00  ); // DAT5
  axi_write('h40050000+'h18 , 'h0000A00  ); // DAT6
  axi_write('h40050000+'h1c , 'h0000A00  ); // DAT7
  axi_write('h40050000+'h20 , 'h0000000  ); // DAT8
  // gen
  axi_write('h40040000+'h14 , 'h0001ffff ); // table size
  axi_write('h40040000+'h20 , 'h00000000 ); // burst data   repetition
//   axi_write('h40040000+'h24 , 'h00001fff ); // burst data   length
  axi_write('h40040000+'h24 , 'h00000008 ); // burst data   length
  axi_write('h40040000+'h28 , 'h00002fff ); // burst period length
  axi_write('h40040000+'h2c , 'h00000003 ); // burst period number
  axi_write('h40040000+'h10 , 'h00000001 ); // Generator mode
  axi_write('h40040000+'h40 , 'h00001000 ); // multiplier (amplitude)
  axi_write('h40040000+'h44 , 'h00000000 ); // adder (offset)
  axi_write('h40040000+'h48 , 'h00000001 ); // output enable
  // osc
  axi_write('h40080000+'h30 , 'h00000000 ); // decimation factor
  axi_write('h40080000+'h3C , 'h00000001 ); // bypass filter
  axi_write('h40080000+'h10 , 'h00000000 ); // delay pre  trigger
  axi_write('h40080000+'h14 , siz        ); // delay post trigger
  axi_write('h40080000+'h04 , 'h00000000 ); // SW event source select
  axi_write('h40080000+'h08 , 'h00000000 ); // HW trigger mask
  
  // ### loop
  axi_write('h40010000+'h04 , 'h00000003 ); // gen->osc loop
  ##1000;
  
  // loop 1
  axi_write('h40040000+'h00 , 'h00000001 ); // reset
  axi_write('h40040000+'h00 , 'h00000002 ); // status
  axi_write('h40040000+'h00 , 'h00000008 ); // trigger
  ##40;
  
  axi_read('h40080000 + 'h18, sts_pre );  // read table
  axi_read('h40080000 + 'h1C, sts_pst );  // read table
  sts_pre = sts_pre & 'h7FFFFFFF;
  sts_pst = sts_pst & 'h7FFFFFFF;
  ptr=(sts_pre + sts_pst) % 16384;
  $display(" pre %d, post %d, ptr %d",sts_pre ,sts_pst,ptr);
  for (int i=(ptr-siz); i<ptr; i++) begin
    axi_read('h40090000 + (i*2), rdata_blk );  // read table
    $display("(%2d) rdata_blk[%H] %H",i-(ptr-siz),'h40090000 + (i*2), i[0] ?  rdata_blk[32-1:18] : rdata_blk[16-1:2] );
  end
  ##1000;
    
  // loop 2
  // gen data
  axi_write('h40050000+'h00 , 'h0000B00  ); // DAT0
  axi_write('h40050000+'h04 , 'h0000B00  ); // DAT1
  axi_write('h40050000+'h08 , 'h0000B00  ); // DAT2
  axi_write('h40050000+'h0c , 'h0000B00  ); // DAT3
  axi_write('h40050000+'h10 , 'h0000B00  ); // DAT4
  axi_write('h40050000+'h14 , 'h0000B00  ); // DAT5
  axi_write('h40050000+'h18 , 'h0000B00  ); // DAT6
  axi_write('h40050000+'h1c , 'h0000B00  ); // DAT7
  axi_write('h40050000+'h20 , 'h0000000  ); // DAT8
  
  axi_write('h40040000+'h00 , 'h00000001 ); // reset
  ##30;
  axi_write('h40040000+'h00 , 'h00000002 ); // status
  ##30;
  axi_write('h40040000+'h00 , 'h00000008 ); // trigger
  ##40;
  
  axi_read('h40080000 + 'h18, sts_pre );  // read table
  axi_read('h40080000 + 'h1C, sts_pst );  // read table
  sts_pre = sts_pre & 'h7FFFFFFF;
  sts_pst = sts_pst & 'h7FFFFFFF;
  ptr=(sts_pre + sts_pst) % 16384;
  $display(" pre %d, post %d, ptr %d",sts_pre ,sts_pst,ptr);
  for (int i=(ptr-siz); i<ptr; i++) begin
    axi_read('h40090000 + (i*2), rdata_blk );  // read table
    $display("(%2d) rdata_blk[%H] %H",i-(ptr-siz),'h40090000 + (i*2), i[0] ?  rdata_blk[32-1:18] : rdata_blk[16-1:2] );
  end
  ##1000;
  
  // loop 3
  axi_write('h40050000+'h00 , 'h0000700  ); // DAT0
  axi_write('h40050000+'h04 , 'h0000700  ); // DAT1
  axi_write('h40050000+'h08 , 'h0000700  ); // DAT2
  axi_write('h40050000+'h0c , 'h0000700  ); // DAT3
  axi_write('h40050000+'h10 , 'h0000700  ); // DAT4
  axi_write('h40050000+'h14 , 'h0000700  ); // DAT5
  axi_write('h40050000+'h18 , 'h0000700  ); // DAT6
  axi_write('h40050000+'h1c , 'h0000700  ); // DAT7
  axi_write('h40050000+'h20 , 'h0000000  ); // DAT8

  axi_write('h40040000+'h00 , 'h00000001 ); // reset
  ##300;
  axi_write('h40040000+'h00 , 'h00000002 ); // status
  ##300;
  axi_write('h40040000+'h00 , 'h00000008 ); // trigger
  ##40;
  
  axi_read('h40080000 + 'h18, sts_pre );  // read table
  axi_read('h40080000 + 'h1C, sts_pst );  // read table
  sts_pre = sts_pre & 'h7FFFFFFF;
  sts_pst = sts_pst & 'h7FFFFFFF;
  ptr=(sts_pre + sts_pst) % 16384;
  $display(" pre %d, post %d, ptr %d",sts_pre ,sts_pst,ptr);
  for (int i=(ptr-siz); i<ptr; i++) begin
    axi_read('h40090000 + (i*2), rdata_blk );  // read table
    $display("(%2d) rdata_blk[%H] %H",i-(ptr-siz),'h40090000 + (i*2), i[0] ?  rdata_blk[32-1:18] : rdata_blk[16-1:2] );
  end
  ##1000;
  
  // loop 4  
  axi_write('h40050000+'h00 , 'h0000D00  ); // DAT0
  axi_write('h40050000+'h04 , 'h0000D00  ); // DAT1
  axi_write('h40050000+'h08 , 'h0000D00  ); // DAT2
  axi_write('h40050000+'h0c , 'h0000D00  ); // DAT3
  axi_write('h40050000+'h10 , 'h0000D00  ); // DAT4
  axi_write('h40050000+'h14 , 'h0000D00  ); // DAT5
  axi_write('h40050000+'h18 , 'h0000D00  ); // DAT6
  axi_write('h40050000+'h1c , 'h0000D00  ); // DAT7
  axi_write('h40050000+'h20 , 'h0000000  ); // DAT8
  
  axi_write('h40040000+'h00 , 'h00000001 ); // reset
  ##17000;
  axi_write('h40040000+'h00 , 'h00000002 ); // status
  ##17000;
  axi_write('h40040000+'h00 , 'h00000008 ); // trigger
  ##40;
  
  axi_read('h40080000 + 'h18, sts_pre );  // read table
  axi_read('h40080000 + 'h1C, sts_pst );  // read table
  sts_pre = sts_pre & 'h7FFFFFFF;
  sts_pst = sts_pst & 'h7FFFFFFF;
  ptr=(sts_pre + sts_pst) % 16384;
  $display(" pre %d, post %d, ptr %d",sts_pre ,sts_pst,ptr);
  for (int i=(ptr-siz); i<ptr; i++) begin
    axi_read('h40090000 + (i*2), rdata_blk );  // read table
    $display("(%2d) rdata_blk[%H] %H",i-(ptr-siz),'h40090000 + (i*2), i[0] ?  rdata_blk[32-1:18] : rdata_blk[16-1:2] );
  end
  ##1000;
  
endtask: test_loopback

task test_osc(
  int unsigned regset,
  int unsigned buffer,
  int unsigned sh = 0
);
  ##10;
  // bypass filter
  axi_write(regset+'h3c, 1'b1);

  // events
  axi_write(regset+'h04, sh);  // SW event select
  axi_write(regset+'h08, '1);  // trigger mask
  // bypass input filter
  axi_write(regset+'h3c, 'h1);

  // configure trigger level
  axi_write(regset+'h20, -'d4);  // level neg
  axi_write(regset+'h24, +'d4);  // level pos
  axi_write(regset+'h28,  'h0);  // edge positive
  axi_write(regset+'h2c,  'h8);  // holdoff

  // configure trigger timing
  axi_write(regset+'h10, 'd08);  // cfg_pre
  axi_write(regset+'h14, 'd24);  // cfg_pst

  // reset, start/trigger acquire
  axi_write(regset+'h00, 4'b0001);  // reset
  axi_write(regset+'h00, 4'b0010);  // start
  //axi_write(regset+'h00, 4'b0100);  // stop
  //axi_write(regset+'h00, 4'b1000);  // trigger
  ##1000;

endtask: test_osc


task test_gen_periodic (
  int unsigned regset,
  int unsigned buffer,
  int unsigned sh = 0
);
  logic signed [ 32-1: 0] rdata_blk [];
  ##10;

  // write table
  for (int i=0; i<buf_len; i++) begin
    axi_write(buffer + (i*2), i<<4);  // write table
  end
//  for (int i=0; i<buf_len; i+=2) begin
//    logic [2-1:0] [16-1:0] data;
//    data [0] = i;
//    data [1] = i+1;
//    axi_write(buffer + (i*2), data);  // write table
//  end
//  // read table
//  rdata_blk = new [80];
//  for (int i=0; i<buf_len; i++) begin
//    axi_read(buffer + (i*2), rdata_blk [i]);  // read table
//  end

  // configure amplitude and DC offset
  axi_write(regset+'h40, 1 << (DWM-2));  // amplitude
  axi_write(regset+'h44, 0);             // DC offset
  axi_write(regset+'h48, 1);             // output enable

  // configure frequency and phase
  axi_write(regset+'h14,  buf_len                    * 2**CWF - 1);  // table size
  axi_write(regset+'h18, (buf_len * (phase/360.0)  ) * 2**CWF    );  // offset
  axi_write(regset+'h1c, 2**CWF);  // step
//axi_write(regset+'h1c, (buf_len * (freq*TP/10**6)) * 2**CWF - 1);  // step
  // configure burst mode
  axi_write(regset+'h10, 2'b00);  // burst disable
  // events
  axi_write(regset+'h04, sh);  // SW event select
  axi_write(regset+'h08, '0);  // trigger mask
  // reset, start, trigger
  axi_write(regset+'h00, 4'b0001);
  axi_write(regset+'h00, 4'b0010);
  axi_write(regset+'h00, 4'b1000);
  ##22;
  // stop (reset)
//  axi_write(regset+'h00, 2'b01);
  ##20;
endtask: test_gen_periodic

task test_gen_burst (
  int unsigned regset,
  int unsigned buffer,
  int unsigned sh = 0
);
  // write table
  for (int i=0; i<8; i++) begin
    axi_write(buffer + (i*4), i);  // write table
  end

  // configure amplitude and DC offset
  axi_write(regset+'h40, 1 << (DWM-2));  // amplitude
  axi_write(regset+'h44, 0);             // DC offset
  axi_write(regset+'h48, 1);             // output enable

  // configure burst mode
  axi_write(regset+'h10, 2'b11);  // burst disable
  // burst mode
  axi_write(regset+'h20,  1 - 1);  // burst data repetitions
  axi_write(regset+'h24,  2 - 1);  // burst data length
  axi_write(regset+'h28, 16 - 1);  // burst period length
  axi_write(regset+'h2c,  4 - 1);  // burst period number
  // events
  axi_write(regset+'h04, sh);  // SW event select
  axi_write(regset+'h08, '0);  // trigger mask
  // reset, start, trigger
  axi_write(regset+'h00, 4'b0001);
  axi_write(regset+'h00, 4'b0010);
  axi_write(regset+'h00, 4'b1000);
  ##22;
  // stop (reset)
//  axi_write(regset+'h00, 2'b01);
  ##20;
endtask: test_gen_burst

task test_gen_bst_ext_trig (
  int unsigned regset,
  int unsigned buffer,
  int unsigned sh = 0
);
  // write table
  for (int i=0; i<8; i++) begin
    axi_write(buffer + (i*4), i);  // write table
  end

  // enable GPIO
  exp_p_oe[0] = 1'b1;

  // configure amplitude and DC offset
  axi_write(regset+'h40, 1 << (DWM-2));  // amplitude
  axi_write(regset+'h44, 0);             // DC offset
  axi_write(regset+'h48, 1);             // output enable

  // configure burst mode
  axi_write(regset+'h10, 2'b01);  // select burst mode
  // burst mode
  axi_write(regset+'h20,  1 - 1);  // burst data repetitions
  axi_write(regset+'h24,  6 - 1);  // burst data length
  axi_write(regset+'h28, 16 - 1);  // burst period length
  axi_write(regset+'h2c,  4 - 1);  // burst period number
  // events
  axi_write(regset+'h04, sh);  // SW event select
  axi_write(regset+'h08, 1<<5);  // trigger mask
  // reset, start, trigger
  axi_write(regset+'h00, 4'b0001);
  axi_write(regset+'h00, 4'b0010);
  
  // SW triggers
//   repeat(10) begin
//   axi_write(regset+'h00, 4'b1000);
//     ##4;
//   end
  // setup LA trigger
  // enable posedge trigger
  axi_write('h400e0000+'h28, 16'h0001);
  // enable input mask
  axi_write('h400e0000+'h40, 16'h0001);

  // create a series of pulses on exp_p_io[0]
  for (int unsigned i=0; i<12; i++) begin
    exp_p_od[0] = 1'b1;
    ##1;
    exp_p_od[0] = 1'b0;
    ##6; 
  end
  
  // stop (reset)
//  axi_write(regset+'h00, 2'b01);
  ##20;
endtask: test_gen_bst_ext_trig

task test_gen_per_ext_trig (
  int unsigned regset,
  int unsigned buffer,
  int unsigned sh = 0
);
  // write table
  for (int i=0; i<16; i++) begin
    axi_write(buffer + (i*4), i);  // write table
  end

  // enable GPIO
  exp_p_oe[0] = 1'b1;

  // configure amplitude and DC offset
  axi_write(regset+'h40, 1 << (DWM-2));  // amplitude
  axi_write(regset+'h44, 0);             // DC offset
  axi_write(regset+'h48, 1);             // output enable

  // configure burst mode
  axi_write(regset+'h10, 2'b00);  // select burst mode
  // burst mode
  axi_write(regset+'h14, 16 << 16 );  // table size
  axi_write(regset+'h18,  1 << 16 );  // address initial offset (phase)
  axi_write(regset+'h1c,  1 << 16 );  // address increment step (frequency)
  // events
  axi_write(regset+'h04, sh);  // SW event select
  axi_write(regset+'h08, 1<<5);  // trigger mask
  // reset, start, trigger
  axi_write(regset+'h00, 4'b0001);
  axi_write(regset+'h00, 4'b0010);
  
  // SW triggers
//   repeat(10) begin
//   axi_write(regset+'h00, 4'b1000);
//     ##4;
//   end
  // setup LA trigger
  // enable posedge trigger
  axi_write('h400e0000+'h28, 16'h0001);
  // enable input mask
  axi_write('h400e0000+'h40, 16'h0001);

  // create a series of pulses on exp_p_io[0]
  for (int unsigned i=0; i<12; i++) begin
    exp_p_od[0] = 1'b1;
    ##1;
    exp_p_od[0] = 1'b0;
    ##6; 
  end

  // stop (reset)
//  axi_write(regset+'h00, 2'b01);
  ##20;
endtask: test_gen_per_ext_trig


// calibration regset test
task test_clb (
  int unsigned regset
);
  int dat;
  ##10;
  // write all registers
  for (int unsigned i=0; i<8; i++) begin
    axi_write(regset+i*4, i);
  end
  // read all registers
  for (int unsigned i=0; i<8; i++) begin
    axi_read(regset+i*4, dat);
    $display ("clb: @%04x = %08x", i*4, dat);
  end
  ##10;
endtask: test_clb


task test_mgmt (
  int unsigned regset,
  int unsigned cfg_iom = '0,
  int unsigned cfg_loop = '0
);
  axi_write(regset+'h00, cfg_iom);
  axi_write(regset+'h04, cfg_loop);
endtask: test_mgmt

task test_lg_burst (
  int unsigned regset,
  int unsigned buffer,
  int unsigned sh = 0
);
  ##10;
  // write table
//   for (int i=0; i<buf_len; i++) begin
    axi_write(buffer + (0), 1);  // write table
    axi_write(buffer + (4), 0);  // write table

//   end
  // configure LG output enable
  axi_write(regset+'h40, '1);  // output enable 0
  axi_write(regset+'h44, '1);  // output enable 1
  axi_write(regset+'h48, '0);  // mask
  axi_write(regset+'h4c, '0);  // value/polarity

  ##4;
  // configure burst mode
  axi_write(regset+'h10, 2'b01);  // select burst mode
  // burst mode
  axi_write(regset+'h20,  7 - 1);  // burst data repetitions
  axi_write(regset+'h24,  2 - 1);  // burst data length
  axi_write(regset+'h28, 23 - 1);  // burst period length
  axi_write(regset+'h2c,  4 - 1);  // burst period number

  // events
  ##4;
  axi_write(regset+'h04, sh);  // SW event select
  axi_write(regset+'h08, '0);  // trigger mask
  // reset/start/trigger
  ##4;
  axi_write(regset+'h00, 4'b0001); 
  axi_write(regset+'h00, 4'b0010);
  axi_write(regset+'h00, 4'b1000);
  ##100;
  // stop (reset)
endtask: test_lg_burst


task test_la_trg (
  int unsigned regset
);
  // set GPIO into neutral state
  ##10;
  // configure trigger
  axi_write(regset+'h20, 16'h0000);  // cfg_cmp_msk
  axi_write(regset+'h24, 16'h0000);  // cfg_cmp_val
  axi_write(regset+'h28, 16'h0001);  // cfg_edg_pos
  axi_write(regset+'h2c, 16'h0000);  // cfg_edg_neg
  ##10;
endtask: test_la_trg

task test_la (
  int unsigned regset,
  int unsigned buffer,
  int unsigned sh = 0
);
  ##10;
  // configure trigger
  axi_write(regset+'h20, 16'h0000);  // cfg_cmp_msk
  axi_write(regset+'h24, 16'h0000);  // cfg_cmp_val
  axi_write(regset+'h28, 16'h0001);  // cfg_edg_pos
  axi_write(regset+'h2c, 16'h0000);  // cfg_edg_neg

  axi_write(regset+'h10, 'd8 );  // cfg_pre
  axi_write(regset+'h14, 'd16);  // cfg_pst
  // enable LA trigger source
  axi_write(regset+'h08, 'b0010);
  // start acquire
  axi_write(regset+'h00, 4'b0100);
  ##1000;
endtask: test_la


task test_la_automatic (
  int unsigned regset
);
  ##10;

  // enable automatic mode
  axi_write(regset+'h04, 'h2);  // cfg_aut <= 1
  // configure trigger
  axi_write(regset+'h10, 'd0);  // cfg_pre
  axi_write(regset+'h14, 'd4);  // cfg_pst
  // ignore triggers
  axi_write(regset+'h08, 'b0000);
  // start acquire
  axi_write(regset+'h00, 4'b0100);
  ##1000;
endtask: test_la_automatic


task test_id (
  int unsigned regset
);
  int unsigned dat;
  // configure trigger
  axi_read(regset+'h20, dat);
  axi_read(regset+'h24, dat);
  axi_read(regset+'h28, dat);
  axi_read(regset+'h2c, dat);
  axi_read(regset+'h30, dat);
endtask: test_id

////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

red_pitaya_top #(
  .GITH (160'ha0a1a2a3b0b1b2b3c0c1c2c3d0d1d2d3e0e1e2e3)
) top (
  // PS connections
  .FIXED_IO_mio      (FIXED_IO_mio     ),
  .FIXED_IO_ps_clk   (FIXED_IO_ps_clk  ),
  .FIXED_IO_ps_porb  (FIXED_IO_ps_porb ),
  .FIXED_IO_ps_srstb (FIXED_IO_ps_srstb),
  .FIXED_IO_ddr_vrn  (FIXED_IO_ddr_vrn ),
  .FIXED_IO_ddr_vrp  (FIXED_IO_ddr_vrp ),
  // DDR
  .DDR_addr       (DDR_addr   ),
  .DDR_ba         (DDR_ba     ),
  .DDR_cas_n      (DDR_cas_n  ),
  .DDR_ck_n       (DDR_ck_n   ),
  .DDR_ck_p       (DDR_ck_p   ),
  .DDR_cke        (DDR_cke    ),
  .DDR_cs_n       (DDR_cs_n   ),
  .DDR_dm         (DDR_dm     ),
  .DDR_dq         (DDR_dq     ),
  .DDR_dqs_n      (DDR_dqs_n  ),
  .DDR_dqs_p      (DDR_dqs_p  ),
  .DDR_odt        (DDR_odt    ),
  .DDR_ras_n      (DDR_ras_n  ),
  .DDR_reset_n    (DDR_reset_n),
  .DDR_we_n       (DDR_we_n   ),

  // Red Pitaya periphery
  
  // ADC
  .adc_dat_i      (adc_dat),
  .adc_clk_i      (adc_clk),
  .adc_clk_o      (adc_clk_o),
  .adc_cdcs_o     (adc_cdcs_o),
  // DAC
  .dac_dat_o      (dac_dat),
  .dac_wrt_o      (dac_wrt),
  .dac_sel_o      (dac_sel),
  .dac_clk_o      (dac_clk),
  .dac_rst_o      (dac_rst),
  // PDM DAC
  .dac_pwm_o      (dac_pwm),
  // XADC
  .vinp_i         (vinp),
  .vinn_i         (vinn),
  // Expansion connector
  .exp_p_io       (exp_p_io),
  .exp_n_io       (exp_n_io),
  // SATA connector
  .daisy_p_o      (),
  .daisy_n_o      (),
  .daisy_p_i      ('0),
  .daisy_n_i      ('0),
  // LED
  .led_o          (led)
);

bufif1 bufif_exp_p_io [8-1:0] (exp_p_io, exp_p_od, exp_p_oe);
bufif1 bufif_exp_n_io [8-1:0] (exp_n_io, exp_n_od, exp_n_oe);

////////////////////////////////////////////////////////////////////////////////
// simulated inputs
////////////////////////////////////////////////////////////////////////////////

//localparam int unsigned SIZ_REF = 64;
//
//bit [16-1:0] dat_ref [SIZ_REF];
//
//initial begin
//  logic signed [16-1:0] dat;
//  for (int unsigned i=0; i<SIZ_REF; i++) begin
//      dat = -SIZ_REF/2+i;
//      dat_ref[i] = {dat[16-1], ~dat[16-2:0]};
//  end
//end

bit [16-1:0] dat_ref [2*15];

initial begin
  for (int unsigned i=0; i<15; i++) begin
    dat_ref [16-1-i] = {1'b1, 15'(1<<i)};
    dat_ref [16  +i] = {1'b0, 15'(1<<i)};
  end
end

// ADC
assign adc_dat[0] = dat_ref[cyc % $size(dat_ref)];
assign adc_dat[1] = dat_ref[cyc % $size(dat_ref)];
assign adc_clk[1] =  clk;
assign adc_clk[0] = ~clk;
// adc_clk_o

// XADC
assign vinp = '0;
assign vinn = '0;

// Expansion connector
//assign exp_p_io = 8'h0;
//assign exp_n_io = 8'h0;

// LED

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("top_tb.vcd");
  $dumpvars(0, top_tb);
end

endmodule: top_tb
