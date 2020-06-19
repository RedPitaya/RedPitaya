#
# $Id: red_pitaya.xdc 961 2014-01-21 11:40:39Z matej.oblak $
#
# @brief Red Pitaya location constraints.
#
# @Author Matej Oblak
#
# (c) Red Pitaya  http://www.redpitaya.com
#

############################################################################
# IO constraints                                                           #
############################################################################

### ADC

# ADC data
set_property IOSTANDARD DIFF_SSTL18_II [get_ports {adc_dat_?_i[*][*]}]
set_property IOB TRUE [get_ports {adc_dat_?_i[*][*]}]
#set_property DIFF_TERM TRUE [get_ports {adc_dat_?_i[*][*]}]

# ADC 0 data
set_property PACKAGE_PIN W18 [get_ports {adc_dat_p_i[0][0]}] ; # ADAxP
set_property PACKAGE_PIN W19 [get_ports {adc_dat_n_i[0][0]}] ; # ADAxN
set_property PACKAGE_PIN U13 [get_ports {adc_dat_p_i[0][1]}] ; # ADA0P
set_property PACKAGE_PIN V13 [get_ports {adc_dat_n_i[0][1]}] ; # ADA0N
set_property PACKAGE_PIN Y16 [get_ports {adc_dat_p_i[0][2]}] ; # ADA2P
set_property PACKAGE_PIN Y17 [get_ports {adc_dat_n_i[0][2]}] ; # ADA2N
set_property PACKAGE_PIN V12 [get_ports {adc_dat_p_i[0][3]}] ; # ADA4P
set_property PACKAGE_PIN W13 [get_ports {adc_dat_n_i[0][3]}] ; # ADA4N
set_property PACKAGE_PIN T11 [get_ports {adc_dat_p_i[0][4]}] ; # ADA6P
set_property PACKAGE_PIN T10 [get_ports {adc_dat_n_i[0][4]}] ; # ADA6N
set_property PACKAGE_PIN V16 [get_ports {adc_dat_p_i[0][5]}] ; # ADA8P
set_property PACKAGE_PIN W16 [get_ports {adc_dat_n_i[0][5]}] ; # ADA8N
set_property PACKAGE_PIN T20 [get_ports {adc_dat_p_i[0][6]}] ; # ADA10P
set_property PACKAGE_PIN U20 [get_ports {adc_dat_n_i[0][6]}] ; # ADA10N

# ADC 1 data
set_property PACKAGE_PIN N17 [get_ports {adc_dat_p_i[1][0]}] ; # ADBxP
set_property PACKAGE_PIN P18 [get_ports {adc_dat_n_i[1][0]}] ; # ADBxN
set_property PACKAGE_PIN T17 [get_ports {adc_dat_p_i[1][1]}] ; # ADB0P
set_property PACKAGE_PIN R18 [get_ports {adc_dat_n_i[1][1]}] ; # ADB0N
set_property PACKAGE_PIN T16 [get_ports {adc_dat_p_i[1][2]}] ; # ADB2P
set_property PACKAGE_PIN U17 [get_ports {adc_dat_n_i[1][2]}] ; # ADB2N
set_property PACKAGE_PIN V20 [get_ports {adc_dat_p_i[1][3]}] ; # ADB4P
set_property PACKAGE_PIN W20 [get_ports {adc_dat_n_i[1][3]}] ; # ADB4N
set_property PACKAGE_PIN Y18 [get_ports {adc_dat_p_i[1][4]}] ; # ADB6P
set_property PACKAGE_PIN Y19 [get_ports {adc_dat_n_i[1][4]}] ; # ADB6N
set_property PACKAGE_PIN T14 [get_ports {adc_dat_p_i[1][5]}] ; # ADB8P
set_property PACKAGE_PIN T15 [get_ports {adc_dat_n_i[1][5]}] ; # ADB8N
set_property PACKAGE_PIN V15 [get_ports {adc_dat_p_i[1][6]}] ; # ADB10P
set_property PACKAGE_PIN W15 [get_ports {adc_dat_n_i[1][6]}] ; # ADB10N


