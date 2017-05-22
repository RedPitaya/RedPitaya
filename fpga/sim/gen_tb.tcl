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
add wave -noupdate           /${top}/${dut}/trg
add wave -noupdate           /${top}/${dut}/tro
add wave -noupdate           /${top}/${dut}/ctl_trg
# events configuration
add wave -noupdate -bin      /${top}/${dut}/cfg_evn
add wave -noupdate -bin      /${top}/${dut}/cfg_trg
# frequency/phase
add wave -noupdate -hex      /${top}/${dut}/cfg_siz
add wave -noupdate -hex      /${top}/${dut}/cfg_off
add wave -noupdate -hex      /${top}/${dut}/cfg_ste
# generator mode
add wave -noupdate           /${top}/${dut}/cfg_ben
add wave -noupdate           /${top}/${dut}/cfg_inf
# burst configuration
add wave -noupdate -unsigned /${top}/${dut}/cfg_bdr
add wave -noupdate -unsigned /${top}/${dut}/cfg_bdl
add wave -noupdate -unsigned /${top}/${dut}/cfg_bpl
add wave -noupdate -unsigned /${top}/${dut}/cfg_bpn
# burst status
add wave -noupdate -unsigned /${top}/${dut}/sts_bpl
add wave -noupdate -unsigned /${top}/${dut}/sts_bpn
# offset and amplitude
add wave -noupdate -unsigned /${top}/${dut}/cfg_mul
add wave -noupdate -unsigned /${top}/${dut}/cfg_sum

# buffer signals
add wave -noupdate -group buffer -hex      /${top}/${dut}/asg/buf_rdata
add wave -noupdate -group buffer -hex      /${top}/${dut}/asg/buf_raddr
add wave -noupdate -group buffer -hex      /${top}/${dut}/asg/buf_ptr
add wave -noupdate -group buffer           /${top}/${dut}/asg/buf_adr_vld
add wave -noupdate -group buffer           /${top}/${dut}/asg/buf_adr_lst
add wave -noupdate -group buffer           /${top}/${dut}/asg/sts_adr
add wave -noupdate -group buffer           /${top}/${dut}/asg/sts_rdy

# common signals
add wave -noupdate -group common           /${top}/${dut}/asg/ctl_run
add wave -noupdate -group common           /${top}/${dut}/asg/ctl_end

# continuous/periodic engine signals
add wave -noupdate -group period           /${top}/${dut}/asg/sts_adr_per
add wave -noupdate -group period           /${top}/${dut}/asg/ctl_end_per
add wave -noupdate -group period -hex      /${top}/${dut}/asg/ptr_cur
add wave -noupdate -group period -hex      /${top}/${dut}/asg/ptr_nxt
add wave -noupdate -group period -hex      /${top}/${dut}/asg/ptr_nxt_sub
add wave -noupdate -group period           /${top}/${dut}/asg/ptr_nxt_sub_neg

# burst engine signals
add wave -noupdate -group burst            /${top}/${dut}/asg/sts_adr_bst
add wave -noupdate -group burst            /${top}/${dut}/asg/ctl_end_bst
add wave -noupdate -group burst  -hex      /${top}/${dut}/asg/sts_bdr
add wave -noupdate -group burst  -hex      /${top}/${dut}/asg/sts_bdl
add wave -noupdate -group burst  -hex      /${top}/${dut}/asg/sts_bpl
add wave -noupdate -group burst  -hex      /${top}/${dut}/asg/sts_bpn
add wave -noupdate -group burst            /${top}/${dut}/asg/end_bdr
add wave -noupdate -group burst            /${top}/${dut}/asg/end_bdl
add wave -noupdate -group burst            /${top}/${dut}/asg/end_bpl
add wave -noupdate -group burst            /${top}/${dut}/asg/end_bpn

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
