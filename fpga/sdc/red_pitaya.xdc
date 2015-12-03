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

# ADC A data
set_property IOSTANDARD LVCMOS18 [get_ports {adc_dat_a_i[*]}]
set_property IOB        TRUE     [get_ports {adc_dat_a_i[*]}]
#set_property PACKAGE_PIN V17     [get_ports {adc_dat_a_i[0]}]
#set_property PACKAGE_PIN U17     [get_ports {adc_dat_a_i[1]}]
set_property PACKAGE_PIN Y17     [get_ports {adc_dat_a_i[2]}]
set_property PACKAGE_PIN W16     [get_ports {adc_dat_a_i[3]}]
set_property PACKAGE_PIN Y16     [get_ports {adc_dat_a_i[4]}]
set_property PACKAGE_PIN W15     [get_ports {adc_dat_a_i[5]}]
set_property PACKAGE_PIN W14     [get_ports {adc_dat_a_i[6]}]
set_property PACKAGE_PIN Y14     [get_ports {adc_dat_a_i[7]}]
set_property PACKAGE_PIN W13     [get_ports {adc_dat_a_i[8]}]
set_property PACKAGE_PIN V12     [get_ports {adc_dat_a_i[9]}]
set_property PACKAGE_PIN V13     [get_ports {adc_dat_a_i[10]}]
set_property PACKAGE_PIN T14     [get_ports {adc_dat_a_i[11]}]
set_property PACKAGE_PIN T15     [get_ports {adc_dat_a_i[12]}]
set_property PACKAGE_PIN V15     [get_ports {adc_dat_a_i[13]}]
set_property PACKAGE_PIN T16     [get_ports {adc_dat_a_i[14]}]
set_property PACKAGE_PIN V16     [get_ports {adc_dat_a_i[15]}]

set_property IOB TRUE [get_cells {adc_dat_a_reg[0]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[1]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[2]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[3]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[4]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[5]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[6]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[7]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[8]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[9]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[10]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[11]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[12]}]
set_property IOB TRUE [get_cells {adc_dat_a_reg[13]}]

# ADC B data
set_property IOSTANDARD LVCMOS18 [get_ports {adc_dat_b_i[*]}]
set_property IOB        TRUE     [get_ports {adc_dat_b_i[*]}]
#set_property PACKAGE_PIN T17     [get_ports {adc_dat_b_i[0]}]
#set_property PACKAGE_PIN R16     [get_ports {adc_dat_b_i[1]}]
set_property PACKAGE_PIN R18     [get_ports {adc_dat_b_i[2]}]
set_property PACKAGE_PIN P16     [get_ports {adc_dat_b_i[3]}]
set_property PACKAGE_PIN P18     [get_ports {adc_dat_b_i[4]}]
set_property PACKAGE_PIN N17     [get_ports {adc_dat_b_i[5]}]
set_property PACKAGE_PIN R19     [get_ports {adc_dat_b_i[6]}]
set_property PACKAGE_PIN T20     [get_ports {adc_dat_b_i[7]}]
set_property PACKAGE_PIN T19     [get_ports {adc_dat_b_i[8]}]
set_property PACKAGE_PIN U20     [get_ports {adc_dat_b_i[9]}]
set_property PACKAGE_PIN V20     [get_ports {adc_dat_b_i[10]}]
set_property PACKAGE_PIN W20     [get_ports {adc_dat_b_i[11]}]
set_property PACKAGE_PIN W19     [get_ports {adc_dat_b_i[12]}]
set_property PACKAGE_PIN Y19     [get_ports {adc_dat_b_i[13]}]
set_property PACKAGE_PIN W18     [get_ports {adc_dat_b_i[14]}]
set_property PACKAGE_PIN Y18     [get_ports {adc_dat_b_i[15]}]

set_property IOB TRUE [get_cells {adc_dat_b_reg[0]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[1]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[2]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[3]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[4]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[5]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[6]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[7]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[8]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[9]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[10]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[11]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[12]}]
set_property IOB TRUE [get_cells {adc_dat_b_reg[13]}]

set_property IOSTANDARD DIFF_HSTL_I_18 [get_ports adc_clk_p_i]
set_property IOSTANDARD DIFF_HSTL_I_18 [get_ports adc_clk_n_i]
set_property PACKAGE_PIN U18           [get_ports adc_clk_p_i]
set_property PACKAGE_PIN U19           [get_ports adc_clk_n_i]

# Output ADC clock
set_property IOSTANDARD LVCMOS18 [get_ports {adc_clk_o[*]}]
set_property SLEW       FAST     [get_ports {adc_clk_o[*]}]
set_property DRIVE      8        [get_ports {adc_clk_o[*]}]

