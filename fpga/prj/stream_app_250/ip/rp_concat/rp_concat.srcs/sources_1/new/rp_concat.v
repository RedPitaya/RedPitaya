`timescale 1ns / 1ps

module rp_concat
  #(parameter EVENT_SRC_NUM = 5,
    parameter TRIG_SRC_NUM  = 5)(  
  input  wire [3:0]               gen1_event_ip,
  input  wire                     gen1_trig_ip,
  //
  input  wire [3:0]               gen2_event_ip,
  input  wire                     gen2_trig_ip,  
  //       
  input  wire [3:0]               osc1_event_ip,
  input  wire                     osc1_trig_ip,
  //
  input  wire [3:0]               osc2_event_ip,
  input  wire                     osc2_trig_ip,
  //
  input  wire [3:0]               la_event_ip,
  input  wire                     la_trig_ip,
  //
  output wire [EVENT_SRC_NUM-1:0] event_trig,
  output wire [EVENT_SRC_NUM-1:0] event_stop,
  output wire [EVENT_SRC_NUM-1:0] event_start,
  output wire [EVENT_SRC_NUM-1:0] event_reset,
  output wire [EVENT_SRC_NUM-1:0] trig     
);

assign event_trig   = {la_event_ip[0], osc2_event_ip[0], osc1_event_ip[0], gen2_event_ip[0], gen1_event_ip[0]};
assign event_stop   = {la_event_ip[1], osc2_event_ip[1], osc1_event_ip[1], gen2_event_ip[1], gen1_event_ip[1]};
assign event_start  = {la_event_ip[2], osc2_event_ip[2], osc1_event_ip[2], gen2_event_ip[2], gen1_event_ip[2]};
assign event_reset  = {la_event_ip[3], osc2_event_ip[3], osc1_event_ip[3], gen2_event_ip[3], gen1_event_ip[3]};
assign trig         = {la_trig_ip, osc2_trig_ip, osc1_trig_ip, gen2_trig_ip, gen1_trig_ip};

endmodule