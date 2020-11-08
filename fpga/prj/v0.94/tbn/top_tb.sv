////////////////////////////////////////////////////////////////////////////////
// Module: Red Pitaya top FPGA module
// Author: Iztok Jeras
// (c) Red Pitaya  (redpitaya.com)
////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ps

module top_tb #(
  // time period
  realtime  TP = 8.0ns,  // 125MHz
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

logic                  intr;

logic               clk,clk1 ;
logic               clkn;
wire               rstn_out;
logic               rstn;
wire clkout_125;
wire clkout;
//glbl glbl();

localparam OSC_DW = 64;
localparam REG_DW = 32;
localparam OSC_AW = 32;
localparam REG_AW = 12;
localparam IW = 12;
localparam LW = 8;

localparam GEN1_EVENT = 0;
localparam GEN2_EVENT = 1;
localparam OSC1_EVENT = 2;
localparam OSC2_EVENT = 3;
localparam LA_EVENT = 4;



////////////////////////////////////////////////////////////////////////////////
// Clock and reset generation
////////////////////////////////////////////////////////////////////////////////
logic clk_start;

assign clkn = ~clk;
// clock
/*initial begin
  clk_start = 1'b0;
  clk = 1'b0;
      clk1 = 1'b0;


  #(TP) clk_start = 1'b1;
  #(TP) clk_start = 1'b0;
  #(4*TP) clk_start = 1'b1;

end

always begin
    #(TP/2) clk1 = ~clk1;
    clk = clk1 && clk_start;
    
end*/

initial        clk = 1'b0;
always #(TP/2) clk = ~clk;

initial        pll_ref = 1'b0;
always #(RP/2) pll_ref = ~pll_ref;



// default clocking 
default clocking cb @ (posedge clk);
  input  rstn;
  input  exp_p_od, exp_p_oe;
  input  exp_n_od, exp_n_oe;
endclocking: cb

// reset
initial begin
        rstn = 1'b0;
  ##4;  rstn = 1'b1;
end

// clock cycle counter
int unsigned cyc=0;
always_ff @ (posedge clk)
cyc <= cyc+1;

/*always begin
  red_pitaya_top.ps.fclk_rstn <= {rstn,rstn,rstn,rstn};
end
*/

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
  ##500;

   //top_tc.test_hk                 (0<<20, 32'h55);
   //top_tc.test_sata               (5<<20, 32'h55);
   //top_tc.test_osc                (32'h40100000, OSC1_EVENT);

     top_tc.test_asg                (32'h40200000, 32'h0, 2);


  ##1600000000;
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

bit [14-1:0] dat_ref [2*15];

