ALTERA_PATH=$(HOME)/intelFPGA
XILINX_PATH=/opt/Xilinx

MODELSIM_BIN=$(ALTERA_PATH)/16.1/modelsim_ase/bin

PATH_LIB=$(XILINX_PATH)/Vivado/2017.2/data/verilog/src
PATH_SIP=$(XILINX_PATH)/Vivado/2017.2/data/secureip
PATH_RTL=../rtl
PATH_TBN=../tbn

# secure IP
LIB=$(PATH_LIB)/unisims/DNA_PORT.v \
    $(PATH_LIB)/unisims/EFUSE_USR.v \
    $(PATH_LIB)/unisims/XADC.v \
    $(PATH_LIB)/unisims/PLLE2_ADV.v \
    $(PATH_LIB)/unisims/BUFG.v \
    $(PATH_LIB)/unisims/IBUFDS.v \
    $(PATH_LIB)/unisims/OBUFDS.v \
    $(PATH_LIB)/unisims/IOBUF.v \
    $(PATH_LIB)/unisims/IDDR.v \
    $(PATH_LIB)/unisims/ODDR.v \
    $(PATH_LIB)/unisims/OSERDESE2.v \
    $(PATH_LIB)/unisims/ISERDESE2.v \
    $(PATH_LIB)/retarget/IBUFGDS.v \
    $(PATH_LIB)/unisims/BUFIO.v \
    $(PATH_LIB)/unisims/BUFR.v \
    $(PATH_LIB)/glbl.v

SIP=$(PATH_SIP)/oserdese2/oserdese2_002.vp \
    $(PATH_SIP)/iserdese2/iserdese2_002.vp

# RTL files
RTL_PKG =$(wildcard ../../../rtl/*_pkg.sv)
RTL_PKG+=$(wildcard ../../../rtl/*/*_pkg.sv)
RTL_PKG+=$(wildcard $(PATH_RTL)/*_pkg.sv)
RTL_SRC =$(wildcard ../../../rtl/*.sv)
RTL_SRC+=$(wildcard ../../../rtl/*/*.sv)
RTL_SRC+=$(wildcard ../../../rtl/*/*.v)
RTL_SRC+=$(wildcard $(PATH_RTL)/*.sv)
RTL_SRC+=$(wildcard $(PATH_RTL)/*.v)
RTL_DIF =$(filter-out $(RTL_PKG),$(RTL_SRC))
RTL = $(RTL_PKG) $(RTL_DIF)

# testbench files
TBN_PKG =$(wildcard $(PATH_TBN)/*_pkg.sv)
TBN_SRC =$(wildcard $(PATH_TBN)/*.sv)
TBN_SRC+=$(wildcard ../../../tbn/*.sv)
TBN_DIF =$(filter-out $(TBN_PKG),$(TBN_SRC))
TBN = $(TBN_PKG) $(TBN_DIF)

# targets
TGT=$(RTL:.sv=)

# simulation configuration
CFG  = -postsimdataflow

# simulation script
#TCL  = set WildcardFilter [lsearch -not -all -inline $$WildcardFilter Memory];
TCL  = lappend WildcardFilter Memory;
TCL += log -r /*;
TCL += run -all; quit;

.PHONY: compile 

all: $(TGT)

compile: $(LIB) $(SIP) $(RTL) $(TBN)
	$(MODELSIM_BIN)/vlib work
	$(MODELSIM_BIN)/vlog $(LIB) $(SIP) $(RTL) $(TBN)

%_tb: compile
	$(MODELSIM_BIN)/vsim $(CFG) -c -do '$(TCL)' $@
ifdef WAV
	$(MODELSIM_BIN)/vsim -do 'dataset open vsim.wlf; do $@.tcl'
endif

clean:
	rm -rf work *.vcd
