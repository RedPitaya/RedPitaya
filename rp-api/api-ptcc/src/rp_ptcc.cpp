/**
 * $Id: $
 *
 * @brief Red Pitaya API for the VIGO Photonics PTCC-01 controller.
 *
 * Thin C layer over ptcc::Bridge, which delegates all protocol work to the
 * upstream Python library.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include "rp_ptcc.h"

#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "ptcc_bridge.h"
#include "rp_log.h"

#ifndef VERSION
#define VERSION 0.00-0000
#endif

#ifndef REVISION
#define REVISION devbuild
#endif

#define STRINGIFY_INNER(x) #x
#define STRINGIFY(x) STRINGIFY_INNER(x)

namespace {

std::mutex g_mutex;

// Deliberately leaked: the bridge owns PyObject references, and destroying it
// during static destruction would touch an already finalized interpreter. It
// is created once and only closed, never deleted.
ptcc::Bridge *g_bridge = nullptr;
bool g_device_open = false;

rp_ptcc_error toApiError(ptcc::Result result) {
    switch (result) {
        case ptcc::Result::OK:
            return RP_PTCC_OK;
        case ptcc::Result::NOT_INITIALIZED:
            return RP_PTCC_ENOINIT;
        case ptcc::Result::OPEN_FAILED:
            return RP_PTCC_EOPEN;
        case ptcc::Result::IO_ERROR:
            return RP_PTCC_EIO;
        case ptcc::Result::TIMEOUT:
            return RP_PTCC_ETIMEOUT;
        case ptcc::Result::BAD_RESPONSE:
            return RP_PTCC_ERESP;
        case ptcc::Result::INVALID_PARAM:
            return RP_PTCC_EIP;
        case ptcc::Result::OUT_OF_RANGE:
            return RP_PTCC_ERANGE;
        case ptcc::Result::NOT_SUPPORTED:
            return RP_PTCC_ENOTSUP;
        case ptcc::Result::NO_DEVICE:
            return RP_PTCC_ENODEV;
        case ptcc::Result::PYTHON_ERROR:
            return RP_PTCC_EPYTHON;
        default:
            return RP_PTCC_EIO;
    }
}

/** Bridge for lookups that need no device; creates it on first use. */
ptcc::Bridge *lookupBridge() {
    if (g_bridge == nullptr) {
        auto *instance = new ptcc::Bridge();
        if (instance->initialize() != ptcc::Result::OK) {
            delete instance;
            return nullptr;
        }
        g_bridge = instance;
    }
    return g_bridge;
}

/** Bridge for device operations, null unless a controller is open.
 *  The lookup helpers above keep g_bridge alive after a release, so the open
 *  state has to be tracked separately. */
ptcc::Bridge *bridge() { return g_device_open ? g_bridge : nullptr; }

void copyString(char *destination, size_t size, const std::string &source) {
    if (destination == nullptr || size == 0) {
        return;
    }
    const size_t length = source.size() < (size - 1) ? source.size() : (size - 1);
    std::memcpy(destination, source.data(), length);
    destination[length] = '\0';
}

void fillMonitor(rp_ptcc_monitor_t *out, const ptcc::MonitorData &data) {
    out->t_det = static_cast<float>(data.t_det_k);
    out->t_int = static_cast<float>(data.t_int_c);
    out->i_tec = static_cast<float>(data.i_tec_a);
    out->u_tec = static_cast<float>(data.u_tec_v);
    out->i_sup_plus = static_cast<float>(data.i_sup_plus_a);
    out->i_sup_minus = static_cast<float>(data.i_sup_minus_a);
    out->u_sup_plus = static_cast<float>(data.u_sup_plus_v);
    out->u_sup_minus = static_cast<float>(data.u_sup_minus_v);
    out->i_fan = static_cast<float>(data.i_fan_a);
    out->th_resistance = static_cast<float>(data.th_resistance);
    out->pwm = data.pwm;
    out->status = data.status;
    out->supply_on = data.supply_on;
    out->fan_on = data.fan_on;
    out->timestamp = data.timestamp_ms;
    out->valid = data.valid;
}

}  // namespace

