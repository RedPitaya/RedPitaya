////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya top FPGA module
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module top_tb #(
  // time period
  realtime  TP = 4.0ns,  // 250MHz
  realtime  RP = 100.1ns,  // ~10MHz
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
logic [2-1:0] [ 7-1:0] adc_dat;
logic         [ 2-1:0] adc_dco;
// DAC
logic [2-1:0] [14-1:0] dac_dat;     // DAC combined data
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
wire                   exp_9_io;
// Expansion output data/enable
logic         [ 9-1:0] exp_p_od, exp_p_oe;
logic         [ 9-1:0] exp_n_od, exp_n_oe;
logic                  exp_9_od, exp_9_oe;
// SATA
logic         [ 4-1:0] daisy_p;
logic         [ 4-1:0] daisy_n;

// LED
wire          [ 8-1:0] led;

logic         [ 2-1:0] temp_prot;
logic                  pll_lo;
logic                  pll_hi;
logic                  pll_ref;
logic                  trig;

glbl glbl();

////////////////////////////////////////////////////////////////////////////////
// Clock and reset generation
////////////////////////////////////////////////////////////////////////////////

logic               clk ;
logic               rstn;

// clock
initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

initial        pll_ref = 1'b0;
always #(RP/2) pll_ref = ~pll_ref;

// reset
initial begin
        rstn = 1'b0;
  ##4;  rstn = 1'b1;
end

// default clocking 
default clocking cb @ (posedge clk);
  input  rstn;
  input  exp_p_od, exp_p_oe;
  input  exp_n_od, exp_n_oe;
endclocking: cb



// clock cycle counter
int unsigned cyc=0;
always_ff @ (posedge clk)
cyc <= cyc+1;






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

//initial begin
//  ##6000;
//  $display("ERROR: timeout!");
//  $finish();
//end

initial begin
  ##100;

   top_tc.test_hk                 (0<<20, 32'h55);
   top_tc.test_sata               (5<<20, 32'h55);
   top_tc.test_osc                (1<<20, 32'h40090000, 2);

//   top_tc.test_asg                (2<<20, 32'h40090000, 2);


  ##16000000;
  $finish();
end



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

always begin
  trig <= 1'b0;
  ##100000;
  trig <= 1'b1;
  ##1200;
  trig <= 1'b0;
end


always begin
  temp_prot <= 2'b00;
  ##50000;
  temp_prot <= 2'b10;
  ##1000;
  temp_prot <= 2'b00;
end


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
  for (int unsigned i=0; i<31; i++) begin
    dat_ref [i] = {i, 2'b0};
//    dat_ref [16-1-i] = {1'b1, 15'(1<<i)};
//    dat_ref [16  +i] = {1'b0, 15'(1<<i)};
  end
end

// ADC
logic [2-1:0] [16-1:0] adc_dr ;
assign adc_dr[0] =  dat_ref[cyc % $size(dat_ref)];
assign adc_dr[1] = ~dat_ref[cyc % $size(dat_ref)];

always @(clk) begin
  if (clk==1) begin
    #(0.1);
    adc_dat[0] <= {adc_dr[0][14], adc_dr[0][12], adc_dr[0][10], adc_dr[0][8], adc_dr[0][6], adc_dr[0][4], adc_dr[0][2]};
    adc_dat[1] <= {adc_dr[1][14], adc_dr[1][12], adc_dr[1][10], adc_dr[1][8], adc_dr[1][6], adc_dr[1][4], adc_dr[1][2]};
  end else begin
    #(0.1);
    adc_dat[0] <= {adc_dr[0][15], adc_dr[0][13], adc_dr[0][11], adc_dr[0][9], adc_dr[0][7], adc_dr[0][5], adc_dr[0][3]};
    adc_dat[1] <= {adc_dr[1][15], adc_dr[1][13], adc_dr[1][11], adc_dr[1][9], adc_dr[1][7], adc_dr[1][5], adc_dr[1][3]};
  end
end

always @(clk) begin
  if (clk==1) begin
    #(0.7);
    adc_dco[1] <= 1;
    adc_dco[0] <= 0;
  end else begin
    #(0.7);
    adc_dco[1] <= 0;
    adc_dco[0] <= 1;
  end
end

// XADC
assign vinp = '0;
assign vinn = '0;

// Expansion connector
//assign exp_p_io = 8'h0;
//assign exp_n_io = 8'h0;

// LED


assign #0.2 daisy_p[3] = daisy_p[1] ;
assign #0.2 daisy_n[3] = daisy_n[1] ;
assign #0.2 daisy_p[2] = daisy_p[0] ;
assign #0.2 daisy_n[2] = daisy_n[0] ;






























////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

// module under test
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
  .temp_prot_i    (temp_prot),
  .pll_lo_o       (pll_lo),
  .pll_hi_o       (pll_hi),
  .pll_ref_i      (pll_ref),
  .trig_i         (trig),
  // ADC
  .adc_dat_p_i      (adc_dat),
  .adc_dat_n_i      (~adc_dat),
  .adc_clk_i        (adc_dco),
  .adc_spi_csb      ( ),
  .adc_spi_sdio     ( ),
  .adc_spi_clk      ( ),
  .adc_sync_o       ( ),
  // DAC
  .dac_dat_o      (dac_dat),
  .dac_dco_i      (dac_wrt),
  .dac_reset_o    (dac_rst),
  .dac_spi_csb      ( ),
  .dac_spi_sdio     ( ),
  .dac_spi_clk      ( ),
  // PDM DAC
  .dac_pwm_o      (dac_pwm),
  // XADC
  .vinp_i         (vinp),
  .vinn_i         (vinn),
  // Expansion connector
  .exp_p_io       (exp_p_io),
  .exp_n_io       (exp_n_io),
  .exp_9_io       (exp_9_io),
  // SATA connector
  .daisy_p_o       ( daisy_p[1:0]  ),  //!< TX data and clock [1]-clock, [0]-data
  .daisy_n_o       ( daisy_n[1:0]  ),  //!< TX data and clock [1]-clock, [0]-data
  .daisy_p_i       ( daisy_p[3:2]  ),  //!< RX data and clock [1]-clock, [0]-data
  .daisy_n_i       ( daisy_n[3:2]  ),  //!< RX data and clock [1]-clock, [0]-data
  // LED
  .led_o          (led)
);

bufif1 bufif_exp_p_io [9-1:0] (exp_p_io, exp_p_od, exp_p_oe);
bufif1 bufif_exp_n_io [9-1:0] (exp_n_io, exp_n_od, exp_n_oe);
bufif1 bufif_exp_9_io         (exp_9_io, exp_9_od, exp_9_oe);
// testcases
top_tc top_tc();


////////////////////////////////////////////////////////////////////////////////
// waveforms
////////////////////////////////////////////////////////////////////////////////

initial begin
  $dumpfile("top_tb.vcd");
  $dumpvars(0, top_tb);
end



endmodule: top_tb
