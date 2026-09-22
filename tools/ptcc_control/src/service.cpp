/**
 * @file service.cpp
 * @brief Websocket service mode, see service.h.
 */

#include "service.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <deque>
#include <functional>
#include <algorithm>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

extern "C" {
#include "rp_ptcc.h"
}

#include "web/rp_websocket.h"

namespace {

std::atomic_bool g_stop{false};

int64_t nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

/** Reads that fail are logged rather than dropped: a panel missing half its
 *  fields is otherwise indistinguishable from a device that has nothing to
 *  report. */
void logFailure(const char *what, rp_ptcc_error result) {
    fprintf(stderr, "[Warning] %s failed: %s (%s)\n", what, rp_PtccGetErrorText(result),
            rp_PtccGetLastPythonError());
    fflush(stderr);
}

const char *moduleName(rp_ptcc_module_t type) {
    switch (type) {
        case RP_PTCC_MODULE_NOMEM:
            return "NOMEM";
        case RP_PTCC_MODULE_MEM:
            return "MEM";
        case RP_PTCC_MODULE_LAB_M:
            return "LAB_M";
        default:
            return "NONE";
    }
}

/**
 * Publishes to every connected client, dropping values that have not moved.
 *
 * The panel has around eighty fields and most of them sit still, so sending the
 * whole set every period would be mostly noise. What did change is collected
 * and sent as one object per flush: the library's own cache batches the keys,
 * which keeps a full snapshot to a single message instead of eighty.
 */
class Publisher {
   public:
    explicit Publisher(rp_websocket::CWEBServer::Ptr server) : m_server(std::move(server)) {}

    // Reached from the service loop and, for resendAll(), straight from the
    // websocket thread: a client asking for the state it is missing must not
    // wait behind a device read.

    /**
     * Sends everything published so far, from memory.
     *
     * A client that has just connected needs the whole state, but reading it
     * back from the controller would take seconds: every command waits out the
     * firmware throttle. The values the service already holds are what the
     * device last reported, so they are served straight from here.
     */
    void resendAll() {
        std::lock_guard<std::mutex> guard(m_mutex);
        for (const auto &entry : m_resend) {
            m_pending.push_back(entry.second);
        }
        flushLocked();
    }

    /** Sends everything collected since the last call, as one message. */
    void flush() {
        std::lock_guard<std::mutex> guard(m_mutex);
        flushLocked();
    }

    /** Sends a string even when it repeats: two identical errors in a row are
     *  two events, not one. */
    void sendAlways(const std::string &key, const std::string &value) {
        {
            std::lock_guard<std::mutex> guard(m_mutex);
            m_cache.erase(key);
        }
        send(key, value);
    }

    void send(const std::string &key, bool value) { store(key, value ? "1" : "0", value); }

    void send(const std::string &key, int value) { store(key, std::to_string(value), value); }

    void send(const std::string &key, uint32_t value) { store(key, std::to_string(value), value); }

    void send(const std::string &key, float value) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.6g", static_cast<double>(value));
        store(key, buffer, value);
    }

    void send(const std::string &key, const std::string &value) { store(key, value, value); }

   private:
    void flushLocked() {
        if (m_pending.empty()) {
            return;
        }

        // The first entry clears whatever the library still holds from the
        // previous batch, the rest add to it.
        bool first = true;
        for (const auto &entry : m_pending) {
            entry(first);
            first = false;
        }
        m_pending.clear();
        m_server->sendCache();
    }

    template <typename T>
    void store(const std::string &key, const std::string &text, T value) {
        std::lock_guard<std::mutex> guard(m_mutex);

        auto server = m_server;
        auto action = [server, key, value](bool reset) { server->sendRequest(key, value, reset); };
        m_resend[key] = action;

        auto entry = m_cache.find(key);
        if (entry != m_cache.end() && entry->second == text) {
            return;
        }
        m_cache[key] = text;
        m_pending.push_back(action);
    }

    std::mutex m_mutex;
    rp_websocket::CWEBServer::Ptr m_server;
    std::map<std::string, std::string> m_cache;
    std::map<std::string, std::function<void(bool)>> m_resend;
    std::vector<std::function<void(bool)>> m_pending;
};

