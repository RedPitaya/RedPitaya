
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

# This script was generated for a remote BD.
set str_bd_folder c:/Users/espero/git/RedPitaya_RadioBox/fpga/bd
set str_bd_filepath ${str_bd_folder}/${design_name}/${design_name}.bd

# Check if remote design exists on disk
if { [file exists $str_bd_filepath ] == 1 } {
   puts "ERROR: The remote BD file path <$str_bd_filepath> already exists!\n"

   puts "INFO: Please modify the variable <str_bd_folder> to another path or modify the variable <design_name>."

   return 1
}

# Check if design exists in memory
set list_existing_designs [get_bd_designs -quiet $design_name]
if { $list_existing_designs ne "" } {
   puts "ERROR: The design <$design_name> already exists in this project!"
   puts "ERROR: Will not create the remote BD <$design_name> at the folder <$str_bd_folder>.\n"

   puts "INFO: Please modify the variable <design_name>."

   return 1
}

# Check if design exists on disk within project
set list_existing_designs [get_files */${design_name}.bd]
if { $list_existing_designs ne "" } {
   puts "ERROR: The design <$design_name> already exists in this project at location:"
   puts "   $list_existing_designs"
   puts "ERROR: Will not create the remote BD <$design_name> at the folder <$str_bd_folder>.\n"

   puts "INFO: Please modify the variable <design_name>."

   return 1
}

