source "str_bus_if.tcl"
source "sys_bus_if.tcl"

onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /asg_top_tb/clk
add wave -noupdate /asg_top_tb/rstn

# busses
str_bus_if str /asg_top_tb/str
sys_bus_if ps_sys /asg_top_tb/bus

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
