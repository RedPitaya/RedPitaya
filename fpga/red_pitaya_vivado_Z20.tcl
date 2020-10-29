################################################################################
# Vivado tcl script for building RedPitaya FPGA in non project mode
#
# Usage:
# vivado -mode tcl -source red_pitaya_vivado_Z20.tcl -tclargs projectname
################################################################################

set prj_name [lindex $argv 0]
puts "Project name: $prj_name"
cd prj/$prj_name
#cd prj/$::argv 0

################################################################################
# install UltraFast Design Methodology from TCL Store
################################################################################

tclapp::install -quiet ultrafast

################################################################################
# define paths
################################################################################

set path_brd ../../brd
set path_rtl rtl
set path_ip  ip
set path_bd  .srcs/sources_1/bd/system/hdl
set path_sdc ../../sdc
set path_sdc_prj sdc

set path_out out
set path_sdk sdk

file mkdir $path_out
file mkdir $path_sdk

################################################################################
# list board files
################################################################################

set_param board.repoPaths [list $path_brd]

################################################################################
# setup an in memory project
################################################################################

set part xc7z020clg400-1

create_project -in_memory -part $part

################################################################################
# create PS BD (processing system block design)
################################################################################

# file was created from GUI using "write_bd_tcl -force ip/systemZ20.tcl"
# create PS BD
source                            $path_ip/systemZ20.tcl

# generate SDK files
generate_target all [get_files    system.bd]
write_hwdef -force       -file    $path_sdk/red_pitaya.hwdef

################################################################################
# read files:
# 1. RTL design sources
# 2. IP database files
# 3. constraints
################################################################################

add_files -quiet                  [glob -nocomplain ../../$path_rtl/*_pkg.sv]
add_files -quiet                  [glob -nocomplain       $path_rtl/*_pkg.sv]
add_files                         ../../$path_rtl
add_files                               $path_rtl
add_files                               $path_bd

add_files -fileset constrs_1      $path_sdc/red_pitaya.xdc
add_files -fileset constrs_1      $path_sdc_prj/red_pitaya.xdc

################################################################################
# ser parameter containing Git hash
################################################################################

set gith [exec git log -1 --format="%H"]
set_property generic "GITH=160'h$gith" [current_fileset]

################################################################################
# run synthesis
# report utilization and timing estimates
# write checkpoint design
################################################################################

#synth_design -top red_pitaya_top_Z20
synth_design -top red_pitaya_top_Z20 -flatten_hierarchy none -bufg 16 -keep_equivalent_registers

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
report_io                -file    $path_out/post_imp_io.rpt
#write_verilog            -force   $path_out/bft_impl_netlist.v
#write_xdc -no_fixed_only -force   $path_out/bft_impl.xdc

xilinx::ultrafast::report_io_reg -verbose -file $path_out/post_route_iob.rpt

################################################################################
# generate a bitstream
################################################################################

set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

write_bitstream -force            $path_out/red_pitaya.bit
write_bitstream -force -bin_file  $path_out/red_pitaya

################################################################################
# generate system definition
################################################################################

write_sysdef -force      -hwdef   $path_sdk/red_pitaya.hwdef \
                         -bitfile $path_out/red_pitaya.bit \
                         -file    $path_sdk/red_pitaya.sysdef

exit
