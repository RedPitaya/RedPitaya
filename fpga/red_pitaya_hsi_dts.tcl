################################################################################
# HSI tcl script for building RedPitaya DTS (device tree)
#
# Usage:
# hsi -mode tcl -source red_pitaya_hsi_dts.tcl
################################################################################

set path_sdk sdk

#set boot_args {console=ttyPS0,115200n8 root=/dev/ram rw initrd=0x00800000,16M earlyprintk mtdparts=physmap-flash.0:512K(nor-fsbl),512K(nor-u-boot),5M(nor-linux),9M(nor-user),1M(nor-scratch),-(nor-rootfs)}

open_hw_design $path_sdk/red_pitaya.sysdef

set_repo_path ../tmp/device-tree-xlnx-xilinx-v2015.1/

create_sw_design device-tree -os device_tree -proc ps7_cortexa9_0

set_property CONFIG.kernel_version {2015.1} [get_os]
#set_property CONFIG.bootargs $boot_args [get_os]

generate_target -dir $path_sdk/dts

exit
