set path_inf "../../../sim"

source "${path_inf}/axi4_if.tcl"
source "${path_inf}/axi4_lite_if.tcl"
source "${path_inf}/axi4_stream_if.tcl"
source "${path_inf}/sys_bus_if.tcl"

# set top hierarcy name
set top top_tb
set dut top_tb/top

onerror {resume}
quietly WaveActivateNextPane {} 0
# IO signals
add wave -noupdate /${top}/clk
add wave -noupdate /${top}/rstn
add wave -noupdate /${top}/led
add wave -noupdate /${top}/exp_p_io
add wave -noupdate /${top}/exp_n_io
# busses
#axi4_lite_if axi4_lite /${dut}/ps/axi4_lite
axi4_if      axi_gp    /${dut}/ps/axi_gp
sys_bus_if   ps_sys    /${dut}/ps_sys

# streams
axi4_stream_if axi_dac_0 /${dut}/str_dac\[0\]
axi4_stream_if axi_dac_1 /${dut}/str_dac\[1\]
axi4_stream_if axi_gen_0 /${dut}/str_gen\[0\]
axi4_stream_if axi_gen_1 /${dut}/str_gen\[1\]
axi4_stream_if axi_osc_0 /${dut}/str_osc\[0\]
axi4_stream_if axi_osc_1 /${dut}/str_osc\[1\]
axi4_stream_if axi_adc_0 /${dut}/str_adc\[0\]
axi4_stream_if axi_adc_1 /${dut}/str_adc\[1\]
axi4_stream_if axi_lg    /${dut}/str_lg
axi4_stream_if axi_la    /${dut}/str_la

# interrupts and events
add wave -noupdate /${dut}/irq
add wave -noupdate /${dut}/evn
add wave -noupdate /${dut}/trg

# LG/LA
#add wave -noupdate -group LG /${dut}/lg/*
#add wave -noupdate -group LA /${dut}/la/*

TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {0 ps} 0}
quietly wave cursor active 0
configure wave -namecolwidth 204
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ps
update
WaveRestoreZoom {0 ps} {132300 ps}
