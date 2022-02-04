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
set_property IOSTANDARD LVCMOS18 [get_ports {adc_dat_i[*][*]}]
set_property IOB        TRUE     [get_ports {adc_dat_i[*][*]}]

# ADC 0 data
set_property PACKAGE_PIN W16     [get_ports {adc_dat_i[0][0]}]
set_property PACKAGE_PIN W15     [get_ports {adc_dat_i[0][1]}]
set_property PACKAGE_PIN Y14     [get_ports {adc_dat_i[0][2]}]
set_property PACKAGE_PIN V12     [get_ports {adc_dat_i[0][3]}]
set_property PACKAGE_PIN T14     [get_ports {adc_dat_i[0][4]}]
set_property PACKAGE_PIN V15     [get_ports {adc_dat_i[0][5]}]
set_property PACKAGE_PIN V16     [get_ports {adc_dat_i[0][6]}]

# ADC 1 data
set_property PACKAGE_PIN P16     [get_ports {adc_dat_i[1][0]}]
set_property PACKAGE_PIN N17     [get_ports {adc_dat_i[1][1]}]
set_property PACKAGE_PIN T20     [get_ports {adc_dat_i[1][2]}]
set_property PACKAGE_PIN U20     [get_ports {adc_dat_i[1][3]}]
set_property PACKAGE_PIN W20     [get_ports {adc_dat_i[1][4]}]
set_property PACKAGE_PIN Y19     [get_ports {adc_dat_i[1][5]}]
set_property PACKAGE_PIN Y18     [get_ports {adc_dat_i[1][6]}]

set_property IOSTANDARD DIFF_HSTL_I_18 [get_ports adc_clk_i[*]]
set_property PACKAGE_PIN U18           [get_ports adc_clk_i[1]]
set_property PACKAGE_PIN U19           [get_ports adc_clk_i[0]]

# Output ADC clock
set_property IOSTANDARD LVCMOS18 [get_ports {adc_clk_o[*]}]
set_property SLEW       FAST     [get_ports {adc_clk_o[*]}]
set_property DRIVE      8        [get_ports {adc_clk_o[*]}]
#set_property IOB        TRUE     [get_ports {adc_clk_o[*]}]

set_property PACKAGE_PIN N20 [get_ports {adc_clk_o[0]}]
set_property PACKAGE_PIN P20 [get_ports {adc_clk_o[1]}]

# SPI interface
set_property IOSTANDARD  LVCMOS18 [get_ports spi_*_o]
set_property SLEW        FAST     [get_ports spi_*_o]
set_property DRIVE       8        [get_ports spi_*_o]

set_property PACKAGE_PIN V18      [get_ports spi_cs_o]
set_property PACKAGE_PIN R16      [get_ports spi_clk_o]
set_property PACKAGE_PIN T17      [get_ports spi_mosi_o]

### PWM DAC
set_property IOSTANDARD LVCMOS18 [get_ports {dac_pwm_o[*]}]
set_property SLEW       FAST     [get_ports {dac_pwm_o[*]}]
set_property DRIVE      12       [get_ports {dac_pwm_o[*]}]
set_property IOB        TRUE     [get_ports {dac_pwm_o[*]}]

set_property PACKAGE_PIN T10 [get_ports {dac_pwm_o[0]}]
set_property PACKAGE_PIN T11 [get_ports {dac_pwm_o[1]}]
set_property PACKAGE_PIN P15 [get_ports {dac_pwm_o[2]}]
set_property PACKAGE_PIN U13 [get_ports {dac_pwm_o[3]}]

