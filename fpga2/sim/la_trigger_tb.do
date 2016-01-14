onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /la_trigger_tb/clk
add wave -noupdate /la_trigger_tb/rstn
add wave -noupdate /la_trigger_tb/la_trigger/ctl_rst
add wave -noupdate /la_trigger_tb/la_trigger/cfg_old_val
add wave -noupdate /la_trigger_tb/la_trigger/cfg_old_msk
add wave -noupdate /la_trigger_tb/la_trigger/cfg_cur_val
add wave -noupdate /la_trigger_tb/la_trigger/cfg_cur_msk
add wave -noupdate /la_trigger_tb/la_trigger/sts_trg
add wave -noupdate -expand -group str /la_trigger_tb/str/*
add wave -noupdate -expand -group str /la_trigger_tb/str_src/str_trn
add wave -noupdate -expand -group str /la_trigger_tb/str_src/str_ena
add wave -noupdate -expand -group str /la_trigger_tb/str_src/buf_siz

# difine Radix
radix signal /la_trigger_tb/la_trigger/cfg_old_val -hexadecimal
radix signal /la_trigger_tb/la_trigger/cfg_old_msk -hexadecimal
radix signal /la_trigger_tb/la_trigger/cfg_cur_val -hexadecimal
radix signal /la_trigger_tb/la_trigger/cfg_cur_msk -hexadecimal
radix signal /la_trigger_tb/str/dat -hexadecimal
radix signal /la_trigger_tb/str_src/buf_siz -decimal -unsigned
radix signal /la_trigger_tb/str_drn/buf_siz -decimal -unsigned
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
