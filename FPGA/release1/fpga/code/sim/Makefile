MODELSIM_BIN=$(HOME)/altera/14.1/modelsim_ase/bin

PATH_LIB=/opt/Xilinx/Vivado/2014.4/data/verilog/src
PATH_RTL=../rtl
PATH_TBN=../bench

LIB=$(PATH_LIB)/unisims/DNA_PORT.v \
    $(PATH_LIB)/glbl.v

RTL=$(PATH_RTL)/axi_master.v \
    $(PATH_RTL)/axi_slave.v \
    $(PATH_RTL)/axi_wr_fifo.v \
    $(PATH_RTL)/red_pitaya_ams.v \
    $(PATH_RTL)/red_pitaya_analog.v \
    $(PATH_RTL)/red_pitaya_asg_ch.v \
    $(PATH_RTL)/red_pitaya_asg.v \
    $(PATH_RTL)/red_pitaya_daisy_rx.v \
    $(PATH_RTL)/red_pitaya_daisy_test.v \
    $(PATH_RTL)/red_pitaya_daisy_tx.v \
    $(PATH_RTL)/red_pitaya_daisy.v \
    $(PATH_RTL)/red_pitaya_dfilt1.v \
    $(PATH_RTL)/red_pitaya_hk.v \
    $(PATH_RTL)/red_pitaya_pid_block.v \
    $(PATH_RTL)/red_pitaya_pid.v \
    $(PATH_RTL)/red_pitaya_ps.v \
    $(PATH_RTL)/red_pitaya_scope.v \
    $(PATH_RTL)/red_pitaya_top.v

TBN=$(PATH_TBN)/axi_master_model.sv \
    $(PATH_TBN)/axi_slave_tb.sv \
    $(PATH_TBN)/red_pitaya_ams_tb.sv \
    $(PATH_TBN)/red_pitaya_analog_tb.sv \
    $(PATH_TBN)/red_pitaya_asg_tb.sv \
    $(PATH_TBN)/red_pitaya_daisy_tb.sv \
    $(PATH_TBN)/red_pitaya_dfilt1_tb.sv \
    $(PATH_TBN)/red_pitaya_hk_tb.sv \
    $(PATH_TBN)/red_pitaya_pid_tb.sv \
    $(PATH_TBN)/red_pitaya_scope_tb.sv \
    $(PATH_TBN)/sys_bus_model.sv

all: scope asg hk

compile: $(LIB) $(RTL) $(TBN)
	$(MODELSIM_BIN)/vlib work
	$(MODELSIM_BIN)/vlog $(LIB) $(RTL) $(TBN)

scope: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_scope_tb

asg: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_asg_tb

hk: compile
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_hk_tb

clean:
	rm -rf work
