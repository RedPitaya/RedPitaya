onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /asg_tb/clk
add wave -noupdate /asg_tb/rstn
add wave -noupdate /asg_tb/asg/ctl_rst
add wave -noupdate /asg_tb/asg/trg_i
add wave -noupdate /asg_tb/asg/trg_o
add wave -noupdate /asg_tb/asg/cfg_trg
add wave -noupdate /asg_tb/asg/cfg_siz
add wave -noupdate /asg_tb/asg/cfg_stp
add wave -noupdate /asg_tb/asg/cfg_off
add wave -noupdate /asg_tb/asg/cfg_ben
add wave -noupdate /asg_tb/asg/cfg_inf
add wave -noupdate /asg_tb/asg/cfg_bdl
add wave -noupdate /asg_tb/asg/cfg_bil
add wave -noupdate /asg_tb/asg/cfg_bnm
add wave -noupdate -expand -group str /asg_tb/str/*
add wave -noupdate -expand -group str /asg_tb/str_drn/str_trn
add wave -noupdate -expand -group str /asg_tb/str_drn/str_ena
add wave -noupdate -expand -group str /asg_tb/str_drn/buf_siz

# difine Radix
radix signal /asg_tb/asg/cfg_siz -decimal -unsigned
radix signal /asg_tb/asg/sts_stp -decimal -unsigned
radix signal /asg_tb/asg/cfg_off -decimal -unsigned
radix signal /asg_tb/asg/sts_bdl -decimal -unsigned
radix signal /asg_tb/asg/cts_bil -decimal -unsigned
radix signal /asg_tb/asg/cts_bnm -decimal -unsigned
radix signal /asg_tb/str/dat -hexadecimal
radix signal /asg_tb/str_drn/buf_siz -decimal -unsigned
#-fpoint

TreeUpdate [SetDefaultTree]
WaveRestrreCursors {{Cursor 1} {0 ps} 0}
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
WaveRestrreZoom {0 ps} {132300 ps}
