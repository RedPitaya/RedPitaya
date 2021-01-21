#simulation preparation script
#change bd design for simulation
################################################################
# This is a generated script based on design: system
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

namespace eval _tcl {
proc get_script_folder {} {
   set script_path [file normalize [info script]]
   set script_folder [file dirname $script_path]
   return $script_folder
}
}
variable script_folder
set script_folder [_tcl::get_script_folder]

# CHANGE DESIGN NAME HERE
variable design_name
set design_name system



current_bd_design $design_name



##################################################################
# DESIGN PROCs
##################################################################



# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  variable script_folder
  variable design_name

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj
set_property verilog_define {SIMULATION} [get_filesets sim_1]

delete_bd_objs [get_bd_intf_nets processing_system7_0_M_AXI_GP0]
delete_bd_objs [get_bd_intf_nets axi_interconnect_0_M00_AXI]
delete_bd_objs [get_bd_nets processing_system7_0_FCLK_RESET2_N]
connect_bd_net [get_bd_ports frstn_2] [get_bd_pins processing_system7_0/FCLK_RESET2_N]

create_bd_port -dir O -type rst rstn_out
connect_bd_net [get_bd_ports rstn_out] [get_bd_pins rst_gen/peripheral_aresetn]

delete_bd_objs [get_bd_nets processing_system7_0_FCLK_RESET0_N]
create_bd_port -dir I -type rst rst_in
set_property CONFIG.POLARITY ACTIVE_HIGH [get_bd_ports rst_in]
connect_bd_net [get_bd_ports rst_in] [get_bd_pins rst_gen/ext_reset_in]

create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:6.0 clk_wiz_0
connect_bd_net [get_bd_ports clkin_125] [get_bd_pins clk_wiz_0/clk_in1]
set_property -dict [list CONFIG.PRIM_IN_FREQ.VALUE_SRC USER] [get_bd_cells clk_wiz_0]
set_property -dict [list CONFIG.PRIM_IN_FREQ {125.000} CONFIG.CLKOUT1_REQUESTED_OUT_FREQ {62.500} CONFIG.CLKIN1_JITTER_PS {80.0} CONFIG.MMCM_CLKFBOUT_MULT_F {8.000} CONFIG.MMCM_CLKIN1_PERIOD {8.000} CONFIG.MMCM_CLKOUT0_DIVIDE_F {16.000} CONFIG.CLKOUT1_JITTER {137.150} CONFIG.CLKOUT1_PHASE_ERROR {96.948}] [get_bd_cells clk_wiz_0]
set_property -dict [list CONFIG.USE_LOCKED {false} CONFIG.USE_RESET {false}] [get_bd_cells clk_wiz_0]
set_property -dict [list CONFIG.CLKOUT2_USED {true} CONFIG.CLKOUT2_REQUESTED_OUT_FREQ {125.000} CONFIG.MMCM_CLKOUT1_DIVIDE {8} CONFIG.NUM_OUT_CLKS {2} CONFIG.CLKOUT2_JITTER {119.348} CONFIG.CLKOUT2_PHASE_ERROR {96.948}] [get_bd_cells clk_wiz_0]
create_bd_port -dir O -type clk clkout_625
connect_bd_net [get_bd_ports clkout_625] [get_bd_pins clk_wiz_0/clk_out1]



create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M_AXI_OSC
set_property -dict [list \
CONFIG.CLK_DOMAIN {/clk_wiz_0_clk_out1} \
CONFIG.HAS_REGION {0} \
CONFIG.NUM_WRITE_OUTSTANDING {8} \
CONFIG.NUM_READ_OUTSTANDING {8} \
CONFIG.FREQ_HZ {125000000} \
CONFIG.PROTOCOL {AXI3} \
CONFIG.DATA_WIDTH {64} \
] [get_bd_intf_ports M_AXI_OSC]
connect_bd_intf_net [get_bd_intf_ports M_AXI_OSC] -boundary_type upper [get_bd_intf_pins axi_interconnect_0/M00_AXI]

create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 S_AXI_REG
set_property -dict [list \
CONFIG.MAX_BURST_LENGTH {16} \
CONFIG.NUM_WRITE_OUTSTANDING {8} \
CONFIG.NUM_READ_OUTSTANDING {8} \
CONFIG.NUM_READ_THREADS {4} \
CONFIG.NUM_WRITE_THREADS {4} \
CONFIG.SUPPORTS_NARROW_BURST {0} \
CONFIG.ID_WIDTH {12} \
CONFIG.DATA_WIDTH {32} \
CONFIG.PROTOCOL {AXI3} \
CONFIG.HAS_REGION {0} \
CONFIG.CLK_DOMAIN {/clk_wiz_0_clk_out1} \
CONFIG.FREQ_HZ {62500000} \
] [get_bd_intf_ports S_AXI_REG]
connect_bd_intf_net [get_bd_intf_ports S_AXI_REG] -boundary_type upper [get_bd_intf_pins axi_reg/S00_AXI]