# Now can create the remote BD
create_bd_design -dir $str_bd_folder $design_name
current_bd_design $design_name

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
  set M_AXIS_XADC [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:axis_rtl:1.0 M_AXIS_XADC ]
  set M_AXI_GP0 [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M_AXI_GP0 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.DATA_WIDTH {32} \
CONFIG.FREQ_HZ {125000000} \
CONFIG.NUM_READ_OUTSTANDING {1} \
CONFIG.NUM_WRITE_OUTSTANDING {1} \
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
  set_property -dict [ list \
CONFIG.ASSOCIATED_BUSIF {M_AXIS_XADC} \
 ] $FCLK_CLK0
  set FCLK_CLK1 [ create_bd_port -dir O -type clk FCLK_CLK1 ]
  set FCLK_CLK2 [ create_bd_port -dir O -type clk FCLK_CLK2 ]
  set FCLK_CLK3 [ create_bd_port -dir O -type clk FCLK_CLK3 ]
  set FCLK_RESET0_N [ create_bd_port -dir O -type rst FCLK_RESET0_N ]
  set FCLK_RESET1_N [ create_bd_port -dir O -type rst FCLK_RESET1_N ]
  set FCLK_RESET2_N [ create_bd_port -dir O -type rst FCLK_RESET2_N ]
  set FCLK_RESET3_N [ create_bd_port -dir O -type rst FCLK_RESET3_N ]
  set IRQ_F2P [ create_bd_port -dir I -from 14 -to 0 -type intr IRQ_F2P ]
  set_property -dict [ list \
CONFIG.PortWidth {15} \
 ] $IRQ_F2P
  set LEDs_gpio_io_o [ create_bd_port -dir O -from 8 -to 0 LEDs_gpio_io_o ]
  set M_AXIS_XADC_aclk [ create_bd_port -dir O -type clk M_AXIS_XADC_aclk ]
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

  # Create instance: axi_crossbar_GP1, and set properties
  set axi_crossbar_GP1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_crossbar:2.1 axi_crossbar_GP1 ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {32} \
CONFIG.DATA_WIDTH {32} \
CONFIG.ID_WIDTH {12} \
CONFIG.M00_A00_ADDR_WIDTH {12} \
CONFIG.M00_A00_BASE_ADDR {0x0000000081200000} \
CONFIG.M01_A00_ADDR_WIDTH {12} \
CONFIG.M01_A00_BASE_ADDR {0x0000000083C00000} \
CONFIG.PROTOCOL {AXI3} \
CONFIG.S00_SINGLE_THREAD {1} \
CONFIG.S01_BASE_ID {0x00001000} \
CONFIG.S02_BASE_ID {0x00002000} \
CONFIG.S03_BASE_ID {0x00003000} \
CONFIG.S04_BASE_ID {0x00004000} \
CONFIG.S05_BASE_ID {0x00005000} \
CONFIG.S06_BASE_ID {0x00006000} \
CONFIG.S07_BASE_ID {0x00007000} \
CONFIG.S08_BASE_ID {0x00008000} \
CONFIG.S09_BASE_ID {0x00009000} \
CONFIG.S10_BASE_ID {0x0000a000} \
CONFIG.S11_BASE_ID {0x0000b000} \
CONFIG.S12_BASE_ID {0x0000c000} \
CONFIG.S13_BASE_ID {0x0000d000} \
CONFIG.S14_BASE_ID {0x0000e000} \
CONFIG.S15_BASE_ID {0x0000f000} \
 ] $axi_crossbar_GP1

  # Create instance: axi_gpio_GP1_LEDs, and set properties
  set axi_gpio_GP1_LEDs [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_GP1_LEDs ]
  set_property -dict [ list \
CONFIG.C_ALL_INPUTS_2 {0} \
CONFIG.C_ALL_OUTPUTS {1} \
CONFIG.C_GPIO2_WIDTH {32} \
CONFIG.C_GPIO_WIDTH {9} \
CONFIG.C_IS_DUAL {0} \
 ] $axi_gpio_GP1_LEDs

  # Create instance: axi_protocol_converter_GP1_LEDs, and set properties
  set axi_protocol_converter_GP1_LEDs [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_protocol_converter:2.1 axi_protocol_converter_GP1_LEDs ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {12} \
CONFIG.DATA_WIDTH {32} \
CONFIG.ID_WIDTH {12} \
CONFIG.MI_PROTOCOL {AXI4LITE} \
CONFIG.READ_WRITE_MODE {READ_WRITE} \
CONFIG.SI_PROTOCOL {AXI3} \
CONFIG.TRANSLATION_MODE {2} \
 ] $axi_protocol_converter_GP1_LEDs

  # Create instance: axi_protocol_converter_GP1_XADC, and set properties
  set axi_protocol_converter_GP1_XADC [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_protocol_converter:2.1 axi_protocol_converter_GP1_XADC ]
  set_property -dict [ list \
CONFIG.ADDR_WIDTH {12} \
CONFIG.DATA_WIDTH {32} \
CONFIG.ID_WIDTH {12} \
CONFIG.MI_PROTOCOL {AXI4LITE} \
CONFIG.READ_WRITE_MODE {READ_WRITE} \
CONFIG.SI_PROTOCOL {AXI3} \
CONFIG.TRANSLATION_MODE {2} \
 ] $axi_protocol_converter_GP1_XADC

  # Create instance: proc_sys_reset_0, and set properties
  set proc_sys_reset_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 proc_sys_reset_0 ]
  set_property -dict [ list \
CONFIG.C_AUX_RESET_HIGH {1} \
CONFIG.C_EXT_RST_WIDTH {1} \
 ] $proc_sys_reset_0

  # Create instance: processing_system7_0, and set properties
  set processing_system7_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 processing_system7_0 ]
  set_property -dict [ list \
CONFIG.PCW_APU_CLK_RATIO_ENABLE {6:2:1} \
CONFIG.PCW_ARMPLL_CTRL_FBDIV {40} \
CONFIG.PCW_CORE0_FIQ_INTR {0} \
CONFIG.PCW_CPU_PERIPHERAL_CLKSRC {ARM PLL} \
CONFIG.PCW_CPU_PERIPHERAL_DIVISOR0 {2} \
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
CONFIG.PCW_M_AXI_GP1_ENABLE_STATIC_REMAP {0} \
CONFIG.PCW_OVERRIDE_BASIC_CLOCK {0} \
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
CONFIG.PCW_SPI1_GRP_SS1_ENABLE {0} \
CONFIG.PCW_SPI1_GRP_SS2_ENABLE {0} \
CONFIG.PCW_SPI1_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_SPI1_SPI1_IO {MIO 10 .. 15} \
CONFIG.PCW_TTC0_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_UART0_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_UART0_UART0_IO {MIO 14 .. 15} \
CONFIG.PCW_UART1_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_UART1_UART1_IO {MIO 8 .. 9} \
CONFIG.PCW_UIPARAM_DDR_BUS_WIDTH {16 Bit} \
CONFIG.PCW_UIPARAM_DDR_PARTNO {MT41J256M16 RE-125} \
CONFIG.PCW_UIPARAM_GENERATE_SUMMARY {NONE} \
CONFIG.PCW_USB0_PERIPHERAL_ENABLE {1} \
CONFIG.PCW_USB0_RESET_ENABLE {1} \
CONFIG.PCW_USB0_RESET_IO {MIO 48} \
CONFIG.PCW_USE_AXI_NONSECURE {1} \
CONFIG.PCW_USE_EXPANDED_IOP {0} \
CONFIG.PCW_USE_FABRIC_INTERRUPT {1} \
CONFIG.PCW_USE_M_AXI_GP1 {1} \
CONFIG.PCW_USE_S_AXI_GP0 {0} \
CONFIG.PCW_USE_S_AXI_HP0 {1} \
CONFIG.PCW_USE_S_AXI_HP1 {1} \
CONFIG.PCW_WDT_PERIPHERAL_ENABLE {0} \
 ] $processing_system7_0

  # Create instance: xadc_GP1, and set properties
  set xadc_GP1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xadc_wiz:3.2 xadc_GP1 ]
  set_property -dict [ list \
CONFIG.ADC_CONVERSION_RATE {768} \
CONFIG.AVERAGE_ENABLE_VAUXP0_VAUXN0 {true} \
CONFIG.AVERAGE_ENABLE_VAUXP1_VAUXN1 {true} \
CONFIG.AVERAGE_ENABLE_VAUXP8_VAUXN8 {true} \
CONFIG.AVERAGE_ENABLE_VAUXP9_VAUXN9 {true} \
CONFIG.CHANNEL_AVERAGING {16} \
CONFIG.CHANNEL_ENABLE_VAUXP0_VAUXN0 {true} \
CONFIG.CHANNEL_ENABLE_VAUXP1_VAUXN1 {true} \
CONFIG.CHANNEL_ENABLE_VAUXP8_VAUXN8 {true} \
CONFIG.CHANNEL_ENABLE_VAUXP9_VAUXN9 {true} \
CONFIG.CHANNEL_ENABLE_VP_VN {true} \
CONFIG.DCLK_FREQUENCY {125} \
CONFIG.ENABLE_AXI4STREAM {true} \
CONFIG.ENABLE_RESET {false} \
CONFIG.FIFO_DEPTH {8} \
CONFIG.INTERFACE_SELECTION {Enable_AXI} \
CONFIG.NUM_WAVE {1000} \
CONFIG.SEQUENCER_MODE {Off} \
CONFIG.STIMULUS_FREQ {4.8} \
CONFIG.WAVEFORM_TYPE {TRIANGLE} \
CONFIG.XADC_STARUP_SELECTION {simultaneous_sampling} \
 ] $xadc_GP1

  # Create instance: xlconcat_irq, and set properties
  set xlconcat_irq [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_irq ]
  set_property -dict [ list \
CONFIG.IN0_WIDTH {1} \
CONFIG.IN1_WIDTH {15} \
CONFIG.NUM_PORTS {2} \
 ] $xlconcat_irq

  # Create instance: xlconstant_reset, and set properties
  set xlconstant_reset [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:1.1 xlconstant_reset ]

  # Create interface connections
  connect_bd_intf_net -intf_net Vaux0_1 [get_bd_intf_ports Vaux0] [get_bd_intf_pins xadc_GP1/Vaux0]
  connect_bd_intf_net -intf_net Vaux1_1 [get_bd_intf_ports Vaux1] [get_bd_intf_pins xadc_GP1/Vaux1]
  connect_bd_intf_net -intf_net Vaux8_1 [get_bd_intf_ports Vaux8] [get_bd_intf_pins xadc_GP1/Vaux8]
  connect_bd_intf_net -intf_net Vaux9_1 [get_bd_intf_ports Vaux9] [get_bd_intf_pins xadc_GP1/Vaux9]
  connect_bd_intf_net -intf_net Vp_Vn_1 [get_bd_intf_ports Vp_Vn] [get_bd_intf_pins xadc_GP1/Vp_Vn]
  connect_bd_intf_net -intf_net axi_crossbar_GP1_M00_AXI [get_bd_intf_pins axi_crossbar_GP1/M00_AXI] [get_bd_intf_pins axi_protocol_converter_GP1_XADC/S_AXI]
  connect_bd_intf_net -intf_net axi_crossbar_GP1_M01_AXI [get_bd_intf_pins axi_crossbar_GP1/M01_AXI] [get_bd_intf_pins axi_protocol_converter_GP1_LEDs/S_AXI]
  connect_bd_intf_net -intf_net axi_protocol_converter_0_M_AXI1 [get_bd_intf_pins axi_gpio_GP1_LEDs/S_AXI] [get_bd_intf_pins axi_protocol_converter_GP1_LEDs/M_AXI]
  connect_bd_intf_net -intf_net axi_protocol_converter_GP1_XADC_M_AXI [get_bd_intf_pins axi_protocol_converter_GP1_XADC/M_AXI] [get_bd_intf_pins xadc_GP1/s_axi_lite]
  connect_bd_intf_net -intf_net processing_system7_0_M_AXI_GP1 [get_bd_intf_pins axi_crossbar_GP1/S00_AXI] [get_bd_intf_pins processing_system7_0/M_AXI_GP1]
  connect_bd_intf_net -intf_net processing_system7_0_ddr [get_bd_intf_ports DDR] [get_bd_intf_pins processing_system7_0/DDR]
  connect_bd_intf_net -intf_net processing_system7_0_fixed_io [get_bd_intf_ports FIXED_IO] [get_bd_intf_pins processing_system7_0/FIXED_IO]
  connect_bd_intf_net -intf_net processing_system7_0_m_axi_gp0 [get_bd_intf_ports M_AXI_GP0] [get_bd_intf_pins processing_system7_0/M_AXI_GP0]
  connect_bd_intf_net -intf_net s_axi_hp0_1 [get_bd_intf_ports S_AXI_HP0] [get_bd_intf_pins processing_system7_0/S_AXI_HP0]
  connect_bd_intf_net -intf_net s_axi_hp1_1 [get_bd_intf_ports S_AXI_HP1] [get_bd_intf_pins processing_system7_0/S_AXI_HP1]
  connect_bd_intf_net -intf_net xadc_GP1_M_AXIS [get_bd_intf_ports M_AXIS_XADC] [get_bd_intf_pins xadc_GP1/M_AXIS]

  # Create port connections
  connect_bd_net -net IRQ_F2P_1 [get_bd_ports IRQ_F2P] [get_bd_pins xlconcat_irq/In1]
  connect_bd_net -net Net [get_bd_pins axi_crossbar_GP1/aresetn] [get_bd_pins axi_protocol_converter_GP1_LEDs/aresetn] [get_bd_pins axi_protocol_converter_GP1_XADC/aresetn] [get_bd_pins proc_sys_reset_0/interconnect_aresetn]
  connect_bd_net -net axi_gpio_0_gpio_io_o1 [get_bd_ports LEDs_gpio_io_o] [get_bd_pins axi_gpio_GP1_LEDs/gpio_io_o]
  connect_bd_net -net dcm_locked_1 [get_bd_ports dcm_locked] [get_bd_pins proc_sys_reset_0/dcm_locked]
  connect_bd_net -net m_axi_gp0_aclk_1 [get_bd_ports M_AXI_GP0_ACLK] [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK]
  connect_bd_net -net proc_sys_reset_0_peripheral_aresetn [get_bd_pins axi_gpio_GP1_LEDs/s_axi_aresetn] [get_bd_pins proc_sys_reset_0/peripheral_aresetn] [get_bd_pins xadc_GP1/s_axi_aresetn]
  connect_bd_net -net processing_system7_0_FCLK_CLK3 [get_bd_ports FCLK_CLK3] [get_bd_pins processing_system7_0/FCLK_CLK3]
  connect_bd_net -net processing_system7_0_fclk_clk0 [get_bd_ports FCLK_CLK0] [get_bd_ports M_AXIS_XADC_aclk] [get_bd_pins axi_crossbar_GP1/aclk] [get_bd_pins axi_gpio_GP1_LEDs/s_axi_aclk] [get_bd_pins axi_protocol_converter_GP1_LEDs/aclk] [get_bd_pins axi_protocol_converter_GP1_XADC/aclk] [get_bd_pins proc_sys_reset_0/slowest_sync_clk] [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins processing_system7_0/M_AXI_GP1_ACLK] [get_bd_pins xadc_GP1/s_axi_aclk] [get_bd_pins xadc_GP1/s_axis_aclk]
  connect_bd_net -net processing_system7_0_fclk_clk1 [get_bd_ports FCLK_CLK1] [get_bd_pins processing_system7_0/FCLK_CLK1]
  connect_bd_net -net processing_system7_0_fclk_clk2 [get_bd_ports FCLK_CLK2] [get_bd_pins processing_system7_0/FCLK_CLK2]
  connect_bd_net -net processing_system7_0_fclk_reset0_n [get_bd_ports FCLK_RESET0_N] [get_bd_pins proc_sys_reset_0/ext_reset_in] [get_bd_pins processing_system7_0/FCLK_RESET0_N]
  connect_bd_net -net processing_system7_0_fclk_reset1_n [get_bd_ports FCLK_RESET1_N] [get_bd_pins processing_system7_0/FCLK_RESET1_N]
  connect_bd_net -net processing_system7_0_fclk_reset2_n [get_bd_ports FCLK_RESET2_N] [get_bd_pins processing_system7_0/FCLK_RESET2_N]
  connect_bd_net -net processing_system7_0_fclk_reset3_n [get_bd_ports FCLK_RESET3_N] [get_bd_pins processing_system7_0/FCLK_RESET3_N]
  connect_bd_net -net s_axi_hp0_aclk [get_bd_ports S_AXI_HP0_aclk] [get_bd_pins processing_system7_0/S_AXI_HP0_ACLK]
  connect_bd_net -net s_axi_hp1_aclk [get_bd_ports S_AXI_HP1_aclk] [get_bd_pins processing_system7_0/S_AXI_HP1_ACLK]
  connect_bd_net -net xadc_GP1_ip2intc_irpt [get_bd_pins xadc_GP1/ip2intc_irpt] [get_bd_pins xlconcat_irq/In0]
  connect_bd_net -net xlconcat_0_dout [get_bd_pins processing_system7_0/IRQ_F2P] [get_bd_pins xlconcat_irq/dout]
  connect_bd_net -net xlconstant_dout [get_bd_pins proc_sys_reset_0/aux_reset_in] [get_bd_pins proc_sys_reset_0/mb_debug_sys_rst] [get_bd_pins xlconstant_reset/dout]

  # Create address segments
  create_bd_addr_seg -range 0x10000 -offset 0x81200000 [get_bd_addr_spaces processing_system7_0/Data] [get_bd_addr_segs axi_gpio_GP1_LEDs/S_AXI/Reg] SEG_axi_gpio_GP1_LEDs_Reg
  create_bd_addr_seg -range 0x40000000 -offset 0x40000000 [get_bd_addr_spaces processing_system7_0/Data] [get_bd_addr_segs M_AXI_GP0/Reg] SEG_system_Reg
  create_bd_addr_seg -range 0x10000 -offset 0x83C00000 [get_bd_addr_spaces processing_system7_0/Data] [get_bd_addr_segs xadc_GP1/s_axi_lite/Reg] SEG_xadc_GP1_Reg
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces S_AXI_HP0] [get_bd_addr_segs processing_system7_0/S_AXI_HP0/HP0_DDR_LOWOCM] SEG_processing_system7_0_HP0_DDR_LOWOCM
  create_bd_addr_seg -range 0x20000000 -offset 0x0 [get_bd_addr_spaces S_AXI_HP1] [get_bd_addr_segs processing_system7_0/S_AXI_HP1/HP1_DDR_LOWOCM] SEG_processing_system7_0_HP1_DDR_LOWOCM

  # Perform GUI Layout
  regenerate_bd_layout -layout_string {
   guistr: "# # String gsaved with Nlview 6.5.5  2015-06-26 bk=1.3371 VDI=38 GEI=35 GUI=JA:1.6
#  -string -flagsOSRD
preplace port FCLK_CLK3 -pg 1 -y 280 -defaultsOSRD
preplace port S_AXI_HP1 -pg 1 -y 180 -defaultsOSRD
preplace port DDR -pg 1 -y 60 -defaultsOSRD
preplace port Vp_Vn -pg 1 -y 680 -defaultsOSRD
preplace port Vaux0 -pg 1 -y 710 -defaultsOSRD
preplace port FCLK_RESET0_N -pg 1 -y 300 -defaultsOSRD
preplace port M_AXI_GP0_ACLK -pg 1 -y 200 -defaultsOSRD
preplace port Vaux1 -pg 1 -y 730 -defaultsOSRD
preplace port S_AXI_HP0_aclk -pg 1 -y 240 -defaultsOSRD
preplace port M_AXI_GP0 -pg 1 -y 120 -defaultsOSRD
preplace port FCLK_RESET1_N -pg 1 -y 320 -defaultsOSRD
preplace port S_AXI_HP1_aclk -pg 1 -y 260 -defaultsOSRD
preplace port FCLK_RESET3_N -pg 1 -y 360 -defaultsOSRD
preplace port FIXED_IO -pg 1 -y 80 -defaultsOSRD
preplace port FCLK_RESET2_N -pg 1 -y 340 -defaultsOSRD
preplace port M_AXIS_XADC -pg 1 -y 810 -defaultsOSRD
preplace port FCLK_CLK0 -pg 1 -y 220 -defaultsOSRD
preplace port dcm_locked -pg 1 -y 600 -defaultsOSRD
preplace port FCLK_CLK1 -pg 1 -y 240 -defaultsOSRD
preplace port M_AXIS_XADC_aclk -pg 1 -y 790 -defaultsOSRD
preplace port Vaux8 -pg 1 -y 760 -defaultsOSRD
preplace port FCLK_CLK2 -pg 1 -y 260 -defaultsOSRD
preplace port Vaux9 -pg 1 -y 780 -defaultsOSRD
preplace port S_AXI_HP0 -pg 1 -y 160 -defaultsOSRD
preplace portBus IRQ_F2P -pg 1 -y 330 -defaultsOSRD
preplace portBus LEDs_gpio_io_o -pg 1 -y 1240 -defaultsOSRD
preplace inst proc_sys_reset_0 -pg 1 -lvl 2 -y 550 -defaultsOSRD
preplace inst axi_gpio_GP1_LEDs -pg 1 -lvl 5 -y 1230 -defaultsOSRD
preplace inst xlconcat_irq -pg 1 -lvl 3 -y 320 -defaultsOSRD
preplace inst axi_protocol_converter_GP1_LEDs -pg 1 -lvl 4 -y 1210 -defaultsOSRD
preplace inst axi_protocol_converter_GP1_XADC -pg 1 -lvl 4 -y 510 -defaultsOSRD
preplace inst xlconstant_reset -pg 1 -lvl 1 -y 550 -defaultsOSRD
preplace inst xadc_GP1 -pg 1 -lvl 5 -y 940 -defaultsOSRD
preplace inst axi_crossbar_GP1 -pg 1 -lvl 3 -y 500 -defaultsOSRD
preplace inst processing_system7_0 -pg 1 -lvl 5 -y 200 -defaultsOSRD
preplace netloc Vaux0_1 1 0 5 NJ 710 NJ 710 NJ 710 NJ 710 NJ
preplace netloc processing_system7_0_ddr 1 5 1 NJ
preplace netloc processing_system7_0_fclk_reset3_n 1 5 1 NJ
preplace netloc processing_system7_0_FCLK_CLK3 1 5 1 NJ
preplace netloc s_axi_hp0_1 1 0 5 NJ 160 NJ 160 NJ 160 NJ 160 NJ
preplace netloc processing_system7_0_fclk_reset2_n 1 5 1 NJ
preplace netloc axi_protocol_converter_GP1_XADC_M_AXI 1 4 1 1310
preplace netloc xlconstant_dout 1 1 1 30
preplace netloc processing_system7_0_fclk_reset1_n 1 5 1 NJ
preplace netloc Vp_Vn_1 1 0 5 NJ 680 NJ 680 NJ 680 NJ 680 1300
preplace netloc processing_system7_0_M_AXI_GP1 1 2 4 580 -10 NJ -10 NJ -10 1750
preplace netloc s_axi_hp0_aclk 1 0 5 NJ 240 NJ 240 NJ 240 NJ 240 NJ
preplace netloc dcm_locked_1 1 0 2 NJ 600 NJ
preplace netloc s_axi_hp1_1 1 0 5 NJ 180 NJ 180 NJ 180 NJ 180 NJ
preplace netloc xadc_GP1_ip2intc_irpt 1 2 4 590 410 NJ 410 NJ 410 1740
preplace netloc Vaux8_1 1 0 5 NJ 760 NJ 760 NJ 760 NJ 760 NJ
preplace netloc axi_crossbar_GP1_M00_AXI 1 3 1 N
preplace netloc xlconcat_0_dout 1 3 2 850 280 N
preplace netloc s_axi_hp1_aclk 1 0 5 NJ 260 NJ 260 NJ 260 NJ 260 NJ
preplace netloc processing_system7_0_fclk_reset0_n 1 1 5 20 0 NJ 0 NJ 0 NJ 0 1770
preplace netloc axi_gpio_0_gpio_io_o1 1 5 1 NJ
preplace netloc Vaux9_1 1 0 5 NJ 780 NJ 780 NJ 780 NJ 780 NJ
preplace netloc processing_system7_0_fixed_io 1 5 1 NJ
preplace netloc axi_crossbar_GP1_M01_AXI 1 3 1 850
preplace netloc processing_system7_0_fclk_clk0 1 1 5 30 460 570 580 860 580 1290 400 1750
preplace netloc proc_sys_reset_0_peripheral_aresetn 1 2 3 N 590 N 590 1260
preplace netloc Vaux1_1 1 0 5 NJ 730 NJ 730 NJ 730 NJ 730 NJ
preplace netloc processing_system7_0_fclk_clk1 1 5 1 NJ
preplace netloc m_axi_gp0_aclk_1 1 0 5 NJ 200 NJ 200 NJ 200 NJ 200 NJ
preplace netloc Net 1 2 2 590 570 870
preplace netloc xadc_GP1_M_AXIS 1 5 1 NJ
preplace netloc axi_protocol_converter_0_M_AXI1 1 4 1 N
preplace netloc processing_system7_0_fclk_clk2 1 5 1 NJ
preplace netloc processing_system7_0_m_axi_gp0 1 5 1 NJ
preplace netloc IRQ_F2P_1 1 0 3 NJ 330 NJ 330 N
levelinfo -pg 1 -200 -60 410 730 1110 1530 1820 -top -20 -bot 1300
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


