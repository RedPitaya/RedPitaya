
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
set scripts_vivado_version 2015.3
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
  set M_AXIS_GP1_xadc [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_GP1_xadc ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXIS_GP1_xadc
  set M_AXI_GP0 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M_AXI_GP0 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.DATA_WIDTH {32} \
CONFIG.FREQ_HZ {125000000} \
CONFIG.PROTOCOL {AXI3} \
 ] $M_AXI_GP0
  set S_AXI_HP0 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 S_AXI_HP0 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.ARUSER_WIDTH {0} \
CONFIG.AWUSER_WIDTH {0} \
CONFIG.BUSER_WIDTH {0} \
CONFIG.DATA_WIDTH {64} \
CONFIG.FREQ_HZ {125000000} \
CONFIG.HAS_BRESP {1} \
CONFIG.HAS_BURST {1} \
CONFIG.HAS_CACHE {1} \
CONFIG.HAS_LOCK {1} \
CONFIG.HAS_PROT {1} \
CONFIG.HAS_QOS {1} \
CONFIG.HAS_REGION {1} \
CONFIG.HAS_RRESP {1} \
CONFIG.HAS_WSTRB {1} \
CONFIG.ID_WIDTH {0} \
CONFIG.MAX_BURST_LENGTH {16} \
CONFIG.NUM_READ_OUTSTANDING {1} \
CONFIG.NUM_WRITE_OUTSTANDING {1} \
CONFIG.PHASE {0.000} \
CONFIG.PROTOCOL {AXI3} \
CONFIG.READ_WRITE_MODE {READ_WRITE} \
CONFIG.RUSER_WIDTH {0} \
CONFIG.SUPPORTS_NARROW_BURST {1} \
CONFIG.WUSER_WIDTH {0} \
 ] $S_AXI_HP0
  set S_AXI_HP1 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 S_AXI_HP1 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.ARUSER_WIDTH {0} \
