source "axi4_if.tcl"
source "axi4_lite_if.tcl"
source "axi4_stream_if.tcl"
source "sys_bus_if.tcl"

# set top hierarcy name
set top asg_tb
set dut asg

onerror {resume}
quietly WaveActivateNextPane {} 0

add wave -noupdate           /${top}/clk
add wave -noupdate           /${top}/rstn
add wave -noupdate           /${top}/asg/ctl_rst
add wave -noupdate           /${top}/asg/trg_i
add wave -noupdate           /${top}/asg/trg_o
add wave -noupdate           /${top}/asg/cfg_trg
add wave -noupdate -hex      /${top}/asg/cfg_siz
add wave -noupdate -hex      /${top}/asg/cfg_stp
add wave -noupdate -hex      /${top}/asg/cfg_off
add wave -noupdate           /${top}/asg/cfg_ben
add wave -noupdate           /${top}/asg/cfg_inf
add wave -noupdate -unsigned /${top}/asg/cfg_bdl
add wave -noupdate -unsigned /${top}/asg/cfg_bln
add wave -noupdate -unsigned /${top}/asg/cfg_bnm
add wave -noupdate -expand -group str /${top}/str_drn/str_ena
add wave -noupdate -expand -group str -unsigned /${top}/str_drn/buf_siz

# busses
axi4_stream_if str /${top}/str
sys_bus_if     bus /${top}/bus

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
