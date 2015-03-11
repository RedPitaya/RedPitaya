#
# $Id: Makefile 1026 2014-02-07 09:54:51Z matej.oblak $
#
# Red Pitaya Makefile for building ZYNQ PL and PS binary
#
# Produces:
#   1. FPGA bit file & XSDC configuration.
#   2. XSDK products.
#   3. Booting image & device-tree.
#


.PHONY: all clean image

#### Parameters
FPGA_TOOL=vivado

SDK_DIR=$(FPGA_TOOL)/red_pitaya.sdk/SDK/SDK_Export


#### Cover task
all: fpga sw_package image
	echo ' \n'


#### CLEAN
clean:
	vivado -mode tcl -source $$PWD/run_vivado.tcl -tclargs clean
	make -C $(SDK_DIR)/fsbl_bsp/ clean
	make -C $(SDK_DIR)/fsbl/Debug/ clean
	make -C $(SDK_DIR)/device-tree_bsp_0/ clean
	make -C $(SDK_DIR)/uboot_bsp_0/ clean
	make -C image/ FPGA_TOOL=$(FPGA_TOOL) -k clean


#### Prepare images
image:
	make -C image/ FPGA_TOOL=$(FPGA_TOOL) -k all


#### Build SDK projects
sw_package: hw_src_copy fsbl_bsp fsbl devicetree uboot

hw_src_copy:
	sed -i 's/C_SDIO_CLK_FREQ_HZ\"\ VALUE=\"50000000/C_SDIO_CLK_FREQ_HZ\"\ VALUE=\"125000000/' $(SDK_DIR)/hw/system.xml
	cp $(SDK_DIR)/hw/ps7_init.*    $(SDK_DIR)/fsbl/src/ 

uboot:
	make -C $(SDK_DIR)/uboot_bsp_0/ -k all

devicetree:
	make -C $(SDK_DIR)/device-tree_bsp_0/ -k all

fsbl:
	make -C $(SDK_DIR)/fsbl/Debug/ -k all

fsbl_bsp:
	make -C $(SDK_DIR)/fsbl_bsp/ -k all


#### Run FPGA tool
fpga:
	vivado -mode tcl -source $$PWD/run_vivado.tcl -tclargs build

