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
// DAC signal generation
////////////////////////////////////////////////////////////////////////////////


logic               clk ;
logic               rstn;

logic [DAC_DW-1: 0] dac_a;
logic [DAC_DW-1: 0] dac_b;

logic               trig;

// DAC clock
initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

// DAC reset
initial begin
  rstn = 1'b0;
  repeat(4) @(posedge clk);
  rstn = 1'b1;
end

// ADC cycle counter
int unsigned dac_cyc=0;
always_ff @ (posedge clk)
dac_cyc <= dac_cyc+1;

always begin
  trig <= 1'b0 ;
  repeat(100000) @(posedge clk);
  trig <= 1'b1 ;
  repeat(1200) @(posedge clk);
  trig <= 1'b0 ;
end

////////////////////////////////////////////////////////////////////////////////
// test sequence
////////////////////////////////////////////////////////////////////////////////

initial begin
  repeat(10000) @(posedge clk);
  $finish();
end

initial begin
  repeat(100) @(posedge clk);
  axi_write (0,'h01234567);
  axi_write ((0 << 19) + 'h30, 'ha5);
  repeat(1000) @(posedge clk);
  test_id (32'h40000000);
  test_lg (32'h402c0000);
  test_la (32'h40300000);
  test_la_automatic (32'h40300000);
  repeat(16) @(posedge clk);
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

localparam int unsigned CWM = 14;
localparam int unsigned CWF = 16;
localparam ADR_BUF = 1 << (CWM+2);

//int buf_len = 2**CWM;
int buf_len = 'hff+1;
real freq  = 10_000; // 10kHz
real phase = 0; // DEG

task test_lg (
  int unsigned base
);
  logic signed [ 32-1: 0] rdata_blk [];
  repeat(10) @(posedge clk);

//  // configure amplitude and DC offset
//  axi_write(base+'h38, 1 << (DWM-2));  // amplitude
//  axi_write(base+'h3c, 0);             // DC offset

  // write table
  for (int i=0; i<buf_len; i++) begin
    axi_write(base+ADR_BUF + (i*4), i);  // write table
  end
//  // read table
//  rdata_blk = new [80];
//  for (int i=0; i<buf_len; i++) begin
//    axi_read(base+ADR_BUF + (i*4), rdata_blk [i]);  // read table
//  end

  // configure LG output enable
  axi_write(base+'h38, '1);  // output ebable
  axi_write(base+'h3c, '0);  // open drain
//axi_write(base+'h3c, 2);  // open drain

  // configure frequency and phase
  axi_write(base+'h10,  buf_len                    * 2**CWF - 1);  // table size
  axi_write(base+'h14, (buf_len * (phase/360.0)  ) * 2**CWF    );  // offset
//axi_write(base+'h18, (buf_len * (freq*TP/10**6)) * 2**CWF - 1);  // step
  axi_write(base+'h18, 1                           * 2**CWF - 1);  // step
  // configure burst mode
  axi_write(base+'h20, 2'b00);  // burst disable
  // enable SW trigger
  axi_write(base+'h04, 'b100);
  // start
  axi_write(base+'h00, 2'b10);
  repeat(22) @(posedge clk);
  // stop (reset)
  axi_write(base+'h00, 2'b01);
  repeat(20) @(posedge clk);

  // burst mode
  axi_write(base+'h24, buf_len - 1);  // burst data length
  axi_write(base+'h28, buf_len - 1);  // burst idle length
  axi_write(base+'h2c, 100);  // repetitions
  axi_write(base+'h20, 'b11);  // enable burst mode and infinite repetitions
  // start
  axi_write(base+'h00, 2'b10);
  repeat(100) @(posedge clk);
  // stop (reset)
//axi_write(base+'h00, 2'b01);
//repeat(20) @(posedge clk);
endtask: test_lg


task test_la (
  int unsigned base
);
  repeat(10) @(posedge clk);

  // configure trigger
  axi_write(base+'h40, 16'h0000);  // cfg_cmp_msk
  axi_write(base+'h44, 16'h0000);  // cfg_cmp_val
  axi_write(base+'h48, 16'h0001);  // cfg_edg_pos
  axi_write(base+'h4c, 16'h0000);  // cfg_edg_neg

  axi_write(base+'h10, 'd8 );  // cfg_pre
  axi_write(base+'h14, 'd16);  // cfg_pst
  // enable LA trigger source
  axi_write(base+'h08, 'b0010);
  // start acquire
  axi_write(base+'h00, 4'b0100);
  repeat(1000) @(posedge clk);
endtask: test_la


task test_la_automatic (
  int unsigned base
);
  repeat(10) @(posedge clk);

  // enable automatic mode
  axi_write(base+'h04, 'h2);  // cfg_aut <= 1
  // configure trigger
  axi_write(base+'h10, 'd0);  // cfg_pre
  axi_write(base+'h14, 'd4);  // cfg_pst
  // ignore triggers
  axi_write(base+'h08, 'b0000);
  // start acquire
  axi_write(base+'h00, 4'b0100);
  repeat(1000) @(posedge clk);
endtask: test_la_automatic


task test_id (
  int unsigned base
);
  int unsigned dat;
  // configure trigger
  axi_read(base+'h20, dat);
  axi_read(base+'h24, dat);
  axi_read(base+'h28, dat);
  axi_read(base+'h2c, dat);
  axi_read(base+'h30, dat);
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
logic [2-1:0] [16-1:2] adc_dat;
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
wire          [ 8-1:0] exp_p_io;
wire          [ 8-1:0] exp_n_io;
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

// ADC
assign adc_dat    = '0;
assign adc_clk[1] =  clk;
assign adc_clk[0] = ~clk;
// adc_clk_o
// adc_cdcs

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
