#
# Authors: Matej Oblak, Iztok Jeras
# (C) Red Pitaya 2013-2015
#
# Red Pitaya FPGA/SoC Makefile 
#
# Produces:
#   3. FPGA bit file.
#   1. FSBL (First stage bootloader) ELF binary.
#   2. Memtest (stand alone memory test) ELF binary.
#   4. Linux device tree source (dts).

PRJ ?= logic

# build artefacts
FPGA_BIT    = prj/$(PRJ)/out/red_pitaya.bit
FSBL_ELF    = prj/$(PRJ)/sdk/fsbl/executable.elf
MEMTEST_ELF = prj/$(PRJ)/sdk/dram_test/executable.elf
DEVICE_TREE = prj/$(PRJ)/sdk/dts/system.dts

# Vivado from Xilinx provides IP handling, FPGA compilation
# hsi (hardware software interface) provides software integration
# both tools are run in batch mode with an option to avoid log/journal files
VIVADO = vivado -nolog -nojournal -mode batch
HSI    = hsi    -nolog -nojournal -mode batch

.PHONY: all clean project

all: $(FPGA_BIT) $(FSBL_ELF) $(DEVICE_TREE)

# TODO: clean should go into each project
clean:
	rm -rf out .Xil .srcs sdk project

project:
	vivado -source red_pitaya_vivado_project.tcl -tclargs $(PRJ)

$(FPGA_BIT):
	$(VIVADO) -source red_pitaya_vivado.tcl -tclargs $(PRJ)

$(FSBL_ELF): $(FPGA_BIT)
	$(HSI) -source red_pitaya_hsi_fsbl.tcl -tclargs $(PRJ)

$(DEVICE_TREE): $(FPGA_BIT)
	$(HSI) -source red_pitaya_hsi_dts.tcl -tclargs $(PRJ)