CONFIG.AWUSER_WIDTH {0} \
CONFIG.BUSER_WIDTH {0} \
CONFIG.DATA_WIDTH {64} \
CONFIG.FREQ_HZ {125000000} \
CONFIG.HAS_BRESP {1} \
CONFIG.HAS_BURST {1} \
CONFIG.HAS_CACHE {1} \
CONFIG.HAS_LOCK {1} \
CONFIG.HAS_PROT {1} \
CONFIG.HAS_QOS {1} \
CONFIG.HAS_REGION {1} \
CONFIG.HAS_RRESP {1} \
CONFIG.HAS_WSTRB {1} \
CONFIG.ID_WIDTH {0} \
CONFIG.MAX_BURST_LENGTH {16} \
CONFIG.NUM_READ_OUTSTANDING {1} \
CONFIG.NUM_WRITE_OUTSTANDING {1} \
CONFIG.PHASE {0.000} \
CONFIG.PROTOCOL {AXI3} \
CONFIG.READ_WRITE_MODE {READ_WRITE} \
CONFIG.RUSER_WIDTH {0} \
CONFIG.SUPPORTS_NARROW_BURST {1} \
CONFIG.WUSER_WIDTH {0} \
 ] $S_AXI_HP1
  set Vaux0 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux0 ]
  set Vaux1 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux1 ]
  set Vaux8 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux8 ]
  set Vaux9 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vaux9 ]
  set Vp_Vn [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_analog_io_rtl:1.0 Vp_Vn ]

  # Create ports
  set FCLK_CLK0 [ create_bd_port -dir O -type clk FCLK_CLK0 ]
  set FCLK_CLK1 [ create_bd_port -dir O -type clk FCLK_CLK1 ]
  set FCLK_CLK2 [ create_bd_port -dir O -type clk FCLK_CLK2 ]
  set FCLK_CLK3 [ create_bd_port -dir O -type clk FCLK_CLK3 ]
  set FCLK_RESET0_N [ create_bd_port -dir O -type rst FCLK_RESET0_N ]
  set FCLK_RESET1_N [ create_bd_port -dir O -type rst FCLK_RESET1_N ]
  set FCLK_RESET2_N [ create_bd_port -dir O -type rst FCLK_RESET2_N ]
  set FCLK_RESET3_N [ create_bd_port -dir O -type rst FCLK_RESET3_N ]
  set IRQ_F2P_xlconcat [ create_bd_port -dir I -from 14 -to 0 -type intr IRQ_F2P_xlconcat ]
  set_property -dict [ list \
CONFIG.PortWidth {15} \
 ] $IRQ_F2P_xlconcat
  set M_AXIS_GP1_xadc_aclk [ create_bd_port -dir O -type clk M_AXIS_GP1_xadc_aclk ]
  set M_AXI_GP0_ACLK [ create_bd_port -dir I -type clk M_AXI_GP0_ACLK ]
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {M_AXI_GP0} \
CONFIG.FREQ_HZ {125000000} \
 ] $M_AXI_GP0_ACLK
  set S_AXI_HP0_aclk [ create_bd_port -dir I -type clk S_AXI_HP0_aclk ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
 ] $S_AXI_HP0_aclk
  set S_AXI_HP1_aclk [ create_bd_port -dir I -type clk S_AXI_HP1_aclk ]
  set_property -dict [ list \
CONFIG.FREQ_HZ {125000000} \
 ] $S_AXI_HP1_aclk
  set dcm_locked [ create_bd_port -dir I dcm_locked ]

  # Create instance: axi_protocol_converter_0, and set properties
  set axi_protocol_converter_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_protocol_converter:2.1 axi_protocol_converter_0 ]

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
CONFIG.PCW_FPGA1_PERIPHERAL_FREQMHZ {250} \
CONFIG.PCW_FPGA3_PERIPHERAL_FREQMHZ {200} \
CONFIG.PCW_GPIO_MIO_GPIO_ENABLE {1} \
CONFIG.PCW_I2C0_I2C0_IO {MIO 50 .. 51} \
CONFIG.PCW_I2C0_PERIPHERAL_ENABLE {1} \
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
CONFIG.PCW_PRESET_BANK1_VOLTAGE {LVCMOS 2.5V} \
CONFIG.PCW_QSPI_GRP_SINGLE_SS_ENABLE {1} \
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
CONFIG.PCW_USE_FABRIC_INTERRUPT {1} \
CONFIG.PCW_USE_M_AXI_GP1 {1} \
CONFIG.PCW_USE_S_AXI_HP0 {1} \
CONFIG.PCW_USE_S_AXI_HP1 {1} \
 ] $processing_system7

  # Create instance: xadc, and set properties
  set xadc [ create_bd_cell -type ip -vlnv xilinx.com:ip:xadc_wiz:3.2 xadc ]
  set_property -dict [ list \
CONFIG.ADC_CONVERSION_RATE {768} \
CONFIG.AVERAGE_ENABLE_VAUXP0_VAUXN0 {true} \
CONFIG.AVERAGE_ENABLE_VAUXP1_VAUXN1 {true} \
CONFIG.AVERAGE_ENABLE_VAUXP8_VAUXN8 {true} \
CONFIG.AVERAGE_ENABLE_VAUXP9_VAUXN9 {true} \
CONFIG.BIPOLAR_VAUXP0_VAUXN0 {false} \
CONFIG.BIPOLAR_VAUXP1_VAUXN1 {false} \
CONFIG.BIPOLAR_VAUXP8_VAUXN8 {false} \
CONFIG.BIPOLAR_VAUXP9_VAUXN9 {false} \
CONFIG.CHANNEL_AVERAGING {16} \
CONFIG.CHANNEL_ENABLE_VAUXP0_VAUXN0 {true} \
CONFIG.CHANNEL_ENABLE_VAUXP1_VAUXN1 {true} \
CONFIG.CHANNEL_ENABLE_VAUXP8_VAUXN8 {true} \
CONFIG.CHANNEL_ENABLE_VAUXP9_VAUXN9 {true} \
CONFIG.CHANNEL_ENABLE_VP_VN {true} \
CONFIG.DCLK_FREQUENCY {125} \
CONFIG.ENABLE_AXI4STREAM {true} \
CONFIG.EXTERNAL_MUX_CHANNEL {VP_VN} \
CONFIG.FIFO_DEPTH {8} \
CONFIG.SEQUENCER_MODE {Off} \
CONFIG.SINGLE_CHANNEL_SELECTION {TEMPERATURE} \
CONFIG.STIMULUS_FREQ {1.0} \
CONFIG.WAVEFORM_TYPE {TRIANGLE} \
CONFIG.XADC_STARUP_SELECTION {simultaneous_sampling} \
 ] $xadc

  # Create instance: xlconcat_0, and set properties
  set xlconcat_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0 ]
  set_property -dict [ list \
CONFIG.IN0_WIDTH {1} \
CONFIG.IN1_WIDTH {15} \
 ] $xlconcat_0

  # Create instance: xlconstant, and set properties
  set xlconstant [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 xlconstant ]

  # Create interface connections
  connect_bd_intf_net -intf_net Vaux0_1 [get_bd_intf_ports Vaux0] [get_bd_intf_pins xadc/Vaux0]
  connect_bd_intf_net -intf_net Vaux1_1 [get_bd_intf_ports Vaux1] [get_bd_intf_pins xadc/Vaux1]
  connect_bd_intf_net -intf_net Vaux8_1 [get_bd_intf_ports Vaux8] [get_bd_intf_pins xadc/Vaux8]
  connect_bd_intf_net -intf_net Vaux9_1 [get_bd_intf_ports Vaux9] [get_bd_intf_pins xadc/Vaux9]
  connect_bd_intf_net -intf_net Vp_Vn_1 [get_bd_intf_ports Vp_Vn] [get_bd_intf_pins xadc/Vp_Vn]
  connect_bd_intf_net -intf_net axi_protocol_converter_0_M_AXI [get_bd_intf_pins axi_protocol_converter_0/M_AXI] [get_bd_intf_pins xadc/s_axi_lite]
  connect_bd_intf_net -intf_net processing_system7_0_M_AXI_GP0 [get_bd_intf_ports M_AXI_GP0] [get_bd_intf_pins processing_system7/M_AXI_GP0]
  connect_bd_intf_net -intf_net processing_system7_0_M_AXI_GP1 [get_bd_intf_pins axi_protocol_converter_0/S_AXI] [get_bd_intf_pins processing_system7/M_AXI_GP1]
  connect_bd_intf_net -intf_net processing_system7_0_ddr [get_bd_intf_ports DDR] [get_bd_intf_pins processing_system7/DDR]
  connect_bd_intf_net -intf_net processing_system7_0_fixed_io [get_bd_intf_ports FIXED_IO] [get_bd_intf_pins processing_system7/FIXED_IO]
  connect_bd_intf_net -intf_net s_axi_hp0_1 [get_bd_intf_ports S_AXI_HP0] [get_bd_intf_pins processing_system7/S_AXI_HP0]
  connect_bd_intf_net -intf_net s_axi_hp1_1 [get_bd_intf_ports S_AXI_HP1] [get_bd_intf_pins processing_system7/S_AXI_HP1]
  connect_bd_intf_net -intf_net xadc_M_AXIS [get_bd_intf_ports M_AXIS_GP1_xadc] [get_bd_intf_pins xadc/M_AXIS]

  # Create port connections
  connect_bd_net -net IRQ_F2P_xlconcat_1 [get_bd_ports IRQ_F2P_xlconcat] [get_bd_pins xlconcat_0/In1]
  connect_bd_net -net dcm_locked_proc_sys_reset_1 [get_bd_ports dcm_locked] [get_bd_pins proc_sys_reset/dcm_locked]
  connect_bd_net -net m_axi_gp0_aclk_1 [get_bd_ports M_AXI_GP0_ACLK] [get_bd_pins processing_system7/M_AXI_GP0_ACLK]
  connect_bd_net -net proc_sys_reset_0_interconnect_aresetn [get_bd_pins axi_protocol_converter_0/aresetn] [get_bd_pins proc_sys_reset/interconnect_aresetn]
  connect_bd_net -net proc_sys_reset_0_peripheral_aresetn [get_bd_pins proc_sys_reset/peripheral_aresetn] [get_bd_pins xadc/s_axi_aresetn]
  connect_bd_net -net processing_system7_0_fclk_clk0 [get_bd_ports FCLK_CLK0] [get_bd_ports M_AXIS_GP1_xadc_aclk] [get_bd_pins axi_protocol_converter_0/aclk] [get_bd_pins proc_sys_reset/slowest_sync_clk] [get_bd_pins processing_system7/FCLK_CLK0] [get_bd_pins processing_system7/M_AXI_GP1_ACLK] [get_bd_pins xadc/s_axi_aclk] [get_bd_pins xadc/s_axis_aclk]
  connect_bd_net -net processing_system7_0_fclk_clk1 [get_bd_ports FCLK_CLK1] [get_bd_pins processing_system7/FCLK_CLK1]
  connect_bd_net -net processing_system7_0_fclk_clk2 [get_bd_ports FCLK_CLK2] [get_bd_pins processing_system7/FCLK_CLK2]
  connect_bd_net -net processing_system7_0_fclk_clk3 [get_bd_ports FCLK_CLK3] [get_bd_pins processing_system7/FCLK_CLK3]
  connect_bd_net -net processing_system7_0_fclk_reset0_n [get_bd_ports FCLK_RESET0_N] [get_bd_pins proc_sys_reset/ext_reset_in] [get_bd_pins processing_system7/FCLK_RESET0_N]
  connect_bd_net -net processing_system7_0_fclk_reset1_n [get_bd_ports FCLK_RESET1_N] [get_bd_pins processing_system7/FCLK_RESET1_N]
  connect_bd_net -net processing_system7_0_fclk_reset2_n [get_bd_ports FCLK_RESET2_N] [get_bd_pins processing_system7/FCLK_RESET2_N]
  connect_bd_net -net processing_system7_0_fclk_reset3_n [get_bd_ports FCLK_RESET3_N] [get_bd_pins processing_system7/FCLK_RESET3_N]
  connect_bd_net -net s_axi_hp0_aclk [get_bd_ports S_AXI_HP0_aclk] [get_bd_pins processing_system7/S_AXI_HP0_ACLK]
  connect_bd_net -net s_axi_hp1_aclk [get_bd_ports S_AXI_HP1_aclk] [get_bd_pins processing_system7/S_AXI_HP1_ACLK]
  connect_bd_net -net xadc_ip2intc_irpt [get_bd_pins xadc/ip2intc_irpt] [get_bd_pins xlconcat_0/In0]
  connect_bd_net -net xlconcat_0_dout [get_bd_pins processing_system7/IRQ_F2P] [get_bd_pins xlconcat_0/dout]
  connect_bd_net -net xlconstant_dout [get_bd_pins proc_sys_reset/aux_reset_in] [get_bd_pins xlconstant/dout]

  # Create address segments
  create_bd_addr_seg -range 0x40000000 -offset 0x40000000 [get_bd_addr_spaces processing_system7/Data] [get_bd_addr_segs M_AXI_GP0/Reg] SEG_system_Reg
  create_bd_addr_seg -range 0x10000 -offset 0x83C00000 [get_bd_addr_spaces processing_system7/Data] [get_bd_addr_segs xadc/s_axi_lite/Reg] SEG_xadc_wiz_0_Reg
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces S_AXI_HP0] [get_bd_addr_segs processing_system7/S_AXI_HP0/HP0_DDR_LOWOCM] SEG_processing_system7_0_HP0_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces S_AXI_HP1] [get_bd_addr_segs processing_system7/S_AXI_HP1/HP1_DDR_LOWOCM] SEG_processing_system7_0_HP1_DDR_LOWOCM

  # Perform GUI Layout
  regenerate_bd_layout -layout_string {
   guistr: "# # String gsaved with Nlview 6.5.5  2015-06-26 bk=1.3371 VDI=38 GEI=35 GUI=JA:1.6
#  -string -flagsOSRD
preplace port FCLK_CLK3 -pg 1 -y 290 -defaultsOSRD
preplace port M_AXIS_GP1_xadc_aclk -pg 1 -y 530 -defaultsOSRD
preplace port S_AXI_HP1 -pg 1 -y 200 -defaultsOSRD
preplace port DDR -pg 1 -y 70 -defaultsOSRD
preplace port Vp_Vn -pg 1 -y 620 -defaultsOSRD
preplace port Vaux0 -pg 1 -y 640 -defaultsOSRD
preplace port FCLK_RESET0_N -pg 1 -y 310 -defaultsOSRD
preplace port M_AXI_GP0_ACLK -pg 1 -y 220 -defaultsOSRD
preplace port Vaux1 -pg 1 -y 660 -defaultsOSRD
preplace port S_AXI_HP0_aclk -pg 1 -y 240 -defaultsOSRD
preplace port M_AXI_GP0 -pg 1 -y 130 -defaultsOSRD
preplace port FCLK_RESET1_N -pg 1 -y 330 -defaultsOSRD
preplace port S_AXI_HP1_aclk -pg 1 -y 260 -defaultsOSRD
preplace port FCLK_RESET3_N -pg 1 -y 370 -defaultsOSRD
preplace port FIXED_IO -pg 1 -y 90 -defaultsOSRD
preplace port FCLK_RESET2_N -pg 1 -y 350 -defaultsOSRD
preplace port dcm_locked -pg 1 -y 470 -defaultsOSRD
preplace port FCLK_CLK0 -pg 1 -y 150 -defaultsOSRD
preplace port FCLK_CLK1 -pg 1 -y 250 -defaultsOSRD
preplace port Vaux8 -pg 1 -y 680 -defaultsOSRD
preplace port FCLK_CLK2 -pg 1 -y 270 -defaultsOSRD
preplace port M_AXIS_GP1_xadc -pg 1 -y 550 -defaultsOSRD
preplace port Vaux9 -pg 1 -y 700 -defaultsOSRD
preplace port S_AXI_HP0 -pg 1 -y 180 -defaultsOSRD
preplace portBus IRQ_F2P_xlconcat -pg 1 -y 320 -defaultsOSRD
preplace inst xlconstant -pg 1 -lvl 1 -y 420 -defaultsOSRD
preplace inst axi_protocol_converter_0 -pg 1 -lvl 3 -y 420 -defaultsOSRD
preplace inst xlconcat_0 -pg 1 -lvl 3 -y 310 -defaultsOSRD
preplace inst processing_system7 -pg 1 -lvl 4 -y 220 -defaultsOSRD
preplace inst xadc -pg 1 -lvl 4 -y 680 -defaultsOSRD
preplace inst proc_sys_reset -pg 1 -lvl 2 -y 420 -defaultsOSRD
preplace netloc Vaux0_1 1 0 4 NJ 640 NJ 640 NJ 640 NJ
preplace netloc processing_system7_0_ddr 1 4 1 NJ
preplace netloc IRQ_F2P_xlconcat_1 1 0 3 NJ 320 NJ 320 NJ
preplace netloc processing_system7_0_fclk_reset3_n 1 4 1 NJ
preplace netloc s_axi_hp0_1 1 0 4 NJ 180 NJ 180 NJ 180 NJ
preplace netloc processing_system7_0_fclk_reset2_n 1 4 1 NJ
preplace netloc processing_system7_0_M_AXI_GP0 1 4 1 NJ
preplace netloc dcm_locked_proc_sys_reset_1 1 0 2 NJ 470 NJ
preplace netloc xadc_ip2intc_irpt 1 2 3 500 500 NJ 500 1210
preplace netloc xlconstant_dout 1 1 1 NJ
preplace netloc processing_system7_0_fclk_reset1_n 1 4 1 NJ
preplace netloc Vp_Vn_1 1 0 4 NJ 620 NJ 620 NJ 620 NJ
preplace netloc processing_system7_0_M_AXI_GP1 1 2 3 490 10 NJ 10 1210
preplace netloc s_axi_hp0_aclk 1 0 4 NJ 240 NJ 240 NJ 240 NJ
preplace netloc s_axi_hp1_1 1 0 4 NJ 200 NJ 200 NJ 200 NJ
preplace netloc proc_sys_reset_0_interconnect_aresetn 1 2 1 N
preplace netloc Vaux8_1 1 0 4 NJ 680 NJ 680 NJ 680 NJ
preplace netloc axi_protocol_converter_0_M_AXI 1 3 1 770
preplace netloc xlconcat_0_dout 1 3 1 770
preplace netloc s_axi_hp1_aclk 1 0 4 NJ 250 NJ 250 NJ 250 NJ
preplace netloc processing_system7_0_fclk_reset0_n 1 1 4 140 20 NJ 20 NJ 20 1220
preplace netloc Vaux9_1 1 0 4 NJ 700 NJ 700 NJ 700 NJ
preplace netloc processing_system7_0_fixed_io 1 4 1 NJ
preplace netloc processing_system7_0_fclk_clk0 1 1 4 150 330 480 230 780 420 1230
preplace netloc proc_sys_reset_0_peripheral_aresetn 1 2 2 490 490 NJ
preplace netloc Vaux1_1 1 0 4 NJ 660 NJ 660 NJ 660 NJ
preplace netloc processing_system7_0_fclk_clk1 1 4 1 NJ
preplace netloc m_axi_gp0_aclk_1 1 0 4 NJ 220 NJ 220 NJ 220 NJ
preplace netloc xadc_M_AXIS 1 4 1 NJ
preplace netloc processing_system7_0_fclk_clk2 1 4 1 NJ
preplace netloc processing_system7_0_fclk_clk3 1 4 1 NJ
levelinfo -pg 1 -10 80 320 630 1000 1250 -top 0 -bot 860
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


