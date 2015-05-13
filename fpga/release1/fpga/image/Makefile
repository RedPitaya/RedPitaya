#
# $Id: Makefile 961 2014-01-21 11:40:39Z matej.oblak $
#
# Red Pitaya Makefile for building boot image and device-tree
#
# Produces:
#   1. Boot image 
#   2. Device-tree
#


INSTAL_DIR ?= .

# settings
SRC_DIR=src
OUT_DIR=out
FPGA_TOOL=vivado
SDK_EXPORT=../$(FPGA_TOOL)/red_pitaya.sdk/SDK/SDK_Export

# build artefacts
FPGA_BIT=../$(FPGA_TOOL)/red_pitaya.runs/impl_1/red_pitaya_top.bit
FSBL_ELF=$(SDK_EXPORT)/fsbl/Debug/fsbl.elf
UBOOT_ELF=$(SRC_DIR)/u-boot.elf
DEV_TREE=$(SDK_EXPORT)/device-tree_bsp_0/ps7_cortexa9_0/libsrc/device-tree_v0_00_x/xilinx.dts

.PHONY: clean

all: bootbin devicetree

bootbin:
	cp $(FPGA_BIT) $(OUT_DIR)/fpga.bit
	cp $(FSBL_ELF) $(OUT_DIR)/fsbl.elf
	@echo img:{[bootloader] $(OUT_DIR)/fsbl.elf $(OUT_DIR)/fpga.bit $(UBOOT_ELF) } > $(OUT_DIR)/boot.bif
	bootgen -image $(OUT_DIR)/boot.bif -w -o i $(OUT_DIR)/boot.bin

devicetree:
	cp $(DEV_TREE) $(OUT_DIR)/devicetree.dts
	patch -N $(OUT_DIR)/devicetree.dts $(SRC_DIR)/device_tree_$(FPGA_TOOL).patch
	$(SRC_DIR)/dtc -I dts -O dtb -o $(OUT_DIR)/devicetree.dtb $(OUT_DIR)/devicetree.dts

clean:
	rm -rf $(OUT_DIR)/*