delete_bd_objs [get_bd_intf_nets axi_reg_M01_AXI] [get_bd_intf_ports m_axi_hk]
  set m_axi_hk [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 m_axi_hk ]
  set_property -dict [ list \
   CONFIG.ADDR_WIDTH {32} \
   CONFIG.DATA_WIDTH {32} \
   CONFIG.FREQ_HZ {125000000} \
   CONFIG.HAS_REGION {0} \
   CONFIG.PROTOCOL {AXI3} \
   CONFIG.CLK_DOMAIN {/clk_wiz_0_clk_out1} \
   ] $m_axi_hk
connect_bd_intf_net -intf_net axi_reg_M01_AXI [get_bd_intf_ports m_axi_hk] [get_bd_intf_pins axi_reg/M01_AXI]


delete_bd_objs [get_bd_addr_segs processing_system7_0/Data/SEG_m_axi_hk_Reg] [get_bd_addr_segs processing_system7_0/Data/SEG_rp_oscilloscope_reg0] [get_bd_addr_segs rp_oscilloscope/m_axi_osc1/SEG_processing_system7_0_HP0_DDR_LOWOCM] [get_bd_addr_segs rp_oscilloscope/m_axi_osc2/SEG_processing_system7_0_HP0_DDR_LOWOCM]
assign_bd_address [get_bd_addr_segs rp_oscilloscope/s_axi_reg/reg0]
set_property offset 0x40100000 [get_bd_addr_segs {S_AXI_REG/SEG_rp_oscilloscope_reg0}]
set_property range 1M [get_bd_addr_segs {S_AXI_REG/SEG_rp_oscilloscope_reg0}]
assign_bd_address [get_bd_addr_segs {rp_oscilloscope/m_axi_osc1/reg0 }]
assign_bd_address [get_bd_addr_segs {rp_oscilloscope/m_axi_osc2/reg0 }]
assign_bd_address [get_bd_addr_segs {processing_system7_0/Data/SEG_m_axi_hk_Reg}]

delete_bd_objs [get_bd_nets clkin_125_1]
connect_bd_net [get_bd_ports clkin_125] [get_bd_pins clk_wiz_0/clk_in1]
connect_bd_net [get_bd_pins rst_gen/slowest_sync_clk] [get_bd_pins clk_wiz_0/clk_out1]
connect_bd_net [get_bd_pins axi_reg/ACLK] [get_bd_pins clk_wiz_0/clk_out1]
connect_bd_net [get_bd_pins axi_reg/S00_ACLK] [get_bd_pins clk_wiz_0/clk_out1]
connect_bd_net [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK] [get_bd_pins clk_wiz_0/clk_out1]
connect_bd_net [get_bd_pins clk_wiz_0/clk_out2] [get_bd_pins axi_interconnect_0/ACLK]
connect_bd_net [get_bd_pins axi_interconnect_0/M00_ACLK] [get_bd_pins clk_wiz_0/clk_out2]
connect_bd_net [get_bd_pins clk_wiz_0/clk_out2] [get_bd_pins axi_interconnect_0/S00_ACLK]
connect_bd_net [get_bd_pins axi_interconnect_0/S01_ACLK] [get_bd_pins clk_wiz_0/clk_out2]
connect_bd_net [get_bd_pins axi_reg/M00_ACLK] [get_bd_pins clk_wiz_0/clk_out2]
connect_bd_net [get_bd_pins axi_reg/M01_ACLK] [get_bd_pins clk_wiz_0/clk_out2]
connect_bd_net [get_bd_pins processing_system7_0/S_AXI_HP0_ACLK] [get_bd_pins clk_wiz_0/clk_out2]
connect_bd_net [get_bd_pins rp_oscilloscope/s_axi_reg_aclk] [get_bd_pins clk_wiz_0/clk_out2]
connect_bd_net [get_bd_pins rp_oscilloscope/m_axi_osc1_aclk] [get_bd_pins clk_wiz_0/clk_out2]
connect_bd_net [get_bd_pins rp_oscilloscope/m_axi_osc2_aclk] [get_bd_pins clk_wiz_0/clk_out2]
connect_bd_net [get_bd_pins xadc/s_axi_aclk] [get_bd_pins clk_wiz_0/clk_out2]
create_bd_port -dir O -type clk clkout_125
connect_bd_net [get_bd_ports clkout_125] [get_bd_pins clk_wiz_0/clk_out2]
  
  current_bd_instance $oldCurInst
  validate_bd_design
  save_bd_design
generate_target all [get_files    system.bd]

}
# End of create_root_design()



##################################################################
# MAIN FLOW
##################################################################

create_root_design ""



