source "axi4_if.tcl"
source "axi4_lite_if.tcl"
source "axi4_stream_if.tcl"
source "sys_bus_if.tcl"

# set top hierarcy name
set top gen_tb
set dut gen

onerror {resume}
quietly WaveActivateNextPane {} 0

# system signals
add wave -noupdate /${top}/clk
add wave -noupdate /${top}/rstn

# events
add wave -noupdate           /${top}/${dut}/evi
add wave -noupdate           /${top}/${dut}/evo
# control/status
add wave -noupdate           /${top}/${dut}/ctl_rst
add wave -noupdate           /${top}/${dut}/ctl_str
add wave -noupdate           /${top}/${dut}/sts_str
add wave -noupdate           /${top}/${dut}/ctl_stp
add wave -noupdate           /${top}/${dut}/sts_stp
add wave -noupdate           /${top}/${dut}/ctl_trg
add wave -noupdate           /${top}/${dut}/sts_trg
# event control/configuration
add wave -noupdate -bin      /${top}/${dut}/cfg_rst
add wave -noupdate -bin      /${top}/${dut}/cfg_str
add wave -noupdate -bin      /${top}/${dut}/cfg_stp
add wave -noupdate -bin      /${top}/${dut}/cfg_trg
# frequency/phase
add wave -noupdate -hex      /${top}/${dut}/cfg_siz
add wave -noupdate -hex      /${top}/${dut}/cfg_off
add wave -noupdate -hex      /${top}/${dut}/cfg_ste
# burst configuration
add wave -noupdate           /${top}/${dut}/cfg_ben
add wave -noupdate           /${top}/${dut}/cfg_inf
add wave -noupdate -unsigned /${top}/${dut}/cfg_bdl
add wave -noupdate -unsigned /${top}/${dut}/cfg_bln
add wave -noupdate -unsigned /${top}/${dut}/cfg_bnm
# burst status
add wave -noupdate -unsigned /${top}/${dut}/sts_bln
add wave -noupdate -unsigned /${top}/${dut}/sts_bnm
# offset and amplitude
add wave -noupdate -unsigned /${top}/${dut}/cfg_mul
add wave -noupdate -unsigned /${top}/${dut}/cfg_sum

# counter end status
add wave -noupdate           /${top}/${dut}/asg/end_bdl
add wave -noupdate           /${top}/${dut}/asg/end_bln
add wave -noupdate           /${top}/${dut}/asg/end_bnm
# status
add wave -noupdate           /${top}/${dut}/asg/sts_adr
add wave -noupdate           /${top}/${dut}/asg/sts_vld
add wave -noupdate           /${top}/${dut}/asg/sts_lst
add wave -noupdate           /${top}/${dut}/asg/sts_rdy
# events
add wave -noupdate           /${top}/${dut}/asg/ctl_run
add wave -noupdate           /${top}/${dut}/asg/ctl_end
add wave -noupdate           /${top}/${dut}/asg/ctl_rpt

# busses
axi4_stream_if stg     /${top}/${dut}/stg
axi4_stream_if str     /${top}/str
sys_bus_if     bus     /${top}/bus
sys_bus_if     bus_tbl /${top}/bus_tbl

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