set_property IOSTANDARD DIFF_SSTL18_II [get_ports {adc_clk_i[*]}]
#set_property DIFF_TERM TRUE [get_ports {adc_clk_i[*]}]
set_property PACKAGE_PIN U18 [get_ports {adc_clk_i[1]}] ; # ADCLKP
set_property PACKAGE_PIN U19 [get_ports {adc_clk_i[0]}] ; # ADCLKN

# ADC SPI
set_property IOSTANDARD LVCMOS18 [get_ports adc_spi_*]
set_property SLEW SLOW [get_ports adc_spi_*]
set_property DRIVE 8 [get_ports adc_spi_*]
set_property PACKAGE_PIN Y14 [get_ports adc_spi_csb]
set_property PACKAGE_PIN V18 [get_ports adc_spi_sdio]
set_property PACKAGE_PIN W14 [get_ports adc_spi_clk]

# ADC SYNC
set_property IOSTANDARD LVCMOS18 [get_ports adc_sync_o]
set_property SLEW SLOW [get_ports adc_sync_o]
set_property DRIVE 8 [get_ports adc_sync_o]
set_property PACKAGE_PIN P15 [get_ports adc_sync_o]




### DAC

set_property IOSTANDARD LVCMOS33 [get_ports dac_dco_i]
set_property PACKAGE_PIN L16 [get_ports dac_dco_i]

set_property IOSTANDARD LVCMOS33 [get_ports dac_reset_o]
set_property PACKAGE_PIN F16 [get_ports dac_reset_o]

# data
set_property IOSTANDARD LVCMOS33 [get_ports {dac_dat_o[*][*]}]
set_property SLEW FAST [get_ports {dac_dat_o[*][*]}]
set_property DRIVE 8 [get_ports {dac_dat_o[*][*]}]
set_property IOB TRUE [get_ports {dac_dat_o[*]}]

set_property PACKAGE_PIN L19 [get_ports {dac_dat_o[0][0]}]
set_property PACKAGE_PIN L20 [get_ports {dac_dat_o[0][1]}]
set_property PACKAGE_PIN K19 [get_ports {dac_dat_o[0][2]}]
set_property PACKAGE_PIN J19 [get_ports {dac_dat_o[0][3]}]
set_property PACKAGE_PIN J20 [get_ports {dac_dat_o[0][4]}]
set_property PACKAGE_PIN J18 [get_ports {dac_dat_o[0][5]}]
set_property PACKAGE_PIN H20 [get_ports {dac_dat_o[0][6]}]
set_property PACKAGE_PIN G19 [get_ports {dac_dat_o[0][7]}]
set_property PACKAGE_PIN G20 [get_ports {dac_dat_o[0][8]}]
set_property PACKAGE_PIN F17 [get_ports {dac_dat_o[0][9]}]
set_property PACKAGE_PIN F20 [get_ports {dac_dat_o[0][10]}]
set_property PACKAGE_PIN F19 [get_ports {dac_dat_o[0][11]}]
set_property PACKAGE_PIN D20 [get_ports {dac_dat_o[0][12]}]
set_property PACKAGE_PIN D19 [get_ports {dac_dat_o[0][13]}]

set_property PACKAGE_PIN G18 [get_ports {dac_dat_o[1][0]}]
set_property PACKAGE_PIN G17 [get_ports {dac_dat_o[1][1]}]
set_property PACKAGE_PIN H17 [get_ports {dac_dat_o[1][2]}]
set_property PACKAGE_PIN H18 [get_ports {dac_dat_o[1][3]}]
set_property PACKAGE_PIN J16 [get_ports {dac_dat_o[1][4]}]
set_property PACKAGE_PIN K16 [get_ports {dac_dat_o[1][5]}]
set_property PACKAGE_PIN K17 [get_ports {dac_dat_o[1][6]}]
set_property PACKAGE_PIN L15 [get_ports {dac_dat_o[1][7]}]
set_property PACKAGE_PIN M20 [get_ports {dac_dat_o[1][8]}]
set_property PACKAGE_PIN M19 [get_ports {dac_dat_o[1][9]}]
set_property PACKAGE_PIN M17 [get_ports {dac_dat_o[1][10]}]
set_property PACKAGE_PIN M18 [get_ports {dac_dat_o[1][11]}]
set_property PACKAGE_PIN L17 [get_ports {dac_dat_o[1][12]}]
set_property PACKAGE_PIN K18 [get_ports {dac_dat_o[1][13]}]