class Service {
   public:
    Service(uint32_t period_ms, const char *device, uint32_t baudrate)
        : m_period_ms(period_ms < RP_PTCC_THROTTLE_MS ? RP_PTCC_THROTTLE_MS : period_ms),
          m_device(device != nullptr ? device : ""),
          m_baudrate(baudrate) {}

    int run(uint16_t port);

   private:
    void connectSignals();
    void handleInt(const std::string &key, int value);
    void handleDouble(const std::string &key, float value);
    void queue(std::function<void()> action);
    void drainQueue();

    void publishIdentification();
    void publishLimits();
    void publishParams();
    void publishLabMParams();
    void publishLabMLimits();
    void publishGains();
    void publishMonitor(bool fresh);
    void publishLabMMonitor(bool fresh);
    void publishEverything();

    /** Publishes the outcome of a write and the values it changed. */
    void report(const char *what, rp_ptcc_error result);
    void afterParamWrite(const char *what, rp_ptcc_error result);
    void afterLabMWrite(const char *what, rp_ptcc_error result);

    /** Writes the supply triplet, filling the fields the client left out. */
    rp_ptcc_error writeSupply(bool set_mode, rp_ptcc_ctrl_t mode, bool set_plus, float u_plus,
                              bool set_minus, float u_minus);

    void checkLink();
    void reconnect();

    uint32_t m_period_ms;
    std::string m_device;
    uint32_t m_baudrate;

    rp_websocket::CWEBServer::Ptr m_server;
    std::unique_ptr<Publisher> m_out;

    std::mutex m_queue_mutex;
    std::deque<std::function<void()>> m_queue;

    bool m_is_labm = false;
    bool m_connected = true;
    int64_t m_last_sample = 0;
    int64_t m_last_alive = 0;
    int64_t m_started = 0;
    int64_t m_last_empty_report = 0;
    std::chrono::steady_clock::time_point m_last_reconnect{};
};

void Service::queue(std::function<void()> action) {
    std::lock_guard<std::mutex> guard(m_queue_mutex);
    m_queue.push_back(std::move(action));
}

void Service::drainQueue() {
    for (;;) {
        std::function<void()> action;
        {
            std::lock_guard<std::mutex> guard(m_queue_mutex);
            if (m_queue.empty()) {
                return;
            }
            action = m_queue.front();
            m_queue.pop_front();
        }
        action();
        m_out->flush();
    }
}

void Service::report(const char *what, rp_ptcc_error result) {
    m_out->send("PTCC_LAST_ERROR_CODE", static_cast<int>(result));
    if (result == RP_PTCC_OK) {
        m_out->send("PTCC_LAST_ERROR", std::string(""));
        return;
    }

    std::string text = std::string(what) + ": " + rp_PtccGetErrorText(result);
    const char *detail = rp_PtccGetLastPythonError();
    if (detail != nullptr && detail[0] != '\0') {
        text += " (";
        text += detail;
        text += ")";
    }
    m_out->sendAlways("PTCC_LAST_ERROR", text);
}

void Service::afterParamWrite(const char *what, rp_ptcc_error result) {
    report(what, result);
    publishParams();
}

void Service::afterLabMWrite(const char *what, rp_ptcc_error result) {
    report(what, result);
    publishLabMParams();
}

rp_ptcc_error Service::writeSupply(bool set_mode, rp_ptcc_ctrl_t mode, bool set_plus, float u_plus,
                                   bool set_minus, float u_minus) {
    rp_ptcc_params_t params;
    memset(&params, 0, sizeof(params));

    // The protocol carries mode and both rails in one message, so the fields
    // the client did not send have to go back as they are.
    const rp_ptcc_error read = rp_PtccGetParams(RP_PTCC_REG_USER_SET, &params);
    if (read != RP_PTCC_OK) {
        return read;
    }

    return rp_PtccSetSupply(set_mode ? mode : params.supply_ctrl,
                            set_plus ? u_plus : params.u_sup_plus,
                            set_minus ? u_minus : params.u_sup_minus);
}

