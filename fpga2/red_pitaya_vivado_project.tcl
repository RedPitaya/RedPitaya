################################################################################
# Vivado tcl script for building RedPitaya FPGA in non project mode
#
# Usage:
# vivado -mode batch -source red_pitaya_vivado_project.tcl
################################################################################

################################################################################
# define paths
################################################################################

set path_rtl rtl
set path_ip  ip
set path_sdc sdc

################################################################################
# setup an in memory project
################################################################################

set part xc7z010clg400-1

create_project -part $part -force redpitaya ./project

################################################################################
# create PS BD (processing system block design)
################################################################################

# create PS BD
source                            $path_ip/system_bd.tcl

# generate SDK files
generate_target all [get_files    system.bd]

################################################################################
# read files:
# 1. RTL design sources
# 2. IP database files
# 3. constraints
################################################################################

read_verilog                      ./project/redpitaya.srcs/sources_1/bd/system/hdl/system_wrapper.v

add_files                         $path_rtl/axi4_slave.sv
add_files                         $path_rtl/axi4_lite_slave.v

add_files                         $path_rtl/axi4_if.sv
add_files                         $path_rtl/axi4_lite_if.sv
add_files                         $path_rtl/axi4_stream_if.sv
add_files                         $path_rtl/axi4_stream_pas.sv
add_files                         $path_rtl/axi4_stream_reg.sv
add_files                         $path_rtl/axi4_stream_mux.sv
add_files                         $path_rtl/axi4_stream_demux.sv
add_files                         $path_rtl/axi4_stream_cnt.sv
add_files                         $path_rtl/sys_bus_if.sv
add_files                         $path_rtl/sys_bus_interconnect.sv
add_files                         $path_rtl/sys_bus_stub.sv
add_files                         $path_rtl/str_to_ram.sv

add_files                         $path_rtl/id.sv
add_files                         $path_rtl/red_pitaya_calib.sv
add_files                         $path_rtl/red_pitaya_pid_block.sv
add_files                         $path_rtl/red_pitaya_pid.sv
add_files                         $path_rtl/red_pitaya_pll.sv
add_files                         $path_rtl/red_pitaya_ps.sv
add_files                         $path_rtl/red_pitaya_top.sv
add_files                         $path_rtl/sys_reg_array_o.sv
add_files                         $path_rtl/cts.sv
add_files                         $path_rtl/muxctl.sv
add_files                         $path_rtl/gpio.sv
add_files                         $path_rtl/pdm.sv
add_files                         $path_rtl/pwm.sv
add_files                         $path_rtl/linear.sv
add_files                         $path_rtl/asg_top.sv
add_files                         $path_rtl/asg.sv
add_files                         $path_rtl/scope_top.sv
add_files                         $path_rtl/scope_filter.sv
add_files                         $path_rtl/scope_dec_avg.sv
add_files                         $path_rtl/scope_edge.sv
add_files                         $path_rtl/acq.sv
add_files                         $path_rtl/rle.sv
add_files                         $path_rtl/debounce.sv
add_files                         $path_rtl/la_top.sv
add_files                         $path_rtl/la_trigger.sv
add_files                         $path_rtl/str_dec.sv

add_files -fileset constrs_1      $path_sdc/red_pitaya.xdc

import_files -force

update_compile_order -fileset sources_1

################################################################################
################################################################################

#start_gui
