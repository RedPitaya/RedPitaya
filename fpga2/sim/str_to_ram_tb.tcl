source "axi4_if.tcl"
source "axi4_lite_if.tcl"
source "axi4_stream_if.tcl"
source "sys_bus_if.tcl"

# set top hierarcy name
set top str_to_ram_tb
set dut str_to_ram

onerror {resume}
quietly WaveActivateNextPane {} 0

# system signals
add wave -noupdate /${top}/clk
add wave -noupdate /${top}/rstn

# configuration/control/status
add wave -noupdate           /${top}/${dut}/ctl_rst
add wave -noupdate           /${top}/${dut}/buf_mem
add wave -noupdate           /${top}/${dut}/buf_wen
add wave -noupdate -hex      /${top}/${dut}/buf_wdata
add wave -noupdate -hex      /${top}/${dut}/buf_waddr
add wave -noupdate           /${top}/${dut}/buf_ren
add wave -noupdate -hex      /${top}/${dut}/buf_rdata
add wave -noupdate -hex      /${top}/${dut}/buf_raddr

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
