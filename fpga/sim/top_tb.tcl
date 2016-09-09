source "axi4_if.tcl"
source "axi4_lite_if.tcl"
source "axi4_stream_if.tcl"
source "sys_bus_if.tcl"

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
sys_bus_if   sys_lg    /${top}/top/sys\[11\]
sys_bus_if   sys_la    /${top}/top/sys\[12\]

axi4_stream_if str_lgo   /${top}/top/str_lgo

axi4_stream_if axi_drx_0 /${top}/top/axi_drx\[0\]
axi4_stream_if axi_drx_1 /${top}/top/axi_drx\[1\]
axi4_stream_if axi_drx_2 /${top}/top/axi_drx\[2\]
axi4_stream_if axi_drx_3 /${top}/top/axi_drx\[3\]
axi4_stream_if axi_dtx_0 /${top}/top/axi_dtx\[0\]
axi4_stream_if axi_dtx_1 /${top}/top/axi_dtx\[1\]
axi4_stream_if axi_dtx_2 /${top}/top/axi_dtx\[2\]
axi4_stream_if axi_dtx_3 /${top}/top/axi_dtx\[3\]

axi4_stream_if axi_exe_0 /${top}/top/axi_exe\[0\]
axi4_stream_if axi_exe_1 /${top}/top/axi_exe\[1\]
axi4_stream_if axi_exo_0 /${top}/top/axi_exo\[0\]
axi4_stream_if axi_exo_1 /${top}/top/axi_exo\[1\]
axi4_stream_if axi_exi_0 /${top}/top/axi_exi\[0\]
axi4_stream_if axi_exi_1 /${top}/top/axi_exi\[1\]

axi4_stream_if exp_exe /${top}/top/exp_exe
axi4_stream_if exp_exo /${top}/top/exp_exo
axi4_stream_if exp_exi /${top}/top/exp_exi

# LG/LA
add wave -noupdate -group LG /${top}/top/lg/*
add wave -noupdate -group LA /${top}/top/la/*

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
