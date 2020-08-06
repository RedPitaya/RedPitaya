################################################################################
# Vivado tcl script for building RedPitaya FPGA in non project mode
#
# Usage:
# vivado -mode tcl -source red_pitaya_vivado_Z20.tcl -tclargs projectname
################################################################################

set prj_name [lindex $argv 0]
set prj_top "red_pitaya_top"
set prj_dir "build"
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

set path_brd brd
set path_rtl rtl_250
set path_ip  ip
set path_sdc sdc_250
set path_bd  $prj_dir/redpitaya.srcs/sources_1/bd/system/hdl

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

set part xc7z020clg400-3

create_project -part $part -force redpitaya $prj_dir

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

#add_files -quiet                  [glob -nocomplain ../../$path_rtl/*_pkg.sv]
#add_files -quiet                  [glob -nocomplain       $path_rtl/*_pkg.sv]
add_files                         ../../$path_rtl
add_files                         $path_rtl
add_files                         $path_bd

## search for HWID parameter to select xdc
foreach item $argv {
  puts "Input arfguments: $argv"
  if {[lsearch -all $item "*HWID*"] >= 0} {
    set hwid [split $item "="]
    if {[lindex $hwid 1] ne ""} {
      set board [lindex $hwid 1]
      puts "Special board: $board"
    }
  }
}

if {[info exists board]} {
  puts "Special board: $board"
  add_files -fileset constrs_1  ../../$path_sdc/red_pitaya_${board}.xdc
} else {
  puts "Reading standard board constraints."
  add_files -fileset constrs_1  ../../$path_sdc/red_pitaya.xdc
}



################################################################################
# ser parameter containing Git hash
################################################################################

set gith [exec git log -1 --format="%H"]
set_property generic "GITH=160'h$gith" [current_fileset]

set_property top $prj_top [current_fileset]




################################################################################
# run synthesis
# report utilization and timing estimates
# write checkpoint design
################################################################################

update_compile_order -fileset sources_1

launch_runs synth_1
wait_on_run synth_1

set rptFiles [glob -directory ./$prj_dir/redpitaya.runs/synth_1/  *.rpt]
file copy -force $rptFiles ./$path_out/


################################################################################
# run placement and router
# report utilization and timing estimates
# write checkpoint design
################################################################################


launch_runs impl_1 -jobs 2
wait_on_run impl_1

set rptFiles [glob -directory ./$prj_dir/redpitaya.runs/impl_1/  *.rpt]
foreach file $rptFiles {
   file copy -force $file ./$path_out/
}


################################################################################
# generate a bitstream
################################################################################

#set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]

launch_runs impl_1 -to_step write_bitstream

open_run impl_1
write_bitstream -force            $path_out/red_pitaya.bit
write_bitstream -force -bin_file  $path_out/red_pitaya


################################################################################
# generate system definition
################################################################################


write_sysdef -force      -hwdef   $path_sdk/red_pitaya.hwdef \
                         -bitfile $path_out/red_pitaya.bit \
                         -file    $path_sdk/red_pitaya.sysdef

exit
