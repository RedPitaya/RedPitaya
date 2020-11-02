// (c) Copyright 1995-2020 Xilinx, Inc. All rights reserved.
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
// 
// DO NOT MODIFY THIS FILE.


// IP VLNV: redpitaya.com:user:rp_concat:1.0
// IP Revision: 3

`timescale 1ns/1ps

(* IP_DEFINITION_SOURCE = "package_project" *)
(* DowngradeIPIdentifiedWarnings = "yes" *)
module system_rp_concat_0 (
  gen1_event_ip,
  gen1_trig_ip,
  gen2_event_ip,
  gen2_trig_ip,
  osc1_event_ip,
  osc1_trig_ip,
  osc2_event_ip,
  osc2_trig_ip,
  la_event_ip,
  la_trig_ip,
  event_trig,
  event_stop,
  event_start,
  event_reset,
  trig
);

input wire [3 : 0] gen1_event_ip;
input wire gen1_trig_ip;
input wire [3 : 0] gen2_event_ip;
input wire gen2_trig_ip;
input wire [3 : 0] osc1_event_ip;
input wire osc1_trig_ip;
input wire [3 : 0] osc2_event_ip;
input wire osc2_trig_ip;
input wire [3 : 0] la_event_ip;
input wire la_trig_ip;
output wire [4 : 0] event_trig;
output wire [4 : 0] event_stop;
output wire [4 : 0] event_start;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME event_reset, POLARITY ACTIVE_LOW, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:reset:1.0 event_reset RST" *)
output wire [4 : 0] event_reset;
output wire [4 : 0] trig;

  rp_concat #(
    .EVENT_SRC_NUM(5),
    .TRIG_SRC_NUM(5)
  ) inst (
    .gen1_event_ip(gen1_event_ip),
    .gen1_trig_ip(gen1_trig_ip),
    .gen2_event_ip(gen2_event_ip),
    .gen2_trig_ip(gen2_trig_ip),
    .osc1_event_ip(osc1_event_ip),
    .osc1_trig_ip(osc1_trig_ip),
    .osc2_event_ip(osc2_event_ip),
    .osc2_trig_ip(osc2_trig_ip),
    .la_event_ip(la_event_ip),
    .la_trig_ip(la_trig_ip),
    .event_trig(event_trig),
    .event_stop(event_stop),
    .event_start(event_start),
    .event_reset(event_reset),
    .trig(trig)
  );
endmodule
