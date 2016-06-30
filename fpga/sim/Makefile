MODELSIM_BIN=$(HOME)/altera/15.1/modelsim_ase/bin

PATH_LIB=/opt/Xilinx/Vivado/2016.2/data/verilog/src
PATH_SIP=/opt/Xilinx/Vivado/2016.2/data/secureip
PATH_RTL=../rtl
PATH_TBN=../tbn

# secure IP
LIB=$(PATH_LIB)/unisims/DNA_PORT.v \
    $(PATH_LIB)/unisims/XADC.v \
    $(PATH_LIB)/unisims/IBUFDS.v \
    $(PATH_LIB)/unisims/IOBUF.v \
    $(PATH_LIB)/unisims/BUFG.v \
    $(PATH_LIB)/unisims/PLLE2_ADV.v \
    $(PATH_LIB)/unisims/ODDR.v \
    $(PATH_LIB)/unisims/OBUFDS.v \
    $(PATH_LIB)/unisims/OSERDESE2.v \
    $(PATH_LIB)/retarget/IBUFGDS.v \
    $(PATH_LIB)/unisims/BUFIO.v \
    $(PATH_LIB)/unisims/BUFR.v \
    $(PATH_LIB)/unisims/ISERDESE2.v \
    $(PATH_LIB)/glbl.v

SIP=$(PATH_SIP)/oserdese2/oserdese2_002.vp \
    $(PATH_SIP)/iserdese2/iserdese2_002.vp

# RTL files
RTL=$(wildcard $(PATH_RTL)/*.sv)
RTL+=$(wildcard $(PATH_RTL)/*.v)

# testbench files
TBN=$(wildcard $(PATH_TBN)/*.sv)

# targets
TGT=$(RTL:.sv=)

.PHONY: compile 

all: $(TGT)

compile: $(LIB) $(SIP) $(RTL) $(TBN)
	$(MODELSIM_BIN)/vlib work
	$(MODELSIM_BIN)/vlog $(LIB) $(SIP) $(RTL) $(TBN)

%_tb: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' $@

clean:
	rm -rf work *.vcd
