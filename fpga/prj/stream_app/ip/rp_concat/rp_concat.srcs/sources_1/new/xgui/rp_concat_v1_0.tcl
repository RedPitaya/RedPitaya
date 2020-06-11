# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "EVENT_SRC_NUM" -parent ${Page_0}
  ipgui::add_param $IPINST -name "TRIG_SRC_NUM" -parent ${Page_0}


}

proc update_PARAM_VALUE.EVENT_SRC_NUM { PARAM_VALUE.EVENT_SRC_NUM } {
	# Procedure called to update EVENT_SRC_NUM when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.EVENT_SRC_NUM { PARAM_VALUE.EVENT_SRC_NUM } {
	# Procedure called to validate EVENT_SRC_NUM
	return true
}

proc update_PARAM_VALUE.TRIG_SRC_NUM { PARAM_VALUE.TRIG_SRC_NUM } {
	# Procedure called to update TRIG_SRC_NUM when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.TRIG_SRC_NUM { PARAM_VALUE.TRIG_SRC_NUM } {
	# Procedure called to validate TRIG_SRC_NUM
	return true
}


proc update_MODELPARAM_VALUE.EVENT_SRC_NUM { MODELPARAM_VALUE.EVENT_SRC_NUM PARAM_VALUE.EVENT_SRC_NUM } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.EVENT_SRC_NUM}] ${MODELPARAM_VALUE.EVENT_SRC_NUM}
}

proc update_MODELPARAM_VALUE.TRIG_SRC_NUM { MODELPARAM_VALUE.TRIG_SRC_NUM PARAM_VALUE.TRIG_SRC_NUM } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.TRIG_SRC_NUM}] ${MODELPARAM_VALUE.TRIG_SRC_NUM}
}

