
################################################################
# This is a generated script based on design: system
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

################################################################
# Check if script is running in correct Vivado version.
################################################################
set scripts_vivado_version 2015.4
set current_vivado_version [version -short]

if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
   puts ""
   puts "ERROR: This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Please run the script in Vivado <$scripts_vivado_version> then open the design in Vivado <$current_vivado_version>. Upgrade the design by running \"Tools => Report => Report IP Status...\", then run write_bd_tcl to create an updated script."

   return 1
}

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source system_script.tcl

# If you do not already have a project created,
# you can create a project using the following command:
#    create_project project_1 myproj -part xc7z010clg400-1

# CHECKING IF PROJECT EXISTS
if { [get_projects -quiet] eq "" } {
   puts "ERROR: Please open or create a project!"
   return 1
}



# CHANGE DESIGN NAME HERE
set design_name system

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design $design_name

# Creating design if needed
set errMsg ""
set nRet 0

set cur_design [current_bd_design -quiet]
set list_cells [get_bd_cells -quiet]

if { ${design_name} eq "" } {
   # USE CASES:
   #    1) Design_name not set

   set errMsg "ERROR: Please set the variable <design_name> to a non-empty value."
   set nRet 1

} elseif { ${cur_design} ne "" && ${list_cells} eq "" } {
   # USE CASES:
   #    2): Current design opened AND is empty AND names same.
   #    3): Current design opened AND is empty AND names diff; design_name NOT in project.
   #    4): Current design opened AND is empty AND names diff; design_name exists in project.

   if { $cur_design ne $design_name } {
      puts "INFO: Changing value of <design_name> from <$design_name> to <$cur_design> since current design is empty."
      set design_name [get_property NAME $cur_design]
   }
   puts "INFO: Constructing design in IPI design <$cur_design>..."

} elseif { ${cur_design} ne "" && $list_cells ne "" && $cur_design eq $design_name } {
   # USE CASES:
   #    5) Current design opened AND has components AND same names.

   set errMsg "ERROR: Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 1
} elseif { [get_files -quiet ${design_name}.bd] ne "" } {
   # USE CASES: 
   #    6) Current opened design, has components, but diff names, design_name exists in project.
   #    7) No opened design, design_name exists in project.

   set errMsg "ERROR: Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 2

} else {
   # USE CASES:
   #    8) No opened design, design_name not in project.
   #    9) Current opened design, has components, but diff names, design_name not in project.

   puts "INFO: Currently there is no design <$design_name> in project, so creating one..."

   create_bd_design $design_name

   puts "INFO: Making design <$design_name> as current_bd_design."
   current_bd_design $design_name

}

puts "INFO: Currently the variable <design_name> is equal to \"$design_name\"."

if { $nRet != 0 } {
   puts $errMsg
   return $nRet
}

##################################################################
# DESIGN PROCs
##################################################################



# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     puts "ERROR: Unable to find parent cell <$parentCell>!"
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     puts "ERROR: Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set DDR [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:ddrx_rtl:1.0 DDR ]
  set FIXED_IO [ create_bd_intf_port -mode Master -vlnv xilinx.com:display_processing_system7:fixedio_rtl:1.0 FIXED_IO ]
  set M_AXI4_LITE_0 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M_AXI4_LITE_0 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.DATA_WIDTH {32} \
CONFIG.PROTOCOL {AXI4LITE} \
 ] $M_AXI4_LITE_0
  set M_AXI_GP0 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M_AXI_GP0 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.DATA_WIDTH {32} \
CONFIG.FREQ_HZ {125000000} \
CONFIG.PROTOCOL {AXI3} \
 ] $M_AXI_GP0
  set M_AXI_STR_TX0 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXI_STR_TX0 ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXI_STR_TX0
  set M_AXI_STR_TX1 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXI_STR_TX1 ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXI_STR_TX1
  set M_AXI_STR_TX2 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXI_STR_TX2 ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXI_STR_TX2
  set M_AXI_STR_TX3 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXI_STR_TX3 ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXI_STR_TX3
  set S_AXI_STR_RX0 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXI_STR_RX0 ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
CONFIG.HAS_TKEEP {0} \
CONFIG.HAS_TLAST {1} \
CONFIG.HAS_TREADY {1} \
CONFIG.HAS_TSTRB {0} \
CONFIG.LAYERED_METADATA {undef} \
CONFIG.PHASE {0.000} \
CONFIG.TDATA_NUM_BYTES {2} \
CONFIG.TDEST_WIDTH {0} \
CONFIG.TID_WIDTH {0} \
CONFIG.TUSER_WIDTH {0} \
 ] $S_AXI_STR_RX0
  set S_AXI_STR_RX1 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXI_STR_RX1 ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
CONFIG.HAS_TKEEP {0} \
CONFIG.HAS_TLAST {1} \
CONFIG.HAS_TREADY {1} \
CONFIG.HAS_TSTRB {0} \
CONFIG.LAYERED_METADATA {undef} \
CONFIG.PHASE {0.000} \
CONFIG.TDATA_NUM_BYTES {2} \
CONFIG.TDEST_WIDTH {0} \
CONFIG.TID_WIDTH {0} \
CONFIG.TUSER_WIDTH {0} \
 ] $S_AXI_STR_RX1
  set S_AXI_STR_RX2 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXI_STR_RX2 ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
CONFIG.HAS_TKEEP {0} \
CONFIG.HAS_TLAST {1} \
CONFIG.HAS_TREADY {1} \
CONFIG.HAS_TSTRB {0} \
CONFIG.LAYERED_METADATA {undef} \
CONFIG.PHASE {0.000} \
CONFIG.TDATA_NUM_BYTES {2} \
CONFIG.TDEST_WIDTH {0} \
CONFIG.TID_WIDTH {0} \
CONFIG.TUSER_WIDTH {0} \
 ] $S_AXI_STR_RX2
  set S_AXI_STR_RX3 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:axis_rtl:1.0 S_AXI_STR_RX3 ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