rp_ptcc_error rp_PtccInit() { return rp_PtccInitEx(nullptr, RP_PTCC_DEFAULT_BAUDRATE); }

rp_ptcc_error rp_PtccInitDevice(const char *dev) { return rp_PtccInitEx(dev, RP_PTCC_DEFAULT_BAUDRATE); }

rp_ptcc_error rp_PtccInitEx(const char *dev, uint32_t baudrate) {
    std::lock_guard<std::mutex> guard(g_mutex);

    ptcc::Bridge *instance = lookupBridge();
    if (instance == nullptr) {
        return RP_PTCC_EPYTHON;
    }

    if (g_device_open) {
        instance->close();
        g_device_open = false;
    }

    const std::string path = (dev != nullptr) ? std::string(dev) : std::string();
    const ptcc::Result result = instance->open(path, baudrate);
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }

    g_device_open = true;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccRelease() {
    std::lock_guard<std::mutex> guard(g_mutex);

    if (g_bridge != nullptr) {
        g_bridge->close();
    }
    g_device_open = false;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccIsConnected(bool *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    *_out_value = g_device_open && g_bridge != nullptr && g_bridge->isOpen();
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetDevicePath(char *_out_value, size_t size) {
    if (_out_value == nullptr || size == 0) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    copyString(_out_value, size, bridge()->devicePath());
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetModuleType(rp_ptcc_module_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    int module = 0;
    const ptcc::Result result = bridge()->moduleType(module);
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }
    *_out_value = static_cast<rp_ptcc_module_t>(module);
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccListPorts(char *_out_value, size_t size, uint32_t *_out_count) {
    if (_out_value == nullptr || size == 0 || _out_count == nullptr) {
        return RP_PTCC_EIP;
    }

    std::lock_guard<std::mutex> guard(g_mutex);
    ptcc::Bridge *instance = lookupBridge();
    if (instance == nullptr) {
        return RP_PTCC_EPYTHON;
    }

    const std::string joined = instance->listPorts();
    copyString(_out_value, size, joined);

    uint32_t count = joined.empty() ? 0 : 1;
    for (const char character : joined) {
        if (character == '\n') {
            count++;
        }
    }
    *_out_count = count;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccSetThrottle(uint32_t milliseconds) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    if (milliseconds < RP_PTCC_THROTTLE_MS) {
        WARNING("throttle %u ms is below the firmware limit of %d ms, commands may be lost", milliseconds,
                RP_PTCC_THROTTLE_MS);
    }
    return toApiError(bridge()->setThrottleMs(milliseconds));
}

rp_ptcc_error rp_PtccSetTimeout(uint32_t milliseconds) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setTimeoutMs(milliseconds));
}

rp_ptcc_error rp_PtccReadMonitor(rp_ptcc_monitor_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    ptcc::MonitorData data;
    const ptcc::Result result = bridge()->readMonitor(data);
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }

    fillMonitor(_out_value, data);
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetMonitor(rp_ptcc_monitor_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    fillMonitor(_out_value, bridge()->cachedMonitor());
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccStartMonitoring(uint32_t period_ms) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    if (period_ms < RP_PTCC_THROTTLE_MS) {
        WARNING("polling period %u ms is shorter than the throttle interval %d ms", period_ms,
                RP_PTCC_THROTTLE_MS);
    }
    return toApiError(bridge()->startMonitoring(period_ms));
}

rp_ptcc_error rp_PtccStopMonitoring() {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    bridge()->stopMonitoring();
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetTemperature(float *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }

    rp_ptcc_monitor_t monitor;
    std::memset(&monitor, 0, sizeof(monitor));
    const rp_ptcc_error result = rp_PtccReadMonitor(&monitor);
    if (result != RP_PTCC_OK) {
        return result;
    }

    *_out_value = monitor.t_det;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetParams(rp_ptcc_register_t target, rp_ptcc_params_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    ptcc::BasicParams params;
    const ptcc::Result result = bridge()->readBasicParams(params, static_cast<int>(target));
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }

    _out_value->setpoint = static_cast<float>(params.setpoint_k);
    _out_value->i_tec_max = static_cast<float>(params.i_tec_max_a);
    _out_value->u_sup_plus = static_cast<float>(params.u_sup_plus_v);
    _out_value->u_sup_minus = static_cast<float>(params.u_sup_minus_v);
    _out_value->pwm = params.pwm;
    _out_value->supply_ctrl = static_cast<rp_ptcc_ctrl_t>(params.supply_ctrl);
    _out_value->fan_ctrl = static_cast<rp_ptcc_ctrl_t>(params.fan_ctrl);
    _out_value->tec_ctrl = static_cast<rp_ptcc_ctrl_t>(params.tec_ctrl);
    _out_value->valid = params.valid;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetSetpoint(float *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }

    rp_ptcc_params_t params;
    std::memset(&params, 0, sizeof(params));
    const rp_ptcc_error result = rp_PtccGetParams(RP_PTCC_REG_USER_SET, &params);
    if (result != RP_PTCC_OK) {
        return result;
    }

    *_out_value = params.setpoint;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetLimits(rp_ptcc_limits_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    ptcc::Limits limits;
    const ptcc::Result result = bridge()->readLimits(limits);
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }

    _out_value->setpoint_min = static_cast<float>(limits.setpoint_min_k);
    _out_value->setpoint_max = static_cast<float>(limits.setpoint_max_k);
    _out_value->i_tec_max_min = static_cast<float>(limits.i_tec_max_min_a);
    _out_value->i_tec_max_max = static_cast<float>(limits.i_tec_max_max_a);
    _out_value->valid = limits.valid;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccSetSetpoint(float kelvin) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setTemperature(static_cast<double>(kelvin)));
}

