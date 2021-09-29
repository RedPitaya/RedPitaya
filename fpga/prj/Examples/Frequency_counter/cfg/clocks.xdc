create_clock -period 8.000 -name adc_clk [get_ports adc_clk_p_i]

set_input_delay -max 3.400 -clock adc_clk [get_ports adc_dat_a_i[*]]
set_input_delay -max 3.400 -clock adc_clk [get_ports adc_dat_b_i[*]]

create_clock -period 4.000 -name rx_clk [get_ports daisy_p_i[1]]


#set_false_path -from [get_clocks clk_fpga_0] -to [get_clocks adc_clk]
#set_false_path -from [get_clocks adc_clk] -to [get_clocks clk_fpga_0]