# DAC SPI
set_property IOSTANDARD LVCMOS33 [get_ports dac_spi_*]
set_property SLEW SLOW [get_ports dac_spi_*]
set_property DRIVE 8 [get_ports dac_spi_*]
set_property PACKAGE_PIN G14 [get_ports dac_spi_csb]
set_property PACKAGE_PIN H16 [get_ports dac_spi_sdio]
set_property PACKAGE_PIN G15 [get_ports dac_spi_clk]


### PWM DAC
set_property IOSTANDARD LVCMOS18 [get_ports {dac_pwm_o[*]}]
set_property SLEW FAST [get_ports {dac_pwm_o[*]}]
set_property DRIVE 12 [get_ports {dac_pwm_o[*]}]
set_property IOB TRUE [get_ports {dac_pwm_o[*]}]

set_property PACKAGE_PIN P20 [get_ports {dac_pwm_o[0]}]
set_property PACKAGE_PIN P16 [get_ports {dac_pwm_o[1]}]
set_property PACKAGE_PIN R17 [get_ports {dac_pwm_o[2]}]
set_property PACKAGE_PIN R19 [get_ports {dac_pwm_o[3]}]

### XADC
set_property IOSTANDARD LVCMOS33 [get_ports {vinp_i[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports {vinn_i[*]}]
#AD0
#AD1
#AD8
#AD9
#V_0
set_property PACKAGE_PIN E18 [get_ports {vinp_i[3]}]
set_property PACKAGE_PIN E19 [get_ports {vinn_i[3]}]
set_property PACKAGE_PIN E17 [get_ports {vinp_i[2]}]
set_property PACKAGE_PIN D18 [get_ports {vinn_i[2]}]
set_property PACKAGE_PIN C20 [get_ports {vinp_i[1]}]
set_property PACKAGE_PIN B20 [get_ports {vinn_i[1]}]
set_property PACKAGE_PIN L10 [get_ports {vinn_i[4]}]
set_property PACKAGE_PIN B19 [get_ports {vinp_i[0]}]
set_property PACKAGE_PIN A20 [get_ports {vinn_i[0]}]
set_property PACKAGE_PIN K9 [get_ports {vinp_i[4]}]

### Trigger
set_property IOSTANDARD LVCMOS18 [get_ports trig_i]
set_property PACKAGE_PIN N20 [get_ports trig_i]

### PLL
set_property IOSTANDARD LVCMOS33 [get_ports pll_*]
set_property PACKAGE_PIN U7 [get_ports pll_ref_i]
set_property PACKAGE_PIN V6 [get_ports pll_hi_o]
set_property PACKAGE_PIN V5 [get_ports pll_lo_o]

### Temperature protection
set_property IOSTANDARD LVCMOS33 [get_ports {temp_prot_i[*]}]
set_property PACKAGE_PIN W6 [get_ports {temp_prot_i[0]}]
set_property PACKAGE_PIN V7 [get_ports {temp_prot_i[1]}]


### Expansion connector
set_property IOSTANDARD LVCMOS33 [get_ports {exp_p_io[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports {exp_n_io[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports {exp_9_io}]
set_property SLEW FAST [get_ports {exp_p_io[*]}]
set_property SLEW FAST [get_ports {exp_n_io[*]}]
set_property SLEW FAST [get_ports {exp_9_io}]
set_property DRIVE 8 [get_ports {exp_p_io[*]}]
set_property DRIVE 8 [get_ports {exp_n_io[*]}]
set_property DRIVE 8 [get_ports {exp_9_io}]

set_property PACKAGE_PIN W10 [get_ports {exp_p_io[0]}]
set_property PACKAGE_PIN W9 [get_ports {exp_n_io[0]}]
set_property PACKAGE_PIN T9 [get_ports {exp_p_io[1]}]
set_property PACKAGE_PIN U10 [get_ports {exp_n_io[1]}]
set_property PACKAGE_PIN Y9 [get_ports {exp_p_io[2]}]
set_property PACKAGE_PIN Y8 [get_ports {exp_n_io[2]}]
set_property PACKAGE_PIN U9 [get_ports {exp_p_io[3]}]
set_property PACKAGE_PIN U8 [get_ports {exp_n_io[3]}]
set_property PACKAGE_PIN V8 [get_ports {exp_p_io[4]}]
set_property PACKAGE_PIN W8 [get_ports {exp_n_io[4]}]
set_property PACKAGE_PIN V11 [get_ports {exp_p_io[5]}]
set_property PACKAGE_PIN V10 [get_ports {exp_n_io[5]}]
set_property PACKAGE_PIN W11 [get_ports {exp_p_io[6]}]
set_property PACKAGE_PIN Y11 [get_ports {exp_n_io[6]}]
set_property PACKAGE_PIN Y12 [get_ports {exp_p_io[7]}]
set_property PACKAGE_PIN Y13 [get_ports {exp_n_io[7]}]
set_property PACKAGE_PIN Y7  [get_ports {exp_p_io[8]}]
set_property PACKAGE_PIN Y6  [get_ports {exp_n_io[8]}]
set_property PACKAGE_PIN U5  [get_ports {exp_9_io}]

#set_property PULLDOWN TRUE [get_ports {exp_p_io[0]}]
#set_property PULLDOWN TRUE [get_ports {exp_n_io[0]}]
#set_property PULLUP   TRUE [get_ports {exp_p_io[7]}]
#set_property PULLUP   TRUE [get_ports {exp_n_io[7]}]

### SATA connector
#set_property IOSTANDARD LVCMOS18 [get_ports {daisy_p_o[*]}]
#set_property IOSTANDARD LVCMOS18 [get_ports {daisy_n_o[*]}]
#set_property IOSTANDARD LVCMOS18 [get_ports {daisy_p_i[*]}]
#set_property IOSTANDARD LVCMOS18 [get_ports {daisy_n_i[*]}]

set_property PACKAGE_PIN T12 [get_ports {daisy_p_o[0]}]
set_property PACKAGE_PIN U12 [get_ports {daisy_n_o[0]}]
set_property PACKAGE_PIN U14 [get_ports {daisy_p_o[1]}]
set_property PACKAGE_PIN U15 [get_ports {daisy_n_o[1]}]
set_property PACKAGE_PIN P14 [get_ports {daisy_p_i[0]}]
set_property PACKAGE_PIN R14 [get_ports {daisy_n_i[0]}]
set_property PACKAGE_PIN N18 [get_ports {daisy_p_i[1]}]
set_property PACKAGE_PIN P19 [get_ports {daisy_n_i[1]}]

### LED
set_property IOSTANDARD LVCMOS33 [get_ports {led_o[*]}]
set_property SLEW SLOW [get_ports {led_o[*]}]
set_property DRIVE 4 [get_ports {led_o[*]}]

set_property PACKAGE_PIN K14 [get_ports {led_o[0]}] ; # 0-3 -> 4:7
set_property PACKAGE_PIN J15 [get_ports {led_o[1]}]
set_property PACKAGE_PIN J14 [get_ports {led_o[2]}]
set_property PACKAGE_PIN H15 [get_ports {led_o[3]}]
set_property PACKAGE_PIN L14 [get_ports {led_o[4]}] ; # 4-7 -> 0:3
set_property PACKAGE_PIN M14 [get_ports {led_o[5]}]
set_property PACKAGE_PIN M15 [get_ports {led_o[6]}]
set_property PACKAGE_PIN N15 [get_ports {led_o[7]}]

############################################################################
# Clock constraints                                                        #
############################################################################

create_clock -period 4.000 -name adc_clk [get_ports {adc_clk_i[1]}]
create_clock -period 4.000 -name dco_clk [get_ports dac_dco_i]
create_clock -period 100.000 -name pll_ref_i -waveform {0.000 50.000} [get_ports pll_ref_i]
create_clock -period 4.000 -name rx_clk [get_ports {daisy_p_i[1]}]

create_generated_clock -name i_hk/dna_clk -source [get_pins pll/pll/CLKOUT1] -divide_by 8 [get_pins i_hk/dna_clk_reg/Q]


set_clock_groups -asynchronous -group pll_adc_clk -group clk_fpga_0 -group pll_adc_clk2d -group adc_clk

set_clock_groups -asynchronous -group pll_adc_clk2d -group i_hk/dna_clk -group par_clk -group pll_pwm_clk -group pll_ref_i





#set_false_path -from [get_clocks clk_fpga_0]    -to [get_clocks pll_adc_clk]
#set_false_path -from [get_clocks pll_adc_clk]   -to [get_clocks clk_fpga_0]

#set_false_path -from [get_clocks pll_adc_clk2d] -to [get_clocks pll_adc_clk]
#set_false_path -from [get_clocks pll_adc_clk]   -to [get_clocks pll_adc_clk2d]

#set_false_path -from [get_clocks pll_adc_clk2d] -to [get_clocks pll_pwm_clk]
#set_false_path -from [get_clocks pll_adc_10mhz] -to [get_clocks pll_adc_clk2d]


set_input_delay -clock [get_clocks adc_clk] -clock_fall -min -add_delay -1.000 [get_ports {adc_dat_n_i[0][*]}]
set_input_delay -clock [get_clocks adc_clk] -clock_fall -max -add_delay -0.400 [get_ports {adc_dat_n_i[0][*]}]
set_input_delay -clock [get_clocks adc_clk] -min -add_delay -1.000 [get_ports {adc_dat_n_i[0][*]}]
set_input_delay -clock [get_clocks adc_clk] -max -add_delay -0.400 [get_ports {adc_dat_n_i[0][*]}]
set_input_delay -clock [get_clocks adc_clk] -clock_fall -min -add_delay -1.000 [get_ports {adc_dat_n_i[1][*]}]
set_input_delay -clock [get_clocks adc_clk] -clock_fall -max -add_delay -0.400 [get_ports {adc_dat_n_i[1][*]}]
set_input_delay -clock [get_clocks adc_clk] -min -add_delay -1.000 [get_ports {adc_dat_n_i[1][*]}]
set_input_delay -clock [get_clocks adc_clk] -max -add_delay -0.400 [get_ports {adc_dat_n_i[1][*]}]
set_input_delay -clock [get_clocks adc_clk] -clock_fall -min -add_delay -1.000 [get_ports {adc_dat_p_i[0][*]}]
set_input_delay -clock [get_clocks adc_clk] -clock_fall -max -add_delay -0.400 [get_ports {adc_dat_p_i[0][*]}]
set_input_delay -clock [get_clocks adc_clk] -min -add_delay -1.000 [get_ports {adc_dat_p_i[0][*]}]
set_input_delay -clock [get_clocks adc_clk] -max -add_delay -0.400 [get_ports {adc_dat_p_i[0][*]}]
set_input_delay -clock [get_clocks adc_clk] -clock_fall -min -add_delay -1.000 [get_ports {adc_dat_p_i[1][*]}]
set_input_delay -clock [get_clocks adc_clk] -clock_fall -max -add_delay -0.400 [get_ports {adc_dat_p_i[1][*]}]
set_input_delay -clock [get_clocks adc_clk] -min -add_delay -1.000 [get_ports {adc_dat_p_i[1][*]}]
set_input_delay -clock [get_clocks adc_clk] -max -add_delay -0.400 [get_ports {adc_dat_p_i[1][*]}]






set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]



