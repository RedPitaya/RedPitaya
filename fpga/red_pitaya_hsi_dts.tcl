################################################################################
# HSI tcl script for building RedPitaya DTS (device tree)
#
# Usage:
# hsi -mode tcl -source red_pitaya_hsi_dts.tcl -tclargs projectname
################################################################################

cd prj/$::argv

set path_sdk sdk

open_hw_design $path_sdk/red_pitaya.sysdef

set_repo_path ../../../tmp/device-tree-xlnx-xilinx-v2017.2/

create_sw_design device-tree -os device_tree -proc ps7_cortexa9_0

set_property CONFIG.kernel_version {2017.2} [get_os]

generate_target -dir $path_sdk/dts

exit