### XADC
set_property IOSTANDARD LVCMOS33 [get_ports {vinp_i[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports {vinn_i[*]}]
#AD0
set_property PACKAGE_PIN C20 [get_ports {vinp_i[1]}]
set_property PACKAGE_PIN B20 [get_ports {vinn_i[1]}]
#AD1
set_property PACKAGE_PIN E17 [get_ports {vinp_i[2]}]
set_property PACKAGE_PIN D18 [get_ports {vinn_i[2]}]
#AD8
set_property PACKAGE_PIN B19 [get_ports {vinp_i[0]}]
set_property PACKAGE_PIN A20 [get_ports {vinn_i[0]}]
#AD9
set_property PACKAGE_PIN E18 [get_ports {vinp_i[3]}]
set_property PACKAGE_PIN E19 [get_ports {vinn_i[3]}]
#V_0
set_property PACKAGE_PIN K9  [get_ports {vinp_i[4]}]
set_property PACKAGE_PIN L10 [get_ports {vinn_i[4]}]

### Expansion connector
set_property IOSTANDARD LVCMOS33 [get_ports {exp_p_io[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports {exp_n_io[*]}]
set_property SLEW       FAST     [get_ports {exp_p_io[*]}]
set_property SLEW       FAST     [get_ports {exp_n_io[*]}]
set_property DRIVE      8        [get_ports {exp_p_io[*]}]
set_property DRIVE      8        [get_ports {exp_n_io[*]}]

set_property PACKAGE_PIN G17 [get_ports {exp_p_io[0]}]
set_property PACKAGE_PIN G18 [get_ports {exp_n_io[0]}]
set_property PACKAGE_PIN H16 [get_ports {exp_p_io[1]}]
set_property PACKAGE_PIN H17 [get_ports {exp_n_io[1]}]
set_property PACKAGE_PIN J18 [get_ports {exp_p_io[2]}]
set_property PACKAGE_PIN H18 [get_ports {exp_n_io[2]}]
set_property PACKAGE_PIN K17 [get_ports {exp_p_io[3]}]
set_property PACKAGE_PIN K18 [get_ports {exp_n_io[3]}]
set_property PACKAGE_PIN L14 [get_ports {exp_p_io[4]}]
set_property PACKAGE_PIN L15 [get_ports {exp_n_io[4]}]
set_property PACKAGE_PIN L16 [get_ports {exp_p_io[5]}]
set_property PACKAGE_PIN L17 [get_ports {exp_n_io[5]}]
set_property PACKAGE_PIN K16 [get_ports {exp_p_io[6]}]
set_property PACKAGE_PIN J16 [get_ports {exp_n_io[6]}]
set_property PACKAGE_PIN M14 [get_ports {exp_p_io[7]}]
set_property PACKAGE_PIN M15 [get_ports {exp_n_io[7]}]

#set_property PULLDOWN TRUE [get_ports {exp_p_io[0]}]
#set_property PULLDOWN TRUE [get_ports {exp_n_io[0]}]
#set_property PULLUP   TRUE [get_ports {exp_p_io[7]}]
#set_property PULLUP   TRUE [get_ports {exp_n_io[7]}]

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
set_property SLEW       SLOW     [get_ports {led_o[*]}]
set_property DRIVE      4        [get_ports {led_o[*]}]

set_property PACKAGE_PIN F16     [get_ports {led_o[0]}]
set_property PACKAGE_PIN F17     [get_ports {led_o[1]}]
set_property PACKAGE_PIN G15     [get_ports {led_o[2]}]
set_property PACKAGE_PIN H15     [get_ports {led_o[3]}]
set_property PACKAGE_PIN K14     [get_ports {led_o[4]}]
set_property PACKAGE_PIN G14     [get_ports {led_o[5]}]
set_property PACKAGE_PIN J15     [get_ports {led_o[6]}]
set_property PACKAGE_PIN J14     [get_ports {led_o[7]}]

############################################################################
# Clock constraints                                                        #
############################################################################

#NET "adc_clk" TNM_NET = "adc_clk";
#TIMESPEC TS_adc_clk = PERIOD "adc_clk" 125 MHz;

create_clock -period 8.000 -name adc_clk [get_ports adc_clk_i[1]]

set_input_delay -clock adc_clk 3.400 [get_ports adc_dat_i[*][*]]

#create_clock -period 4.000 -name rx_clk  [get_ports daisy_p_i[1]]

set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks adc_clk]
set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks dac_clk_1x]
set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks dac_clk_2x]
set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks dac_clk_2p]
set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks ser_clk]
set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks pdm_clk]
set_false_path -from [get_clocks pll_adc_clk] -to [get_clocks adc_clk]
set_false_path -from [get_clocks adc_clk] -to [get_clocks pll_adc_clk]