initial begin
  for (int unsigned i=0; i<31; i++) begin
    dat_ref [i] = {i, 2'b0};
    dat_ref [16-1-i] = {1'b1, 15'(1<<i)};
    dat_ref [16  +i] = {1'b0, 15'(1<<i)};
  end
end

// ADC
logic [2-1:0] [14-1:0] adc_dr ;
assign adc_dr[0] =  dat_ref[cyc % $size(dat_ref)];
assign adc_dr[1] = ~dat_ref[cyc % $size(dat_ref)];

always @(clk) begin
  if (clk==1) begin
    #(0.1);
    adc_dat[0] <= {adc_dr[0][12], adc_dr[0][10], adc_dr[0][8], adc_dr[0][6], adc_dr[0][4], adc_dr[0][2]};
    adc_dat[1] <= {adc_dr[1][12], adc_dr[1][10], adc_dr[1][8], adc_dr[1][6], adc_dr[1][4], adc_dr[1][2]};
  end else begin
    #(0.1);
    adc_dat[0] <= {adc_dr[0][13], adc_dr[0][11], adc_dr[0][9], adc_dr[0][7], adc_dr[0][5], adc_dr[0][3]};
    adc_dat[1] <= {adc_dr[1][13], adc_dr[1][11], adc_dr[1][9], adc_dr[1][7], adc_dr[1][5], adc_dr[1][3]};
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



/*
wire [11:0] wdat1; 
wire [11:0] wdat2;
wire [11:0] wdat3;
wire [11:0] wdat4;


assign wdat1 = red_pitaya_top.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc1_wdata[11:0];
assign wdat2 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc1_wdata[27:16];
assign wdat3 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc1_wdata[43:32];
assign wdat4 = red_pitaya_top_sim.system_wrapper_i.system_i.rp_oscilloscope.m_axi_osc1_wdata[59:48];
*/
reg [12:0] cnter;
always @(clk) begin

    if (rstn==0)
        cnter <= 13'b0;
    else if (cnter==13'hFFF && clk==1)
        cnter <= 13'b0;
    else if (clk == 1)
        cnter <= cnter + 13'b1; 

end







////////////////////////////////////////////////////////////////////////////////
// module instances
////////////////////////////////////////////////////////////////////////////////

// module under test
/*
  // ADC
  input  logic [MNA-1:0] [16-1:0] adc_dat_i,  // ADC data
  input  logic           [ 2-1:0] adc_clk_i,  // ADC clock {p,n}
 // output logic           [ 2-1:0] adc_clk_o,  // optional ADC clock source (unused)
 // output logic                    adc_cdcs_o, // ADC clock duty cycle stabilizer
  // DAC
 // output logic [14-1:0] dac_dat_o  ,  // DAC combined data
 // output logic          dac_wrt_o  ,  // DAC write
 // output logic          dac_sel_o  ,  // DAC channel select
  //output logic          dac_clk_o  ,  // DAC clock
  //output logic          dac_rst_o  ,  // DAC reset
  // PWM DAC
  //output logic [ 4-1:0] dac_pwm_o  ,  // 1-bit PWM DAC
  // XADC
  //input  logic [ 5-1:0] vinp_i     ,  // voltages p
  //input  logic [ 5-1:0] vinn_i     ,  // voltages n
  // Expansion connector
  //inout  logic [ 8-1:0] exp_p_io   ,
  //inout  logic [ 8-1:0] exp_n_io   ,
  // SATA connector
  //output logic [ 2-1:0] daisy_p_o  ,  // line 1 is clock capable
  //output logic [ 2-1:0] daisy_n_o  ,
  //input  logic [ 2-1:0] daisy_p_i  ,  // line 1 is clock capable
  //input  logic [ 2-1:0] daisy_n_i  ,
  // LED
  inout  logic [ 8-1:0] led_o
*/

 red_pitaya_top_sim red_pitaya_top
       (.DDR_addr(),
        .DDR_ba(),
        .DDR_cas_n(),
        .DDR_ck_n(),
        .DDR_ck_p(),
        .DDR_cke(),
        .DDR_cs_n(),
        .DDR_dm(),
        .DDR_dq(),
        .DDR_dqs_n(),
        .DDR_dqs_p(),
        .DDR_odt(),
        .DDR_ras_n(),
        .DDR_reset_n(),
        .DDR_we_n(),
        .FIXED_IO_ddr_vrn(),
        .FIXED_IO_ddr_vrp(),
        .FIXED_IO_mio(),
        .FIXED_IO_ps_clk(),
        .FIXED_IO_ps_porb(),
        .FIXED_IO_ps_srstb(),

        .adc_dat_i({{3'b0,cnter},{3'b0,~cnter}}),  // ADC data
        .adc_clk_i({clk,clkn}),  // ADC clock {p,n}
        .adc_clk_o(),  // optional ADC clock source (unused)
        .adc_cdcs_o(), // ADC clock duty cycle stabilizer
        // DAC
        .dac_dat_o  (),  // DAC combined data
        .dac_wrt_o  (),  // DAC write
        .dac_sel_o  (),  // DAC channel select
        .dac_clk_o  (),  // DAC clock
        .dac_rst_o  (),  // DAC reset
        // PWM DAC
        .dac_pwm_o  (),  // 1-bit PWM DAC
        // XADC
        .vinp_i     (),  // voltages p
        .vinn_i     (),  // voltages n
        // Expansion connector
        .exp_p_io   (),
        .exp_n_io   (),
        // SATA connector
        .daisy_p_o  (),  // line 1 is clock capable
        .daisy_n_o  (),
        .daisy_p_i  (),  // line 1 is clock capable
        .daisy_n_i  (),
        // LED
        .led_o(),
        .rstn(rstn));


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
