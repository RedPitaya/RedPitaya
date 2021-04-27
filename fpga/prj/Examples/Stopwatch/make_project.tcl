
# ==================================================================================================
# block_design.tcl - Create Vivado Project - 3_stopwatch
#
# This script should be run from the base redpitaya-guides/ folder inside Vivado tcl console.
#
# This script is modification of Pavel Demin's project.tcl and block_design.tcl files
# by Anton Potocnik, 20.11.2016
# Tested with Vivado 2016.3
# ==================================================================================================


#set project_name Stopwatch
set part_name xc7z010clg400-1
set bd_path tmp/$project_name/$project_name.srcs/sources_1/bd/system

file delete -force tmp/$project_name

create_project $project_name tmp/$project_name -part $part_name

create_bd_design system
# open_bd_design {$bd_path/system.bd}

# Load RedPitaya ports
source cfg/ports.tcl

# Set Path for the custom IP cores
set_property IP_REPO_PATHS tmp/cores [current_project]
update_ip_catalog


# Load any additional Verilog files in the project folder
set files [glob -nocomplain projects/$project_name/*.v projects/$project_name/*.sv]
if {[llength $files] > 0} {
  add_files -norecurse $files
}
#update_compile_order -fileset sources_1

# Zynq processing system with RedPitaya specific preset
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7 processing_system7_0
set_property -dict [list CONFIG.PCW_USE_S_AXI_HP0 {1}] [get_bd_cells processing_system7_0]
set_property -dict [list CONFIG.PCW_IMPORT_BOARD_PRESET {cfg/red_pitaya.xml}] [get_bd_cells processing_system7_0]
endgroup

# Buffers for differential IOs
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:util_ds_buf util_ds_buf_0
set_property -dict [list CONFIG.C_SIZE {2}] [get_bd_cells util_ds_buf_0]

create_bd_cell -type ip -vlnv xilinx.com:ip:util_ds_buf util_ds_buf_1
set_property -dict [list CONFIG.C_SIZE {2}] [get_bd_cells util_ds_buf_1]

create_bd_cell -type ip -vlnv xilinx.com:ip:util_ds_buf util_ds_buf_2
set_property -dict [list CONFIG.C_SIZE {2}] [get_bd_cells util_ds_buf_2]
set_property -dict [list CONFIG.C_BUF_TYPE {OBUFDS}] [get_bd_cells util_ds_buf_2]
endgroup


# Define all 8 LEDs
set_property LEFT 7 [get_bd_ports led_o]

# slice LEDs
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice xls_LEDs
set_property -dict [list CONFIG.DIN_TO {24} CONFIG.DIN_FROM {31} CONFIG.DIN_WITDH {32} CONFIG.DOUT_WIDTH {8}] [get_bd_cells xls_LEDs]
endgroup


# slice CE
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice xls_CE
set_property -dict [list CONFIG.DIN_TO {0} CONFIG.DIN_FROM {0} CONFIG.DIN_WITDH {32} CONFIG.DOUT_WIDTH {1}] [get_bd_cells xls_CE]
endgroup

# slice SCLR
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice xls_SCLR
set_property -dict [list CONFIG.DIN_TO {1} CONFIG.DIN_FROM {1} CONFIG.DIN_WITDH {32} CONFIG.DOUT_WIDTH {1}] [get_bd_cells xls_SCLR]
endgroup

# Binary Counter - 32bit
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:c_counter_binary c_counter_binary_0
set_property -dict [list CONFIG.Output_Width {32} CONFIG.CE {true} CONFIG.SCLR {true}] [get_bd_cells c_counter_binary_0]
endgroup

# AXI GPIO IP core
startgroup
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio axi_gpio_0
set_property -dict [list CONFIG.C_IS_DUAL {1} CONFIG.C_ALL_INPUTS_2 {1}] [get_bd_cells axi_gpio_0]
endgroup


# ====================================================================================
# Connections 

connect_bd_net [get_bd_ports adc_clk_p_i] [get_bd_pins util_ds_buf_0/IBUF_DS_P]
connect_bd_net [get_bd_ports adc_clk_n_i] [get_bd_pins util_ds_buf_0/IBUF_DS_N]
connect_bd_net [get_bd_ports daisy_p_i] [get_bd_pins util_ds_buf_1/IBUF_DS_P]
connect_bd_net [get_bd_ports daisy_n_i] [get_bd_pins util_ds_buf_1/IBUF_DS_N]
connect_bd_net [get_bd_ports daisy_p_o] [get_bd_pins util_ds_buf_2/OBUF_DS_P]
connect_bd_net [get_bd_ports daisy_n_o] [get_bd_pins util_ds_buf_2/OBUF_DS_N]
connect_bd_net [get_bd_pins util_ds_buf_1/IBUF_OUT] [get_bd_pins util_ds_buf_2/OBUF_IN]
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 -config {make_external "FIXED_IO, DDR" Master "Disable" Slave "Disable" }  [get_bd_cells processing_system7_0]
connect_bd_net [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]
connect_bd_net [get_bd_pins processing_system7_0/S_AXI_HP0_ACLK] [get_bd_pins processing_system7_0/FCLK_CLK0]


connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins c_counter_binary_0/CLK]
connect_bd_net [get_bd_ports led_o] [get_bd_pins xls_LEDs/Dout]
connect_bd_net [get_bd_pins xls_LEDs/Din] [get_bd_pins c_counter_binary_0/Q]

connect_bd_net [get_bd_pins xls_SCLR/Din] [get_bd_pins axi_gpio_0/gpio_io_o]
connect_bd_net [get_bd_pins xls_CE/Din] [get_bd_pins axi_gpio_0/gpio_io_o]
connect_bd_net [get_bd_pins axi_gpio_0/gpio_io_i] [get_bd_pins axi_gpio_0/gpio_io_o]

connect_bd_net [get_bd_pins xls_CE/Dout] [get_bd_pins c_counter_binary_0/CE]
connect_bd_net [get_bd_pins xls_SCLR/Dout] [get_bd_pins c_counter_binary_0/SCLR]

connect_bd_net [get_bd_pins axi_gpio_0/gpio2_io_i] [get_bd_pins c_counter_binary_0/Q]

apply_bd_automation -rule xilinx.com:bd_rule:axi4 -config {Master "/processing_system7_0/M_AXI_GP0" Clk "Auto" }  [get_bd_intf_pins axi_gpio_0/S_AXI]


set_property offset 0x42000000 [get_bd_addr_segs {processing_system7_0/Data/SEG_axi_gpio_0_Reg}]
set_property range 4K [get_bd_addr_segs {processing_system7_0/Data/SEG_axi_gpio_0_Reg}]

# ====================================================================================
# Generate output products and wrapper, add constraint 

generate_target all [get_files  $bd_path/system.bd]

make_wrapper -files [get_files $bd_path/system.bd] -top
add_files -norecurse $bd_path/hdl/system_wrapper.v


# Load RedPitaya constraint files
set files [glob -nocomplain cfg/*.xdc]
if {[llength $files] > 0} {
  add_files -norecurse -fileset constrs_1 $files
}

#set_property top system_wrapper [current_fileset]

set_property VERILOG_DEFINE {TOOL_VIVADO} [current_fileset]
set_property STRATEGY Flow_PerfOptimized_High [get_runs synth_1]
set_property STRATEGY Performance_NetDelay_high [get_runs impl_1]

