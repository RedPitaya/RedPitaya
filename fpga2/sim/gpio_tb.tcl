onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /gpio_tb/clk
add wave -noupdate /gpio_tb/rstn
add wave -noupdate /gpio_tb/gpio_e
add wave -noupdate /gpio_tb/gpio_o
add wave -noupdate /gpio_tb/gpio_io
# AXI4-Lite bus
add wave -noupdate -expand -group axi4_lite -group param /gpio_tb/axi4_lite/AW
add wave -noupdate -expand -group axi4_lite -group param /gpio_tb/axi4_lite/DW
add wave -noupdate -expand -group axi4_lite -group param /gpio_tb/axi4_lite/SW
add wave -noupdate -expand -group axi4_lite -group AW    /gpio_tb/axi4_lite/AW*
add wave -noupdate -expand -group axi4_lite -group  W    /gpio_tb/axi4_lite/W*
add wave -noupdate -expand -group axi4_lite -group  B    /gpio_tb/axi4_lite/B*
add wave -noupdate -expand -group axi4_lite -group AR    /gpio_tb/axi4_lite/AR*
add wave -noupdate -expand -group axi4_lite -group  R    /gpio_tb/axi4_lite/R*

# difine Radix
radix signal /gpio_tb/axi4_lite/AW -decimal -unsigned
radix signal /gpio_tb/axi4_lite/DW -decimal -unsigned
radix signal /gpio_tb/axi4_lite/SW -decimal -unsigned
radix signal /gpio_tb/axi4_lite/AWADDR -hexadecimal
radix signal /gpio_tb/axi4_lite/WDATA  -hexadecimal
radix signal /gpio_tb/axi4_lite/ARADDR -hexadecimal
radix signal /gpio_tb/axi4_lite/RDATA  -hexadecimal
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
