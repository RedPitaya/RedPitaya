################################################################################
# Vivado tcl script for building RedPitaya FPGA in non project mode
#
# Usage:
# vivado -mode batch -source red_pitaya_vivado_project.tcl
################################################################################


################################################################################
# define paths
################################################################################

set path_bd  bd
set path_ip  ip
set path_rtl rtl
set path_sdc sdc


################################################################################
# setup an in memory project
################################################################################

set part xc7z010clg400-1

create_project -part $part -force redpitaya ./project

#set_property strategy {Vivado Synthesis Defaults} [get_runs synth_1]

#set_property strategy {Vivado Implementation Defaults} [get_runs impl_1]
#set_property strategy Performance_NetDelay_medium [get_runs impl_1]


################################################################################
# create PS BD (processing system block design)
################################################################################

# create PS BD
source                            $path_ip/system_bd.tcl

# generate SDK files
generate_target all               [get_files system.bd]

# generate system_wrapper.v file to the target directory
make_wrapper -files               [get_files project/redpitaya.srcs/sources_1/bd/system/system.bd] -top
add_files -norecurse              project/redpitaya.srcs/sources_1/bd/system/hdl/system_wrapper.v


################################################################################
# read files:
# 1. RTL design sources
# 2. IP database files
# 3. constraints
################################################################################

#read_bd                          [get_files system.bd]

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
add_files                         $path_rtl/red_pitaya_rst_clken.sv
add_files                         $path_rtl/red_pitaya_scope.v
add_files                         $path_rtl/red_pitaya_top.v

read_ip                           $path_ip/rb_addsub_48M48.xcix
read_ip                           $path_ip/rb_cic_125M_to_5M_18T18.xcix
read_ip                           $path_ip/rb_cic_200k_to_8k_18T18.xcix
read_ip                           $path_ip/rb_cic_48k_to_8k_18T18.xcix
read_ip                           $path_ip/rb_cic_5M_to_200k_18T18.xcix
read_ip                           $path_ip/rb_cic_8k_to_41M664_18T18.xcix
read_ip                           $path_ip/rb_cic_8k_to_48k_18T18.xcix
read_ip                           $path_ip/rb_cordic_T_WS_O_SR_18T18_NE_CR_EM_B.xcix
read_ip                           $path_ip/rb_dds_48_16_125.xcix
read_ip                           $path_ip/rb_div_32Div13R13.xcix
read_ip                           $path_ip/rb_dsp48_AaDmBaC_A18_D18_B18_C36_P37.xcix
read_ip                           $path_ip/rb_fir_8k_to_8k_25c23_17i16_35o33.xcix
read_ip                           $path_ip/rb_fir1_8k_to_8k_25c_17i16_35o32.xcix
read_ip                           $path_ip/rb_fir2_8k_to_8k_25c_17i16_35o32.xcix
read_ip                           $path_ip/rb_fir3_200k_to_200k_24c_17i16_35o.xcix

add_files -fileset constrs_1      $path_sdc/red_pitaya.xdc

#import_files -force

update_compile_order -fileset sources_1

################################################################################
################################################################################

#start_gui
