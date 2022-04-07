# ==================================================================================================
# block_design.tcl - Create Vivado Project - 4_frequency_counter
#
# This script should be run from the base redpitaya-guides/ folder inside Vivado tcl console.
#
# This script is modification of Pavel Demin's project.tcl and block_design.tcl files
# by Anton Potocnik, 08.01.2017
# Tested with Vivado 2016.4
# ==================================================================================================

# Create cores
source make_cores.tcl

# Create basic Red Pitaya Block Design
source basic_red_pitaya_bd.tcl

# add source files
add_files -norecurse frequency_counter.v
add_files -norecurse pow2.v
add_files -norecurse signal_decoder.v
add_files -norecurse signal_split.v

# ====================================================================================
# IP cores

# GPIO
set_property -dict [list CONFIG.C_ALL_INPUTS {1} CONFIG.C_ALL_INPUTS_2 {0}] [get_bd_cells axi_gpio_0]

# xlslices
startgroup
# slice_trigger
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice xls_log2Ncycles
set_property -dict [list CONFIG.DIN_TO {0} CONFIG.DIN_FROM {4}] [get_bd_cells xls_log2Ncycles]
# slice_phase
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice xls_phase
set_property -dict [list CONFIG.DIN_TO {5} CONFIG.DIN_FROM {31}] [get_bd_cells xls_phase]
endgroup



# Constant for AXIS aresetn
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant xlc_reset
endgroup

# DDS compiler
set_property -dict [list CONFIG.Parameter_Entry {System_Parameters} CONFIG.Phase_Increment {Streaming} CONFIG.Phase_offset {None} CONFIG.Amplitude_Mode {Full_Range} CONFIG.Has_Phase_Out {false} CONFIG.S_PHASE_Has_TUSER {Not_Required} CONFIG.Latency_Configuration {Auto} CONFIG.Frequency_Resolution {0.5} CONFIG.Noise_Shaping {Auto} CONFIG.Phase_Width {28} CONFIG.Output_Width {14} CONFIG.DATA_Has_TLAST {Not_Required} CONFIG.S_PHASE_Has_TUSER {Not_Required} CONFIG.M_DATA_Has_TUSER {Not_Required} CONFIG.M_PHASE_Has_TUSER {Not_Required} CONFIG.Latency {8} CONFIG.Output_Frequency1 {0} CONFIG.PINC1 {0}] [get_bd_cells dds_compiler_0]


# AXIS Constant from Pavel Demin
create_bd_cell -type ip -vlnv pavel-demin:user:axis_constant axis_constant_0


# ====================================================================================
# RTL modules

# signal split
create_bd_cell -type module -reference signal_split signal_split_0

# pow2 module
create_bd_cell -type module -reference pow2 pos2_0

# signal_decoder
create_bd_cell -type module -reference signal_decoder signal_decoder_0

# frequency_coutner
create_bd_cell -type module -reference frequency_counter frequency_counter_0



# ====================================================================================
# Connections 

# signal_split
connect_bd_intf_net [get_bd_intf_pins signal_split_0/S_AXIS] [get_bd_intf_pins axis_red_pitaya_adc_0/M_AXIS]

# signal_decoder
connect_bd_intf_net [get_bd_intf_pins frequency_counter_0/M_AXIS_OUT] [get_bd_intf_pins signal_decoder_0/S_AXIS]
connect_bd_net [get_bd_pins signal_decoder_0/clk] [get_bd_pins axis_red_pitaya_adc_0/adc_clk]
connect_bd_net [get_bd_pins xlc_reset/dout] [get_bd_pins signal_decoder_0/rst]
connect_bd_net [get_bd_ports led_o] [get_bd_pins signal_decoder_0/led_out]

# frequency_counter
connect_bd_intf_net [get_bd_intf_pins frequency_counter_0/S_AXIS_IN] [get_bd_intf_pins signal_split_0/M_AXIS_PORT1]
connect_bd_net [get_bd_pins frequency_counter_0/clk] [get_bd_pins axis_red_pitaya_adc_0/adc_clk]
connect_bd_net [get_bd_pins frequency_counter_0/rst] [get_bd_pins xlc_reset/dout]
connect_bd_net [get_bd_pins pos2_0/N] [get_bd_pins frequency_counter_0/Ncycles]
connect_bd_net [get_bd_pins xls_log2Ncycles/Dout] [get_bd_pins pos2_0/log2N]

# to GPIO
connect_bd_net [get_bd_pins frequency_counter_0/counter_output] [get_bd_pins axi_gpio_0/gpio_io_i]
connect_bd_net [get_bd_pins axi_gpio_0/gpio2_io_i] [get_bd_pins axi_gpio_0/gpio2_io_o]
connect_bd_net [get_bd_pins xls_log2Ncycles/Din] [get_bd_pins axi_gpio_0/gpio2_io_o]


# DDS compiler
connect_bd_intf_net [get_bd_intf_pins axis_constant_0/M_AXIS] [get_bd_intf_pins dds_compiler_0/S_AXIS_PHASE]


# AXIS Constant
connect_bd_net [get_bd_pins axis_red_pitaya_adc_0/adc_clk] [get_bd_pins axis_constant_0/aclk]
connect_bd_net [get_bd_pins xls_phase/Dout] [get_bd_pins axis_constant_0/cfg_data]
connect_bd_net [get_bd_pins xls_phase/Din] [get_bd_pins axi_gpio_0/gpio2_io_o]


# ====================================================================================
# Hierarchies

group_bd_cells SignalGenerator [get_bd_cells axis_red_pitaya_dac_0] [get_bd_cells dds_compiler_0] [get_bd_cells clk_wiz_0] [get_bd_cells axis_constant_0] [get_bd_cells xls_phase]

group_bd_cells DataAcquisition [get_bd_cells axis_red_pitaya_adc_0] [get_bd_cells signal_split_0]

group_bd_cells FrequencyCounter [get_bd_cells xls_log2Ncycles] [get_bd_cells pos2_0] [get_bd_cells frequency_counter_0]

group_bd_cells PS7 [get_bd_cells processing_system7_0] [get_bd_cells rst_ps7_0_125M] [get_bd_cells ps7_0_axi_periph]


# ====================================================================================
# Addresses
