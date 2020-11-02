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

PRJ   ?= logic
MODEL ?= Z10
HWID  ?= ""

# build artefacts
FPGA_BIT    = prj/$(PRJ)/out/red_pitaya.bit
FSBL_ELF    = prj/$(PRJ)/sdk/fsbl/executable.elf
MEMTEST_ELF = prj/$(PRJ)/sdk/dram_test/executable.elf
DEVICE_TREE = prj/$(PRJ)/sdk/dts/system.dts

# Vivado from Xilinx provides IP handling, FPGA compilation
# hsi (hardware software interface) provides software integration
# both tools are run in batch mode with an option to avoid log/journal files
VIVADO = vivado -nojournal -mode batch
HSI    = hsi    -nolog -nojournal -mode batch

.PHONY: all clean project

all: $(FPGA_BIT) $(FSBL_ELF) $(DEVICE_TREE)

# TODO: clean should go into each project
clean:
	rm -rf out .Xil .srcs sdk project
	rm -rf prj/$(PRJ)/out prj/$(PRJ)/.Xil prj/$(PRJ)/.srcs prj/$(PRJ)/sdk prj/$(PRJ)/project

project:
ifneq ($(HWID),"")
	vivado -source red_pitaya_vivado_project_$(MODEL).tcl -tclargs $(PRJ) HWID=$(HWID)
else
	vivado -source red_pitaya_vivado_project_$(MODEL).tcl -tclargs $(PRJ)
endif

$(FPGA_BIT):
ifneq ($(HWID),"")
	$(VIVADO) -source red_pitaya_vivado_$(MODEL).tcl -tclargs $(PRJ) HWID=$(HWID)
else
	$(VIVADO) -source red_pitaya_vivado_$(MODEL).tcl -tclargs $(PRJ)
endif
	./synCheck.sh

$(FSBL_ELF): $(FPGA_BIT)
	$(HSI) -source red_pitaya_hsi_fsbl.tcl -tclargs $(PRJ)

$(DEVICE_TREE): $(FPGA_BIT)
	$(HSI) -source red_pitaya_hsi_dts.tcl -tclargs $(PRJ)

