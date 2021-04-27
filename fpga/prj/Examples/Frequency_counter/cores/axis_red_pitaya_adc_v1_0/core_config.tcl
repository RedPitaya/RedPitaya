set display_name {AXI4-Stream Red Pitaya ADC}

set core [ipx::current_core]

set_property DISPLAY_NAME $display_name $core
set_property DESCRIPTION $display_name $core

set_property VENDOR {pavel-demin} $core
set_property VENDOR_DISPLAY_NAME {Pavel Demin} $core
set_property COMPANY_URL {https://github.com/pavel-demin/red-pitaya-notes} $core

core_parameter AXIS_TDATA_WIDTH {AXIS TDATA WIDTH} {Width of the M_AXIS data bus.}
core_parameter ADC_DATA_WIDTH {ADC DATA WIDTH} {Width of the ADC data bus.}

set bus [ipx::get_bus_interfaces -of_objects $core m_axis]
set_property NAME M_AXIS $bus
set_property INTERFACE_MODE master $bus

set bus [ipx::get_bus_interfaces adc_clk]
set parameter [ipx::add_bus_parameter ASSOCIATED_BUSIF $bus]
set_property VALUE M_AXIS $parameter
set parameter [ipx::add_bus_parameter FREQ_HZ $bus]
set_property VALUE 125000000 $parameter
