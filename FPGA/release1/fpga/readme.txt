

### Build fpga image

Set system environments (depends on computer)
 "source /opt/xilinx146/14.6/ISE_DS/settings32.sh"



## Using Makefile

make clean  --clean SW temporary files

make fpga   --run PlanAhead project
make xsdk   --run XSDK project
make image  --makes BOOT.BIN file
make all    --makes fpga and SW projects


## FPGA related 
 "planAhead -mode tcl -source $PWD/run_planahead.tcl" in this directory

result can be found at
 ahead/red_pitaya.runs/impl_1/red_pitaya_top.bit


