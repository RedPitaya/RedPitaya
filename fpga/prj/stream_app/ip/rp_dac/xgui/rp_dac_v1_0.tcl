# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  ipgui::add_page $IPINST -name "Page 0"


}

proc update_PARAM_VALUE.AXI_BURST_LEN { PARAM_VALUE.AXI_BURST_LEN } {
	# Procedure called to update AXI_BURST_LEN when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.AXI_BURST_LEN { PARAM_VALUE.AXI_BURST_LEN } {
	# Procedure called to validate AXI_BURST_LEN
	return true
}

proc update_PARAM_VALUE.DAC_DATA_BITS { PARAM_VALUE.DAC_DATA_BITS } {
	# Procedure called to update DAC_DATA_BITS when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.DAC_DATA_BITS { PARAM_VALUE.DAC_DATA_BITS } {
	# Procedure called to validate DAC_DATA_BITS
	return true
}

proc update_PARAM_VALUE.EVENT_SRC_NUM { PARAM_VALUE.EVENT_SRC_NUM } {
	# Procedure called to update EVENT_SRC_NUM when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.EVENT_SRC_NUM { PARAM_VALUE.EVENT_SRC_NUM } {
	# Procedure called to validate EVENT_SRC_NUM
	return true
}

proc update_PARAM_VALUE.ID_WIDTH { PARAM_VALUE.ID_WIDTH } {
	# Procedure called to update ID_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.ID_WIDTH { PARAM_VALUE.ID_WIDTH } {
	# Procedure called to validate ID_WIDTH
	return true
}

proc update_PARAM_VALUE.M_AXI_DAC_ADDR_BITS { PARAM_VALUE.M_AXI_DAC_ADDR_BITS } {
	# Procedure called to update M_AXI_DAC_ADDR_BITS when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.M_AXI_DAC_ADDR_BITS { PARAM_VALUE.M_AXI_DAC_ADDR_BITS } {
	# Procedure called to validate M_AXI_DAC_ADDR_BITS
	return true
}

proc update_PARAM_VALUE.M_AXI_DAC_DATA_BITS { PARAM_VALUE.M_AXI_DAC_DATA_BITS } {
	# Procedure called to update M_AXI_DAC_DATA_BITS when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.M_AXI_DAC_DATA_BITS { PARAM_VALUE.M_AXI_DAC_DATA_BITS } {
	# Procedure called to validate M_AXI_DAC_DATA_BITS
	return true
}

proc update_PARAM_VALUE.M_AXI_DAC_DATA_BITS_O { PARAM_VALUE.M_AXI_DAC_DATA_BITS_O } {
	# Procedure called to update M_AXI_DAC_DATA_BITS_O when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.M_AXI_DAC_DATA_BITS_O { PARAM_VALUE.M_AXI_DAC_DATA_BITS_O } {
	# Procedure called to validate M_AXI_DAC_DATA_BITS_O
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

proc update_MODELPARAM_VALUE.M_AXI_DAC_ADDR_BITS { MODELPARAM_VALUE.M_AXI_DAC_ADDR_BITS PARAM_VALUE.M_AXI_DAC_ADDR_BITS } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.M_AXI_DAC_ADDR_BITS}] ${MODELPARAM_VALUE.M_AXI_DAC_ADDR_BITS}
}

proc update_MODELPARAM_VALUE.M_AXI_DAC_DATA_BITS { MODELPARAM_VALUE.M_AXI_DAC_DATA_BITS PARAM_VALUE.M_AXI_DAC_DATA_BITS } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.M_AXI_DAC_DATA_BITS}] ${MODELPARAM_VALUE.M_AXI_DAC_DATA_BITS}
}

proc update_MODELPARAM_VALUE.DAC_DATA_BITS { MODELPARAM_VALUE.DAC_DATA_BITS PARAM_VALUE.DAC_DATA_BITS } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.DAC_DATA_BITS}] ${MODELPARAM_VALUE.DAC_DATA_BITS}
}

proc update_MODELPARAM_VALUE.EVENT_SRC_NUM { MODELPARAM_VALUE.EVENT_SRC_NUM PARAM_VALUE.EVENT_SRC_NUM } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.EVENT_SRC_NUM}] ${MODELPARAM_VALUE.EVENT_SRC_NUM}
}

proc update_MODELPARAM_VALUE.TRIG_SRC_NUM { MODELPARAM_VALUE.TRIG_SRC_NUM PARAM_VALUE.TRIG_SRC_NUM } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.TRIG_SRC_NUM}] ${MODELPARAM_VALUE.TRIG_SRC_NUM}
}

proc update_MODELPARAM_VALUE.M_AXI_DAC_DATA_BITS_O { MODELPARAM_VALUE.M_AXI_DAC_DATA_BITS_O PARAM_VALUE.M_AXI_DAC_DATA_BITS_O } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.M_AXI_DAC_DATA_BITS_O}] ${MODELPARAM_VALUE.M_AXI_DAC_DATA_BITS_O}
}

proc update_MODELPARAM_VALUE.ID_WIDTH { MODELPARAM_VALUE.ID_WIDTH PARAM_VALUE.ID_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.ID_WIDTH}] ${MODELPARAM_VALUE.ID_WIDTH}
}

proc update_MODELPARAM_VALUE.AXI_BURST_LEN { MODELPARAM_VALUE.AXI_BURST_LEN PARAM_VALUE.AXI_BURST_LEN } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.AXI_BURST_LEN}] ${MODELPARAM_VALUE.AXI_BURST_LEN}
}

