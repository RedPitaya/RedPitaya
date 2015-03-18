################################################################################
# Vivado tcl script for building RedPitaya FPGA in non project mode
#
# Usage:
# vivado -mode tcl -source red_pitaya_fpga.tcl
################################################################################

set part xc7z010clg400-1

create_project -in_memory -part $part

################################################################################
# define paths
################################################################################

set path_rtl ../code/rtl
set path_ip  ../code/ip
set path_xdc ../code/

set path_out out
set path_sdk sdk

file mkdir $path_out
file mkdir $path_sdk

################################################################################
# read files:
# 1. RTL design sources
# 2. IP database files
# 3. constraints
################################################################################

#read_verilog $path_rtl/

read_verilog $path_rtl/axi_master.v
read_verilog $path_rtl/axi_slave.v
read_verilog $path_rtl/axi_wr_fifo.v
read_verilog $path_rtl/bus_clk_bridge.v

read_verilog $path_rtl/red_pitaya_acum.sv
read_verilog $path_rtl/red_pitaya_ams.v
read_verilog $path_rtl/red_pitaya_analog.v
read_verilog $path_rtl/red_pitaya_asg_ch.v
read_verilog $path_rtl/red_pitaya_asg.v
read_verilog $path_rtl/red_pitaya_daisy_rx.v
read_verilog $path_rtl/red_pitaya_daisy_test.v
read_verilog $path_rtl/red_pitaya_daisy_tx.v
read_verilog $path_rtl/red_pitaya_daisy.v
read_verilog $path_rtl/red_pitaya_dfilt1.v
read_verilog $path_rtl/red_pitaya_hk.v
read_verilog $path_rtl/red_pitaya_pid_block.v
read_verilog $path_rtl/red_pitaya_pid.v
read_verilog $path_rtl/red_pitaya_ps.v
read_verilog $path_rtl/red_pitaya_scope.v
read_verilog $path_rtl/red_pitaya_test.v
read_verilog $path_rtl/red_pitaya_top.v

read_verilog $path_ip/system_wrapper.v
read_ip      $path_ip/system_processing_system7_0_0.xci
read_bd      $path_ip/system.bd

read_xdc     $path_xdc/red_pitaya.xdc

################################################################################
# run synthesis
# report utilization and timing estimates
# write checkpoint design
################################################################################

# export PS configuration
generate_target all [get_ips system_processing_system7_0_0]
#generate_target synthesis [get_ips system_processing_system7_0_0]
#generate_target misc      [get_ips system_processing_system7_0_0]

# generate SDK files
generate_target all [get_files $path_ip/system.bd]
#open_bd_design      $path_ip/system.bd
# TODO
#export_hardware     [get_files $path_ip/system.bd] -dir $path_sdk
#close_bd_design     system

synth_design -top red_pitaya_top

write_checkpoint      -force $path_out/post_synth
report_timing_summary -file  $path_out/post_synth_timing_summary.rpt
report_power          -file  $path_out/post_synth_power.rpt

################################################################################
# run placement and logic optimization
# report utilization and timing estimates
# write checkpoint design
################################################################################

opt_design
power_opt_design
place_design
phys_opt_design
write_checkpoint      -force $path_out/post_place
report_timing_summary -file  $path_out/post_place_timing_summary.rpt

################################################################################
# run router
# report actual utilization and timing,
# write checkpoint design
# run drc, write verilog and xdc out
################################################################################

route_design
#write_checkpoint         -force $path_out/post_route
#report_timing_summary    -file  $path_out/post_route_timing_summary.rpt
#report_timing -sort_by group -max_paths 100 -path_type summary -file $path_out/post_route_timing.rpt
#report_clock_utilization -file  $path_out/clock_util.rpt
#report_utilization       -file  $path_out/post_route_util.rpt
#report_power             -file  $path_out/post_route_power.rpt
#report_drc               -file  $path_out/post_imp_drc.rpt
#write_verilog            -force $path_out/bft_impl_netlist.v
#write_xdc -no_fixed_only -force $path_out/bft_impl.xdc

################################################################################
# generate a bitstream
################################################################################

write_bitstream -force $path_out/red_pitaya.bit

exit
