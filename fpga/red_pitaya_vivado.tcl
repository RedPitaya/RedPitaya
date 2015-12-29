################################################################################
# Vivado tcl script for building RedPitaya FPGA in non project mode
#
# Usage:
# vivado -mode tcl -source red_pitaya_vivado.tcl
################################################################################


################################################################################
# define paths
################################################################################

set path_bd  bd
set path_ip  ip
set path_rtl rtl
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
generate_target all               [get_files system.bd]
write_hwdef    -force -file       $path_sdk/red_pitaya.hwdef


################################################################################
# read files:
# 1. RTL design sources
# 2. IP database files
# 3. constraints
################################################################################

# template
#read_verilog                     $path_rtl/...

#read_bd                          [get_files system.bd]
read_verilog                      .srcs/sources_1/bd/system/hdl/system_wrapper.v

read_verilog                      $path_rtl/axi_master.v
read_verilog                      $path_rtl/axi_pc2leds.v
read_verilog                      $path_rtl/axi_slave.v
read_verilog                      $path_rtl/axi_wr_fifo.v

read_verilog   -sv                $path_rtl/pwm.sv

read_verilog                      $path_rtl/red_pitaya_ams.v
read_verilog                      $path_rtl/red_pitaya_asg.v
read_verilog                      $path_rtl/red_pitaya_asg_ch.v
read_verilog                      $path_rtl/red_pitaya_dfilt1.v
read_verilog                      $path_rtl/red_pitaya_hk.v
read_verilog                      $path_rtl/red_pitaya_pid.v
read_verilog                      $path_rtl/red_pitaya_pid_block.v
read_verilog   -sv                $path_rtl/red_pitaya_pll.sv
read_verilog                      $path_rtl/red_pitaya_ps.v
read_verilog   -sv                $path_rtl/red_pitaya_pwm.sv
read_verilog   -sv                $path_rtl/red_pitaya_radiobox.sv
read_verilog                      $path_rtl/red_pitaya_scope.v
read_verilog                      $path_rtl/red_pitaya_top.v

read_ip                           $path_ip/rb_cic_48k_to_8k_32T32_lat13.xcix
read_ip                           $path_ip/rb_cic_8k_to_41M664_32T32_lat14.xcix
read_ip                           $path_ip/rb_dds_48_16_125.xcix
read_ip                           $path_ip/rb_dsp48_AaDmB_A16_D16_B16_P32.xcix
read_ip                           $path_ip/rb_dsp48_AaDmBaC_A17_D17_B17_C35_P36.xcix
read_ip                           $path_ip/rb_dsp48_AmB_A16_B16_P32.xcix
read_ip                           $path_ip/rb_dsp48_CONaC_CON48_C48_P48.xcix
read_ip                           $path_ip/rb_fir_8k_8k_25c23_17i16_35o33_lat42.xcix

read_xdc                          $path_sdc/red_pitaya.xdc


################################################################################
# run synthesis
# report utilization and timing estimates
# write checkpoint design
################################################################################

synth_ip                          [get_ips clk_adc_pll]
synth_ip                          [get_ips rb_*]

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
#write_hwdef      -force -file    $path_sdk/red_pitaya.hwdef


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

write_bitstream -force $path_out/red_pitaya.bit


################################################################################
# generate system definition
################################################################################

write_sysdef -force      -hwdef   $path_sdk/red_pitaya.hwdef \
                         -bitfile $path_out/red_pitaya.bit \
                         -file    $path_sdk/red_pitaya.sysdef

exit
