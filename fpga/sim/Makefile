MODELSIM_BIN=$(HOME)/altera/15.0/modelsim_ase/bin

PATH_LIB=/opt/Xilinx/Vivado/2015.2/data/verilog/src
PATH_SIP=/opt/Xilinx/Vivado/2015.2/data/secureip
PATH_RTL=../rtl
PATH_TBN=../tbn

# secure IP
LIB=$(PATH_LIB)/unisims/DNA_PORT.v \
    $(PATH_LIB)/unisims/XADC.v \
    $(PATH_LIB)/unisims/IBUFDS.v \
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

RTL=$(PATH_RTL)/axi_master.v \
    $(PATH_RTL)/axi_slave.v \
    $(PATH_RTL)/axi_wr_fifo.v \
    $(PATH_RTL)/red_pitaya_ams.v \
    $(PATH_RTL)/red_pitaya_pwm.sv \
    $(PATH_RTL)/red_pitaya_pll.sv \
    $(PATH_RTL)/red_pitaya_asg_ch.v \
    $(PATH_RTL)/red_pitaya_asg.v \
    $(PATH_RTL)/red_pitaya_dfilt1.v \
    $(PATH_RTL)/red_pitaya_hk.v \
    $(PATH_RTL)/red_pitaya_pid_block.v \
    $(PATH_RTL)/red_pitaya_pid.v \
    $(PATH_RTL)/red_pitaya_ps.v \
    $(PATH_RTL)/red_pitaya_scope.v \
    $(PATH_RTL)/red_pitaya_top.v \
    $(PATH_RTL)/pwm.sv

TBN=$(PATH_TBN)/axi_master_model.sv \
    $(PATH_TBN)/axi_slave_tb.sv \
    $(PATH_TBN)/red_pitaya_ams_tb.sv \
    $(PATH_TBN)/red_pitaya_pwm_tb.sv \
    $(PATH_TBN)/red_pitaya_pll_tb.sv \
    $(PATH_TBN)/red_pitaya_asg_tb.sv \
    $(PATH_TBN)/red_pitaya_dfilt1_tb.sv \
    $(PATH_TBN)/red_pitaya_hk_tb.sv \
    $(PATH_TBN)/red_pitaya_pid_tb.sv \
    $(PATH_TBN)/red_pitaya_scope_tb.sv \
    $(PATH_TBN)/sys_bus_model.sv \
    $(PATH_TBN)/pwm_tb.sv

all: scope asg hk

compile: $(LIB) $(SIP) $(RTL) $(TBN)
	$(MODELSIM_BIN)/vlib work
	$(MODELSIM_BIN)/vlog $(LIB) $(SIP) $(RTL) $(TBN)

scope: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_scope_tb

asg: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_asg_tb

hk: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_hk_tb

ams: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_ams_tb

pll: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_pll_tb

dfilt1: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_dfilt1_tb

pid: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_pid_tb

pwm: compile
#	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_pwm_tb
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' pwm_tb

clean:
	rm -rf work *.vcd
