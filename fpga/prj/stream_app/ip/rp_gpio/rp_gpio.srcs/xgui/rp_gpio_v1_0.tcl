# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  ipgui::add_page $IPINST -name "Page 0"


}

proc update_PARAM_VALUE.EVENT_SRC_NUM { PARAM_VALUE.EVENT_SRC_NUM } {
	# Procedure called to update EVENT_SRC_NUM when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.EVENT_SRC_NUM { PARAM_VALUE.EVENT_SRC_NUM } {
	# Procedure called to validate EVENT_SRC_NUM
	return true
}

proc update_PARAM_VALUE.GPIO_BITS { PARAM_VALUE.GPIO_BITS } {
	# Procedure called to update GPIO_BITS when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.GPIO_BITS { PARAM_VALUE.GPIO_BITS } {
	# Procedure called to validate GPIO_BITS
	return true
}

proc update_PARAM_VALUE.M_AXI_GPIO_ADDR_BITS { PARAM_VALUE.M_AXI_GPIO_ADDR_BITS } {
	# Procedure called to update M_AXI_GPIO_ADDR_BITS when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.M_AXI_GPIO_ADDR_BITS { PARAM_VALUE.M_AXI_GPIO_ADDR_BITS } {
	# Procedure called to validate M_AXI_GPIO_ADDR_BITS
	return true
}

proc update_PARAM_VALUE.M_AXI_GPIO_DATA_BITS { PARAM_VALUE.M_AXI_GPIO_DATA_BITS } {
	# Procedure called to update M_AXI_GPIO_DATA_BITS when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.M_AXI_GPIO_DATA_BITS { PARAM_VALUE.M_AXI_GPIO_DATA_BITS } {
	# Procedure called to validate M_AXI_GPIO_DATA_BITS
	return true
}

proc update_PARAM_VALUE.S_AXI_REG_ADDR_BITS { PARAM_VALUE.S_AXI_REG_ADDR_BITS } {
	# Procedure called to update S_AXI_REG_ADDR_BITS when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.S_AXI_REG_ADDR_BITS { PARAM_VALUE.S_AXI_REG_ADDR_BITS } {
	# Procedure called to validate S_AXI_REG_ADDR_BITS
	return true
}

proc update_PARAM_VALUE.TRIG_SRC_NUM { PARAM_VALUE.TRIG_SRC_NUM } {
	# Procedure called to update TRIG_SRC_NUM when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.TRIG_SRC_NUM { PARAM_VALUE.TRIG_SRC_NUM } {
	# Procedure called to validate TRIG_SRC_NUM
	return true
}


proc update_MODELPARAM_VALUE.S_AXI_REG_ADDR_BITS { MODELPARAM_VALUE.S_AXI_REG_ADDR_BITS PARAM_VALUE.S_AXI_REG_ADDR_BITS } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.S_AXI_REG_ADDR_BITS}] ${MODELPARAM_VALUE.S_AXI_REG_ADDR_BITS}
}

proc update_MODELPARAM_VALUE.M_AXI_GPIO_ADDR_BITS { MODELPARAM_VALUE.M_AXI_GPIO_ADDR_BITS PARAM_VALUE.M_AXI_GPIO_ADDR_BITS } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.M_AXI_GPIO_ADDR_BITS}] ${MODELPARAM_VALUE.M_AXI_GPIO_ADDR_BITS}
}

proc update_MODELPARAM_VALUE.M_AXI_GPIO_DATA_BITS { MODELPARAM_VALUE.M_AXI_GPIO_DATA_BITS PARAM_VALUE.M_AXI_GPIO_DATA_BITS } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.M_AXI_GPIO_DATA_BITS}] ${MODELPARAM_VALUE.M_AXI_GPIO_DATA_BITS}
}

proc update_MODELPARAM_VALUE.GPIO_BITS { MODELPARAM_VALUE.GPIO_BITS PARAM_VALUE.GPIO_BITS } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.GPIO_BITS}] ${MODELPARAM_VALUE.GPIO_BITS}
}

proc update_MODELPARAM_VALUE.EVENT_SRC_NUM { MODELPARAM_VALUE.EVENT_SRC_NUM PARAM_VALUE.EVENT_SRC_NUM } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.EVENT_SRC_NUM}] ${MODELPARAM_VALUE.EVENT_SRC_NUM}
}

proc update_MODELPARAM_VALUE.TRIG_SRC_NUM { MODELPARAM_VALUE.TRIG_SRC_NUM PARAM_VALUE.TRIG_SRC_NUM } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.TRIG_SRC_NUM}] ${MODELPARAM_VALUE.TRIG_SRC_NUM}
}

