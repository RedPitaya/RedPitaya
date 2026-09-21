%module rp_ptcc

%include <typemaps.i>
%include <cstring.i>
%include <std_string.i>
%include <carrays.i>
%include <std_vector.i>
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
%apply int *OUTPUT { rp_ptcc_error *_out_status };

// The gain list is a caller allocated array in C. Wrapped as a vector so the
// Python side gets a list instead of having to manage a buffer.
%template(FloatVector) std::vector<float>;

%inline %{
#include <vector>

std::vector<float> rp_PtccGetLabMGainList(rp_ptcc_error *_out_status) {
    std::vector<float> values;
    uint32_t count = 0;

    *_out_status = rp_PtccGetLabMGainValues(NULL, 0, &count);
    if (*_out_status != RP_PTCC_OK || count == 0) {
        return values;
    }

    values.resize(count);
    *_out_status = rp_PtccGetLabMGainValues(values.data(), values.size(), &count);
    if (*_out_status != RP_PTCC_OK) {
        values.clear();
    }
    return values;
}
%}

// Structures are returned through caller allocated pointers. Let SWIG
// allocate them so the Python side gets a normal object back.
%pointer_functions(rp_ptcc_monitor_t, p_rp_ptcc_monitor_t);
%pointer_functions(rp_ptcc_params_t, p_rp_ptcc_params_t);
%pointer_functions(rp_ptcc_labm_params_t, p_rp_ptcc_labm_params_t);
%pointer_functions(rp_ptcc_labm_monitor_t, p_rp_ptcc_labm_monitor_t);
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


def read_lab_m_monitor(cached=False):
    """Reads the detection module monitor and returns it as a dictionary.

    With cached=True the sample kept by the background poller is returned
    instead of talking to the device.
    """
    holder = new_p_rp_ptcc_labm_monitor_t()
    try:
        reader = rp_PtccGetLabMMonitor if cached else rp_PtccReadLabMMonitor
        status = reader(holder)
        if status != RP_PTCC_OK:
            raise RuntimeError(rp_PtccGetErrorText(status))
        value = p_rp_ptcc_labm_monitor_t_value(holder)
        return {
            "u_sup_plus": value.u_sup_plus,
            "u_sup_minus": value.u_sup_minus,
            "u_fan": value.u_fan,
            "i_tec_plus": value.i_tec_plus,
            "i_tec_minus": value.i_tec_minus,
            "u_th1": value.u_th1,
            "u_th2": value.u_th2,
            "u_det": value.u_det,
            "u_1st": value.u_1st,
            "u_out": value.u_out,
            "temperature": value.temperature,
            "timestamp": value.timestamp,
            "valid": value.valid,
        }
    finally:
        delete_p_rp_ptcc_labm_monitor_t(holder)


def read_lab_m_params(target=RP_PTCC_REG_USER_SET):
    """Reads the detection module parameters and returns them as a dictionary."""
    holder = new_p_rp_ptcc_labm_params_t()
    try:
        status = rp_PtccGetLabMParams(target, holder)
        if status != RP_PTCC_OK:
            raise RuntimeError(rp_PtccGetErrorText(status))
        value = p_rp_ptcc_labm_params_t_value(holder)
        return {
            "det_bias_u": value.det_bias_u,
            "det_bias_i": value.det_bias_i,
            "offset": value.offset,
            "gain": value.gain,
            "gain_code": value.gain_code,
            "gain_known": value.gain_known,
            "varactor": value.varactor,
            "transimpedance": value.transimpedance,
            "coupling": value.coupling,
            "bandwidth": value.bandwidth,
            "valid": value.valid,
        }
    finally:
        delete_p_rp_ptcc_labm_params_t(holder)


def lab_m_gain_values():
    """The gains the device accepts, in V/V, ascending."""
    values, status = rp_PtccGetLabMGainList()
    if status != RP_PTCC_OK:
        raise RuntimeError(rp_PtccGetErrorText(status))
    return list(values)


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
