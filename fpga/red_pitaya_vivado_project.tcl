################################################################################
# Vivado tcl script for building RedPitaya FPGA in non project mode
#
# Usage:
# vivado -mode batch -source red_pitaya_vivado_project.tcl -tclargs projectname
################################################################################

cd prj/$::argv

################################################################################
# define paths
################################################################################

set path_brd brd
set path_rtl rtl
set path_ip  ip
set path_sdc sdc

################################################################################
# list board files
################################################################################

set_param board.repoPaths [list $path_brd]

################################################################################
# setup an in memory project
################################################################################

set part xc7z010clg400-1

create_project -part $part -force redpitaya ./project

################################################################################
# create PS BD (processing system block design)
################################################################################

# file was created from GUI using "write_bd_tcl -force ip/system.tcl"
# create PS BD
source                            $path_ip/system.tcl

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

#add_files -fileset constrs_1      $path_sdc/red_pitaya.xdc
add_files -fileset constrs_1      ../../$path_sdc/red_pitaya.xdc

################################################################################
# start gui
################################################################################

import_files -force

update_compile_order -fileset sources_1

set_property top red_pitaya_top [current_fileset]
