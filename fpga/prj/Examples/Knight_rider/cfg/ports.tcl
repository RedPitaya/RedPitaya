
### ADC

create_bd_port -dir I -from 13 -to 0 adc_dat_a_i
create_bd_port -dir I -from 13 -to 0 adc_dat_b_i

create_bd_port -dir I adc_clk_p_i
create_bd_port -dir I adc_clk_n_i

create_bd_port -dir O adc_enc_p_o
create_bd_port -dir O adc_enc_n_o

create_bd_port -dir O adc_csn_o

### DAC

create_bd_port -dir O -from 13 -to 0 dac_dat_o

create_bd_port -dir O dac_clk_o
create_bd_port -dir O dac_rst_o
create_bd_port -dir O dac_sel_o
create_bd_port -dir O dac_wrt_o

### PWM

create_bd_port -dir O -from 3 -to 0 dac_pwm_o

### XADC

create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vp_Vn
create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux0
create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux1
create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux9
create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux8

### Expansion connector

create_bd_port -dir IO -from 7 -to 0 exp_p_tri_io
create_bd_port -dir IO -from 7 -to 0 exp_n_tri_io

### SATA connector

create_bd_port -dir O -from 1 -to 0 daisy_p_o
create_bd_port -dir O -from 1 -to 0 daisy_n_o

create_bd_port -dir I -from 1 -to 0 daisy_p_i
create_bd_port -dir I -from 1 -to 0 daisy_n_i

### LED

create_bd_port -dir O -from 7 -to 0 led_o