CONFIG.HAS_TKEEP {0} \
CONFIG.HAS_TLAST {1} \
CONFIG.HAS_TREADY {1} \
CONFIG.HAS_TSTRB {0} \
CONFIG.LAYERED_METADATA {undef} \
CONFIG.PHASE {0.000} \
CONFIG.TDATA_NUM_BYTES {2} \
CONFIG.TDEST_WIDTH {0} \
CONFIG.TID_WIDTH {0} \
CONFIG.TUSER_WIDTH {0} \
 ] $S_AXI_STR_RX3
  set Vaux0 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux0 ]
  set Vaux1 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux1 ]
  set Vaux8 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux8 ]
  set Vaux9 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux9 ]
  set Vp_Vn [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vp_Vn ]

  # Create ports
  set FCLK_CLK0 [ create_bd_port -dir O -type clk FCLK_CLK0 ]
  set FCLK_CLK1 [ create_bd_port -dir O -type clk FCLK_CLK1 ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {M_AXI4_LITE_0} \
 ] $FCLK_CLK1
  set FCLK_CLK2 [ create_bd_port -dir O -type clk FCLK_CLK2 ]
  set FCLK_CLK3 [ create_bd_port -dir O -type clk FCLK_CLK3 ]
  set FCLK_RESET0_N [ create_bd_port -dir O -type rst FCLK_RESET0_N ]
  set FCLK_RESET1_N [ create_bd_port -dir O -type rst FCLK_RESET1_N ]
  set FCLK_RESET2_N [ create_bd_port -dir O -type rst FCLK_RESET2_N ]
  set FCLK_RESET3_N [ create_bd_port -dir O -type rst FCLK_RESET3_N ]
  set M_AXI_GP0_ACLK [ create_bd_port -dir I -type clk M_AXI_GP0_ACLK ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {M_AXI_GP0} \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXI_GP0_ACLK

  set M_AXI_STR_TX0_aclk [ create_bd_port -dir I -type clk M_AXI_STR_TX0_aclk ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {M_AXI_STR_TX0} \
CONFIG.ASSOCIATED_RESET {M_AXI_STR_TX0_arstn} \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXI_STR_TX0_aclk
  set M_AXI_STR_TX0_aresetn[ create_bd_port -dir I -type rst M_AXI_STR_TX0_aresetn]

  set M_AXI_STR_TX1_aclk [ create_bd_port -dir I -type clk M_AXI_STR_TX1_aclk ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {M_AXI_STR_TX1} \
CONFIG.ASSOCIATED_RESET {M_AXI_STR_TX1_arstn} \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXI_STR_TX1_aclk
  set M_AXI_STR_TX1_aresetn[ create_bd_port -dir I -type rst M_AXI_STR_TX1_aresetn]

  set M_AXI_STR_TX2_aclk [ create_bd_port -dir I -type clk M_AXI_STR_TX2_aclk ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {M_AXI_STR_TX2} \
CONFIG.ASSOCIATED_RESET {M_AXI_STR_TX2_arstn} \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXI_STR_TX2_aclk
  set M_AXI_STR_TX2_aresetn[ create_bd_port -dir I -type rst M_AXI_STR_TX2_aresetn]

  set M_AXI_STR_TX3_aclk [ create_bd_port -dir I -type clk M_AXI_STR_TX3_aclk ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {M_AXI_STR_TX3} \
CONFIG.ASSOCIATED_RESET {M_AXI_STR_TX3_arstn} \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXI_STR_TX3_aclk
  set M_AXI_STR_TX3_aresetn[ create_bd_port -dir I -type rst M_AXI_STR_TX3_aresetn]

  set S_AXI_STR_RX0_aclk [ create_bd_port -dir I -type clk S_AXI_STR_RX0_aclk ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {S_AXI_STR_RX0} \
CONFIG.ASSOCIATED_RESET {S_AXI_STR_RX0_arstn} \
CONFIG.FREQ_HZ {125000000} \
 ] $S_AXI_STR_RX0_aclk
  set S_AXI_STR_RX0_aresetn[ create_bd_port -dir I -type rst S_AXI_STR_RX0_aresetn]

  set S_AXI_STR_RX1_aclk [ create_bd_port -dir I -type clk S_AXI_STR_RX1_aclk ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {S_AXI_STR_RX1} \
CONFIG.ASSOCIATED_RESET {S_AXI_STR_RX1_arstn} \
CONFIG.FREQ_HZ {125000000} \
 ] $S_AXI_STR_RX1_aclk
  set S_AXI_STR_RX1_aresetn[ create_bd_port -dir I -type rst S_AXI_STR_RX1_aresetn]

  set S_AXI_STR_RX2_aclk [ create_bd_port -dir I -type clk S_AXI_STR_RX2_aclk ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {S_AXI_STR_RX2} \
CONFIG.ASSOCIATED_RESET {S_AXI_STR_RX2_arstn} \
CONFIG.FREQ_HZ {125000000} \
 ] $S_AXI_STR_RX2_aclk
  set S_AXI_STR_RX2_aresetn[ create_bd_port -dir I -type rst S_AXI_STR_RX2_aresetn]

  set S_AXI_STR_RX3_aclk [ create_bd_port -dir I -type clk S_AXI_STR_RX3_aclk ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {S_AXI_STR_RX3} \
CONFIG.ASSOCIATED_RESET {S_AXI_STR_RX3_arstn} \
CONFIG.FREQ_HZ {125000000} \
 ] $S_AXI_STR_RX3_aclk
  set S_AXI_STR_RX3_aresetn[ create_bd_port -dir I -type rst S_AXI_STR_RX3_aresetn]

  # Create instance: axi_dma_0, and set properties
  set axi_dma_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_dma:7.1 axi_dma_0 ]
  set_property -dict [ list \
CONFIG.c_include_mm2s {1} \
CONFIG.c_include_sg {1} \
CONFIG.c_m_axi_mm2s_data_width {64} \
CONFIG.c_m_axi_s2mm_data_width {64} \
CONFIG.c_m_axis_mm2s_tdata_width {16} \
CONFIG.c_mm2s_burst_size {64} \
CONFIG.c_s2mm_burst_size {64} \
CONFIG.c_sg_include_stscntrl_strm {0} \
 ] $axi_dma_0

  # Create instance: axi_dma_1, and set properties
  set axi_dma_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_dma:7.1 axi_dma_1 ]
  set_property -dict [ list \
CONFIG.c_include_mm2s {1} \
CONFIG.c_include_sg {1} \
CONFIG.c_m_axi_mm2s_data_width {64} \
CONFIG.c_m_axi_s2mm_data_width {64} \
CONFIG.c_m_axis_mm2s_tdata_width {16} \
CONFIG.c_mm2s_burst_size {64} \
CONFIG.c_s2mm_burst_size {64} \
CONFIG.c_sg_include_stscntrl_strm {0} \
 ] $axi_dma_1

  # Create instance: axi_dma_2, and set properties
  set axi_dma_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_dma:7.1 axi_dma_2 ]
  set_property -dict [ list \
CONFIG.c_include_mm2s {1} \
CONFIG.c_include_sg {1} \
CONFIG.c_m_axi_mm2s_data_width {64} \
CONFIG.c_m_axi_s2mm_data_width {64} \
CONFIG.c_m_axis_mm2s_tdata_width {16} \
CONFIG.c_mm2s_burst_size {64} \
CONFIG.c_s2mm_burst_size {64} \
CONFIG.c_sg_include_stscntrl_strm {0} \
 ] $axi_dma_2

  # Create instance: axi_dma_3, and set properties
  set axi_dma_3 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_dma:7.1 axi_dma_3 ]
  set_property -dict [ list \
CONFIG.c_include_mm2s {1} \
CONFIG.c_include_sg {1} \
CONFIG.c_m_axi_mm2s_data_width {64} \
CONFIG.c_m_axi_s2mm_data_width {64} \
CONFIG.c_m_axis_mm2s_tdata_width {16} \
CONFIG.c_mm2s_burst_size {64} \
CONFIG.c_s2mm_burst_size {64} \
CONFIG.c_sg_include_stscntrl_strm {0} \
 ] $axi_dma_3

  # Create instance: axi_interconnect_0, and set properties
  set axi_interconnect_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_0 ]
  set_property -dict [ list \
CONFIG.NUM_MI {6} \
CONFIG.NUM_SI {1} \
 ] $axi_interconnect_0

  # Create instance: axi_interconnect_1, and set properties
  set axi_interconnect_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_1 ]
  set_property -dict [ list \
CONFIG.NUM_MI {1} \
CONFIG.NUM_SI {3} \
 ] $axi_interconnect_1

  # Create instance: axi_interconnect_2, and set properties
  set axi_interconnect_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_2 ]
  set_property -dict [ list \
CONFIG.NUM_MI {1} \
CONFIG.NUM_SI {3} \
 ] $axi_interconnect_2

  # Create instance: axi_interconnect_3, and set properties
  set axi_interconnect_3 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_3 ]
  set_property -dict [ list \
CONFIG.NUM_MI {1} \
CONFIG.NUM_SI {3} \
 ] $axi_interconnect_3

  # Create instance: axi_interconnect_4, and set properties
  set axi_interconnect_4 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_4 ]
  set_property -dict [ list \
CONFIG.NUM_MI {1} \
CONFIG.NUM_SI {3} \
 ] $axi_interconnect_4

  # Create instance: axis_clock_converter_0, and set properties
  set axis_clock_converter_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_clock_converter:1.1 axis_clock_converter_0 ]

  # Create instance: axis_clock_converter_1, and set properties
  set axis_clock_converter_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_clock_converter:1.1 axis_clock_converter_1 ]

  # Create instance: axis_clock_converter_2, and set properties
  set axis_clock_converter_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_clock_converter:1.1 axis_clock_converter_2 ]

  # Create instance: axis_clock_converter_3, and set properties
  set axis_clock_converter_3 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_clock_converter:1.1 axis_clock_converter_3 ]

  # Create instance: axis_clock_converter_4, and set properties
  set axis_clock_converter_4 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_clock_converter:1.1 axis_clock_converter_4 ]

  # Create instance: axis_clock_converter_5, and set properties
  set axis_clock_converter_5 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_clock_converter:1.1 axis_clock_converter_5 ]

  # Create instance: axis_clock_converter_6, and set properties
  set axis_clock_converter_6 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_clock_converter:1.1 axis_clock_converter_6 ]

  # Create instance: axis_clock_converter_7, and set properties
  set axis_clock_converter_7 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_clock_converter:1.1 axis_clock_converter_7 ]

  # Create instance: axis_data_fifo_0, and set properties
  set axis_data_fifo_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:1.1 axis_data_fifo_0 ]
  set_property -dict [ list \
CONFIG.FIFO_DEPTH {2048} \
CONFIG.FIFO_MODE {2} \
CONFIG.IS_ACLK_ASYNC {0} \
 ] $axis_data_fifo_0

  # Create instance: axis_data_fifo_1, and set properties
  set axis_data_fifo_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:1.1 axis_data_fifo_1 ]
  set_property -dict [ list \
CONFIG.FIFO_DEPTH {2048} \
CONFIG.FIFO_MODE {2} \
CONFIG.IS_ACLK_ASYNC {0} \
 ] $axis_data_fifo_1

  # Create instance: axis_data_fifo_2, and set properties
  set axis_data_fifo_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:1.1 axis_data_fifo_2 ]
  set_property -dict [ list \
CONFIG.FIFO_DEPTH {2048} \
CONFIG.FIFO_MODE {2} \
CONFIG.IS_ACLK_ASYNC {0} \
 ] $axis_data_fifo_2

  # Create instance: axis_data_fifo_3, and set properties
  set axis_data_fifo_3 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:1.1 axis_data_fifo_3 ]
  set_property -dict [ list \
CONFIG.FIFO_DEPTH {2048} \
CONFIG.FIFO_MODE {2} \
CONFIG.IS_ACLK_ASYNC {0} \
 ] $axis_data_fifo_3

  # Create instance: axis_data_fifo_4, and set properties
  set axis_data_fifo_4 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:1.1 axis_data_fifo_4 ]
  set_property -dict [ list \
CONFIG.FIFO_DEPTH {2048} \
CONFIG.FIFO_MODE {2} \
CONFIG.IS_ACLK_ASYNC {0} \
 ] $axis_data_fifo_4

  # Create instance: axis_data_fifo_5, and set properties
  set axis_data_fifo_5 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:1.1 axis_data_fifo_5 ]
  set_property -dict [ list \
CONFIG.FIFO_DEPTH {2048} \
CONFIG.FIFO_MODE {2} \
CONFIG.IS_ACLK_ASYNC {0} \
 ] $axis_data_fifo_5

  # Create instance: axis_data_fifo_6, and set properties
  set axis_data_fifo_6 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:1.1 axis_data_fifo_6 ]
  set_property -dict [ list \
CONFIG.FIFO_DEPTH {2048} \
CONFIG.FIFO_MODE {2} \
CONFIG.IS_ACLK_ASYNC {0} \
 ] $axis_data_fifo_6

  # Create instance: axis_data_fifo_7, and set properties
  set axis_data_fifo_7 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:1.1 axis_data_fifo_7 ]
  set_property -dict [ list \
CONFIG.FIFO_DEPTH {2048} \
CONFIG.FIFO_MODE {2} \
CONFIG.IS_ACLK_ASYNC {0} \
 ] $axis_data_fifo_7

  # Create instance: proc_sys_reset, and set properties
  set proc_sys_reset [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 proc_sys_reset ]
  set_property -dict [ list \
CONFIG.C_EXT_RST_WIDTH {1} \
 ] $proc_sys_reset

  # Create instance: processing_system7, and set properties
  set processing_system7 [ create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7 ]
  set_property -dict [ list \
CONFIG.PCW_ENET0_ENET0_IO {MIO 16 .. 27} \
CONFIG.PCW_ENET0_GRP_MDIO_ENABLE {1} \
CONFIG.PCW_ENET0_PERIPHERAL_CLKSRC {IO PLL} \
CONFIG.PCW_ENET0_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_EN_CLK1_PORT {1} \
CONFIG.PCW_EN_CLK2_PORT {1} \
CONFIG.PCW_EN_CLK3_PORT {1} \
CONFIG.PCW_EN_RST1_PORT {1} \
CONFIG.PCW_EN_RST2_PORT {1} \
CONFIG.PCW_EN_RST3_PORT {1} \
CONFIG.PCW_FPGA0_PERIPHERAL_FREQMHZ {125} \
CONFIG.PCW_FPGA1_PERIPHERAL_FREQMHZ {142} \
CONFIG.PCW_FPGA2_PERIPHERAL_FREQMHZ {166} \
CONFIG.PCW_FPGA3_PERIPHERAL_FREQMHZ {200} \
CONFIG.PCW_GPIO_MIO_GPIO_ENABLE {1} \
CONFIG.PCW_I2C0_I2C0_IO {MIO 50 .. 51} \
CONFIG.PCW_I2C0_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_IOPLL_CTRL_FBDIV {30} \
CONFIG.PCW_IRQ_F2P_INTR {1} \
CONFIG.PCW_MIO_16_PULLUP {disabled} \
CONFIG.PCW_MIO_16_SLEW {fast} \
CONFIG.PCW_MIO_17_PULLUP {disabled} \
CONFIG.PCW_MIO_17_SLEW {fast} \
CONFIG.PCW_MIO_18_PULLUP {disabled} \
CONFIG.PCW_MIO_18_SLEW {fast} \
CONFIG.PCW_MIO_19_PULLUP {disabled} \
CONFIG.PCW_MIO_19_SLEW {fast} \
CONFIG.PCW_MIO_20_PULLUP {disabled} \
CONFIG.PCW_MIO_20_SLEW {fast} \
CONFIG.PCW_MIO_21_PULLUP {disabled} \
CONFIG.PCW_MIO_21_SLEW {fast} \
CONFIG.PCW_MIO_22_PULLUP {disabled} \
CONFIG.PCW_MIO_22_SLEW {fast} \
CONFIG.PCW_MIO_23_PULLUP {disabled} \
CONFIG.PCW_MIO_23_SLEW {fast} \
CONFIG.PCW_MIO_24_PULLUP {disabled} \
CONFIG.PCW_MIO_24_SLEW {fast} \
CONFIG.PCW_MIO_25_PULLUP {disabled} \
CONFIG.PCW_MIO_25_SLEW {fast} \
CONFIG.PCW_MIO_26_PULLUP {disabled} \
CONFIG.PCW_MIO_26_SLEW {fast} \
CONFIG.PCW_MIO_27_PULLUP {disabled} \
CONFIG.PCW_MIO_27_SLEW {fast} \
CONFIG.PCW_MIO_28_PULLUP {disabled} \
CONFIG.PCW_MIO_28_SLEW {fast} \
CONFIG.PCW_MIO_29_PULLUP {disabled} \
CONFIG.PCW_MIO_29_SLEW {fast} \
CONFIG.PCW_MIO_30_PULLUP {disabled} \
CONFIG.PCW_MIO_30_SLEW {fast} \
CONFIG.PCW_MIO_31_PULLUP {disabled} \
CONFIG.PCW_MIO_31_SLEW {fast} \
CONFIG.PCW_MIO_32_PULLUP {disabled} \
CONFIG.PCW_MIO_32_SLEW {fast} \
CONFIG.PCW_MIO_33_PULLUP {disabled} \
CONFIG.PCW_MIO_33_SLEW {fast} \
CONFIG.PCW_MIO_34_PULLUP {disabled} \
CONFIG.PCW_MIO_34_SLEW {fast} \
CONFIG.PCW_MIO_35_PULLUP {disabled} \
CONFIG.PCW_MIO_35_SLEW {fast} \
CONFIG.PCW_MIO_36_PULLUP {disabled} \
CONFIG.PCW_MIO_36_SLEW {fast} \
CONFIG.PCW_MIO_37_PULLUP {disabled} \
CONFIG.PCW_MIO_37_SLEW {fast} \
CONFIG.PCW_MIO_38_PULLUP {disabled} \
CONFIG.PCW_MIO_38_SLEW {fast} \
CONFIG.PCW_MIO_39_PULLUP {disabled} \
CONFIG.PCW_MIO_39_SLEW {fast} \
CONFIG.PCW_OVERRIDE_BASIC_CLOCK {0} \
CONFIG.PCW_PRESET_BANK1_VOLTAGE {LVCMOS 2.5V} \
CONFIG.PCW_QSPI_PERIPHERAL_CLKSRC {IO PLL} \
CONFIG.PCW_QSPI_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_QSPI_PERIPHERAL_FREQMHZ {125} \
CONFIG.PCW_SD0_GRP_CD_ENABLE {1} \
CONFIG.PCW_SD0_GRP_CD_IO {MIO 46} \
CONFIG.PCW_SD0_GRP_WP_ENABLE {1} \
CONFIG.PCW_SD0_GRP_WP_IO {MIO 47} \
CONFIG.PCW_SD0_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_SPI0_PERIPHERAL_ENABLE {0} \
CONFIG.PCW_SPI1_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_SPI1_SPI1_IO {MIO 10 .. 15} \
CONFIG.PCW_S_AXI_HP2_DATA_WIDTH {32} \
CONFIG.PCW_TTC0_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_UART0_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_UART0_UART0_IO {MIO 14 .. 15} \
CONFIG.PCW_UART1_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_UART1_UART1_IO {MIO 8 .. 9} \
CONFIG.PCW_UIPARAM_DDR_BUS_WIDTH {16 Bit} \
CONFIG.PCW_UIPARAM_DDR_PARTNO {MT41J256M16 RE-125} \
CONFIG.PCW_USB0_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_USB0_RESET_ENABLE {1} \
CONFIG.PCW_USB0_RESET_IO {MIO 48} \
CONFIG.PCW_USE_DMA0 {0} \
CONFIG.PCW_USE_FABRIC_INTERRUPT {1} \
CONFIG.PCW_USE_M_AXI_GP1 {1} \
CONFIG.PCW_USE_S_AXI_HP0 {1} \
CONFIG.PCW_USE_S_AXI_HP1 {1} \
CONFIG.PCW_USE_S_AXI_HP2 {1} \
CONFIG.PCW_USE_S_AXI_HP3 {1} \
 ] $processing_system7

  # Create instance: xadc_wiz_0, and set properties
  set xadc_wiz_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xadc_wiz:3.2 xadc_wiz_0 ]
  set_property -dict [ list \
CONFIG.CHANNEL_ENABLE_VAUXP0_VAUXN0 {true} \
CONFIG.CHANNEL_ENABLE_VAUXP1_VAUXN1 {true} \
CONFIG.CHANNEL_ENABLE_VAUXP8_VAUXN8 {true} \
CONFIG.CHANNEL_ENABLE_VAUXP9_VAUXN9 {true} \
CONFIG.CHANNEL_ENABLE_VP_VN {true} \
CONFIG.EXTERNAL_MUX_CHANNEL {VP_VN} \
CONFIG.SEQUENCER_MODE {Off} \
CONFIG.SINGLE_CHANNEL_SELECTION {TEMPERATURE} \
CONFIG.XADC_STARUP_SELECTION {independent_adc} \
 ] $xadc_wiz_0

  # Create instance: xlconcat_0, and set properties
  set xlconcat_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0 ]
  set_property -dict [ list \
CONFIG.NUM_PORTS {9} \
 ] $xlconcat_0

  # Create instance: xlconstant, and set properties
  set xlconstant [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 xlconstant ]

  # Create interface connections
  connect_bd_intf_net -intf_net S_AXI_STR_RX0_1 [get_bd_intf_ports S_AXI_STR_RX0] [get_bd_intf_pins axis_data_fifo_0/S_AXIS]
  connect_bd_intf_net -intf_net S_AXI_STR_RX1_1 [get_bd_intf_ports S_AXI_STR_RX1] [get_bd_intf_pins axis_data_fifo_1/S_AXIS]
  connect_bd_intf_net -intf_net S_AXI_STR_RX2_1 [get_bd_intf_ports S_AXI_STR_RX2] [get_bd_intf_pins axis_data_fifo_2/S_AXIS]
  connect_bd_intf_net -intf_net S_AXI_STR_RX3_1 [get_bd_intf_ports S_AXI_STR_RX3] [get_bd_intf_pins axis_data_fifo_3/S_AXIS]
  connect_bd_intf_net -intf_net Vaux0_1 [get_bd_intf_ports Vaux0] [get_bd_intf_pins xadc_wiz_0/Vaux0]
  connect_bd_intf_net -intf_net Vaux1_1 [get_bd_intf_ports Vaux1] [get_bd_intf_pins xadc_wiz_0/Vaux1]
  connect_bd_intf_net -intf_net Vaux8_1 [get_bd_intf_ports Vaux8] [get_bd_intf_pins xadc_wiz_0/Vaux8]
  connect_bd_intf_net -intf_net Vaux9_1 [get_bd_intf_ports Vaux9] [get_bd_intf_pins xadc_wiz_0/Vaux9]
  connect_bd_intf_net -intf_net Vp_Vn_1 [get_bd_intf_ports Vp_Vn] [get_bd_intf_pins xadc_wiz_0/Vp_Vn]
  connect_bd_intf_net -intf_net axi_dma_0_M_AXIS_MM2S [get_bd_intf_pins axi_dma_0/M_AXIS_MM2S] [get_bd_intf_pins axis_data_fifo_4/S_AXIS]
  connect_bd_intf_net -intf_net axi_dma_0_M_AXI_MM2S [get_bd_intf_pins axi_dma_0/M_AXI_MM2S] [get_bd_intf_pins axi_interconnect_1/S01_AXI]
  connect_bd_intf_net -intf_net axi_dma_0_M_AXI_S2MM [get_bd_intf_pins axi_dma_0/M_AXI_S2MM] [get_bd_intf_pins axi_interconnect_1/S02_AXI]
  connect_bd_intf_net -intf_net axi_dma_0_M_AXI_SG [get_bd_intf_pins axi_dma_0/M_AXI_SG] [get_bd_intf_pins axi_interconnect_1/S00_AXI]
  connect_bd_intf_net -intf_net axi_dma_1_M_AXIS_MM2S [get_bd_intf_pins axi_dma_1/M_AXIS_MM2S] [get_bd_intf_pins axis_data_fifo_5/S_AXIS]
  connect_bd_intf_net -intf_net axi_dma_1_M_AXI_MM2S [get_bd_intf_pins axi_dma_1/M_AXI_MM2S] [get_bd_intf_pins axi_interconnect_2/S01_AXI]
  connect_bd_intf_net -intf_net axi_dma_1_M_AXI_S2MM [get_bd_intf_pins axi_dma_1/M_AXI_S2MM] [get_bd_intf_pins axi_interconnect_2/S02_AXI]
  connect_bd_intf_net -intf_net axi_dma_1_M_AXI_SG [get_bd_intf_pins axi_dma_1/M_AXI_SG] [get_bd_intf_pins axi_interconnect_2/S00_AXI]
  connect_bd_intf_net -intf_net axi_dma_2_M_AXIS_MM2S [get_bd_intf_pins axi_dma_2/M_AXIS_MM2S] [get_bd_intf_pins axis_data_fifo_6/S_AXIS]
  connect_bd_intf_net -intf_net axi_dma_2_M_AXI_MM2S [get_bd_intf_pins axi_dma_2/M_AXI_MM2S] [get_bd_intf_pins axi_interconnect_3/S01_AXI]
  connect_bd_intf_net -intf_net axi_dma_2_M_AXI_S2MM [get_bd_intf_pins axi_dma_2/M_AXI_S2MM] [get_bd_intf_pins axi_interconnect_3/S02_AXI]
  connect_bd_intf_net -intf_net axi_dma_2_M_AXI_SG [get_bd_intf_pins axi_dma_2/M_AXI_SG] [get_bd_intf_pins axi_interconnect_3/S00_AXI]
  connect_bd_intf_net -intf_net axi_dma_3_M_AXIS_MM2S [get_bd_intf_pins axi_dma_3/M_AXIS_MM2S] [get_bd_intf_pins axis_data_fifo_7/S_AXIS]
  connect_bd_intf_net -intf_net axi_dma_3_M_AXI_MM2S [get_bd_intf_pins axi_dma_3/M_AXI_MM2S] [get_bd_intf_pins axi_interconnect_4/S01_AXI]
  connect_bd_intf_net -intf_net axi_dma_3_M_AXI_S2MM [get_bd_intf_pins axi_dma_3/M_AXI_S2MM] [get_bd_intf_pins axi_interconnect_4/S02_AXI]
  connect_bd_intf_net -intf_net axi_dma_3_M_AXI_SG [get_bd_intf_pins axi_dma_3/M_AXI_SG] [get_bd_intf_pins axi_interconnect_4/S00_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M00_AXI [get_bd_intf_pins axi_dma_0/S_AXI_LITE] [get_bd_intf_pins axi_interconnect_0/M00_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M01_AXI [get_bd_intf_pins axi_dma_1/S_AXI_LITE] [get_bd_intf_pins axi_interconnect_0/M01_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M02_AXI [get_bd_intf_pins axi_dma_2/S_AXI_LITE] [get_bd_intf_pins axi_interconnect_0/M02_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M03_AXI [get_bd_intf_pins axi_dma_3/S_AXI_LITE] [get_bd_intf_pins axi_interconnect_0/M03_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_0_M04_AXI [get_bd_intf_pins axi_interconnect_0/M04_AXI] [get_bd_intf_pins xadc_wiz_0/s_axi_lite]
  connect_bd_intf_net -intf_net axi_interconnect_0_M05_AXI [get_bd_intf_ports M_AXI4_LITE_0] [get_bd_intf_pins axi_interconnect_0/M05_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_1_M00_AXI [get_bd_intf_pins axi_interconnect_1/M00_AXI] [get_bd_intf_pins processing_system7/S_AXI_HP0]
  connect_bd_intf_net -intf_net axi_interconnect_2_M00_AXI [get_bd_intf_pins axi_interconnect_2/M00_AXI] [get_bd_intf_pins processing_system7/S_AXI_HP1]
  connect_bd_intf_net -intf_net axi_interconnect_3_M00_AXI [get_bd_intf_pins axi_interconnect_3/M00_AXI] [get_bd_intf_pins processing_system7/S_AXI_HP2]
  connect_bd_intf_net -intf_net axi_interconnect_4_M00_AXI [get_bd_intf_pins axi_interconnect_4/M00_AXI] [get_bd_intf_pins processing_system7/S_AXI_HP3]
  connect_bd_intf_net -intf_net axis_clock_converter_0_M_AXIS [get_bd_intf_pins axi_dma_0/S_AXIS_S2MM] [get_bd_intf_pins axis_clock_converter_0/M_AXIS]
  connect_bd_intf_net -intf_net axis_clock_converter_1_M_AXIS [get_bd_intf_pins axi_dma_1/S_AXIS_S2MM] [get_bd_intf_pins axis_clock_converter_1/M_AXIS]
  connect_bd_intf_net -intf_net axis_clock_converter_2_M_AXIS [get_bd_intf_pins axi_dma_2/S_AXIS_S2MM] [get_bd_intf_pins axis_clock_converter_2/M_AXIS]
  connect_bd_intf_net -intf_net axis_clock_converter_3_M_AXIS [get_bd_intf_pins axi_dma_3/S_AXIS_S2MM] [get_bd_intf_pins axis_clock_converter_3/M_AXIS]
  connect_bd_intf_net -intf_net axis_clock_converter_4_M_AXIS [get_bd_intf_ports M_AXI_STR_TX0] [get_bd_intf_pins axis_clock_converter_4/M_AXIS]
  connect_bd_intf_net -intf_net axis_clock_converter_5_M_AXIS [get_bd_intf_ports M_AXI_STR_TX1] [get_bd_intf_pins axis_clock_converter_5/M_AXIS]
  connect_bd_intf_net -intf_net axis_clock_converter_6_M_AXIS [get_bd_intf_ports M_AXI_STR_TX2] [get_bd_intf_pins axis_clock_converter_6/M_AXIS]
  connect_bd_intf_net -intf_net axis_clock_converter_7_M_AXIS [get_bd_intf_ports M_AXI_STR_TX3] [get_bd_intf_pins axis_clock_converter_7/M_AXIS]
  connect_bd_intf_net -intf_net axis_data_fifo_0_M_AXIS [get_bd_intf_pins axis_clock_converter_0/S_AXIS] [get_bd_intf_pins axis_data_fifo_0/M_AXIS]
  connect_bd_intf_net -intf_net axis_data_fifo_1_M_AXIS [get_bd_intf_pins axis_clock_converter_1/S_AXIS] [get_bd_intf_pins axis_data_fifo_1/M_AXIS]
  connect_bd_intf_net -intf_net axis_data_fifo_2_M_AXIS [get_bd_intf_pins axis_clock_converter_2/S_AXIS] [get_bd_intf_pins axis_data_fifo_2/M_AXIS]
  connect_bd_intf_net -intf_net axis_data_fifo_3_M_AXIS [get_bd_intf_pins axis_clock_converter_3/S_AXIS] [get_bd_intf_pins axis_data_fifo_3/M_AXIS]
  connect_bd_intf_net -intf_net axis_data_fifo_4_M_AXIS [get_bd_intf_pins axis_clock_converter_4/S_AXIS] [get_bd_intf_pins axis_data_fifo_4/M_AXIS]
  connect_bd_intf_net -intf_net axis_data_fifo_5_M_AXIS [get_bd_intf_pins axis_clock_converter_5/S_AXIS] [get_bd_intf_pins axis_data_fifo_5/M_AXIS]
  connect_bd_intf_net -intf_net axis_data_fifo_6_M_AXIS [get_bd_intf_pins axis_clock_converter_6/S_AXIS] [get_bd_intf_pins axis_data_fifo_6/M_AXIS]
  connect_bd_intf_net -intf_net axis_data_fifo_7_M_AXIS [get_bd_intf_pins axis_clock_converter_7/S_AXIS] [get_bd_intf_pins axis_data_fifo_7/M_AXIS]
  connect_bd_intf_net -intf_net processing_system7_0_M_AXI_GP0 [get_bd_intf_ports M_AXI_GP0] [get_bd_intf_pins processing_system7/M_AXI_GP0]
  connect_bd_intf_net -intf_net processing_system7_0_ddr [get_bd_intf_ports DDR] [get_bd_intf_pins processing_system7/DDR]
  connect_bd_intf_net -intf_net processing_system7_0_fixed_io [get_bd_intf_ports FIXED_IO] [get_bd_intf_pins processing_system7/FIXED_IO]
  connect_bd_intf_net -intf_net processing_system7_M_AXI_GP1 [get_bd_intf_pins axi_interconnect_0/S00_AXI] [get_bd_intf_pins processing_system7/M_AXI_GP1]

  # Create port connections
  connect_bd_net -net M_AXI_STR_TX0_aclk_1 [get_bd_ports M_AXI_STR_TX0_aclk] [get_bd_pins axis_clock_converter_4/m_axis_aclk]
  connect_bd_net -net M_AXI_STR_TX0_arstn_1 [get_bd_ports M_AXI_STR_TX0_arstn] [get_bd_pins axis_clock_converter_4/m_axis_aresetn]
  connect_bd_net -net M_AXI_STR_TX1_aclk_1 [get_bd_ports M_AXI_STR_TX1_aclk] [get_bd_pins axis_clock_converter_5/m_axis_aclk]
  connect_bd_net -net M_AXI_STR_TX1_arstn_1 [get_bd_ports M_AXI_STR_TX1_arstn] [get_bd_pins axis_clock_converter_5/m_axis_aresetn]
  connect_bd_net -net M_AXI_STR_TX2_aclk_1 [get_bd_ports M_AXI_STR_TX2_aclk] [get_bd_pins axis_clock_converter_6/m_axis_aclk]
  connect_bd_net -net M_AXI_STR_TX2_arstn_1 [get_bd_ports M_AXI_STR_TX2_arstn] [get_bd_pins axis_clock_converter_6/m_axis_aresetn]
  connect_bd_net -net M_AXI_STR_TX3_aclk_1 [get_bd_ports M_AXI_STR_TX3_aclk] [get_bd_pins axis_clock_converter_7/m_axis_aclk]
  connect_bd_net -net M_AXI_STR_TX3_arstn_1 [get_bd_ports M_AXI_STR_TX3_arstn] [get_bd_pins axis_clock_converter_7/m_axis_aresetn]
  connect_bd_net -net S_AXI_STR_RX0_aclk_1 [get_bd_ports S_AXI_STR_RX0_aclk] [get_bd_pins axis_clock_converter_0/s_axis_aclk] [get_bd_pins axis_data_fifo_0/s_axis_aclk]
  connect_bd_net -net S_AXI_STR_RX0_arstn_1 [get_bd_ports S_AXI_STR_RX0_arstn] [get_bd_pins axis_clock_converter_0/s_axis_aresetn] [get_bd_pins axis_data_fifo_0/s_axis_aresetn]
  connect_bd_net -net S_AXI_STR_RX1_aclk_1 [get_bd_ports S_AXI_STR_RX1_aclk] [get_bd_pins axis_clock_converter_1/s_axis_aclk] [get_bd_pins axis_data_fifo_1/s_axis_aclk]
  connect_bd_net -net S_AXI_STR_RX1_arstn_1 [get_bd_ports S_AXI_STR_RX1_arstn] [get_bd_pins axis_clock_converter_1/s_axis_aresetn] [get_bd_pins axis_data_fifo_1/s_axis_aresetn]
  connect_bd_net -net S_AXI_STR_RX2_aclk_1 [get_bd_ports S_AXI_STR_RX2_aclk] [get_bd_pins axis_clock_converter_2/s_axis_aclk] [get_bd_pins axis_data_fifo_2/s_axis_aclk]
  connect_bd_net -net S_AXI_STR_RX2_arstn_1 [get_bd_ports S_AXI_STR_RX2_arstn] [get_bd_pins axis_clock_converter_2/s_axis_aresetn] [get_bd_pins axis_data_fifo_2/s_axis_aresetn]
  connect_bd_net -net S_AXI_STR_RX3_aclk_1 [get_bd_ports S_AXI_STR_RX3_aclk] [get_bd_pins axis_clock_converter_3/s_axis_aclk] [get_bd_pins axis_data_fifo_3/s_axis_aclk]
  connect_bd_net -net S_AXI_STR_RX3_arstn_1 [get_bd_ports S_AXI_STR_RX3_arstn] [get_bd_pins axis_clock_converter_3/s_axis_aresetn] [get_bd_pins axis_data_fifo_3/s_axis_aresetn]
  connect_bd_net -net axi_dma_0_mm2s_introut [get_bd_pins axi_dma_0/mm2s_introut] [get_bd_pins xlconcat_0/In1]
  connect_bd_net -net axi_dma_0_s2mm_introut [get_bd_pins axi_dma_0/s2mm_introut] [get_bd_pins xlconcat_0/In0]
  connect_bd_net -net axi_dma_1_mm2s_introut [get_bd_pins axi_dma_1/mm2s_introut] [get_bd_pins xlconcat_0/In3]
  connect_bd_net -net axi_dma_1_s2mm_introut [get_bd_pins axi_dma_1/s2mm_introut] [get_bd_pins xlconcat_0/In2]
  connect_bd_net -net axi_dma_2_mm2s_introut [get_bd_pins axi_dma_2/mm2s_introut] [get_bd_pins xlconcat_0/In5]
  connect_bd_net -net axi_dma_2_s2mm_introut [get_bd_pins axi_dma_2/s2mm_introut] [get_bd_pins xlconcat_0/In4]
  connect_bd_net -net axi_dma_3_mm2s_introut [get_bd_pins axi_dma_3/mm2s_introut] [get_bd_pins xlconcat_0/In7]
  connect_bd_net -net axi_dma_3_s2mm_introut [get_bd_pins axi_dma_3/s2mm_introut] [get_bd_pins xlconcat_0/In6]
  connect_bd_net -net m_axi_gp0_aclk_1 [get_bd_ports M_AXI_GP0_ACLK] [get_bd_pins processing_system7/M_AXI_GP0_ACLK]
  connect_bd_net -net proc_sys_reset_interconnect_aresetn [get_bd_pins axi_dma_0/axi_resetn] [get_bd_pins axi_dma_1/axi_resetn] [get_bd_pins axi_dma_2/axi_resetn] [get_bd_pins axi_dma_3/axi_resetn] [get_bd_pins axi_interconnect_0/ARESETN] [get_bd_pins axi_interconnect_0/M00_ARESETN] [get_bd_pins axi_interconnect_0/M01_ARESETN] [get_bd_pins axi_interconnect_0/M02_ARESETN] [get_bd_pins axi_interconnect_0/M03_ARESETN] [get_bd_pins axi_interconnect_0/M04_ARESETN] [get_bd_pins axi_interconnect_0/M05_ARESETN] [get_bd_pins axi_interconnect_0/S00_ARESETN] [get_bd_pins axi_interconnect_1/ARESETN] [get_bd_pins axi_interconnect_1/M00_ARESETN] [get_bd_pins axi_interconnect_1/S00_ARESETN] [get_bd_pins axi_interconnect_1/S01_ARESETN] [get_bd_pins axi_interconnect_1/S02_ARESETN] [get_bd_pins axi_interconnect_2/ARESETN] [get_bd_pins axi_interconnect_2/M00_ARESETN] [get_bd_pins axi_interconnect_2/S00_ARESETN] [get_bd_pins axi_interconnect_2/S01_ARESETN] [get_bd_pins axi_interconnect_2/S02_ARESETN] [get_bd_pins axi_interconnect_3/ARESETN] [get_bd_pins axi_interconnect_3/M00_ARESETN] [get_bd_pins axi_interconnect_3/S00_ARESETN] [get_bd_pins axi_interconnect_3/S01_ARESETN] [get_bd_pins axi_interconnect_3/S02_ARESETN] [get_bd_pins axi_interconnect_4/ARESETN] [get_bd_pins axi_interconnect_4/M00_ARESETN] [get_bd_pins axi_interconnect_4/S00_ARESETN] [get_bd_pins axi_interconnect_4/S01_ARESETN] [get_bd_pins axi_interconnect_4/S02_ARESETN] [get_bd_pins axis_clock_converter_0/m_axis_aresetn] [get_bd_pins axis_clock_converter_1/m_axis_aresetn] [get_bd_pins axis_clock_converter_2/m_axis_aresetn] [get_bd_pins axis_clock_converter_3/m_axis_aresetn] [get_bd_pins axis_clock_converter_4/s_axis_aresetn] [get_bd_pins axis_clock_converter_5/s_axis_aresetn] [get_bd_pins axis_clock_converter_6/s_axis_aresetn] [get_bd_pins axis_clock_converter_7/s_axis_aresetn] [get_bd_pins axis_data_fifo_4/s_axis_aresetn] [get_bd_pins axis_data_fifo_5/s_axis_aresetn] [get_bd_pins axis_data_fifo_6/s_axis_aresetn] [get_bd_pins axis_data_fifo_7/s_axis_aresetn] [get_bd_pins proc_sys_reset/interconnect_aresetn] [get_bd_pins xadc_wiz_0/s_axi_aresetn]
  connect_bd_net -net processing_system7_0_fclk_clk2 [get_bd_ports FCLK_CLK2] [get_bd_pins processing_system7/FCLK_CLK2]
  connect_bd_net -net processing_system7_0_fclk_reset0_n [get_bd_ports FCLK_RESET0_N] [get_bd_pins processing_system7/FCLK_RESET0_N]
  connect_bd_net -net processing_system7_0_fclk_reset1_n [get_bd_ports FCLK_RESET1_N] [get_bd_pins processing_system7/FCLK_RESET1_N]
  connect_bd_net -net processing_system7_0_fclk_reset2_n [get_bd_ports FCLK_RESET2_N] [get_bd_pins processing_system7/FCLK_RESET2_N]
  connect_bd_net -net processing_system7_0_fclk_reset3_n [get_bd_ports FCLK_RESET3_N] [get_bd_pins proc_sys_reset/ext_reset_in] [get_bd_pins processing_system7/FCLK_RESET3_N]
  connect_bd_net -net processing_system7_FCLK_CLK0 [get_bd_ports FCLK_CLK0] [get_bd_pins processing_system7/FCLK_CLK0]
  connect_bd_net -net processing_system7_FCLK_CLK1 [get_bd_ports FCLK_CLK1] [get_bd_pins axi_dma_0/m_axi_mm2s_aclk] [get_bd_pins axi_dma_0/m_axi_s2mm_aclk] [get_bd_pins axi_dma_0/m_axi_sg_aclk] [get_bd_pins axi_dma_0/s_axi_lite_aclk] [get_bd_pins axi_dma_1/m_axi_mm2s_aclk] [get_bd_pins axi_dma_1/m_axi_s2mm_aclk] [get_bd_pins axi_dma_1/m_axi_sg_aclk] [get_bd_pins axi_dma_1/s_axi_lite_aclk] [get_bd_pins axi_dma_2/m_axi_mm2s_aclk] [get_bd_pins axi_dma_2/m_axi_s2mm_aclk] [get_bd_pins axi_dma_2/m_axi_sg_aclk] [get_bd_pins axi_dma_2/s_axi_lite_aclk] [get_bd_pins axi_dma_3/m_axi_mm2s_aclk] [get_bd_pins axi_dma_3/m_axi_s2mm_aclk] [get_bd_pins axi_dma_3/m_axi_sg_aclk] [get_bd_pins axi_dma_3/s_axi_lite_aclk] [get_bd_pins axi_interconnect_0/ACLK] [get_bd_pins axi_interconnect_0/M00_ACLK] [get_bd_pins axi_interconnect_0/M01_ACLK] [get_bd_pins axi_interconnect_0/M02_ACLK] [get_bd_pins axi_interconnect_0/M03_ACLK] [get_bd_pins axi_interconnect_0/M04_ACLK] [get_bd_pins axi_interconnect_0/M05_ACLK] [get_bd_pins axi_interconnect_0/S00_ACLK] [get_bd_pins axi_interconnect_1/ACLK] [get_bd_pins axi_interconnect_1/M00_ACLK] [get_bd_pins axi_interconnect_1/S00_ACLK] [get_bd_pins axi_interconnect_1/S01_ACLK] [get_bd_pins axi_interconnect_1/S02_ACLK] [get_bd_pins axi_interconnect_2/ACLK] [get_bd_pins axi_interconnect_2/M00_ACLK] [get_bd_pins axi_interconnect_2/S00_ACLK] [get_bd_pins axi_interconnect_2/S01_ACLK] [get_bd_pins axi_interconnect_2/S02_ACLK] [get_bd_pins axi_interconnect_3/ACLK] [get_bd_pins axi_interconnect_3/M00_ACLK] [get_bd_pins axi_interconnect_3/S00_ACLK] [get_bd_pins axi_interconnect_3/S01_ACLK] [get_bd_pins axi_interconnect_3/S02_ACLK] [get_bd_pins axi_interconnect_4/ACLK] [get_bd_pins axi_interconnect_4/M00_ACLK] [get_bd_pins axi_interconnect_4/S00_ACLK] [get_bd_pins axi_interconnect_4/S01_ACLK] [get_bd_pins axi_interconnect_4/S02_ACLK] [get_bd_pins axis_clock_converter_0/m_axis_aclk] [get_bd_pins axis_clock_converter_1/m_axis_aclk] [get_bd_pins axis_clock_converter_2/m_axis_aclk] [get_bd_pins axis_clock_converter_3/m_axis_aclk] [get_bd_pins axis_clock_converter_4/s_axis_aclk] [get_bd_pins axis_clock_converter_5/s_axis_aclk] [get_bd_pins axis_clock_converter_6/s_axis_aclk] [get_bd_pins axis_clock_converter_7/s_axis_aclk] [get_bd_pins axis_data_fifo_4/s_axis_aclk] [get_bd_pins axis_data_fifo_5/s_axis_aclk] [get_bd_pins axis_data_fifo_6/s_axis_aclk] [get_bd_pins axis_data_fifo_7/s_axis_aclk] [get_bd_pins proc_sys_reset/slowest_sync_clk] [get_bd_pins processing_system7/FCLK_CLK1] [get_bd_pins processing_system7/M_AXI_GP1_ACLK] [get_bd_pins processing_system7/S_AXI_HP0_ACLK] [get_bd_pins processing_system7/S_AXI_HP1_ACLK] [get_bd_pins processing_system7/S_AXI_HP2_ACLK] [get_bd_pins processing_system7/S_AXI_HP3_ACLK] [get_bd_pins xadc_wiz_0/s_axi_aclk]
  connect_bd_net -net processing_system7_FCLK_CLK3 [get_bd_ports FCLK_CLK3] [get_bd_pins processing_system7/FCLK_CLK3]
  connect_bd_net -net xadc_wiz_0_ip2intc_irpt [get_bd_pins xadc_wiz_0/ip2intc_irpt] [get_bd_pins xlconcat_0/In8]
  connect_bd_net -net xlconcat_0_dout [get_bd_pins processing_system7/IRQ_F2P] [get_bd_pins xlconcat_0/dout]
  connect_bd_net -net xlconstant_dout [get_bd_pins proc_sys_reset/aux_reset_in] [get_bd_pins xlconstant/dout]

  # Create address segments
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_0/Data_SG] [get_bd_addr_segs processing_system7/S_AXI_HP0/HP0_DDR_LOWOCM] SEG_processing_system7_HP0_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_0/Data_MM2S] [get_bd_addr_segs processing_system7/S_AXI_HP0/HP0_DDR_LOWOCM] SEG_processing_system7_HP0_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_0/Data_S2MM] [get_bd_addr_segs processing_system7/S_AXI_HP0/HP0_DDR_LOWOCM] SEG_processing_system7_HP0_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_1/Data_SG] [get_bd_addr_segs processing_system7/S_AXI_HP1/HP1_DDR_LOWOCM] SEG_processing_system7_HP1_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_1/Data_MM2S] [get_bd_addr_segs processing_system7/S_AXI_HP1/HP1_DDR_LOWOCM] SEG_processing_system7_HP1_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_1/Data_S2MM] [get_bd_addr_segs processing_system7/S_AXI_HP1/HP1_DDR_LOWOCM] SEG_processing_system7_HP1_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_2/Data_SG] [get_bd_addr_segs processing_system7/S_AXI_HP2/HP2_DDR_LOWOCM] SEG_processing_system7_HP2_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_2/Data_MM2S] [get_bd_addr_segs processing_system7/S_AXI_HP2/HP2_DDR_LOWOCM] SEG_processing_system7_HP2_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_2/Data_S2MM] [get_bd_addr_segs processing_system7/S_AXI_HP2/HP2_DDR_LOWOCM] SEG_processing_system7_HP2_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_3/Data_SG] [get_bd_addr_segs processing_system7/S_AXI_HP3/HP3_DDR_LOWOCM] SEG_processing_system7_HP3_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_3/Data_MM2S] [get_bd_addr_segs processing_system7/S_AXI_HP3/HP3_DDR_LOWOCM] SEG_processing_system7_HP3_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces axi_dma_3/Data_S2MM] [get_bd_addr_segs processing_system7/S_AXI_HP3/HP3_DDR_LOWOCM] SEG_processing_system7_HP3_DDR_LOWOCM
  create_bd_addr_seg -range 0x400000 -offset 0x83C00000 [get_bd_addr_spaces processing_system7/Data] [get_bd_addr_segs M_AXI4_LITE_0/Reg] SEG_M_AXI4_LITE_0_Reg
  create_bd_addr_seg -range 0x10000 -offset 0x80400000 [get_bd_addr_spaces processing_system7/Data] [get_bd_addr_segs axi_dma_0/S_AXI_LITE/Reg] SEG_axi_dma_0_Reg
  create_bd_addr_seg -range 0x10000 -offset 0x80410000 [get_bd_addr_spaces processing_system7/Data] [get_bd_addr_segs axi_dma_1/S_AXI_LITE/Reg] SEG_axi_dma_1_Reg
  create_bd_addr_seg -range 0x10000 -offset 0x80420000 [get_bd_addr_spaces processing_system7/Data] [get_bd_addr_segs axi_dma_2/S_AXI_LITE/Reg] SEG_axi_dma_2_Reg
  create_bd_addr_seg -range 0x10000 -offset 0x80430000 [get_bd_addr_spaces processing_system7/Data] [get_bd_addr_segs axi_dma_3/S_AXI_LITE/Reg] SEG_axi_dma_3_Reg
  create_bd_addr_seg -range 0x40000000 -offset 0x40000000 [get_bd_addr_spaces processing_system7/Data] [get_bd_addr_segs M_AXI_GP0/Reg] SEG_system_Reg
  create_bd_addr_seg -range 0x10000 -offset 0x84000000 [get_bd_addr_spaces processing_system7/Data] [get_bd_addr_segs xadc_wiz_0/s_axi_lite/Reg] SEG_xadc_wiz_0_Reg

  # Perform GUI Layout
  regenerate_bd_layout -layout_string {
   guistr: "# # String gsaved with Nlview 6.5.5  2015-06-26 bk=1.3371 VDI=38 GEI=35 GUI=JA:1.8
#  -string -flagsOSRD
preplace port S_AXI_STR_RX2_aclk -pg 1 -y 870 -defaultsOSRD
preplace port FCLK_CLK3 -pg 1 -y 1440 -defaultsOSRD
preplace port DDR -pg 1 -y 1220 -defaultsOSRD
preplace port M_AXI_STR_TX1_aclk -pg 1 -y 190 -defaultsOSRD
preplace port Vp_Vn -pg 1 -y 1630 -defaultsOSRD
preplace port S_AXI_STR_RX0_aclk -pg 1 -y 1040 -defaultsOSRD
preplace port Vaux0 -pg 1 -y 1650 -defaultsOSRD
preplace port M_AXI_GP0_ACLK -pg 1 -y 660 -defaultsOSRD
preplace port FCLK_RESET0_N -pg 1 -y 1460 -defaultsOSRD
preplace port Vaux1 -pg 1 -y 1670 -defaultsOSRD
preplace port M_AXI_STR_TX2_aresetn-pg 1 -y 130 -defaultsOSRD
preplace port M_AXI_STR_TX1_aresetn-pg 1 -y 170 -defaultsOSRD
preplace port S_AXI_STR_RX2_aresetn-pg 1 -y 850 -defaultsOSRD
preplace port M_AXI_STR_TX0_aclk -pg 1 -y 110 -defaultsOSRD
preplace port S_AXI_STR_RX0_aresetn-pg 1 -y 1020 -defaultsOSRD
preplace port M_AXI_STR_TX0 -pg 1 -y 90 -defaultsOSRD
preplace port M_AXI_GP0 -pg 1 -y 1280 -defaultsOSRD
preplace port M_AXI_STR_TX3_aclk -pg 1 -y 360 -defaultsOSRD
preplace port S_AXI_STR_RX1_aresetn-pg 1 -y 1180 -defaultsOSRD
preplace port M_AXI_STR_TX1 -pg 1 -y 580 -defaultsOSRD
preplace port M_AXI_STR_TX2_aclk -pg 1 -y 150 -defaultsOSRD
preplace port FCLK_RESET1_N -pg 1 -y 1480 -defaultsOSRD
preplace port M_AXI_STR_TX2 -pg 1 -y 260 -defaultsOSRD
preplace port FCLK_RESET3_N -pg 1 -y 1520 -defaultsOSRD
preplace port M_AXI_STR_TX3 -pg 1 -y 750 -defaultsOSRD
preplace port FIXED_IO -pg 1 -y 1240 -defaultsOSRD
preplace port FCLK_RESET2_N -pg 1 -y 1500 -defaultsOSRD
preplace port S_AXI_STR_RX0 -pg 1 -y 1000 -defaultsOSRD
preplace port M_AXI4_LITE_0 -pg 1 -y 1770 -defaultsOSRD
preplace port M_AXI_STR_TX3_aresetn-pg 1 -y 340 -defaultsOSRD
preplace port S_AXI_STR_RX3_aresetn-pg 1 -y 1980 -defaultsOSRD
preplace port FCLK_CLK0 -pg 1 -y 1380 -defaultsOSRD
preplace port S_AXI_STR_RX1 -pg 1 -y 1160 -defaultsOSRD
preplace port S_AXI_STR_RX3_aclk -pg 1 -y 2000 -defaultsOSRD
preplace port S_AXI_STR_RX1_aclk -pg 1 -y 1200 -defaultsOSRD
preplace port FCLK_CLK1 -pg 1 -y 1400 -defaultsOSRD
preplace port S_AXI_STR_RX2 -pg 1 -y 830 -defaultsOSRD
preplace port Vaux8 -pg 1 -y 1690 -defaultsOSRD
preplace port M_AXI_STR_TX0_aresetn-pg 1 -y 90 -defaultsOSRD
preplace port FCLK_CLK2 -pg 1 -y 1420 -defaultsOSRD
preplace port S_AXI_STR_RX3 -pg 1 -y 1960 -defaultsOSRD
preplace port Vaux9 -pg 1 -y 1710 -defaultsOSRD
preplace inst axis_data_fifo_1 -pg 1 -lvl 4 -y 1180 -defaultsOSRD
preplace inst axi_interconnect_4 -pg 1 -lvl 7 -y 1350 -defaultsOSRD
preplace inst axis_data_fifo_2 -pg 1 -lvl 4 -y 850 -defaultsOSRD
preplace inst axi_dma_0 -pg 1 -lvl 6 -y 1490 -defaultsOSRD
preplace inst axis_data_fifo_3 -pg 1 -lvl 4 -y 1980 -defaultsOSRD
preplace inst axi_dma_1 -pg 1 -lvl 6 -y 1910 -defaultsOSRD
preplace inst xadc_wiz_0 -pg 1 -lvl 4 -y 1680 -defaultsOSRD
preplace inst axis_data_fifo_4 -pg 1 -lvl 7 -y 430 -defaultsOSRD
preplace inst axi_dma_2 -pg 1 -lvl 6 -y 930 -defaultsOSRD
preplace inst xlconstant -pg 1 -lvl 1 -y 1290 -defaultsOSRD
preplace inst axis_data_fifo_5 -pg 1 -lvl 7 -y 580 -defaultsOSRD
preplace inst axi_dma_3 -pg 1 -lvl 6 -y 1240 -defaultsOSRD
preplace inst axis_clock_converter_0 -pg 1 -lvl 5 -y 1080 -defaultsOSRD
preplace inst axis_data_fifo_6 -pg 1 -lvl 7 -y 250 -defaultsOSRD
preplace inst xlconcat_0 -pg 1 -lvl 7 -y 1640 -defaultsOSRD
preplace inst processing_system7 -pg 1 -lvl 8 -y 1370 -defaultsOSRD
preplace inst axis_clock_converter_1 -pg 1 -lvl 5 -y 1260 -defaultsOSRD
preplace inst axis_data_fifo_7 -pg 1 -lvl 7 -y 740 -defaultsOSRD
preplace inst axis_clock_converter_2 -pg 1 -lvl 5 -y 890 -defaultsOSRD
preplace inst axis_clock_converter_3 -pg 1 -lvl 5 -y 1980 -defaultsOSRD
preplace inst axis_clock_converter_4 -pg 1 -lvl 8 -y 90 -defaultsOSRD
preplace inst axi_interconnect_0 -pg 1 -lvl 3 -y 1420 -defaultsOSRD
preplace inst axis_clock_converter_5 -pg 1 -lvl 8 -y 580 -defaultsOSRD
preplace inst axi_interconnect_1 -pg 1 -lvl 7 -y 1950 -defaultsOSRD
preplace inst axis_clock_converter_6 -pg 1 -lvl 8 -y 260 -defaultsOSRD
preplace inst axi_interconnect_2 -pg 1 -lvl 7 -y 2290 -defaultsOSRD
preplace inst axis_clock_converter_7 -pg 1 -lvl 8 -y 750 -defaultsOSRD
preplace inst axis_data_fifo_0 -pg 1 -lvl 4 -y 1020 -defaultsOSRD
preplace inst axi_interconnect_3 -pg 1 -lvl 7 -y 980 -defaultsOSRD
preplace inst proc_sys_reset -pg 1 -lvl 2 -y 1290 -defaultsOSRD
preplace netloc axis_clock_converter_7_M_AXIS 1 8 1 NJ
preplace netloc axi_dma_2_mm2s_introut 1 6 1 2010
preplace netloc S_AXI_STR_RX0_aclk_1 1 0 5 NJ 1040 NJ 1040 NJ 1040 900 930 NJ
preplace netloc S_AXI_STR_RX1_1 1 0 4 NJ 1160 NJ 1160 NJ 1160 NJ
preplace netloc axis_data_fifo_3_M_AXIS 1 4 1 1280
preplace netloc axis_clock_converter_0_M_AXIS 1 5 1 1640
preplace netloc axi_dma_0_mm2s_introut 1 6 1 2070
preplace netloc M_AXI_STR_TX3_aclk_1 1 0 8 NJ 650 NJ 650 NJ 650 NJ 650 NJ 650 NJ 650 NJ 660 NJ
preplace netloc proc_sys_reset_interconnect_aresetn 1 2 6 540 1680 900 1500 1300 990 1590 1050 2110 160 2560
preplace netloc axis_data_fifo_7_M_AXIS 1 7 1 N
preplace netloc M_AXI_STR_TX0_arstn_1 1 0 8 NJ 80 NJ 80 NJ 80 NJ 80 NJ 80 NJ 80 NJ 80 NJ
preplace netloc axi_dma_3_M_AXI_MM2S 1 6 1 2140
preplace netloc axi_dma_2_M_AXI_S2MM 1 6 1 N
preplace netloc axis_data_fifo_5_M_AXIS 1 7 1 2510
preplace netloc processing_system7_0_fclk_reset1_n 1 8 1 NJ
preplace netloc axi_interconnect_0_M04_AXI 1 3 1 890
preplace netloc M_AXI_STR_TX1_aclk_1 1 0 8 NJ 690 NJ 690 NJ 690 NJ 690 NJ 690 NJ 690 NJ 1160 NJ
preplace netloc xlconcat_0_dout 1 7 1 2560
preplace netloc axi_dma_1_M_AXI_MM2S 1 6 1 2000
preplace netloc axi_dma_0_M_AXI_SG 1 6 1 2050
preplace netloc S_AXI_STR_RX3_aclk_1 1 0 5 NJ 2000 NJ 2000 NJ 2000 880 2060 1320
preplace netloc processing_system7_FCLK_CLK0 1 8 1 NJ
preplace netloc axi_dma_3_M_AXI_S2MM 1 6 1 2130
preplace netloc processing_system7_FCLK_CLK1 1 1 8 180 1380 530 1660 910 1510 1310 980 1630 810 2100 120 2530 1570 3030
preplace netloc axi_interconnect_0_M01_AXI 1 3 3 NJ 1390 NJ 1390 1570
preplace netloc axi_interconnect_0_M02_AXI 1 3 3 NJ 770 NJ 770 1650
preplace netloc axi_dma_3_s2mm_introut 1 6 1 2080
preplace netloc processing_system7_FCLK_CLK3 1 8 1 NJ
preplace netloc axi_dma_1_s2mm_introut 1 6 1 2160
preplace netloc axi_interconnect_1_M00_AXI 1 7 1 2550
preplace netloc axis_data_fifo_1_M_AXIS 1 4 1 1250
preplace netloc m_axi_gp0_aclk_1 1 0 8 NJ 660 NJ 660 NJ 660 NJ 660 NJ 660 NJ 660 NJ 1170 NJ
preplace netloc axi_dma_3_M_AXI_SG 1 6 1 2150
preplace netloc M_AXI_STR_TX2_arstn_1 1 0 8 NJ 130 NJ 130 NJ 130 NJ 130 NJ 130 NJ 130 NJ 130 NJ
preplace netloc axi_dma_3_M_AXIS_MM2S 1 6 1 2090
preplace netloc axi_dma_0_M_AXIS_MM2S 1 6 1 2070
preplace netloc axi_interconnect_0_M05_AXI 1 3 6 NJ 1380 NJ 1380 NJ 1370 NJ 1770 NJ 1770 NJ
preplace netloc axi_dma_1_M_AXI_SG 1 6 1 2010
preplace netloc axi_interconnect_4_M00_AXI 1 7 1 2490
preplace netloc processing_system7_0_ddr 1 8 1 NJ
preplace netloc axi_dma_2_M_AXIS_MM2S 1 6 1 2000
preplace netloc S_AXI_STR_RX3_arstn_1 1 0 5 NJ 1980 NJ 1980 NJ 1980 880 1900 1260
preplace netloc axis_clock_converter_4_M_AXIS 1 8 1 NJ
preplace netloc axis_clock_converter_2_M_AXIS 1 5 1 N
preplace netloc processing_system7_0_fixed_io 1 8 1 NJ
preplace netloc axi_dma_0_M_AXI_MM2S 1 6 1 2040
preplace netloc axi_interconnect_3_M00_AXI 1 7 1 2510
preplace netloc S_AXI_STR_RX0_arstn_1 1 0 5 NJ 1020 NJ 1020 NJ 1020 910 940 1260
preplace netloc axi_dma_3_mm2s_introut 1 6 1 2090
preplace netloc axis_clock_converter_6_M_AXIS 1 8 1 NJ
preplace netloc axi_dma_0_M_AXI_S2MM 1 6 1 2020
preplace netloc Vp_Vn_1 1 0 4 NJ 1630 NJ 1630 NJ 1640 NJ
preplace netloc axis_data_fifo_2_M_AXIS 1 4 1 1300
preplace netloc S_AXI_STR_RX1_arstn_1 1 0 5 NJ 1180 NJ 1180 NJ 1180 890 740 NJ
preplace netloc processing_system7_0_fclk_reset3_n 1 1 8 170 330 NJ 330 NJ 330 NJ 330 NJ 330 NJ 330 NJ 360 3020
preplace netloc Vaux0_1 1 0 4 NJ 1650 NJ 1650 NJ 1650 NJ
preplace netloc axis_data_fifo_0_M_AXIS 1 4 1 1250
preplace netloc axis_data_fifo_6_M_AXIS 1 7 1 N
preplace netloc axis_clock_converter_5_M_AXIS 1 8 1 NJ
preplace netloc S_AXI_STR_RX1_aclk_1 1 0 5 NJ 1200 NJ 1200 NJ 1200 880 730 NJ
preplace netloc axi_dma_2_M_AXI_SG 1 6 1 N
preplace netloc axi_dma_1_mm2s_introut 1 6 1 2170
preplace netloc axis_data_fifo_4_M_AXIS 1 7 1 2490
preplace netloc axi_interconnect_2_M00_AXI 1 7 1 2570
preplace netloc processing_system7_M_AXI_GP1 1 2 7 540 350 NJ 350 NJ 350 NJ 350 NJ 350 NJ 350 3010
preplace netloc axis_clock_converter_1_M_AXIS 1 5 1 1580
preplace netloc Vaux8_1 1 0 4 NJ 1690 NJ 1690 NJ 1690 NJ
preplace netloc S_AXI_STR_RX2_aclk_1 1 0 5 NJ 870 NJ 870 NJ 870 860 710 NJ
preplace netloc xadc_wiz_0_ip2intc_irpt 1 4 3 N 1560 NJ 1610 NJ
preplace netloc S_AXI_STR_RX2_1 1 0 4 NJ 830 NJ 830 NJ 830 NJ
preplace netloc processing_system7_0_fclk_clk2 1 8 1 NJ
preplace netloc processing_system7_0_fclk_reset2_n 1 8 1 NJ
preplace netloc S_AXI_STR_RX3_1 1 0 4 NJ 1960 NJ 1960 NJ 1960 NJ
preplace netloc S_AXI_STR_RX2_arstn_1 1 0 5 NJ 850 NJ 850 NJ 850 870 750 1310
preplace netloc axi_dma_2_M_AXI_MM2S 1 6 1 N
preplace netloc processing_system7_0_fclk_reset0_n 1 8 1 NJ
preplace netloc M_AXI_STR_TX0_aclk_1 1 0 8 NJ 100 NJ 100 NJ 100 NJ 100 NJ 100 NJ 100 NJ 100 NJ
preplace netloc S_AXI_STR_RX0_1 1 0 4 NJ 1000 NJ 1000 NJ 1000 NJ
preplace netloc axis_clock_converter_3_M_AXIS 1 5 1 1610
preplace netloc axi_dma_1_M_AXIS_MM2S 1 6 1 2120
preplace netloc M_AXI_STR_TX1_arstn_1 1 0 8 NJ 170 NJ 170 NJ 170 NJ 170 NJ 170 NJ 170 NJ 170 NJ
preplace netloc M_AXI_STR_TX3_arstn_1 1 0 8 NJ 340 NJ 340 NJ 340 NJ 340 NJ 340 NJ 340 NJ 340 NJ
preplace netloc processing_system7_0_M_AXI_GP0 1 8 1 NJ
preplace netloc Vaux1_1 1 0 4 NJ 1670 NJ 1670 NJ 1670 NJ
preplace netloc M_AXI_STR_TX2_aclk_1 1 0 8 NJ 140 NJ 140 NJ 140 NJ 140 NJ 140 NJ 140 NJ 140 NJ
preplace netloc axi_dma_1_M_AXI_S2MM 1 6 1 1990
preplace netloc Vaux9_1 1 0 4 NJ 1710 NJ 1710 NJ 1710 NJ
preplace netloc axi_dma_0_s2mm_introut 1 6 1 N
preplace netloc xlconstant_dout 1 1 1 NJ
preplace netloc axi_interconnect_0_M03_AXI 1 3 3 NJ 1100 NJ 1170 1650
preplace netloc axi_interconnect_0_M00_AXI 1 3 3 NJ 1370 NJ 1370 1600
preplace netloc axi_dma_2_s2mm_introut 1 6 1 2000
levelinfo -pg 1 0 100 350 700 1090 1440 1820 2330 2790 3050 -top 0 -bot 2470
",
}

  # Restore current instance
  current_bd_instance $oldCurInst

  save_bd_design
}
# End of create_root_design()


##################################################################
# MAIN FLOW
##################################################################

create_root_design ""


