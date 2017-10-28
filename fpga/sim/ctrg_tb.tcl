source "axi4_if.tcl"
source "axi4_lite_if.tcl"
source "axi4_stream_if.tcl"
source "sys_bus_if.tcl"

# set top hierarcy name
set top ctrg_tb
set dut ctrg

onerror {resume}
quietly WaveActivateNextPane {} 0

# system signals
add wave -noupdate /${top}/clk
add wave -noupdate /${top}/rstn

# events
add wave -noupdate           /${top}/${dut}/evi
add wave -noupdate           /${top}/${dut}/evo
# control/status
add wave -noupdate           /${top}/${dut}/trg
add wave -noupdate           /${top}/${dut}/tro
add wave -noupdate           /${top}/${dut}/ctl_trg
# events configuration
add wave -noupdate -bin      /${top}/${dut}/cfg_evn
add wave -noupdate -bin      /${top}/${dut}/cfg_trg

# busses
sys_bus_if bus /${top}/bus

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