void Service::publishIdentification() {
    rp_ptcc_device_iden_t device;
    rp_ptcc_module_iden_t module;
    memset(&device, 0, sizeof(device));
    memset(&module, 0, sizeof(module));

    char path[256] = {0};
    rp_PtccGetDevicePath(path, sizeof(path));
    m_out->send("PTCC_PORT", std::string(path));

    rp_ptcc_module_t type = RP_PTCC_MODULE_NONE;
    if (rp_PtccGetModuleType(&type) == RP_PTCC_OK) {
        m_is_labm = type == RP_PTCC_MODULE_LAB_M;
        m_out->send("PTCC_MODULE_TYPE", std::string(moduleName(type)));
        m_out->send("PTCC_LABM_PRESENT", m_is_labm);
    }

    if (rp_PtccGetDeviceIden(&device) == RP_PTCC_OK) {
        m_out->send("PTCC_DEV_NAME", std::string(device.name));
        m_out->send("PTCC_DEV_TYPE", std::string(device.type));
        m_out->send("PTCC_DEV_SERIAL", std::string(device.serial));
        m_out->send("PTCC_DEV_FIRMWARE", device.firmware_version);
        m_out->send("PTCC_DEV_HARDWARE", device.hardware_version);
    }

    if (rp_PtccGetModuleIden(&module) == RP_PTCC_OK) {
        m_out->send("PTCC_MOD_NAME", std::string(module.name));
        m_out->send("PTCC_MOD_TYPE", std::string(module.type));
        m_out->send("PTCC_MOD_SERIAL", std::string(module.serial));
        m_out->send("PTCC_DET_NAME", std::string(module.detector_name));
        m_out->send("PTCC_DET_SERIAL", std::string(module.detector_serial));
        m_out->send("PTCC_COOL_TIME", module.cool_time);
    }
}

void Service::publishLimits() {
    rp_ptcc_limits_t limits;
    memset(&limits, 0, sizeof(limits));
    if (rp_PtccGetLimits(&limits) != RP_PTCC_OK) {
        return;
    }

    m_out->send("PTCC_SETPOINT_MIN", limits.setpoint_min);
    m_out->send("PTCC_SETPOINT_MAX", limits.setpoint_max);
    m_out->send("PTCC_I_TEC_MAX_MIN", limits.i_tec_max_min);
    m_out->send("PTCC_I_TEC_MAX_MAX", limits.i_tec_max_max);

    // A module that reports the same minimum and maximum does not let the
    // field be changed at all: the panel needs that as data, not as a rule
    // wired into the markup.
    rp_ptcc_params_t low;
    rp_ptcc_params_t high;
    memset(&low, 0, sizeof(low));
    memset(&high, 0, sizeof(high));
    if (rp_PtccGetParams(RP_PTCC_REG_USER_MIN, &low) == RP_PTCC_OK &&
        rp_PtccGetParams(RP_PTCC_REG_USER_MAX, &high) == RP_PTCC_OK) {
        m_out->send("PTCC_SUP_U_P_MIN", low.u_sup_plus);
        m_out->send("PTCC_SUP_U_P_MAX", high.u_sup_plus);
        m_out->send("PTCC_SUP_U_N_MIN", low.u_sup_minus);
        m_out->send("PTCC_SUP_U_N_MAX", high.u_sup_minus);
        m_out->send("PTCC_PWM_MIN", low.pwm);
        m_out->send("PTCC_PWM_MAX", high.pwm);
    }
}

void Service::publishParams() {
    rp_ptcc_params_t params;
    memset(&params, 0, sizeof(params));
    const rp_ptcc_error result = rp_PtccGetParams(RP_PTCC_REG_USER_SET, &params);
    if (result != RP_PTCC_OK) {
        logFailure("Reading the parameters", result);
        return;
    }

    m_out->send("PTCC_SETPOINT", params.setpoint);
    m_out->send("PTCC_I_TEC_MAX", params.i_tec_max);
    m_out->send("PTCC_SUP_U_P", params.u_sup_plus);
    m_out->send("PTCC_SUP_U_N", params.u_sup_minus);
    m_out->send("PTCC_PWM_SET", params.pwm);
    m_out->send("PTCC_SUP_CTRL", static_cast<int>(params.supply_ctrl));
    m_out->send("PTCC_FAN_CTRL", static_cast<int>(params.fan_ctrl));
    m_out->send("PTCC_TEC_CTRL", static_cast<int>(params.tec_ctrl));
}

