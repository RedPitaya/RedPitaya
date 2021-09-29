# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "C_S00_AXI_DATA_WIDTH" -parent ${Page_0} -widget comboBox
  ipgui::add_param $IPINST -name "C_S00_AXI_ADDR_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "C_S00_AXI_BASEADDR" -parent ${Page_0}
  ipgui::add_param $IPINST -name "C_S00_AXI_HIGHADDR" -parent ${Page_0}

  ipgui::add_param $IPINST -name "SCREEN_WIDTH"
  ipgui::add_param $IPINST -name "SCREEN_HEIGHT"
  ipgui::add_param $IPINST -name "RESET_POSX"
  ipgui::add_param $IPINST -name "RESET_POSY"
  ipgui::add_param $IPINST -name "RESET_COLOR"

}

proc update_PARAM_VALUE.RESET_COLOR { PARAM_VALUE.RESET_COLOR } {
	# Procedure called to update RESET_COLOR when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.RESET_COLOR { PARAM_VALUE.RESET_COLOR } {
	# Procedure called to validate RESET_COLOR
	return true
}

proc update_PARAM_VALUE.RESET_POSX { PARAM_VALUE.RESET_POSX } {
	# Procedure called to update RESET_POSX when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.RESET_POSX { PARAM_VALUE.RESET_POSX } {
	# Procedure called to validate RESET_POSX
	return true
}

proc update_PARAM_VALUE.RESET_POSY { PARAM_VALUE.RESET_POSY } {
	# Procedure called to update RESET_POSY when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.RESET_POSY { PARAM_VALUE.RESET_POSY } {
	# Procedure called to validate RESET_POSY
	return true
}

proc update_PARAM_VALUE.SCREEN_HEIGHT { PARAM_VALUE.SCREEN_HEIGHT } {
	# Procedure called to update SCREEN_HEIGHT when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.SCREEN_HEIGHT { PARAM_VALUE.SCREEN_HEIGHT } {
	# Procedure called to validate SCREEN_HEIGHT
	return true
}

proc update_PARAM_VALUE.SCREEN_WIDTH { PARAM_VALUE.SCREEN_WIDTH } {
	# Procedure called to update SCREEN_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.SCREEN_WIDTH { PARAM_VALUE.SCREEN_WIDTH } {
	# Procedure called to validate SCREEN_WIDTH
	return true
}

proc update_PARAM_VALUE.C_S00_AXI_DATA_WIDTH { PARAM_VALUE.C_S00_AXI_DATA_WIDTH } {
	# Procedure called to update C_S00_AXI_DATA_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_S00_AXI_DATA_WIDTH { PARAM_VALUE.C_S00_AXI_DATA_WIDTH } {
	# Procedure called to validate C_S00_AXI_DATA_WIDTH
	return true
}

proc update_PARAM_VALUE.C_S00_AXI_ADDR_WIDTH { PARAM_VALUE.C_S00_AXI_ADDR_WIDTH } {
	# Procedure called to update C_S00_AXI_ADDR_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_S00_AXI_ADDR_WIDTH { PARAM_VALUE.C_S00_AXI_ADDR_WIDTH } {
	# Procedure called to validate C_S00_AXI_ADDR_WIDTH
	return true
}

proc update_PARAM_VALUE.C_S00_AXI_BASEADDR { PARAM_VALUE.C_S00_AXI_BASEADDR } {
	# Procedure called to update C_S00_AXI_BASEADDR when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_S00_AXI_BASEADDR { PARAM_VALUE.C_S00_AXI_BASEADDR } {
	# Procedure called to validate C_S00_AXI_BASEADDR
	return true
}

proc update_PARAM_VALUE.C_S00_AXI_HIGHADDR { PARAM_VALUE.C_S00_AXI_HIGHADDR } {
	# Procedure called to update C_S00_AXI_HIGHADDR when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.C_S00_AXI_HIGHADDR { PARAM_VALUE.C_S00_AXI_HIGHADDR } {
	# Procedure called to validate C_S00_AXI_HIGHADDR
	return true
}


proc update_MODELPARAM_VALUE.C_S00_AXI_DATA_WIDTH { MODELPARAM_VALUE.C_S00_AXI_DATA_WIDTH PARAM_VALUE.C_S00_AXI_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.C_S00_AXI_DATA_WIDTH}] ${MODELPARAM_VALUE.C_S00_AXI_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.C_S00_AXI_ADDR_WIDTH { MODELPARAM_VALUE.C_S00_AXI_ADDR_WIDTH PARAM_VALUE.C_S00_AXI_ADDR_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.C_S00_AXI_ADDR_WIDTH}] ${MODELPARAM_VALUE.C_S00_AXI_ADDR_WIDTH}
}

proc update_MODELPARAM_VALUE.SCREEN_HEIGHT { MODELPARAM_VALUE.SCREEN_HEIGHT PARAM_VALUE.SCREEN_HEIGHT } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.SCREEN_HEIGHT}] ${MODELPARAM_VALUE.SCREEN_HEIGHT}
}

proc update_MODELPARAM_VALUE.SCREEN_WIDTH { MODELPARAM_VALUE.SCREEN_WIDTH PARAM_VALUE.SCREEN_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.SCREEN_WIDTH}] ${MODELPARAM_VALUE.SCREEN_WIDTH}
}

proc update_MODELPARAM_VALUE.RESET_POSX { MODELPARAM_VALUE.RESET_POSX PARAM_VALUE.RESET_POSX } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.RESET_POSX}] ${MODELPARAM_VALUE.RESET_POSX}
}

proc update_MODELPARAM_VALUE.RESET_POSY { MODELPARAM_VALUE.RESET_POSY PARAM_VALUE.RESET_POSY } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.RESET_POSY}] ${MODELPARAM_VALUE.RESET_POSY}
}

proc update_MODELPARAM_VALUE.RESET_COLOR { MODELPARAM_VALUE.RESET_COLOR PARAM_VALUE.RESET_COLOR } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.RESET_COLOR}] ${MODELPARAM_VALUE.RESET_COLOR}
}