update_compile_order -fileset sources_1
set_property SOURCE_SET sources_1 [get_filesets sim_1]
set path_sim tbn
add_files -fileset sim_1      $path_sim
add_files -fileset sim_1 -norecurse {../../tbn/axi_bus_model.sv}
set_property used_in_simulation false [get_files  project/redpitaya.srcs/sources_1/imports/fpga/prj/stream_app_250/rtl_250/red_pitaya_top_Z20.sv]
set_property top top_tb [get_filesets sim_1]
update_compile_order -fileset sim_1
#remove unnecessary files
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_scope.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/la.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/old_la_top.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/gen.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/osc.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/pid.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_pid.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/old_asg_top.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_dly.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/clb.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_asg.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_scope_Z20.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/lg.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/prj/stream_app_250/rtl_250/red_pitaya_top_Z20.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_lite_slave.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_demux.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/axi_master.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/clkdiv.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/ctrg.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/cts.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/debounce.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/divide.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/id.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/mgmt.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/muxctl.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/old_id.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/pdm.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/pwm.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_ams.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_id.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_pwm.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/rp_concat.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/scope_filter.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/str2mm.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/sys_reg_array_o.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/asg.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/divide.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/la_trg.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/old_acq.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/osc_trg.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/acq.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_cnt.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_pas.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/axi_wr_fifo.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/bin_and.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/lin_add.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/lin_mul.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/old_asg.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/pid_block.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_asg_ch.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/red_pitaya_dfilt1.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_pid_block.v] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/rle.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/scope_dec_avg.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/str_dec.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/asg_bst.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/asg_per.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_mux.sv] -no_script -reset -force -quiet
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_reg.sv] -no_script -reset -force -quiet
remove_files  {project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_scope.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/la.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/old_la_top.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/asg_bst.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/asg_per.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_mux.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_reg.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/gen.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/osc.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/pid.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_pid.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/old_asg_top.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_dly.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/clb.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_asg.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_scope_Z20.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/lg.sv project/redpitaya.srcs/sources_1/imports/fpga/prj/stream_app_250/rtl_250/red_pitaya_top_Z20.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_lite_slave.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_demux.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/axi_master.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/clkdiv.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/ctrg.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/cts.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/debounce.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/divide.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/id.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/mgmt.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/muxctl.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/old_id.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/pdm.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/pwm.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_ams.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_id.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_pwm.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/rp_concat.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/scope_filter.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/str2mm.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/sys_reg_array_o.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/asg.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/divide.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/la_trg.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/old_acq.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/osc_trg.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/acq.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_cnt.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/axi4_stream_pas.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/axi_wr_fifo.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/bin_and.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/lin_add.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/lin_mul.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/old_asg.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/pid_block.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_asg_ch.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/red_pitaya_dfilt1.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/classic/red_pitaya_pid_block.v project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/rle.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/scope_dec_avg.sv project/redpitaya.srcs/sources_1/imports/fpga/rtl_250/str_dec.sv}
validate_bd_design
save_bd_design
export_ip_user_files -of_objects  [get_files project/redpitaya.srcs/sources_1/imports/fpga/prj/stream_app_250/bd/system/hdl/system_wrapper.v] -no_script -reset -force -quiet
remove_files  project/redpitaya.srcs/sources_1/imports/fpga/prj/stream_app_250/bd/system/hdl/system_wrapper.v
make_wrapper -files [get_files project/redpitaya.srcs/sources_1/bd/system/system.bd] -top
add_files -norecurse project/redpitaya.srcs/sources_1/bd/system/hdl/system_wrapper.v
reset_target all [get_files  project/redpitaya.srcs/sources_1/bd/system/system.bd]
export_ip_user_files -of_objects  [get_files  project/redpitaya.srcs/sources_1/bd/system/system.bd] -sync -no_script -force -quiet
generate_target Simulation [get_files project/redpitaya.srcs/sources_1/bd/system/system.bd]
export_ip_user_files -of_objects [get_files project/redpitaya.srcs/sources_1/bd/system/system.bd] -no_script -sync -force -quiet
export_simulation -of_objects [get_files project/redpitaya.srcs/sources_1/bd/system/system.bd] -directory project/redpitaya.ip_user_files/sim_scripts -ip_user_files_dir project/redpitaya.ip_user_files -ipstatic_source_dir project/redpitaya.ip_user_files/ipstatic -lib_map_path [list {modelsim=project/redpitaya.cache/compile_simlib/modelsim} {questa=project/redpitaya.cache/compile_simlib/questa} {ies=project/redpitaya.cache/compile_simlib/ies} {xcelium=project/redpitaya.cache/compile_simlib/xcelium} {vcs=project/redpitaya.cache/compile_simlib/vcs} {riviera=project/redpitaya.cache/compile_simlib/riviera}] -use_ip_compiled_libs -force -quiet