rp_ptcc_error rp_PtccSetMaxCurrent(float amperes) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setMaxCurrent(static_cast<double>(amperes)));
}

rp_ptcc_error rp_PtccSetCooler(rp_ptcc_ctrl_t mode) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setCooler(static_cast<int>(mode)));
}

rp_ptcc_error rp_PtccSetFan(rp_ptcc_ctrl_t mode) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setFan(static_cast<int>(mode)));
}

rp_ptcc_error rp_PtccSetSupply(rp_ptcc_ctrl_t mode, float u_plus, float u_minus) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setSupply(static_cast<int>(mode), static_cast<double>(u_plus),
                                          static_cast<double>(u_minus)));
}

rp_ptcc_error rp_PtccSetPwm(uint32_t value) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setPwm(value));
}

/* -- Detection module (LAB_M) --------------------------------------------- */

static void copyLabMMonitor(const ptcc::LabMMonitor &from, rp_ptcc_labm_monitor_t *to) {
    to->u_sup_plus = static_cast<float>(from.u_sup_plus_v);
    to->u_sup_minus = static_cast<float>(from.u_sup_minus_v);
    to->u_fan = static_cast<float>(from.u_fan_v);
    to->i_tec_plus = static_cast<float>(from.i_tec_plus_a);
    to->i_tec_minus = static_cast<float>(from.i_tec_minus_a);
    to->u_th1 = static_cast<float>(from.u_th1_v);
    to->u_th2 = static_cast<float>(from.u_th2_v);
    to->u_det = static_cast<float>(from.u_det_v);
    to->u_1st = static_cast<float>(from.u_1st_v);
    to->u_out = static_cast<float>(from.u_out_v);
    to->temperature = static_cast<float>(from.temperature_c);
    to->timestamp = from.timestamp_ms;
    to->valid = from.valid;
}

