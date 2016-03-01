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

add_files                         $path_rtl/axi_master.v
add_files                         $path_rtl/axi_slave.v
add_files                         $path_rtl/axi_wr_fifo.v

add_files                         $path_rtl/red_pitaya_ams.v
add_files                         $path_rtl/red_pitaya_asg_ch.v
add_files                         $path_rtl/red_pitaya_asg.v
add_files                         $path_rtl/red_pitaya_dfilt1.v
add_files                         $path_rtl/red_pitaya_hk.v
add_files                         $path_rtl/red_pitaya_pid_block.v
add_files                         $path_rtl/red_pitaya_pid.v
add_files                         $path_rtl/red_pitaya_pll.sv
add_files                         $path_rtl/red_pitaya_ps.v
add_files                         $path_rtl/red_pitaya_pwm.sv
add_files                         $path_rtl/red_pitaya_scope.v
add_files                         $path_rtl/red_pitaya_top.v

add_files -fileset constrs_1      $path_sdc/red_pitaya.xdc

import_files -force

update_compile_order -fileset sources_1

################################################################################
################################################################################

#start_gui
