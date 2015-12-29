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

#read_bd                          [get_files system.bd]

read_verilog                      .srcs/sources_1/bd/system/hdl/system_wrapper.v

add_files                         $path_rtl/axi_master.v
add_files                         $path_rtl/axi_pc2leds.v
add_files                         $path_rtl/axi_slave.v
add_files                         $path_rtl/axi_wr_fifo.v

add_files                         $path_rtl/pwm.sv

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
add_files                         $path_rtl/red_pitaya_radiobox.sv
add_files                         $path_rtl/red_pitaya_scope.v
add_files                         $path_rtl/red_pitaya_top.v

read_ip                           $path_ip/rb_cic_48k_to_8k_32T32_lat13.xcix
read_ip                           $path_ip/rb_cic_8k_to_41M664_32T32_lat14.xcix
read_ip                           $path_ip/rb_dds_48_16_125.xcix
read_ip                           $path_ip/rb_dsp48_AaDmB_A16_D16_B16_P32.xcix
read_ip                           $path_ip/rb_dsp48_AaDmBaC_A17_D17_B17_C35_P36.xcix
read_ip                           $path_ip/rb_dsp48_AmB_A16_B16_P32.xcix
read_ip                           $path_ip/rb_dsp48_CONaC_CON48_C48_P48.xcix
read_ip                           $path_ip/rb_fir_8k_8k_25c23_17i16_35o33_lat42.xcix

add_files -fileset constrs_1      $path_sdc/red_pitaya.xdc

import_files -force

update_compile_order -fileset sources_1

################################################################################
################################################################################

#start_gui