void Service::publishGains() {
    uint32_t count = 0;
    if (rp_PtccGetLabMGainValues(nullptr, 0, &count) != RP_PTCC_OK || count == 0) {
        return;
    }

    std::vector<float> values(count, 0.0F);
    if (rp_PtccGetLabMGainValues(values.data(), values.size(), &count) != RP_PTCC_OK) {
        return;
    }

    std::string list;
    for (size_t index = 0; index < values.size(); ++index) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%g", static_cast<double>(values[index]));
        if (index > 0) {
            list += ",";
        }
        list += buffer;
    }
    m_out->send("PTCC_LABM_GAINS", list);
}

void Service::publishLabMParams() {
    if (!m_is_labm) {
        return;
    }

    rp_ptcc_labm_params_t params;
    memset(&params, 0, sizeof(params));
    const rp_ptcc_error result = rp_PtccGetLabMParams(RP_PTCC_REG_USER_SET, &params);
    if (result != RP_PTCC_OK) {
        logFailure("Reading the detection module parameters", result);
        return;
    }

    m_out->send("PTCC_LABM_BIAS_U", params.det_bias_u);
    m_out->send("PTCC_LABM_BIAS_I", params.det_bias_i);
    m_out->send("PTCC_LABM_OFFSET", params.offset);
    m_out->send("PTCC_LABM_GAIN", params.gain);
    m_out->send("PTCC_LABM_GAIN_CODE", params.gain_code);
    m_out->send("PTCC_LABM_GAIN_KNOWN", params.gain_known);
    m_out->send("PTCC_LABM_VARACTOR", params.varactor);
    m_out->send("PTCC_LABM_TRANS", static_cast<int>(params.transimpedance));
    m_out->send("PTCC_LABM_COUPLING", static_cast<int>(params.coupling));
    m_out->send("PTCC_LABM_BW", static_cast<int>(params.bandwidth));
}

void Service::publishLabMLimits() {
    if (!m_is_labm) {
        return;
    }

    rp_ptcc_labm_limits_t limits;
    memset(&limits, 0, sizeof(limits));
    if (rp_PtccGetLabMLimits(&limits) != RP_PTCC_OK) {
        return;
    }

    m_out->send("PTCC_LABM_BIAS_U_MIN", limits.det_bias_u_min);
    m_out->send("PTCC_LABM_BIAS_U_MAX", limits.det_bias_u_max);
    m_out->send("PTCC_LABM_BIAS_I_MIN", limits.det_bias_i_min);
    m_out->send("PTCC_LABM_BIAS_I_MAX", limits.det_bias_i_max);
    m_out->send("PTCC_LABM_OFFSET_MIN", limits.offset_min);
    m_out->send("PTCC_LABM_OFFSET_MAX", limits.offset_max);
    m_out->send("PTCC_LABM_VARACTOR_MIN", limits.varactor_min);
    m_out->send("PTCC_LABM_VARACTOR_MAX", limits.varactor_max);
}

void Service::publishMonitor(bool fresh) {
    rp_ptcc_monitor_t monitor;
    memset(&monitor, 0, sizeof(monitor));
    const rp_ptcc_error result =
        fresh ? rp_PtccReadMonitor(&monitor) : rp_PtccGetMonitor(&monitor);
    if (result != RP_PTCC_OK) {
        logFailure("Reading the cached monitor", result);
        return;
    }
    if (!monitor.valid) {
        // The poller has not produced a sample yet, which is only expected
        // right after a reconnect.
        const int64_t now = nowMs();
        if (now - m_last_empty_report > 5000) {
            m_last_empty_report = now;
            fprintf(stderr, "[Warning] no monitor sample yet\n");
            fflush(stderr);
        }
        return;
    }

    m_last_sample = monitor.timestamp;

    m_out->send("PTCC_T_DET", monitor.t_det);
    m_out->send("PTCC_T_INT", monitor.t_int);
    m_out->send("PTCC_I_TEC", monitor.i_tec);
    m_out->send("PTCC_U_TEC", monitor.u_tec);
    m_out->send("PTCC_I_SUP_P", monitor.i_sup_plus);
    m_out->send("PTCC_I_SUP_N", monitor.i_sup_minus);
    m_out->send("PTCC_U_SUP_P", monitor.u_sup_plus);
    m_out->send("PTCC_U_SUP_N", monitor.u_sup_minus);
    m_out->send("PTCC_I_FAN", monitor.i_fan);
    m_out->send("PTCC_TH_RES", monitor.th_resistance);
    m_out->send("PTCC_PWM", monitor.pwm);
    m_out->send("PTCC_SUPPLY_ON", monitor.supply_on);
    m_out->send("PTCC_FAN_ON", monitor.fan_on);
    m_out->send("PTCC_STATUS", static_cast<int>(monitor.status));
    m_out->send("PTCC_STATUS_TEXT", std::string(rp_PtccGetStatusText(monitor.status)));

    bool is_error = false;
    rp_PtccIsErrorStatus(monitor.status, &is_error);
    m_out->send("PTCC_STATUS_IS_ERROR", is_error);
}