rp_ptcc_error rp_PtccReadLabMMonitor(rp_ptcc_labm_monitor_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    ptcc::LabMMonitor monitor;
    const ptcc::Result result = bridge()->readLabMMonitor(monitor);
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }

    copyLabMMonitor(monitor, _out_value);
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetLabMMonitor(rp_ptcc_labm_monitor_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    copyLabMMonitor(bridge()->cachedLabMMonitor(), _out_value);
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetLabMParams(rp_ptcc_register_t target, rp_ptcc_labm_params_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    ptcc::LabMParams params;
    const ptcc::Result result = bridge()->readLabMParams(params, static_cast<int>(target));
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }

    _out_value->det_bias_u = static_cast<float>(params.det_bias_u_v);
    _out_value->det_bias_i = static_cast<float>(params.det_bias_i_a);
    _out_value->offset = static_cast<float>(params.offset_v);
    _out_value->gain = static_cast<float>(params.gain);
    _out_value->gain_code = params.gain_code;
    _out_value->gain_known = params.gain_known;
    _out_value->varactor = params.varactor;
    _out_value->transimpedance = static_cast<rp_ptcc_trans_t>(params.transimpedance);
    _out_value->coupling = static_cast<rp_ptcc_coupling_t>(params.coupling);
    _out_value->bandwidth = static_cast<rp_ptcc_bw_t>(params.bandwidth);
    _out_value->valid = params.valid;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetLabMLimits(rp_ptcc_labm_limits_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    ptcc::LabMLimits limits;
    const ptcc::Result result = bridge()->readLabMLimits(limits);
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }

    _out_value->det_bias_u_min = static_cast<float>(limits.det_bias_u_min_v);
    _out_value->det_bias_u_max = static_cast<float>(limits.det_bias_u_max_v);
    _out_value->det_bias_i_min = static_cast<float>(limits.det_bias_i_min_a);
    _out_value->det_bias_i_max = static_cast<float>(limits.det_bias_i_max_a);
    _out_value->offset_min = static_cast<float>(limits.offset_min_v);
    _out_value->offset_max = static_cast<float>(limits.offset_max_v);
    _out_value->varactor_min = limits.varactor_min;
    _out_value->varactor_max = limits.varactor_max;
    _out_value->valid = limits.valid;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetLabMGainValues(float *_out_values, size_t size, uint32_t *_out_count) {
    if (_out_count == nullptr || (_out_values == nullptr && size > 0)) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    std::vector<double> values;
    const ptcc::Result result = bridge()->labMGainValues(values);
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }

    *_out_count = static_cast<uint32_t>(values.size());
    for (size_t index = 0; index < values.size() && index < size; ++index) {
        _out_values[index] = static_cast<float>(values[index]);
    }
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccSetLabMDetectorBiasVoltage(float volts) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setLabMDetectorBiasVoltage(static_cast<double>(volts)));
}

rp_ptcc_error rp_PtccSetLabMDetectorBiasCurrent(float amperes) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setLabMDetectorBiasCurrent(static_cast<double>(amperes)));
}

rp_ptcc_error rp_PtccSetLabMOffset(float volts) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setLabMOffset(static_cast<double>(volts)));
}

rp_ptcc_error rp_PtccSetLabMGain(float volt_per_volt) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setLabMGain(static_cast<double>(volt_per_volt)));
}

rp_ptcc_error rp_PtccSetLabMGainCode(uint32_t code) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setLabMGainCode(code));
}

rp_ptcc_error rp_PtccSetLabMVaractor(uint32_t code) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setLabMVaractor(code));
}

rp_ptcc_error rp_PtccSetLabMTransimpedance(rp_ptcc_trans_t mode) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setLabMTransimpedance(static_cast<int>(mode)));
}

rp_ptcc_error rp_PtccSetLabMCoupling(rp_ptcc_coupling_t mode) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setLabMCoupling(static_cast<int>(mode)));
}

rp_ptcc_error rp_PtccSetLabMBandwidth(rp_ptcc_bw_t mode) {
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->setLabMBandwidth(static_cast<int>(mode)));
}

