#
# $Id: Makefile 934 2014-01-09 07:48:05Z matej.oblak $
#
# Red Pitaya FPGA/SoC Makefile
#
# Produces:
#   1. First stage bootloader (FSBL) ELF binary.
#   2. First stage bootloader with RAM test.
#   3. FPGA bit file.
#   4. Linux device tree source (dts).
#

INSTAL_DIR ?= .
THIS_DIR := $(shell pwd)

# settings for x86_64 build machine
VM=$(XILINX)/java6/lin64/jre/bin
ECLIPSE=$(XILINX_EDK)/eclipse/lin64/eclipse/eclipse
FPGA_DIR=release1/fpga
FPGA_TOOL=vivado
SDK_EXPORT=$(FPGA_DIR)/$(FPGA_TOOL)/red_pitaya.sdk/SDK/SDK_Export

# build artefacts
FPGA_BIT=$(FPGA_DIR)/$(FPGA_TOOL)/red_pitaya.runs/impl_1/red_pitaya_top.bit
FSBL_ELF=$(SDK_EXPORT)/fsbl/Debug/fsbl.elf
DEV_TREE=$(SDK_EXPORT)/device-tree_bsp_0/ps7_cortexa9_0/libsrc/device-tree_v0_00_x/xilinx.dts

.PHONY: all clean install memtest


# dev_tree is required by FSBL_ELF
# because they are set as .PHONY, basically they will always be remade
# so only depend on the ELF instead of both (instead of making things twice
#all: $(FPGA_BIT) $(FSBL_ELF) $(DEV_TREE)
all: $(FPGA_BIT) $(FSBL_ELF)

$(FPGA_BIT):
	make -C $(FPGA_DIR) FPGA_TOOL=$(FPGA_TOOL) fpga

# such a weird dependency,
# actually FSBL_ELF is dependent on DEV_TREE in some lower Makefil
$(DEV_TREE):$(FSBL_ELF)

$(FSBL_ELF):
	make -C $(FPGA_DIR) FPGA_TOOL=$(FPGA_TOOL) sw_package

clean:
	make -C $(FPGA_DIR) FPGA_TOOL=$(FPGA_TOOL) clean

memtest:
	cp $(FSBL_ELF) $(INSTALL_DIR)
	cd $(FPGA_DIR); cat $(THIS_DIR)/patches/*.patch | patch -p2
	rm $(FSBL_ELF)
	rm -f `find $(SDK_EXPORT)/fsbl/ -name \*.o`
	make -C $(FPGA_DIR) FPGA_TOOL=$(FPGA_TOOL) fsbl
	cp $(FSBL_ELF) $(INSTALL_DIR)/memtest.elf

install: memtest
	cp $(FPGA_BIT) $(INSTALL_DIR)/fpga.bit
	cp $(DEV_TREE) $(INSTALL_DIR)/devicetree.dts
	patch $(INSTALL_DIR)/devicetree.dts $(FPGA_DIR)/image/src/device_tree_$(FPGA_TOOL).patch

# these need to be phony because the Makefile cannot check for dependencies
.PHONY:$(FPGA_BIT) $(FSBL_ELF) $(DEV_TREE)
