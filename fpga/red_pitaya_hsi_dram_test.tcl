################################################################################
# HSI tcl script for building RedPitaya memory test
#
# Usage:
# hsi -mode tcl -source red_pitaya_hsi_dram_test.tcl
################################################################################

set path_sdk sdk

open_hw_design $path_sdk/red_pitaya.sysdef
generate_app -hw system -os standalone -proc ps7_cortexa9_0 -app zynq_dram_test -compile -sw dram_test -dir $path_sdk/dram_test

exit