rp_ptcc_error rp_PtccGetDeviceIden(rp_ptcc_device_iden_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    ptcc::DeviceIden iden;
    const ptcc::Result result = bridge()->readDeviceIden(iden);
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }

    copyString(_out_value->type, RP_PTCC_STR_LEN, iden.type);
    copyString(_out_value->name, RP_PTCC_STR_LEN, iden.name);
    copyString(_out_value->serial, RP_PTCC_STR_LEN, iden.serial);
    _out_value->firmware_version = iden.firmware_version;
    _out_value->hardware_version = iden.hardware_version;
    _out_value->valid = iden.valid;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetModuleIden(rp_ptcc_module_iden_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }

    ptcc::ModuleIden iden;
    const ptcc::Result result = bridge()->readModuleIden(iden);
    if (result != ptcc::Result::OK) {
        return toApiError(result);
    }

    copyString(_out_value->type, RP_PTCC_STR_LEN, iden.type);
    copyString(_out_value->name, RP_PTCC_STR_LEN, iden.name);
    copyString(_out_value->detector_name, RP_PTCC_STR_LEN, iden.detector_name);
    copyString(_out_value->serial, RP_PTCC_STR_LEN, iden.serial);
    copyString(_out_value->detector_serial, RP_PTCC_STR_LEN, iden.detector_serial);
    _out_value->cool_time = iden.cool_time_s;
    _out_value->valid = iden.valid;
    return RP_PTCC_OK;
}

rp_ptcc_error rp_PtccGetErrorCount(uint64_t *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }
    std::lock_guard<std::mutex> guard(g_mutex);
    if (!bridge()) {
        return RP_PTCC_ENOINIT;
    }
    return toApiError(bridge()->errorCount(*_out_value));
}

const char *rp_PtccGetStatusText(uint8_t status) {
    static thread_local std::string text;

    std::lock_guard<std::mutex> guard(g_mutex);
    ptcc::Bridge *instance = lookupBridge();
    if (instance == nullptr) {
        return "unknown status code";
    }

    text = instance->statusText(static_cast<int>(status));
    return text.c_str();
}

rp_ptcc_error rp_PtccIsErrorStatus(uint8_t status, bool *_out_value) {
    if (_out_value == nullptr) {
        return RP_PTCC_EIP;
    }

    std::lock_guard<std::mutex> guard(g_mutex);
    ptcc::Bridge *instance = lookupBridge();
    if (instance == nullptr) {
        return RP_PTCC_EPYTHON;
    }

    return toApiError(instance->isErrorStatus(static_cast<int>(status), *_out_value));
}

const char *rp_PtccGetErrorText(rp_ptcc_error error) {
    switch (error) {
        case RP_PTCC_OK:
            return "OK";
        case RP_PTCC_ENOINIT:
            return "library not initialized";
        case RP_PTCC_EOPEN:
            return "cannot open serial port";
        case RP_PTCC_EIO:
            return "serial I/O error";
        case RP_PTCC_ETIMEOUT:
            return "timed out waiting for a response";
        case RP_PTCC_ERESP:
            return "unexpected or malformed response";
        case RP_PTCC_EIP:
            return "invalid parameter";
        case RP_PTCC_ERANGE:
            return "value out of the allowed range";
        case RP_PTCC_ENOTSUP:
            return "not supported by the attached module";
        case RP_PTCC_ENODEV:
            return "no PTCC device found";
        case RP_PTCC_EPYTHON:
            return "python bridge error";
        default:
            return "unknown error";
    }
}

const char *rp_PtccGetLastPythonError() {
    static thread_local std::string text;
    text = ptcc::lastPythonError();
    return text.c_str();
}

const char *rp_PtccGetVersion() { return STRINGIFY(VERSION) "-" STRINGIFY(REVISION); }

const char *rp_PtccGetProtocolRevision() {
    static thread_local std::string text;

    std::lock_guard<std::mutex> guard(g_mutex);
    ptcc::Bridge *instance = lookupBridge();
    if (instance == nullptr) {
        return "unknown";
    }

    text = instance->protocolRevision();
    return text.c_str();
}
