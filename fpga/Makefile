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

# build artefacts
FPGA_BIT=out/red_pitaya.bit
FSBL_ELF=sdk/fsbl/executable.elf
MEMTEST_ELF=sdk/memtest/executable.elf
DEVICE_TREE=sdk/dts/system.dts
DEVICE_TREE_SRC=xilinx-v2015.1.tar.gz

# Vivado from Xilinx provides IP handling, FPGA compilation
# hsi (hardware software interface) provides software integration
# both tools are run in batch mode with an option to avoid log/journal files
VIVADO = vivado -nolog -nojournal -mode batch
HSI    = hsi    -nolog -nojournal -mode batch

.PHONY: all clean

all: $(FPGA_BIT) $(FSBL_ELF) $(MEMTEST_ELF) $(DEVICE_TREE)

clean:
	rm -rf out .Xil .srcs sdk device-tree-xlnx-xilinx-v2015.1

$(FPGA_BIT):
	$(VIVADO) -source red_pitaya_vivado.tcl

$(FSBL_ELF): $(FPGA_BIT)
	$(HSI) -source red_pitaya_hsi_fsbl.tcl

$(MEMTEST_ELF): $(FPGA_BIT)
	$(HSI) -source red_pitaya_hsi_memtest.tcl

$(DEVICE_TREE_SRC):
	wget https://github.com/Xilinx/device-tree-xlnx/archive/$(DEVICE_TREE_SRC)

$(DEVICE_TREE): $(FPGA_BIT) $(DEVICE_TREE_SRC)
	tar -xzf $(DEVICE_TREE_SRC)
	$(HSI) -source red_pitaya_hsi_dts.tcl
