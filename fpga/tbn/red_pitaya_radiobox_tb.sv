/**
 *
 * @brief Red Pitaya oscilloscope testbench.
 *
 * @Author Matej Oblak, Ulrich Habel (DF4IAH)
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in Verilog hardware description language (HDL).
 * Please visit http://en.wikipedia.org/wiki/SystemVerilog
 * for more details on the language used herein.
 */

/**
 * GENERAL DESCRIPTION:
 *
 * Testbench for the Red Pitaya radiobox module.
 *
 * This testbench populates all registers and function blocks of the
 * radiobox module for correct implementation. Look up the following detailed
 * test plan.
 *
 */

// test plan
// 1. DDS1
// 1.1. signal generation
// n. ...


`timescale 1ns / 1ps

module red_pitaya_radiobox_tb #(
  // time periods
  realtime  TP125 =  8.0ns                      // 125 MHz

  // DUT configuration
/*
  int unsigned ADC_DW = 14,                     // ADC data width
  int unsigned RSZ = 14                         // RAM size is 2**RSZ
*/
);


////////////////////////////////////////////////////////////////////////////////
//
// Settings

//localparam TASK01_FREE1 = 32'd0;


////////////////////////////////////////////////////////////////////////////////
//
// Connections

// System signals
int unsigned               clk_cntr = 999999 ;
reg                        clk_adc_125mhz    ;
reg                        adc_rstn_i        ;

// Output signals
wire                       rb_leds_en        ;  // RB does overwrite LEDs state
wire           [  8-1: 0]  rb_leds_data      ;  // RB LEDs data
wire                       rb_en             ;  // RadioBox is enabled
wire           [ 16-1: 0]  rb_out_ch   [1:0] ;  // RadioBox output signals

// System bus
wire           [ 32-1: 0]  sys_addr          ;
wire           [ 32-1: 0]  sys_wdata         ;
wire           [  4-1: 0]  sys_sel           ;
wire                       sys_wen           ;
wire                       sys_ren           ;
wire           [ 32-1: 0]  sys_rdata         ;
wire                       sys_err           ;
wire                       sys_ack           ;

/*
// Ext. signals and triggering
logic                      trig_ext          ;
*/

// Local
reg            [ 32-1: 0]  task_check        ;

/*
logic          [ 32-1: 0]  rdata;
bit   signed   [ 32-1: 0]  rdata_ref []      ;
int unsigned               rdata_trg [$]     ;
int unsigned               blk_size          ;
*/


////////////////////////////////////////////////////////////////////////////////
//
// Module instances

sys_bus_model bus (
  // system signals
  .clk            ( clk_adc_125mhz          ),
  .rstn           ( adc_rstn_i              ),

  // bus protocol signals
  .sys_addr       ( sys_addr                ),
  .sys_wdata      ( sys_wdata               ),
  .sys_sel        ( sys_sel                 ),
  .sys_wen        ( sys_wen                 ),
  .sys_ren        ( sys_ren                 ),
  .sys_rdata      ( sys_rdata               ),
  .sys_err        ( sys_err                 ),
  .sys_ack        ( sys_ack                 )
);

red_pitaya_radiobox #(
//.RSZ            ( RSZ                     )
) radiobox        (
  // ADC
  .clk_adc_125mhz ( clk_adc_125mhz          ),  // ADC based clock, 125 MHz
  .adc_rstn_i     ( adc_rstn_i              ),  // ADC reset - active low

  .rb_leds_en     ( rb_leds_en              ),  // RB does overwrite LEDs state
  .rb_leds_data   ( rb_leds_data            ),  // RB LEDs data

  /*
  .adc_a_i        (                         ),  // ADC data CHA
  .adc_b_i        (                         ),  // ADC data CHB
  */

  // DAC data
  .rb_en          ( rb_en                   ),  // RadioBox is enabled
  .rb_out_ch      ( rb_out_ch               ),  // RadioBox output signals

  // System bus
  .sys_addr       ( sys_addr                ),
  .sys_wdata      ( sys_wdata               ),
  .sys_sel        ( sys_sel                 ),
  .sys_wen        ( sys_wen                 ),
  .sys_ren        ( sys_ren                 ),
  .sys_rdata      ( sys_rdata               ),
  .sys_err        ( sys_err                 ),
  .sys_ack        ( sys_ack                 )
);


////////////////////////////////////////////////////////////////////////////////
//
// Helpers

/*
// Task: read_blk
logic signed   [ 32-1: 0]  rdata_blk [];

task read_blk (
  input int          adr,
  input int unsigned len
);
  rdata_blk = new [len];
  for (int unsigned i=0; i<len; i++) begin
    bus.read(adr + 4*i, rdata_blk[i]);
  end
endtask: read_blk
*/


////////////////////////////////////////////////////////////////////////////////
//
// Stimuli

// Clock and Reset generation
initial begin
   clk_adc_125mhz   = 1'b0;
   adc_rstn_i       = 1'b0;

   repeat(10) @(negedge clk_adc_125mhz);
   adc_rstn_i = 1'b1;
end

always begin
   #(TP125 / 2)
   clk_adc_125mhz = 1'b1;

   if (adc_rstn_i)
      clk_cntr = clk_cntr + 1;
   else
      clk_cntr = 32'd0;


   #(TP125 / 2)
   clk_adc_125mhz = 1'b0;
end


// main FSM
initial begin
   // presets
/*
   blk_size = 20 ;
   trig_ext = 1'b0 ;                            // external trigger
*/

  // get to initial state
  wait (adc_rstn_i)
  repeat(2) @(posedge clk_adc_125mhz);

  // TASK 01: addition - set two registers and request the result
  bus.write(20'h00000, 32'h00000000);           // control
  bus.write(20'h00008, 32'h00000000);           // clear ICR
  bus.write(20'h00010, 32'h00000000);           // clear DMA

  //bus.write(20'h0001C, 32'h00000005);         // LEDs: showing OSC2 output
  bus.write(20'h0001C, 32'h00000004);           // LEDs: showing OSC2 mixer output
  //bus.write(20'h0001C, 32'h00000003);         // LEDs: showing OSC1 output
  //bus.write(20'h0001C, 32'h00000002);         // LEDs: showing OSC1 mixer output

  bus.write(20'h00020, 32'h56789abc);           // OSC1 INC LO
  bus.write(20'h00024, 32'h00001234);           // OSC1 INC HI --> 160,127.987 Hz
  bus.write(20'h00028, 32'h00000000);           // OSC1 OFS LO
  bus.write(20'h0002C, 32'h00000000);           // OSC1 OFS HI
  bus.write(20'h00030, 32'h7fffffff);           // OSC1 MIX GAIN, SIGNED
  bus.write(20'h00038, 32'h00000000);           // OSC1 MIX OFS LO
  bus.write(20'h0003C, 32'h00000000);           // OSC1 MIX OFS HI

  bus.write(20'h00040, 32'h6789abcd);           // OSC2 INC LO
  bus.write(20'h00044, 32'h00000345);           // OSC2 INC HI -->  28,772.998 Hz
  bus.write(20'h00048, 32'h00000000);           // OSC2 OFS LO
  bus.write(20'h0004C, 32'h00000000);           // OSC2 OFS HI
  bus.write(20'h00050, 32'h7fffffff);           // OSC2 MIX GAIN, SIGNED
  bus.write(20'h00058, 32'h00000000);           // OSC2 MIX OFS LO
  bus.write(20'h0005C, 32'h00000000);           // OSC2 MIX OFS HI

  bus.read (20'h00000, task_check);             // read result register
  if (task_check != 32'h00000000)
     $display("FAIL - Task:01.01 read REG_RW_RB_CTRL, read=%d, (should be: %d)", task_check, 32'h00000000);
  else
     $display("PASS - Task:01.01 read REG_RW_RB_CTRL");

  bus.read (20'h00008, task_check);             // read result register
  if (task_check != 32'h00000000)
     $display("FAIL - Task:01.02 read REG_RW_RB_ICR, read=%d, (should be: %d)", task_check, 32'h00000000);
  else
     $display("PASS - Task:01.02 read REG_RW_RB_ICR");

  bus.read (20'h00010, task_check);             // read result register
  if (task_check != 32'h00000000)
     $display("FAIL - Task:01.03 read REG_RW_RB_DMA_CTRL, read=%d, (should be: %d)", task_check, 32'h00000000);
  else
     $display("PASS - Task:01.03 read REG_RW_RB_DMA_CTRL");


  $display("INFO - Task:11 Start DDS OSC1 & OSC2");
  bus.write(20'h00000, 32'h00000001);           // control: enable RadioBox - starts DDS oscillators
  repeat(100) @(posedge clk_adc_125mhz);        // OSC1 / OSC2 having 9 stages

  $display("INFO - Task:12 Resync DDS OSC1 & OSC2");
  bus.write(20'h00000, 32'h00001011);           // control: resync OSC1 & OSC2
  repeat(5) @(posedge clk_adc_125mhz);
  bus.write(20'h00000, 32'h00001017);           // control: resync OSC1 & OSC2 and reset OSC1 & OSC2
  repeat(10) @(posedge clk_adc_125mhz);
  bus.write(20'h00000, 32'h00001011);           // control: resync OSC1 & OSC2
  repeat(5) @(posedge clk_adc_125mhz);

  $display("INFO - Task:13 Resync off, going to normal enabled state");
  bus.write(20'h00000, 32'h00000001);           // control: normal enabled state
  repeat(250) @(posedge clk_adc_125mhz);

  $display("INFO - Task:14 reversing OSC1 mixer and OSC2 mixer output");
  bus.write(20'h00030, 32'h80000000);           // OSC1 MIX GAIN, SIGNED
  bus.write(20'h00050, 32'h80000000);           // OSC2 MIX GAIN, SIGNED
  repeat(250) @(posedge clk_adc_125mhz);


  $display("INFO - Task:21 AM modulated signal: OSC1 RF, OSC2 Modulation, AM 100%% modulation");
  bus.write(20'h00000, 32'h00001017);           // control: resync OSC1 & OSC2
  bus.write(20'h00040, 32'h56789abc);           // OSC2 INC LO
  bus.write(20'h00044, 32'h00000234);           // OSC2 INC HI -->  19,390.498 Hz
  bus.write(20'h00050, 32'h3fffffff);           // OSC2 MIX GAIN,     SIGNED
  bus.write(20'h00058, 32'hffffffff);           // OSC2 MIX OFS LO, UNSIGNED - 100% modulation
  bus.write(20'h0005C, 32'h00003fff);           // OSC2 MIX OFS HI, UNSIGNED
  repeat(20) @(posedge clk_adc_125mhz);
  bus.write(20'h00000, 32'h00000081);           // control: amplitude modulation
  repeat(250) @(posedge clk_adc_125mhz);

  $display("INFO - Task:22 AM modulated signal: OSC1 RF, OSC2 Modulation, AM 100%% modulation, Phase reversed");
  bus.write(20'h00050, 32'hc0000000);           // OSC2 MIX GAIN,     SIGNED
  repeat(250) @(posedge clk_adc_125mhz);


  $display("INFO - Task:31 FM modulated signal: OSC1 RF, OSC2 Modulation, FM 100%% modulation");
  bus.write(20'h00000, 32'h00001017);           // control: resync OSC1 & OSC2
  bus.write(20'h00050, 32'h12345677);           // OSC2 MIX GAIN, SIGNED
  bus.write(20'h00058, 32'h56789abc);           // OSC2 MIX OFS LO - FM base frequency
  bus.write(20'h0005C, 32'h00001234);           // OSC2 MIX OFS HI - FM base frequency
  repeat(20) @(posedge clk_adc_125mhz);
  bus.write(20'h00000, 32'h00000021);           // control: frequency modulation
  repeat(250) @(posedge clk_adc_125mhz);

  $display("INFO - Task:32 FM modulated signal: OSC1 RF, OSC2 Modulation, FM 100%% modulation, Phase 90°");
  bus.write(20'h00050, (32'hffffffff - 32'h12345676));  // OSC2 MIX GAIN, SIGNED
  repeat(250) @(posedge clk_adc_125mhz);


  $display("INFO - Task:41 PM modulated signal: OSC1 RF, OSC2 Modulation, PM 360° modulation");
  bus.write(20'h00000, 32'h00001017);           // control: resync OSC1 & OSC2
  bus.write(20'h00050, 32'h12345677);           // OSC2 MIX GAIN, SIGNED
  bus.write(20'h00058, 32'h00000000);           // OSC2 MIX OFS  - no PM offset
  bus.write(20'h0005C, 32'h00000000);           // OSC2 MIX OFS  - no PM offset
  repeat(20) @(posedge clk_adc_125mhz);
  bus.write(20'h00000, 32'h00000041);           // control: phase modulation
  repeat(250) @(posedge clk_adc_125mhz);

  $display("INFO - Task:42 PM modulated signal: OSC1 RF, OSC2 Modulation, PM 360° modulation, Phase reversed");
  bus.write(20'h00050, (32'hffffffff - 32'h12345676));  // OSC2 MIX GAIN, SIGNED
  repeat(250) @(posedge clk_adc_125mhz);


  $display("INFO - Task:99 disabling RadioBox sub-module");
  bus.write(20'h00000, 32'h00001017);           // control: resync OSC1 & OSC2
  repeat(20) @(posedge clk_adc_125mhz);
  bus.write(20'h00000, 32'h00000000);           // control: disable RadioBox - switches off OSC1, OSC2, OSC1/OSC2 mixer
  repeat(1000) @(posedge clk_adc_125mhz);

  $display("FINISH");
  $finish () ;
end


////////////////////////////////////////////////////////////////////////////////
// Waveforms output
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_radiobox_tb.vcd");
  $dumpvars(0, red_pitaya_radiobox_tb);
end

endmodule: red_pitaya_radiobox_tb
