################################################################################
# define paths
################################################################################

set fpga_path ../../..
set path_brd ./$fpga_path/brd
set path_rtl rtl
set path_ip  ip
set path_bd  project/redpitaya.srcs/sources_1/bd/system/hdl
set path_sdc ./$fpga_path/sdc
set path_sdc_prj sdc
set path_tbn tbn

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

# file was created from GUI using "write_bd_tcl -force ip/systemZ10.tcl"
# create PS BD
source                            $path_ip/systemZ10.tcl

# generate SDK files
generate_target all [get_files    system.bd]

################################################################################
# read files:
# 1. RTL design sources
# 2. IP database files
# 3. constraints
################################################################################

add_files                         $fpga_path/$path_rtl
add_files                         $path_rtl
add_files                         $path_bd

<<<<<<< HEAD
=======
update_files -from_files $path_rtl/red_pitaya_scope.v -to_files ../$fpga_path/$path_rtl/classic/red_pitaya_scope.v -filesets [get_filesets *]
>>>>>>> dev-250-12
add_files -fileset sim_1 -norecurse $path_tbn/red_pitaya_proc_tb.vhd

add_files -fileset constrs_1      $path_sdc/red_pitaya.xdc
add_files -fileset constrs_1      $path_sdc_prj/red_pitaya.xdc


################################################################################
# start gui
################################################################################

import_files -force

set_property top red_pitaya_top [current_fileset]

update_compile_order -fileset sources_1
