onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /top_tb/clk
add wave -noupdate /top_tb/rstn
add wave -noupdate /top_tb/led
# AXI bus
add wave -noupdate -expand -group axi_gp -group param /top_tb/top/ps/axi_gp/AW
add wave -noupdate -expand -group axi_gp -group param /top_tb/top/ps/axi_gp/DW
add wave -noupdate -expand -group axi_gp -group param /top_tb/top/ps/axi_gp/SW
add wave -noupdate -expand -group axi_gp -group param /top_tb/top/ps/axi_gp/IW
add wave -noupdate -expand -group axi_gp -group param /top_tb/top/ps/axi_gp/LW
add wave -noupdate -expand -group axi_gp -group AW    /top_tb/top/ps/axi_gp/AW*
add wave -noupdate -expand -group axi_gp -group  W    /top_tb/top/ps/axi_gp/W*
add wave -noupdate -expand -group axi_gp -group  B    /top_tb/top/ps/axi_gp/B*
add wave -noupdate -expand -group axi_gp -group AR    /top_tb/top/ps/axi_gp/AR*
add wave -noupdate -expand -group axi_gp -group  R    /top_tb/top/ps/axi_gp/R*
# AXI4-Lite bus
add wave -noupdate -expand -group axi4_lite -group param /top_tb/top/ps/axi4_lite/AW
add wave -noupdate -expand -group axi4_lite -group param /top_tb/top/ps/axi4_lite/DW
add wave -noupdate -expand -group axi4_lite -group param /top_tb/top/ps/axi4_lite/SW
add wave -noupdate -expand -group axi4_lite -group AW    /top_tb/top/ps/axi4_lite/AW*
add wave -noupdate -expand -group axi4_lite -group  W    /top_tb/top/ps/axi4_lite/W*
add wave -noupdate -expand -group axi4_lite -group  B    /top_tb/top/ps/axi4_lite/B*
add wave -noupdate -expand -group axi4_lite -group AR    /top_tb/top/ps/axi4_lite/AR*
add wave -noupdate -expand -group axi4_lite -group  R    /top_tb/top/ps/axi4_lite/R*
# SYS bus
add wave -noupdate -expand -group ps_sys /top_tb/top/ps_sys/*

# difine Radix
radix signal /top_tb/top/ps_sys/addr  -hexadecimal
radix signal /top_tb/top/ps_sys/wdata -hexadecimal
radix signal /top_tb/top/ps_sys/rdata -hexadecimal
radix signal /top_tb/top/ps_sys/*W -decimal -unsigned
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
