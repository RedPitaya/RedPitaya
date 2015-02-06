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



#### Parameters
# vivado or ahead
FPGA_TOOL=vivado

SDK_DIR=$(FPGA_TOOL)/red_pitaya.sdk/SDK/SDK_Export
SYSTEM_XML=$(SDK_DIR)/hw/system.xml
#FIXME: this is broken for plan_ahead, honestly, I don't care since plan_ahead
# should not be used anymore
SYSTEM_BD=$(FPGA_TOOL)/red_pitaya.srcs/sources_1/bd/system/system.bd


#### Cover task
all: fpga sw_package image
	echo ' \n'


#### CLEAN
clean:
ifeq ($(FPGA_TOOL),ahead)
	planAhead -mode tcl -source $$PWD/run_planahead.tcl -tclargs clean
endif
ifeq ($(FPGA_TOOL),vivado)
	vivado -mode tcl -source $$PWD/run_vivado.tcl -tclargs clean
endif
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

# FIXME: There seems to be a bug in the hardware export that
# makes the system think that the clk is running at 50MHz instead of 125
# it seems that this is either a Vivado bug, or has something to do with
# IO PLL, not worth my time to fix now
hw_src_copy: $(SYSTEM_XML)
ifeq ($(FPGA_TOOL),vivado)
	sed -i 's/C_SDIO_CLK_FREQ_HZ\"\ VALUE=\"50000000/C_SDIO_CLK_FREQ_HZ\"\ VALUE=\"125000000/' $(SDK_DIR)/hw/system.xml
endif

#FIXME: broken for planhead
$(SYSTEM_XML):$(SYSTEM_BD)
	vivado -mode tcl -source $$PWD/run_vivado.tcl -tclargs build_system_xml
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
ifeq ($(FPGA_TOOL),ahead)
	planAhead -mode tcl -source $$PWD/run_planahead.tcl -tclargs build
endif
ifeq ($(FPGA_TOOL),vivado)
	vivado -mode tcl -source $$PWD/run_vivado.tcl -tclargs build
endif


.PHONY: all clean image sw_package hw_src_copy uboot devicetree fsbl_bsp


