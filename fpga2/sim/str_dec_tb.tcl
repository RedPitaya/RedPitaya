onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /str_dec_tb/clk
add wave -noupdate /str_dec_tb/rstn
add wave -noupdate /str_dec_tb/str_dec/ctl_rst
add wave -noupdate /str_dec_tb/str_dec/cfg_dec
add wave -noupdate -expand -group sti /str_dec_tb/sti/*
add wave -noupdate -expand -group sti /str_dec_tb/str_src/str_trn
add wave -noupdate -expand -group sti /str_dec_tb/str_src/str_ena
add wave -noupdate -expand -group sti /str_dec_tb/str_src/buf_siz
add wave -noupdate -expand -group sto /str_dec_tb/sto/*
add wave -noupdate -expand -group sto /str_dec_tb/str_drn/str_trn
add wave -noupdate -expand -group sto /str_dec_tb/str_drn/str_ena
add wave -noupdate -expand -group sto /str_dec_tb/str_drn/buf_siz

# difine Radix
radix signal /str_dec_tb/str_dec/cfg_dec -decimal -unsigned
radix signal /str_dec_tb/str_src/buf_siz -decimal -unsigned
radix signal /str_dec_tb/str_drn/buf_siz -decimal -unsigned
radix signal /str_dec_tb/sti/dat -hexadecimal
radix signal /str_dec_tb/sto/dat -hexadecimal
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
