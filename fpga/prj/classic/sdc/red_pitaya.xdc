set_property LOC XADC_X0Y0 [get_cells i_ams/XADC_inst]

############################################################################
# Clock constraints                                                        #
############################################################################

set_false_path -from [get_clocks adc_clk]     -to [get_clocks dac_clk_out]
set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks ser_clk_out]
set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks dac_2clk_out]
set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks adc_clk]
set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks par_clk]
set_false_path -from [get_clocks dac_clk_out] -to [get_clocks dac_2clk_out]
set_false_path -from [get_clocks dac_clk_out] -to [get_clocks dac_2ph_out]


### SATA connector
set_property IOSTANDARD LVCMOS18 [get_ports {daisy_p_o[*]}]
set_property IOSTANDARD LVCMOS18 [get_ports {daisy_n_o[*]}]
set_property IOSTANDARD LVCMOS18 [get_ports {daisy_p_i[*]}]
set_property IOSTANDARD LVCMOS18 [get_ports {daisy_n_i[*]}]