source "axi4_if.tcl"
source "axi4_lite_if.tcl"
source "axi4_stream_if.tcl"
source "sys_bus_if.tcl"

# set top hierarcy name
set top la_trigger_tb
set dut la_trigger

onerror {resume}
quietly WaveActivateNextPane {} 0

add wave -noupdate              /${top}/clk
add wave -noupdate              /${top}/rstn
add wave -noupdate              /${top}/${dut}/ctl_rst
add wave -noupdate -hexadecimal /${top}/${dut}/cfg_cmp_msk
add wave -noupdate -hexadecimal /${top}/${dut}/cfg_cmp_val
add wave -noupdate -hexadecimal /${top}/${dut}/cfg_edg_pos
add wave -noupdate -hexadecimal /${top}/${dut}/cfg_edg_neg
add wave -noupdate              /${top}/${dut}/sts_trg

# busses
axi4_stream_if sti /${top}/sti
axi4_stream_if sto /${top}/sto

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
