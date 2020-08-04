################################################################################
# Vivado tcl script for building RedPitaya FPGA in non project mode
#
# Usage:
# vivado -mode batch -source red_pitaya_vivado_project_Z20.tcl -tclargs projectname
################################################################################

set prj_name [lindex $argv 0]
puts "Project name: $prj_name"
cd prj/$prj_name
#cd prj/$::argv


################################################################################
# define paths
################################################################################

set path_brd brd
set path_rtl rtl_250
set path_ip  ip
set path_sdc sdc_250
set path_bd  project/redpitaya.srcs/sources_1/bd/system/hdl


################################################################################
# list board files
################################################################################

set_param board.repoPaths [list $path_brd]

################################################################################
# setup an in memory project
################################################################################

set part xc7z020clg400-3

create_project -part $part -force redpitaya ./project

################################################################################
# create PS BD (processing system block design)
################################################################################

# file was created from GUI using "write_bd_tcl -force ip/systemZ20.tcl"
# create PS BD
source                            $path_ip/systemZ20.tcl

# generate SDK files
generate_target all [get_files    system.bd]

################################################################################
# read files:
# 1. RTL design sources
# 2. IP database files
# 3. constraints
################################################################################

add_files                         ../../$path_rtl
add_files                         $path_rtl
add_files                         $path_bd

## search for HWID parameter to select xdc
foreach item $argv {
  puts "Input arfguments: $argv"
  if {[lsearch -all $item "*HWID*"] >= 0} {
    set hwid [split $item "="]
    set board [lindex $hwid 1]
    puts "Special board: $board"
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
# start gui
################################################################################

import_files -force

update_compile_order -fileset sources_1

set_property top red_pitaya_top [current_fileset]
