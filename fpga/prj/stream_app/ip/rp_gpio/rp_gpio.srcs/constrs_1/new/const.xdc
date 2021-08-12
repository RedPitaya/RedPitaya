create_clock -period 8.000 -name clk -waveform {0.000 4.000} [get_ports clk]
create_clock -period 8.000 -name m_axi_osc1_aclk -waveform {0.000 4.000} [get_ports m_axi_osc1_aclk]
create_clock -period 8.000 -name m_axi_osc2_aclk -waveform {0.000 4.000} [get_ports m_axi_osc2_aclk]
create_clock -period 8.000 -name s_axi_reg_aclk -waveform {0.000 4.000} [get_ports s_axi_reg_aclk]
