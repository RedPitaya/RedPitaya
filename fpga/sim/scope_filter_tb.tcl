source "axi4_if.tcl"
source "axi4_lite_if.tcl"
source "axi4_stream_if.tcl"
source "sys_bus_if.tcl"

# set top hierarcy name
set top scope_filter_tb
set dut scope_filter

onerror {resume}
quietly WaveActivateNextPane {} 0

add wave -noupdate              /${top}/clk
add wave -noupdate              /${top}/rstn
add wave -noupdate -hexadecimal /${top}/${dut}/cfg_aa
add wave -noupdate -hexadecimal /${top}/${dut}/cfg_bb
add wave -noupdate -hexadecimal /${top}/${dut}/cfg_pp
add wave -noupdate -hexadecimal /${top}/${dut}/cfg_kk

# busses
axi4_stream_if sti /${top}/sti
axi4_stream_if sto /${top}/sto

add wave vsim:/scope_filter_tb/dfilt1/r2_reg
add wave vsim:/scope_filter_tb/scope_filter/fir0/TDATA
add wave vsim:/scope_filter_tb/scope_filter/firm/TDATA
add wave vsim:/scope_filter_tb/scope_filter/fir_buf
add wave vsim:/scope_filter_tb/scope_filter/fir_cfg
add wave vsim:/scope_filter_tb/dfilt1/r1_reg
add wave vsim:/scope_filter_tb/dfilt1/r01_reg
add wave vsim:/scope_filter_tb/dfilt1/r2_reg
add wave vsim:/scope_filter_tb/dfilt1/r02_reg
add wave vsim:/scope_filter_tb/dfilt1/r2_sum

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
