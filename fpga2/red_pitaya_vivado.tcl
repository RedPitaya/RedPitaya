################################################################################
# Vivado tcl script for building RedPitaya FPGA in non project mode
#
# Usage:
# vivado -mode tcl -source red_pitaya_vivado.tcl
################################################################################

################################################################################
# define paths
################################################################################

set path_rtl rtl
set path_ip  ip
set path_sdc sdc

set path_out out
set path_sdk sdk

file mkdir $path_out
file mkdir $path_sdk

################################################################################
# setup an in memory project
################################################################################

set part xc7z010clg400-1

create_project -in_memory -part $part

# experimental attempts to avoid a warning
#get_projects
#get_designs
#list_property  [current_project]
#set_property FAMILY 7SERIES [current_project]
#set_property SIM_DEVICE 7SERIES [current_project]

################################################################################
# create PS BD (processing system block design)
################################################################################

# file was created from GUI using "write_bd_tcl -force ip/system_bd.tcl"
# create PS BD
source                            $path_ip/system_bd.tcl

# generate SDK files
generate_target all [get_files    system.bd]
write_hwdef -force       -file    $path_sdk/red_pitaya.hwdef

################################################################################
# read files:
# 1. RTL design sources
# 2. IP database files
# 3. constraints
################################################################################

# template
#read_verilog                      $path_rtl/...

read_verilog                      .srcs/sources_1/bd/system/hdl/system_wrapper.v

read_verilog                      $path_rtl/axi4_slave.sv
read_verilog                      $path_rtl/axi4_lite_slave.v

read_verilog                      $path_rtl/axi4_if.sv
read_verilog                      $path_rtl/axi4_lite_if.sv
read_verilog                      $path_rtl/axi4_stream_if.sv
read_verilog                      $path_rtl/axi4_stream_pas.sv
read_verilog                      $path_rtl/axi4_stream_reg.sv
read_verilog                      $path_rtl/axi4_stream_mux.sv
read_verilog                      $path_rtl/axi4_stream_demux.sv
read_verilog                      $path_rtl/axi4_stream_cnt.sv
read_verilog                      $path_rtl/sys_bus_if.sv
read_verilog                      $path_rtl/sys_bus_interconnect.sv
read_verilog                      $path_rtl/sys_bus_stub.sv
read_verilog                      $path_rtl/str_to_ram.sv

read_verilog                      $path_rtl/id.sv
read_verilog                      $path_rtl/red_pitaya_calib.sv
read_verilog                      $path_rtl/red_pitaya_pid_block.sv
read_verilog                      $path_rtl/red_pitaya_pid.sv
read_verilog                      $path_rtl/red_pitaya_pll.sv
read_verilog                      $path_rtl/red_pitaya_ps.sv
read_verilog                      $path_rtl/red_pitaya_top.sv
read_verilog                      $path_rtl/sys_reg_array_o.sv
read_verilog                      $path_rtl/cts.sv
read_verilog                      $path_rtl/muxctl.sv
read_verilog                      $path_rtl/gpio.sv
read_verilog                      $path_rtl/pdm.sv
read_verilog                      $path_rtl/pwm.sv
read_verilog                      $path_rtl/linear.sv
read_verilog                      $path_rtl/asg_top.sv
read_verilog                      $path_rtl/asg.sv
read_verilog                      $path_rtl/scope_top.sv
read_verilog                      $path_rtl/scope_filter.sv
read_verilog                      $path_rtl/scope_dec_avg.sv
read_verilog                      $path_rtl/scope_edge.sv
read_verilog                      $path_rtl/acq.sv
read_verilog                      $path_rtl/rle.sv
read_verilog                      $path_rtl/debounce.sv
read_verilog                      $path_rtl/la_top.sv
read_verilog                      $path_rtl/la_trigger.sv
read_verilog                      $path_rtl/str_dec.sv

read_xdc                          $path_sdc/red_pitaya.xdc

################################################################################
# define paths
################################################################################

set gith [exec git log -1 --format="%H"]
set_property generic "GITH=160'h$gith" [current_fileset]

################################################################################
# run synthesis
# report utilization and timing estimates
# write checkpoint design
################################################################################

#synth_design -top red_pitaya_top
synth_design -top red_pitaya_top -flatten_hierarchy none -bufg 16 -keep_equivalent_registers

write_checkpoint         -force   $path_out/post_synth
report_timing_summary    -file    $path_out/post_synth_timing_summary.rpt
report_power             -file    $path_out/post_synth_power.rpt

################################################################################
# run placement and logic optimization
# report utilization and timing estimates
# write checkpoint design
################################################################################

opt_design
power_opt_design
place_design
phys_opt_design
write_checkpoint         -force   $path_out/post_place
report_timing_summary    -file    $path_out/post_place_timing_summary.rpt
#write_hwdef              -file    $path_sdk/red_pitaya.hwdef

################################################################################
# run router
# report actual utilization and timing,
# write checkpoint design
# run drc, write verilog and xdc out
################################################################################

route_design
write_checkpoint         -force   $path_out/post_route
report_timing_summary    -file    $path_out/post_route_timing_summary.rpt
report_timing            -file    $path_out/post_route_timing.rpt -sort_by group -max_paths 100 -path_type summary
report_clock_utilization -file    $path_out/clock_util.rpt
report_utilization       -file    $path_out/post_route_util.rpt
report_power             -file    $path_out/post_route_power.rpt
report_drc               -file    $path_out/post_imp_drc.rpt
#write_verilog            -force   $path_out/bft_impl_netlist.v
#write_xdc -no_fixed_only -force   $path_out/bft_impl.xdc

################################################################################
# generate a bitstream
################################################################################

set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

write_bitstream -force $path_out/red_pitaya.bit

################################################################################
# generate system definition
################################################################################

write_sysdef -force      -hwdef   $path_sdk/red_pitaya.hwdef \
                         -bitfile $path_out/red_pitaya.bit \
                         -file    $path_sdk/red_pitaya.sysdef

exit
