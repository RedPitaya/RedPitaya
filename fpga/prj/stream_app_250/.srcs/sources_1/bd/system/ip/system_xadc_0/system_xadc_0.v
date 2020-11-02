
// file: system_xadc_0.v
// (c) Copyright 2009 - 2013 Xilinx, Inc. All rights reserved.
// 
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
// 
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
// 
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
// 
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.
`timescale 1ns / 1 ps

(* CORE_GENERATION_INFO = "system_xadc_0,xadc_wiz_v3_3_8,{component_name=system_xadc_0,enable_axi=true,enable_axi4stream=false,dclk_frequency=100,enable_busy=true,enable_convst=false,enable_convstclk=false,enable_dclk=true,enable_drp=false,enable_eoc=true,enable_eos=true,enable_vbram_alaram=false,enable_vccddro_alaram=true,enable_Vccint_Alaram=true,enable_Vccaux_alaram=true,enable_vccpaux_alaram=true,enable_vccpint_alaram=true,ot_alaram=true,user_temp_alaram=true,timing_mode=continuous,channel_averaging=None,sequencer_mode=on,startup_channel_selection=independent_adc}" *)


module system_xadc_0
   (
    input s_axi_aclk,
    input s_axi_aresetn,
    input [10 : 0] s_axi_awaddr,
    input s_axi_awvalid,
    output s_axi_awready,
    input [31 : 0] s_axi_wdata,
    input [3 : 0] s_axi_wstrb,
    input s_axi_wvalid,
    output s_axi_wready,
    output [1 : 0] s_axi_bresp,
    output s_axi_bvalid,
    input s_axi_bready,
    input [10 : 0] s_axi_araddr,
    input s_axi_arvalid,
    output s_axi_arready,
    output [31 : 0] s_axi_rdata,
    output [1 : 0] s_axi_rresp,
    output s_axi_rvalid,
    input s_axi_rready,
    output ip2intc_irpt,
    input vauxp0,                                              
    input vauxn0,                                              
    input vauxp1,                                              
    input vauxn1,                                              
    input vauxp8,                                              
    input vauxn8,                                              
    input vauxp9,                                              
    input vauxn9,                                              
    output [4:0] channel_out,
    output busy_out,        
    output eoc_out, 
    output eos_out,
    output ot_out, 
    output vccddro_alarm_out,
    output vccpaux_alarm_out,
    output vccpint_alarm_out,
    output vccaux_alarm_out,
    output vccint_alarm_out,
    output user_temp_alarm_out,
    output alarm_out ,                                          
    input vp_in,                                               
    input vn_in
);

          wire [7:0]  alm_int;
          assign alarm_out = alm_int[7];
       assign vccddro_alarm_out = alm_int[6];
       assign vccpaux_alarm_out = alm_int[5];
       assign vccpint_alarm_out = alm_int[4];
          assign vccaux_alarm_out = alm_int[2];
          assign vccint_alarm_out = alm_int[1];
          assign user_temp_alarm_out = alm_int[0];

    system_xadc_0_axi_xadc 
   #("system_xadc_0_axi_xadc","virtex7", 11, 32, 1,"design.txt")
    inst 
    (
    .s_axi_aclk      (s_axi_aclk),                    
    .s_axi_aresetn   (s_axi_aresetn),                    
    .s_axi_awaddr    (s_axi_awaddr),                    
    .s_axi_awvalid   (s_axi_awvalid),                    
    .s_axi_awready   (s_axi_awready),                    
    .s_axi_wdata     (s_axi_wdata),                    
    .s_axi_wstrb     (s_axi_wstrb),                    
    .s_axi_wvalid    (s_axi_wvalid),                    
    .s_axi_wready    (s_axi_wready),                    
    .s_axi_bresp     (s_axi_bresp),                    
    .s_axi_bvalid    (s_axi_bvalid),                    
    .s_axi_bready    (s_axi_bready),                    
    .s_axi_araddr    (s_axi_araddr),                    
    .s_axi_arvalid   (s_axi_arvalid),                    
    .s_axi_arready   (s_axi_arready),                    
    .s_axi_rdata     (s_axi_rdata),                    
    .s_axi_rresp     (s_axi_rresp),                    
    .s_axi_rvalid    (s_axi_rvalid),                    
    .s_axi_rready    (s_axi_rready),                    
    .ip2intc_irpt    (ip2intc_irpt),  
    .vauxp0 (vauxp0),
    .vauxn0 (vauxn0),
    .vauxp1 (vauxp1),
    .vauxn1 (vauxn1),
    .vauxp8 (vauxp8),
    .vauxn8 (vauxn8),
    .vauxp9 (vauxp9),
    .vauxn9 (vauxn9),
    .channel_out(channel_out),
    .busy_out(busy_out), 
    .eoc_out(eoc_out), 
    .eos_out(eos_out),
    .ot_out(ot_out),
    .alarm_out  (alm_int),
    .vp_in (vp_in),
    .vn_in (vn_in)

          );


endmodule