void Service::publishLabMMonitor(bool fresh) {
    if (!m_is_labm) {
        return;
    }

    rp_ptcc_labm_monitor_t monitor;
    memset(&monitor, 0, sizeof(monitor));
    const rp_ptcc_error result =
        fresh ? rp_PtccReadLabMMonitor(&monitor) : rp_PtccGetLabMMonitor(&monitor);
    if (result != RP_PTCC_OK) {
        logFailure("Reading the cached detection module monitor", result);
        return;
    }
    if (!monitor.valid) {
        return;
    }

    m_out->send("PTCC_LABM_U_SUP_P", monitor.u_sup_plus);
    m_out->send("PTCC_LABM_U_SUP_N", monitor.u_sup_minus);
    m_out->send("PTCC_LABM_U_FAN", monitor.u_fan);
    m_out->send("PTCC_LABM_I_TEC_P", monitor.i_tec_plus);
    m_out->send("PTCC_LABM_I_TEC_N", monitor.i_tec_minus);
    m_out->send("PTCC_LABM_TH1", monitor.u_th1);
    m_out->send("PTCC_LABM_TH2", monitor.u_th2);
    m_out->send("PTCC_LABM_U_DET", monitor.u_det);
    m_out->send("PTCC_LABM_U_1ST", monitor.u_1st);
    m_out->send("PTCC_LABM_U_OUT", monitor.u_out);
    m_out->send("PTCC_LABM_TEMP", monitor.temperature);
}

void Service::publishEverything() {
    m_out->send("PTCC_CONNECTED", m_connected);
    m_out->send("PTCC_PERIOD", m_period_ms);
    m_out->send("PTCC_VERSION", std::string(rp_PtccGetVersion()));
    m_out->send("PTCC_PROTOCOL", std::string(rp_PtccGetProtocolRevision()));
    publishIdentification();
    publishLimits();
    publishParams();
    publishGains();
    publishLabMLimits();
    publishLabMParams();
    publishMonitor(true);
    publishLabMMonitor(true);
}

void Service::checkLink() {
    rp_ptcc_monitor_t monitor;
    memset(&monitor, 0, sizeof(monitor));
    rp_PtccGetMonitor(&monitor);

    const int64_t now = nowMs();

    // The poller keeps the cache, so a timestamp that stops moving is the only
    // sign of a controller that went away.
    if (monitor.valid && monitor.timestamp != m_last_sample) {
        m_last_sample = monitor.timestamp;
        m_last_alive = now;
    }

    // Counted from the start, not from the first sample: the first read is
    // still on its way when the loop begins, and treating that as a lost link
    // would reopen the port for no reason.
    const int64_t age_limit = std::max<int64_t>(static_cast<int64_t>(m_period_ms) * 4, 3000);
    const bool connected = (now - m_last_alive) < age_limit;
    if (connected == m_connected) {
        return;
    }

    m_connected = connected;
    m_out->send("PTCC_CONNECTED", m_connected);
}

