%module rp_ptcc

%include <typemaps.i>
%include <cstring.i>
%include <std_string.i>
%include <carrays.i>
%include <cpointer.i>

// Basic type mappings
%apply unsigned char { uint8_t }
%apply unsigned short { uint16_t }
%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }
%apply long long { int64_t }
%apply int { int32_t }

// Output parameter mappings
%apply float *OUTPUT { float *_out_value };
%apply bool *OUTPUT { bool *_out_value };
%apply unsigned int *OUTPUT { uint32_t *_out_value };
%apply unsigned int *OUTPUT { uint32_t *_out_count };
%apply unsigned long long *OUTPUT { uint64_t *_out_value };

#define __attribute__(x)

%{
#define SWIG_FILE_WITH_INIT
#include "rp_ptcc.h"
%}

// Enumerations are returned by pointer, expose them as plain integers.
%apply int *OUTPUT { rp_ptcc_module_t *_out_value };

// Structures are returned through caller allocated pointers. Let SWIG
// allocate them so the Python side gets a normal object back.
%pointer_functions(rp_ptcc_monitor_t, p_rp_ptcc_monitor_t);
%pointer_functions(rp_ptcc_params_t, p_rp_ptcc_params_t);
%pointer_functions(rp_ptcc_device_iden_t, p_rp_ptcc_device_iden_t);
%pointer_functions(rp_ptcc_module_iden_t, p_rp_ptcc_module_iden_t);

// char* out buffers used by rp_PtccGetDevicePath and rp_PtccListPorts.
%cstring_bounded_output(char *_out_value, 1024);

%include "rp_ptcc.h"

%pythoncode %{

def read_monitor():
    """Reads the monitor container and returns it as a dictionary.

    Raises RuntimeError when the device reports an error.
    """
    holder = new_p_rp_ptcc_monitor_t()
    try:
        status = rp_PtccReadMonitor(holder)
        if status != RP_PTCC_OK:
            raise RuntimeError(rp_PtccGetErrorText(status))
        value = p_rp_ptcc_monitor_t_value(holder)
        return {
            "t_det": value.t_det,
            "t_int": value.t_int,
            "i_tec": value.i_tec,
            "u_tec": value.u_tec,
            "i_sup_plus": value.i_sup_plus,
            "i_sup_minus": value.i_sup_minus,
            "u_sup_plus": value.u_sup_plus,
            "u_sup_minus": value.u_sup_minus,
            "i_fan": value.i_fan,
            "th_resistance": value.th_resistance,
            "pwm": value.pwm,
            "status": value.status,
            "status_text": rp_PtccGetStatusText(value.status),
            "supply_on": value.supply_on,
            "fan_on": value.fan_on,
            "timestamp": value.timestamp,
            "valid": value.valid,
        }
    finally:
        delete_p_rp_ptcc_monitor_t(holder)


def read_params(target=RP_PTCC_REG_USER_SET):
    """Reads a parameter register and returns it as a dictionary."""
    holder = new_p_rp_ptcc_params_t()
    try:
        status = rp_PtccGetParams(target, holder)
        if status != RP_PTCC_OK:
            raise RuntimeError(rp_PtccGetErrorText(status))
        value = p_rp_ptcc_params_t_value(holder)
        return {
            "setpoint": value.setpoint,
            "i_tec_max": value.i_tec_max,
            "u_sup_plus": value.u_sup_plus,
            "u_sup_minus": value.u_sup_minus,
            "pwm": value.pwm,
            "supply_ctrl": value.supply_ctrl,
            "fan_ctrl": value.fan_ctrl,
            "tec_ctrl": value.tec_ctrl,
            "valid": value.valid,
        }
    finally:
        delete_p_rp_ptcc_params_t(holder)


def read_device_iden():
    """Reads controller identification and returns it as a dictionary."""
    holder = new_p_rp_ptcc_device_iden_t()
    try:
        status = rp_PtccGetDeviceIden(holder)
        if status != RP_PTCC_OK:
            raise RuntimeError(rp_PtccGetErrorText(status))
        value = p_rp_ptcc_device_iden_t_value(holder)
        return {
            "type": value.type,
            "name": value.name,
            "serial": value.serial,
            "firmware_version": value.firmware_version,
            "hardware_version": value.hardware_version,
        }
    finally:
        delete_p_rp_ptcc_device_iden_t(holder)


def read_module_iden():
    """Reads module identification and returns it as a dictionary."""
    holder = new_p_rp_ptcc_module_iden_t()
    try:
        status = rp_PtccGetModuleIden(holder)
        if status != RP_PTCC_OK:
            raise RuntimeError(rp_PtccGetErrorText(status))
        value = p_rp_ptcc_module_iden_t_value(holder)
        return {
            "type": value.type,
            "name": value.name,
            "detector_name": value.detector_name,
            "serial": value.serial,
            "detector_serial": value.detector_serial,
            "cool_time": value.cool_time,
        }
    finally:
        delete_p_rp_ptcc_module_iden_t(holder)
%}
