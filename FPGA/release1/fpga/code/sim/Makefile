MODELSIM_BIN=$(HOME)/altera/14.1/modelsim_ase/bin

PATH_RTL=../rtl
PATH_TBN=../bench

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

TBN=$(PATH_TBN)/axi_master_model.v \
    $(PATH_TBN)/axi_slave_tb.v \
    $(PATH_TBN)/red_pitaya_ams_tb.v \
    $(PATH_TBN)/red_pitaya_analog_tb.v \
    $(PATH_TBN)/red_pitaya_asg_tb.v \
    $(PATH_TBN)/red_pitaya_daisy_tb.v \
    $(PATH_TBN)/red_pitaya_dfilt1_tb.v \
    $(PATH_TBN)/red_pitaya_hk_tb.v \
    $(PATH_TBN)/red_pitaya_pid_tb.v \
    $(PATH_TBN)/red_pitaya_scope_tb.sv \
    $(PATH_TBN)/sys_bus_model.sv

scope:
	$(MODELSIM_BIN)/vlib work
	$(MODELSIM_BIN)/vlog $(RTL) $(TBN)
	$(MODELSIM_BIN)/vsim -c -do 'run -all;quit' red_pitaya_scope_tb
