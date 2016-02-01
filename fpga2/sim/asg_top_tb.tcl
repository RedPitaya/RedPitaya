source "axi4_if.tcl"
source "axi4_lite_if.tcl"
source "axi4_stream_if.tcl"
source "str_bus_if.tcl"
source "sys_bus_if.tcl"

# set top hierarcy name
set top asg_top_tb
set dut asg_top

onerror {resume}
quietly WaveActivateNextPane {} 0

# system signals
add wave -noupdate /${top}/clk
add wave -noupdate /${top}/rstn

# configuration/control/status
add wave -noupdate           /${top}/asg_top/asg/ctl_rst
add wave -noupdate           /${top}/asg_top/asg/trg_i
add wave -noupdate           /${top}/asg_top/asg/trg_o
add wave -noupdate           /${top}/asg_top/asg/cfg_trg
add wave -noupdate -hex      /${top}/asg_top/asg/cfg_siz
add wave -noupdate -hex      /${top}/asg_top/asg/cfg_stp
add wave -noupdate -hex      /${top}/asg_top/asg/cfg_off
add wave -noupdate           /${top}/asg_top/asg/cfg_ben
add wave -noupdate           /${top}/asg_top/asg/cfg_inf
add wave -noupdate -unsigned /${top}/asg_top/asg/cfg_bdl
add wave -noupdate -unsigned /${top}/asg_top/asg/cfg_bil
add wave -noupdate -unsigned /${top}/asg_top/asg/cfg_bnm

# busses
str_bus_if str /asg_top_tb/str
sys_bus_if bus /asg_top_tb/bus

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
WaveRestoreZoom {
0 ps} {132300 ps}
