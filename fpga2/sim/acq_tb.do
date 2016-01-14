onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /acq_tb/clk
add wave -noupdate /acq_tb/rstn
add wave -noupdate /acq_tb/acq/cts
add wave -noupdate /acq_tb/acq/ctl_rst
add wave -noupdate /acq_tb/acq/cfg_con
add wave -noupdate /acq_tb/acq/cfg_aut
add wave -noupdate /acq_tb/acq/cfg_pre
add wave -noupdate /acq_tb/acq/sts_pre
add wave -noupdate /acq_tb/acq/cfg_pst
add wave -noupdate /acq_tb/acq/sts_pst
add wave -noupdate /acq_tb/acq/ctl_acq
add wave -noupdate /acq_tb/acq/sts_acq
add wave -noupdate /acq_tb/acq/cts_acq
add wave -noupdate /acq_tb/acq/ctl_trg
add wave -noupdate /acq_tb/acq/sts_trg
add wave -noupdate /acq_tb/acq/cts_trg
add wave -noupdate /acq_tb/acq/ctl_stp
add wave -noupdate /acq_tb/acq/sts_stp
add wave -noupdate /acq_tb/acq/cts_stp
add wave -noupdate -expand -group sti /acq_tb/sti/*
add wave -noupdate -expand -group sti /acq_tb/str_src/str_trn
add wave -noupdate -expand -group sti /acq_tb/str_src/str_ena
add wave -noupdate -expand -group sti /acq_tb/str_src/buf_siz
add wave -noupdate -expand -group sto /acq_tb/sto/*
add wave -noupdate -expand -group sto /acq_tb/str_drn/str_trn
add wave -noupdate -expand -group sto /acq_tb/str_drn/str_ena
add wave -noupdate -expand -group sto /acq_tb/str_drn/buf_siz

# difine Radix
radix signal /acq_tb/acq/cts     -decimal -unsigned
radix signal /acq_tb/acq/cfg_pre -decimal -unsigned
radix signal /acq_tb/acq/sts_pre -decimal -unsigned
radix signal /acq_tb/acq/cfg_pst -decimal -unsigned
radix signal /acq_tb/acq/sts_pst -decimal -unsigned
radix signal /acq_tb/acq/cts_acq -decimal -unsigned
radix signal /acq_tb/acq/cts_trg -decimal -unsigned
radix signal /acq_tb/acq/cts_stp -decimal -unsigned
radix signal /acq_tb/sti/dat -hexadecimal
radix signal /acq_tb/sto/dat -hexadecimal
radix signal /acq_tb/str_src/buf_siz -decimal -unsigned
radix signal /acq_tb/str_drn/buf_siz -decimal -unsigned
#-fpoint

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