set_property PACKAGE_PIN N20 [get_ports {adc_clk_o[0]}]
set_property PACKAGE_PIN P20 [get_ports {adc_clk_o[1]}]

#set_property IOB        TRUE     [get_ports {adc_clk_o[*]}]


# ADC clock stabilizer
set_property IOSTANDARD LVCMOS18 [get_ports adc_cdcs_o]
set_property PACKAGE_PIN V18     [get_ports adc_cdcs_o]
set_property SLEW       FAST     [get_ports adc_cdcs_o]
set_property DRIVE      8        [get_ports adc_cdcs_o]

### DAC
set_property IOSTANDARD LVCMOS33 [get_ports {dac_dat_o[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports dac_clk_o]
set_property IOSTANDARD LVCMOS33 [get_ports dac_rst_o]
set_property IOSTANDARD LVCMOS33 [get_ports dac_sel_o]
set_property IOSTANDARD LVCMOS33 [get_ports dac_wrt_o]

set_property SLEW       SLOW     [get_ports {dac_dat_o[*]}]
set_property SLEW       FAST     [get_ports dac_clk_o]
set_property SLEW       FAST     [get_ports dac_rst_o]
set_property SLEW       FAST     [get_ports dac_sel_o]
set_property SLEW       FAST     [get_ports dac_wrt_o]

set_property DRIVE      4        [get_ports {dac_dat_o[*]}]
set_property DRIVE      8        [get_ports dac_clk_o]
set_property DRIVE      8        [get_ports dac_rst_o]
set_property DRIVE      8        [get_ports dac_sel_o]
set_property DRIVE      8        [get_ports dac_wrt_o]

set_property PACKAGE_PIN M19 [get_ports {dac_dat_o[0]}]
set_property PACKAGE_PIN M20 [get_ports {dac_dat_o[1]}]
set_property PACKAGE_PIN L19 [get_ports {dac_dat_o[2]}]
set_property PACKAGE_PIN L20 [get_ports {dac_dat_o[3]}]
set_property PACKAGE_PIN K19 [get_ports {dac_dat_o[4]}]
set_property PACKAGE_PIN J19 [get_ports {dac_dat_o[5]}]
set_property PACKAGE_PIN J20 [get_ports {dac_dat_o[6]}]
set_property PACKAGE_PIN H20 [get_ports {dac_dat_o[7]}]
set_property PACKAGE_PIN G19 [get_ports {dac_dat_o[8]}]
set_property PACKAGE_PIN G20 [get_ports {dac_dat_o[9]}]
set_property PACKAGE_PIN F19 [get_ports {dac_dat_o[10]}]
set_property PACKAGE_PIN F20 [get_ports {dac_dat_o[11]}]
set_property PACKAGE_PIN D20 [get_ports {dac_dat_o[12]}]
set_property PACKAGE_PIN D19 [get_ports {dac_dat_o[13]}]

set_property PACKAGE_PIN M17 [get_ports dac_wrt_o]
set_property PACKAGE_PIN N16 [get_ports dac_sel_o]
set_property PACKAGE_PIN M18 [get_ports dac_clk_o]
set_property PACKAGE_PIN N15 [get_ports dac_rst_o]

#set_property IOB        TRUE     [get_ports dac_*_o]


### PWM DAC
set_property IOSTANDARD LVCMOS18 [get_ports {dac_pwm_o[*]}]
set_property SLEW SLOW           [get_ports {dac_pwm_o[*]}]
set_property DRIVE 12            [get_ports {dac_pwm_o[*]}]
set_property IOB TRUE            [get_ports {dac_pwm_o[*]}]

set_property PACKAGE_PIN T10 [get_ports {dac_pwm_o[0]}]
set_property PACKAGE_PIN T11 [get_ports {dac_pwm_o[1]}]
set_property PACKAGE_PIN P15 [get_ports {dac_pwm_o[2]}]
set_property PACKAGE_PIN U13 [get_ports {dac_pwm_o[3]}]

### XADC
set_property IOSTANDARD LVCMOS33 [get_ports {vinp_i[*]}]
set_property IOSTANDARD LVCMOS33 [get_ports {vinn_i[*]}]
#set_property LOC XADC_X0Y0 [get_cells i_ps/system_i/system_i/xadc_GP1/inst]
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

### SATA connector
set_property IOSTANDARD LVCMOS18 [get_ports {daisy_p_o[*]}]
set_property IOSTANDARD LVCMOS18 [get_ports {daisy_n_o[*]}]
set_property IOSTANDARD LVCMOS18 [get_ports {daisy_p_i[*]}]
set_property IOSTANDARD LVCMOS18 [get_ports {daisy_n_i[*]}]

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

create_clock -period 8.000 -name adc_clk [get_ports adc_clk_p_i]
create_clock -period 4.000 -name rx_clk  [get_ports daisy_p_i[1]]

# ADC-A
set_input_delay -clock [get_clocks adc_clk] -min 1.000 [get_ports {adc_dat_a_i[*]}]
set_input_delay -clock [get_clocks adc_clk] -max 3.400 [get_ports {adc_dat_a_i[*]}]

# ADC-B
set_input_delay -clock [get_clocks adc_clk] -min 1.000 [get_ports {adc_dat_b_i[*]}]
set_input_delay -clock [get_clocks adc_clk] -max 3.400 [get_ports {adc_dat_b_i[*]}]

# SATA
#create_clock -period 4.000 -name rx_clk [get_ports {daisy_p_i[1]}]
set_input_delay -clock [get_clocks rx_clk] -min 1.000 [get_ports {daisy_p_i[0]}]
set_input_delay -clock [get_clocks rx_clk] -max 2.400 [get_ports {daisy_p_i[0]}]

# DNA clock
create_clock -period 64.000 -name dna_clk [get_nets i_hk/dna_clk]


############################################################################
# Min / Max delay                                                          #
############################################################################

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_ams/*_reg[*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_ams/*_reg[*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_asg/*_reg[*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_asg/*_reg[*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_pid/*_reg[*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_pid/*_reg[*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[0][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[0][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[2][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[2][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[4][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[4][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[5][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[5][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[6][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[6][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[7][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[7][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[8][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[8][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[9][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[9][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[10][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[10][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[11][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[11][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[12][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[12][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[13][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[13][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[14][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[14][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[15][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[15][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[16][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[16][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[17][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[17][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[18][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[18][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[19][*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_radiobox/*_reg[19][*]/D}] 7.000

set_min_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_scope/*_reg[*]/D}] 0.000
set_max_delay -rise_from [get_pins {i_ps/axi_slave_gp0/wr_wdata_reg[*]/C}] -rise_to [get_pins {i_scope/*_reg[*]/D}] 7.000


############################################################################
# False paths                                                              #
############################################################################

set_false_path -from [get_clocks clk_fpga_0]   -to [get_clocks adc_clk]
set_false_path -from [get_clocks clk_fpga_1]   -to [get_clocks adc_clk]
set_false_path -from [get_clocks clk_fpga_2]   -to [get_clocks adc_clk]
set_false_path -from [get_clocks clk_fpga_3]   -to [get_clocks adc_clk]
set_false_path -from [get_clocks adc_clk]      -to [get_clocks clk_fpga_0]
set_false_path -from [get_clocks adc_clk]      -to [get_clocks clk_fpga_1]
set_false_path -from [get_clocks adc_clk]      -to [get_clocks clk_fpga_2]
set_false_path -from [get_clocks adc_clk]      -to [get_clocks clk_fpga_3]

set_false_path -from [get_clocks rx_clk]       -to [get_clocks adc_clk]
set_false_path -from [get_clocks adc_clk]      -to [get_clocks rx_clk]

set_false_path -from [get_ports {exp_p_io[*]}] -to [get_clocks adc_clk]
set_false_path -from [get_ports {exp_n_io[*]}] -to [get_clocks adc_clk]
set_false_path -from [get_clocks adc_clk]      -to [get_ports {exp_p_io[*]}]
set_false_path -from [get_clocks adc_clk]      -to [get_ports {exp_n_io[*]}]

set_false_path -from [get_clocks pll_adc_clk]  -to [get_clocks dna_clk]
set_false_path -from [get_clocks dna_clk]      -to [get_clocks pll_adc_clk]

#set_false_path -from [get_clocks adc_clk]     -to [get_clocks dac_clk_out]
#set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks ser_clk_out]
#set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks dac_2clk_out]
#set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks adc_clk]
#set_false_path -from [get_clocks clk_fpga_0]  -to [get_clocks par_clk]
#set_false_path -from [get_clocks dac_clk_out] -to [get_clocks dac_2clk_out]
#set_false_path -from [get_clocks dac_clk_out] -to [get_clocks dac_2ph_out]


############################################################################
# Bitstream constraints                                                     #
############################################################################

set_property BITSTREAM.CONFIG.OVERTEMPPOWERDOWN ENABLE [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property BITSTREAM.GENERAL.XADCENHANCEDLINEARITY On [current_design]
