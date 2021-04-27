set display_name {AXI BRAM Reader}

set core [ipx::current_core]

set_property DISPLAY_NAME $display_name $core
set_property DESCRIPTION $display_name $core

set_property VENDOR {anton-potocnik} $core
set_property VENDOR_DISPLAY_NAME {Anton Potocnik} $core
set_property COMPANY_URL {http://antonpotocnik.com} $core

core_parameter C_S00_AXI_DATA_WIDTH {C S00 AXI DATA WIDTH} {Width of the S_AXI data bus.}
core_parameter C_S00_AXI_ADDR_WIDTH {C S00 AXI ADDR WIDTH} {Width of the S_AXI addr bus.}
core_parameter BRAM_DATA_WIDTH {BRAM DATA WIDTH} {Width of the BRAM data port.}
core_parameter BRAM_ADDR_WIDTH {BRAM ADDR WIDTH} {Width of the BRAM address port.}

set bus [ipx::get_bus_interfaces -of_objects $core s00_axi]
set_property NAME S_AXI $bus
set_property INTERFACE_MODE slave $bus

set bus [ipx::get_bus_interfaces s00_axi_aclk]
set parameter [ipx::get_bus_parameters -of_objects $bus ASSOCIATED_BUSIF]
set_property VALUE S_AXI $parameter

set bus [ipx::add_bus_interface BRAM_PORTA $core]
set_property ABSTRACTION_TYPE_VLNV xilinx.com:interface:bram_rtl:1.0 $bus
set_property BUS_TYPE_VLNV xilinx.com:interface:bram:1.0 $bus
set_property INTERFACE_MODE master $bus
foreach {logical physical} {
  RST  bram_porta_rst
  CLK  bram_porta_clk
  ADDR bram_porta_addr
  DIN  bram_porta_din
  DOUT bram_porta_dout
  WE   bram_porta_we
  EN   bram_porta_en
} {
  set_property PHYSICAL_NAME $physical [ipx::add_port_map $logical $bus]
}

set bus [ipx::get_bus_interfaces bram_porta_clk]
set parameter [ipx::add_bus_parameter ASSOCIATED_BUSIF $bus]
set_property VALUE BRAM_PORTA $parameter


