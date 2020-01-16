################################################################################
# HSI tcl script for building RedPitaya FSBL
#
# Usage:
# hsi -mode tcl -source red_pitaya_hsi_fsbl.tcl -tclargs projectname
################################################################################

set prj_name [lindex $argv 0]
append prj_name "_250"
puts "Project name: $prj_name"
cd prj/$prj_name


set path_sdk sdk

open_hw_design $path_sdk/red_pitaya.sysdef
generate_app -hw system_0 -os standalone -proc ps7_cortexa9_0 -app zynq_fsbl -compile -sw fsbl -dir $path_sdk/fsbl

exit
