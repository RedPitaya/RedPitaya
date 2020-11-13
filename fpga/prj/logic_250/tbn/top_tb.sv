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
// signal generation
////////////////////////////////////////////////////////////////////////////////

// system signals
logic clk ;
logic               clkn;

logic rstn;

logic trig;

initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

assign clkn = ~clk;
// clocking 

default clocking cb @ (posedge clk);
  input  rstn;
endclocking: cb

// DAC reset
initial begin
  rstn <= 1'b0;
  ##4;
  rstn <= 1'b1;
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

reg [13:0]        cnter;
reg [ 1:0] [6:0] posnum;
reg [ 1:0] [6:0] negnum;

always @(clk) begin

    if (rstn==0)
        cnter <= 14'b0;
    else if (cnter==14'h1FFF && clk==1)
        cnter <= 14'b0;
    else if (clk == 1)
        cnter <= cnter + 14'd16; 

    posnum[1] <= cnter[13:7];
    posnum[0] <= cnter[13:7];
    negnum[1] <= ~cnter[13:7];
    negnum[0] <= ~cnter[13:7];
end

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  ##10000;
  $finish();
end

initial begin
  ##100;
  test_la (32'h40300000);
  //test_la_automatic (32'h40300000);
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
  int r;
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
     .RDelay (0),   .r (r)
  );
  dat = r;
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


task test_lg (
  int unsigned regset,
  int unsigned buffer
);
  logic signed [ 32-1: 0] rdata_blk [];
  ##10;

//  // configure amplitude and DC offset
//  axi_write(regset+'h38, 1 << (DWM-2));  // amplitude
//  axi_write(regset+'h3c, 0);             // DC offset

  // write table
  for (int i=0; i<buf_len; i++) begin
    axi_write(buffer + (i*4), i);  // write table
  end
//  // read table
//  rdata_blk = new [80];
//  for (int i=0; i<buf_len; i++) begin
//    axi_read(buffer + (i*4), rdata_blk [i]);  // read table
//  end

  // configure LG output enable
  axi_write(regset+'h38, '1);  // output ebable
  axi_write(regset+'h3c, '0);  // open drain
//axi_write(regset+'h3c, 2);  // open drain

  // configure frequency and phase
  axi_write(regset+'h10,  buf_len                    * 2**CWF - 1);  // table size
  axi_write(regset+'h14, (buf_len * (phase/360.0)  ) * 2**CWF    );  // offset
//axi_write(regset+'h18, (buf_len * (freq*TP/10**6)) * 2**CWF - 1);  // step
  axi_write(regset+'h18, 1                           * 2**CWF - 1);  // step
  // configure burst mode
  axi_write(regset+'h20, 2'b00);  // burst disable
  // enable SW trigger
  axi_write(regset+'h04, 'b100);
  // start
  axi_write(regset+'h00, 2'b10);
  ##22;
  // stop (reset)
  axi_write(regset+'h00, 2'b01);
  ##20;

  // burst mode
  axi_write(regset+'h24, buf_len - 1);  // burst data length
  axi_write(regset+'h28, buf_len - 1);  // burst idle length
  axi_write(regset+'h2c, 100);  // repetitions
  axi_write(regset+'h20, 'b11);  // enable burst mode and infinite repetitions
  // start
  axi_write(regset+'h00, 2'b10);
  ##100;
  // stop (reset)
//axi_write(regset+'h00, 2'b01);
//##20;
endtask: test_lg


task test_la (
  int unsigned regset
);
  ##10;

  // configure trigger
  axi_write(regset+'h40, 16'h0000);  // cfg_cmp_msk
  axi_write(regset+'h44, 16'h0000);  // cfg_cmp_val
  axi_write(regset+'h48, 16'h0001);  // cfg_edg_pos
  axi_write(regset+'h4c, 16'h0000);  // cfg_edg_neg

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
logic                  adc_cdcs;    // ADC clock duty cycle stabilizer
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
wire          [ 9-1:0] exp_p_io;
wire          [ 9-1:0] exp_n_io;
// LED
wire          [ 8-1:0] led;

glbl glbl();

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
  .pll_ref_i      (clk),
  .dac_dco_i      (clk),
  .vinp_i         (5'b0),
  .vinn_i         (5'b0),
  // Red Pitaya periphery
  
  // ADC
  .adc_clk_i({clk,clkn}),
  .adc_dat_p_i(posnum),
  .adc_dat_n_i(negnum),
  // Expansion connector
  .exp_9_io       (),
  .exp_p_io       (exp_p_io),
  .exp_n_io       (exp_n_io),
  // LED
  .led_o          (led)
);
////////////////////////////////////////////////////////////////////////////////
// simulated inputs
////////////////////////////////////////////////////////////////////////////////

localparam int unsigned SIZ_REF = 64;

bit [16-1:0] dat_ref [SIZ_REF];

initial begin
  logic signed [16-1:0] dat;
  for (int unsigned i=0; i<SIZ_REF; i++) begin
      dat = -SIZ_REF/2+i;
      dat_ref[i] = {dat[16-1], ~dat[16-2:0]};
  end
end

// ADC
assign adc_dat[0] = dat_ref[cyc % SIZ_REF];
assign adc_dat[1] = dat_ref[cyc % SIZ_REF];
assign adc_clk[1] =  clk;
assign adc_clk[0] = ~clk;
// adc_clk_o
// adc_cdcs

// XADC
assign vinp = '0;
assign vinn = '0;

// Expansion connector
assign exp_p_io = 9'h0;
assign exp_n_io = 9'h0;

// LED

////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("top_tb.vcd");
  $dumpvars(0, top_tb);
end

endmodule: top_tb