void Service::reconnect() {
    const auto now = std::chrono::steady_clock::now();
    if (now - m_last_reconnect < std::chrono::seconds(5)) {
        return;
    }
    m_last_reconnect = now;

    rp_PtccRelease();
    const rp_ptcc_error result =
        rp_PtccInitEx(m_device.empty() ? nullptr : m_device.c_str(), m_baudrate);
    if (result != RP_PTCC_OK) {
        report("Reconnect", result);
        return;
    }

    rp_PtccStartMonitoring(m_period_ms);
    m_connected = true;
    m_last_alive = nowMs();
    // The failure that led here is over, so the panel should not keep showing it.
    m_out->sendAlways("PTCC_LAST_ERROR", std::string(""));
    m_out->send("PTCC_LAST_ERROR_CODE", static_cast<int>(RP_PTCC_OK));
    publishEverything();
}

void Service::handleDouble(const std::string &name, float number) {
    if (name == "PTCC_SET_SETPOINT") {
        queue([this, number] { afterParamWrite("Setpoint", rp_PtccSetSetpoint(number)); });
    } else if (name == "PTCC_SET_I_TEC_MAX") {
        queue([this, number] { afterParamWrite("Current limit", rp_PtccSetMaxCurrent(number)); });
    } else if (name == "PTCC_SET_SUP_U_P") {
        queue([this, number] {
            afterParamWrite("Supply",
                            writeSupply(false, RP_PTCC_CTRL_AUTO, true, number, false, 0.0F));
        });
    } else if (name == "PTCC_SET_SUP_U_N") {
        queue([this, number] {
            afterParamWrite("Supply",
                            writeSupply(false, RP_PTCC_CTRL_AUTO, false, 0.0F, true, number));
        });
    } else if (name == "PTCC_SET_LABM_BIAS_U") {
        queue([this, number] {
            afterLabMWrite("Detector bias", rp_PtccSetLabMDetectorBiasVoltage(number));
        });
    } else if (name == "PTCC_SET_LABM_BIAS_I") {
        queue([this, number] {
            afterLabMWrite("Bias current compensation", rp_PtccSetLabMDetectorBiasCurrent(number));
        });
    } else if (name == "PTCC_SET_LABM_OFFSET") {
        queue([this, number] { afterLabMWrite("Offset", rp_PtccSetLabMOffset(number)); });
    } else if (name == "PTCC_SET_LABM_GAIN") {
        queue([this, number] { afterLabMWrite("Gain", rp_PtccSetLabMGain(number)); });
    }
}

void Service::handleInt(const std::string &name, int value) {
    if (name == "PTCC_SET_COOLER") {
        queue([this, value] {
            afterParamWrite("Cooler", rp_PtccSetCooler(static_cast<rp_ptcc_ctrl_t>(value)));
        });
    } else if (name == "PTCC_SET_FAN") {
        queue([this, value] {
            afterParamWrite("Fan", rp_PtccSetFan(static_cast<rp_ptcc_ctrl_t>(value)));
        });
    } else if (name == "PTCC_SET_SUP_CTRL") {
        queue([this, value] {
            afterParamWrite("Supply", writeSupply(true, static_cast<rp_ptcc_ctrl_t>(value), false,
                                                  0.0F, false, 0.0F));
        });
    } else if (name == "PTCC_SET_PWM") {
        queue([this, value] {
            afterParamWrite("PWM", rp_PtccSetPwm(static_cast<uint32_t>(value)));
        });
    } else if (name == "PTCC_SET_LABM_TRANS") {
        queue([this, value] {
            afterLabMWrite("Transimpedance",
                           rp_PtccSetLabMTransimpedance(static_cast<rp_ptcc_trans_t>(value)));
        });
    } else if (name == "PTCC_SET_LABM_COUPLING") {
        queue([this, value] {
            afterLabMWrite("Coupling",
                           rp_PtccSetLabMCoupling(static_cast<rp_ptcc_coupling_t>(value)));
        });
    } else if (name == "PTCC_SET_LABM_BW") {
        queue([this, value] {
            afterLabMWrite("Bandwidth",
                           rp_PtccSetLabMBandwidth(static_cast<rp_ptcc_bw_t>(value)));
        });
    } else if (name == "PTCC_SET_LABM_VARACTOR") {
        queue([this, value] {
            afterLabMWrite("Varactor", rp_PtccSetLabMVaractor(static_cast<uint32_t>(value)));
        });
    } else if (name == "PTCC_SET_LABM_GAIN_CODE") {
        queue([this, value] {
            afterLabMWrite("Gain code", rp_PtccSetLabMGainCode(static_cast<uint32_t>(value)));
        });
    } else if (name == "PTCC_REFRESH") {
        // Answered here rather than through the queue: the loop can be sitting
        // on the driver mutex behind a device read, and a page waiting for its
        // first state would see that as a stall.
        m_out->resendAll();
    } else if (name == "PTCC_RELOAD") {
        queue([this] { publishEverything(); });
    } else if (name == "PTCC_RECONNECT") {
        queue([this] {
            m_last_reconnect = std::chrono::steady_clock::time_point{};
            reconnect();
        });
    } else if (name == "PTCC_STOP") {
        g_stop = true;
    }
}

