set path_inf "../../../sim"

source "${path_inf}/axi4_if.tcl"
source "${path_inf}/axi4_lite_if.tcl"
source "${path_inf}/axi4_stream_if.tcl"
source "${path_inf}/sys_bus_if.tcl"

# set top hierarcy name
set top top_tb

onerror {resume}
quietly WaveActivateNextPane {} 0
# signals
add wave -noupdate /${top}/clk
add wave -noupdate /${top}/rstn
add wave -noupdate /${top}/led
add wave -noupdate /${top}/exp_p_io
add wave -noupdate /${top}/exp_n_io
# busses
#axi4_lite_if axi4_lite /${top}/top/ps/axi4_lite
axi4_if      axi_gp    /${top}/top/ps/axi_gp
sys_bus_if   ps_sys    /${top}/top/ps_sys

# LG
axi4_stream_if str_lgo /${top}/top/str_lgo
# LA
axi4_stream_if exp_exi /${top}/top/exp_exi
axi4_stream_if str_drx /${top}/top/str_drx

# LG/LA
#add wave -noupdate -group LG /${top}/top/lg/*
#add wave -noupdate -group LA /${top}/top/la/*

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
