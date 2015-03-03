/**
 * $Id: red_pitaya_scope_tb.v 961 2014-01-21 11:40:39Z matej.oblak $
 *
 * @brief Red Pitaya oscilloscope testbench.
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
 * Testbench for Red Pitaya oscilloscope module.
 *
 * This testbench generates two signals which are captured into ram. Writing into
 * buffers is done via ARM/trig.
 * Generating also external trigger to test debouncer logic.
 * 
 */

// test plan
// 1. trigger:
// 1.1. software
// 1.2. treshold            p/n
// 1.3. treshold hysteresis p/n
// 1.4. external            p/n
// 1.5. external no repeat  p/n
// 1.6. external from ASG
// 2. filter/decimator configurations
// 2. ...
// 3. accumulation

`timescale 1ns / 1ps

module red_pitaya_scope_tb #(
  // time periods
  realtime  TP_ADC = 8.0ns,  // 125MHz
  realtime  TP_SYS = 9.8ns,  // 102MHz
  // DUT configuration
  int unsigned ADC_DW = 14, // ADC data width
  int unsigned RSZ = 14  // RAM size is 2**RSZ
);

////////////////////////////////////////////////////////////////////////////////
// ADC signal generation
////////////////////////////////////////////////////////////////////////////////

function [ADC_DW-1:0] saw_a (input int unsigned cyc);
  saw_a = ADC_DW'(cyc*23);
endfunction: saw_a

function [ADC_DW-1:0] saw_b (input int unsigned cyc);
  cyc = cyc % (2**ADC_DW/5);
  saw_b = -2**(ADC_DW-1) + ADC_DW'(cyc*5);
endfunction: saw_b

logic              adc_clk ;
logic              adc_rstn;

logic [ADC_DW-1:0] adc_a;
logic [ADC_DW-1:0] adc_b;

assign adc_a = saw_a(adc_cyc);
assign adc_b = saw_b(adc_cyc);

// ADC clock
initial            adc_clk = 1'b0;
always #(TP_ADC/2) adc_clk = ~adc_clk;

// ADC reset
initial begin
  adc_rstn = 1'b0;
  repeat(4) @(posedge adc_clk);
  adc_rstn = 1'b1;
end

// ADC cycle counter
int unsigned adc_cyc=0;
always_ff @ (posedge adc_clk)
adc_cyc <= adc_cyc+1;

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

logic            trig_ext ;

logic            sys_clk  ;
logic            sys_rstn ;
logic [ 32-1: 0] sys_addr ;
logic [ 32-1: 0] sys_wdata;
logic [  4-1: 0] sys_sel  ;
logic            sys_wen  ;
logic            sys_ren  ;
logic [ 32-1: 0] sys_rdata;
logic            sys_err  ;
logic            sys_ack  ;

logic        [ 32-1: 0] rdata;
logic signed [ 32-1: 0] rdata_blk [];
bit   signed [ 32-1: 0] rdata_ref [];
int unsigned            rdata_trg [$];
int unsigned            blk_size;
int unsigned            blk_cnt;
int unsigned            shift;

// system clock & reset
initial begin
  sys_rstn = 1'b0;
  repeat(4) @(posedge sys_clk);
  sys_rstn = 1'b1;
end

initial          sys_clk = 1'b0;
always #(TP_SYS) sys_clk = ~sys_clk;

task bus_read_blk (
  input int          adr,
  input int unsigned len
);
  rdata_blk = new [len];
  for (int unsigned i=0; i<len; i++) begin
    bus.bus_read(adr+4*i, rdata_blk[i]);
  end 
endtask: bus_read_blk


// State machine programming
initial begin
   // external trigger
   trig_ext = 1'b0;

   shift = 0;
   blk_size = 20;
   blk_cnt = 32;

   wait (sys_rstn && adc_rstn)
   repeat(10) @(posedge sys_clk);

   bus.bus_write(32'h08,-32'd0000 );  // A trigger treshold  (trigger at treshold     0 where signal range is -8192:+8191)
   bus.bus_write(32'h0C,-32'd7000 );  // B trigger treshold  (trigger at treshold -7000 where signal range is -8192:+8191)
   bus.bus_write(32'h10, blk_size );  // after trigger delay (the buffer contains 2**14=16384 locations, 16384-10 before and 32 after trigger)
   bus.bus_write(32'h14, 32'd0    );  // data decimation     (data is decimated by a factor of 8)
   bus.bus_write(32'h20, 32'd20   );  // A hysteresis
   bus.bus_write(32'h24, 32'd200  );  // B hysteresis

   // software trigger
   bus.bus_write(32'h00, 32'h1    );  // start aquisition (ARM, start writing data into memory
   repeat(200) @(posedge sys_clk);
   bus.bus_write(32'h04, 32'h1    );  // do SW trigger
   repeat(200) @(posedge sys_clk);
   bus.bus_write(32'h00, 32'h2    );  // reset before aquisition ends
   repeat(200) @(posedge sys_clk);

   // A ch rising edge trigger
   bus.bus_write(32'h04, 32'h2    );  // configure trigger mode
   bus.bus_write(32'h00, 32'h1    );  // start aquisition (ARM, start writing data into memory
   repeat(200) @(posedge sys_clk);
   bus.bus_write(32'h00, 32'h2    );  // reset before aquisition ends
   repeat(200) @(posedge sys_clk);

   // external rising edge trigger
   bus.bus_write(32'h90, 32'h0    );  // set debouncer length to zero
   bus.bus_write(32'h04, 32'h6    );  // configure trigger mode
   bus.bus_write(32'h00, 32'h1    );  // start aquisition (ARM, start writing data into memory
   repeat(200) @(posedge sys_clk);
   trig_ext = 1'b1;
   repeat(200) @(posedge sys_clk);
   trig_ext = 1'b0;

   // accumulator
   bus.bus_write(32'h04, 32'h6     );  // configure trigger mode (external rising edge)
   bus.bus_write(32'h98, blk_cnt -1);  // accumulate blk_cnt triggers
   bus.bus_write(32'ha0, blk_size-1);  // block length
   bus.bus_write(32'h9c, shift     );  // shift accumulator output by 3 bit to get the result
   bus.bus_write(32'h94, 32'd1     );  // enable accumulator
   bus.bus_write(32'h94, 32'd3     );  // run accumulation

   fork
     // provide external trigger
     begin: acu_trg
       // short trigger pulse
       repeat(20) @(posedge sys_clk);       trig_ext = 1'b1;
       repeat( 1) @(posedge sys_clk);       trig_ext = 1'b0;
       repeat(20) @(posedge sys_clk);
       // long trigger pulse
       repeat(20) @(posedge sys_clk);       trig_ext = 1'b1;
       repeat(20) @(posedge sys_clk);       trig_ext = 1'b0;
       repeat(20) @(posedge sys_clk);
       // ignored trigger pulse 
       repeat(20) @(posedge sys_clk);       trig_ext = 1'b1;
       repeat( 2) @(posedge sys_clk);       trig_ext = 1'b0;
       repeat( 2) @(posedge sys_clk);       trig_ext = 1'b1;
       repeat( 2) @(posedge sys_clk);       trig_ext = 1'b0;
       repeat(20) @(posedge sys_clk);
       // a sequence of short triggers
       repeat (blk_cnt) begin
         repeat( 1) @(posedge sys_clk);       trig_ext = 1'b1;
         repeat( 1) @(posedge sys_clk);       trig_ext = 1'b0;
         repeat(20) @(posedge sys_clk);
       end
     end
     // pool accumulation run status
     begin: acu_run
       // pooling loop
       do begin
         bus.bus_read(32'h94, rdata);  // read value from memory
         repeat(20) @(posedge sys_clk);
       end while (rdata & 2);
       repeat(20) @(posedge sys_clk);
       // readout
       bus_read_blk (32'h30000, blk_size);
       // build reference:
       rdata_ref = new [blk_size];
       for (int unsigned i=0; i<rdata_trg.size(); i++) begin
         for (int unsigned j=0; j<blk_size; j++) begin
           rdata_ref[j] = (i ? $signed(rdata_ref[j]) : 'sd0) + $signed(saw_a(rdata_trg[i]+j));
           $write (":%6d>%6d ", $signed(saw_a(rdata_trg[i]+j)), $signed(rdata_ref[j]));
         end
         $write ("\n");
       end
       // readout shift for reference
       for (int unsigned j=0; j<blk_size; j++) begin
         rdata_ref[j] = $signed(rdata_ref[j]) >>> shift;
       end
       // check
       $display ("trigger positions: %p", rdata_trg);
       $display ("data reference: %p", rdata_ref);
       $display ("data read     : %p", rdata_blk);
       if (rdata_ref == rdata_blk) $display ("SUCESS");
       else                        $display ("FAILURE");
     end
   join

//   repeat(800) @(posedge sys_clk);
//   bus.bus_write(32'h00,32'h1     );  // start aquisition
//   repeat(100000) @(posedge sys_clk);
//   bus.bus_write(32'h04,32'h5     );  // do trigger
//
//   repeat(20000) @(posedge adc_clk);
//   repeat(1) @(posedge sys_clk);
//   bus.bus_read(32'h10000, rdata);  // read value from memory
//   bus.bus_read(32'h10004, rdata);  // read value from memory
//   bus.bus_read(32'h20000, rdata);  // read value from memory
//   bus.bus_read(32'h20004, rdata);  // read value from memory

   repeat(100) @(posedge sys_clk);
   $finish ();
end

// trigger log
always_ff @ (posedge adc_clk)
if (scope.acu_ctl_run & scope.adc_trig & ~(|scope.acu_len_cnt)) begin
  rdata_trg.push_back(adc_cyc);
end

////////////////////////////////////////////////////////////////////////////////
// clock & reset
////////////////////////////////////////////////////////////////////////////////

sys_bus_model bus (
  // system signals
  .sys_clk_i      (sys_clk  ),
  .sys_rstn_i     (sys_rstn ),
  // bus protocol signals
  .sys_addr_o     (sys_addr ),
  .sys_wdata_o    (sys_wdata),
  .sys_sel_o      (sys_sel  ),
  .sys_wen_o      (sys_wen  ),
  .sys_ren_o      (sys_ren  ),
  .sys_rdata_i    (sys_rdata),
  .sys_err_i      (sys_err  ),
  .sys_ack_i      (sys_ack  ) 
);

red_pitaya_scope #(
  .RSZ (RSZ)
) scope (
  // ADC
  .adc_clk_i      (adc_clk  ),  // clock
  .adc_rstn_i     (adc_rstn ),  // reset - active low
  .adc_a_i        (adc_a    ),  // CH 1
  .adc_b_i        (adc_b    ),  // CH 2
  // trigger sources
  .trig_ext_i     (trig_ext ),  // external trigger
  .trig_asg_i     (trig_ext ),  // ASG trigger
   // System bus
  .sys_clk_i      (sys_clk  ),  // clock
  .sys_rstn_i     (sys_rstn ),  // reset - active low
  .sys_addr_i     (sys_addr ),  // address
  .sys_wdata_i    (sys_wdata),  // write data
  .sys_sel_i      (sys_sel  ),  // write byte select
  .sys_wen_i      (sys_wen  ),  // write enable
  .sys_ren_i      (sys_ren  ),  // read enable
  .sys_rdata_o    (sys_rdata),  // read data
  .sys_err_o      (sys_err  ),  // error indicator
  .sys_ack_o      (sys_ack  )   // acknowledge signal
);

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("red_pitaya_scope_tb.vcd");
  $dumpvars(0, red_pitaya_scope_tb);
end

endmodule: red_pitaya_scope_tb