void Service::connectSignals() {
    // A browser sends a whole number as int or uint depending on its value, and
    // a setpoint typed without decimals arrives as an integer, so all three
    // numeric signals reach the same handlers.
    m_server->receiveDouble.connect(
        [this](auto key, auto value) { handleDouble(std::string(key), static_cast<float>(value)); });

    m_server->receiveInt.connect([this](auto key, auto value) {
        const std::string name(key);
        handleInt(name, value);
        handleDouble(name, static_cast<float>(value));
    });

    m_server->receiveUInt.connect([this](auto key, auto value) {
        const std::string name(key);
        handleInt(name, static_cast<int>(value));
        handleDouble(name, static_cast<float>(value));
    });

    m_server->receiveBool.connect([this](auto key, auto value) {
        const std::string name(key);
        if (name == "PTCC_REFRESH" && value) {
            m_out->resendAll();
        } else if (name == "PTCC_RELOAD" && value) {
            queue([this] { publishEverything(); });
        } else if (name == "PTCC_STOP" && value) {
            g_stop = true;
        }
    });
}

int Service::run(uint16_t port) {
    m_started = nowMs();
    m_last_alive = m_started;
    m_server = std::make_shared<rp_websocket::CWEBServer>();
    m_out = std::make_unique<Publisher>(m_server);

    connectSignals();
    m_server->startServer(port);

    // Read the whole state before the poller starts. Afterwards every read
    // queues behind it: a LAB_M module costs two commands per period, and each
    // command waits out the firmware throttle.
    publishEverything();
    m_out->flush();

    const uint32_t containers = m_is_labm ? 2 : 1;
    const uint32_t floor_ms = RP_PTCC_THROTTLE_MS * containers + 400;
    if (m_period_ms < floor_ms) {
        fprintf(stderr, "[Warning] poll period raised from %u to %u ms: %u command(s) per "
                        "period at %u ms each, plus room for the panel\n",
                m_period_ms, floor_ms, containers, RP_PTCC_THROTTLE_MS);
        fflush(stderr);
        m_period_ms = floor_ms;
        m_out->send("PTCC_PERIOD", m_period_ms);
        m_out->flush();
    }

    const rp_ptcc_error polling = rp_PtccStartMonitoring(m_period_ms);
    if (polling != RP_PTCC_OK) {
        logFailure("Starting the monitor poller", polling);
    }

    auto last_publish = std::chrono::steady_clock::now();
    while (!g_stop) {
        drainQueue();

        const auto now = std::chrono::steady_clock::now();
        if (now - last_publish >= std::chrono::milliseconds(m_period_ms)) {
            last_publish = now;
            checkLink();
            if (m_connected) {
                publishMonitor(false);
                publishLabMMonitor(false);
            } else {
                reconnect();
            }
            // A heartbeat every period, so a page can tell a live service from
            // a stalled one, and so the socket keeps carrying traffic.
            m_out->send("PTCC_UPTIME", static_cast<uint32_t>((nowMs() - m_started) / 1000));
            m_out->flush();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    rp_PtccStopMonitoring();
    return 0;
}

}  // namespace

void ptcc_service_stop() { g_stop = true; }

int ptcc_service_run(uint16_t port, uint32_t period_ms, const char *device, uint32_t baudrate) {
    g_stop = false;
    Service service(period_ms, device, baudrate);
    return service.run(port);
}
