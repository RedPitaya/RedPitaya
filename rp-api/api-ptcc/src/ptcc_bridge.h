/**
 * @file ptcc_bridge.h
 * @brief Embedded CPython bridge to the upstream VIGO ptcc-library.
 *
 * All PTCC protocol work is done by the Python package; this class only owns
 * the interpreter, holds the GIL around every call and converts the returned
 * dictionaries into plain C++ structs.
 */

#ifndef PTCC_BRIDGE_H
#define PTCC_BRIDGE_H

#include <atomic>
#include <cstdint>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ptcc {

struct MonitorData {
    double t_det_k = 0.0;        ///< [K]
    double t_int_c = 0.0;        ///< [C]
    double i_tec_a = 0.0;        ///< [A]
    double u_tec_v = 0.0;        ///< [V]
    double i_sup_plus_a = 0.0;   ///< [A]
    double i_sup_minus_a = 0.0;  ///< [A]
    double u_sup_plus_v = 0.0;   ///< [V]
    double u_sup_minus_v = 0.0;  ///< [V]
    double i_fan_a = 0.0;        ///< [A]
    double th_resistance = 0.0;  ///< [Ohm]
    uint32_t pwm = 0;
    uint8_t status = 0;  ///< 0-3 status, 128+ error
    bool supply_on = false;
    bool fan_on = false;
    int64_t timestamp_ms = 0;
    bool valid = false;
};

struct BasicParams {
    double setpoint_k = 0.0;     ///< [K]
    double i_tec_max_a = 0.0;    ///< [A]
    double u_sup_plus_v = 0.0;   ///< [V]
    double u_sup_minus_v = 0.0;  ///< [V]
    uint32_t pwm = 0;
    int supply_ctrl = 0;
    int fan_ctrl = 0;
    int tec_ctrl = 0;
    bool valid = false;
};

/** LAB_M detection module parameters. DET_U, DET_I and OFFSET arrive in SI
 *  units; gain is a device code, mapped to V/V by the bridge. */
struct LabMParams {
    double det_bias_u_v = 0.0;  ///< Detector bias voltage [V]
    double det_bias_i_a = 0.0;  ///< Detector bias current compensation [A]
    double offset_v = 0.0;      ///< Output DC offset [V]
    double gain = 0.0;          ///< Preamplifier gain [V/V], 0 when the code is unknown
    uint32_t gain_code = 0;     ///< Raw gain code as stored by the device
    bool gain_known = false;    ///< Whether gain_code maps to a documented V/V value
    uint32_t varactor = 0;      ///< 1st stage frequency compensation, 0..4095
    int transimpedance = 0;     ///< 0 LOW, 1 HIGH
    int coupling = 0;           ///< 0 AC, 1 DC
    int bandwidth = 0;          ///< 0 LOW, 1 MID, 2 HIGH
    bool valid = false;
};

struct LabMMonitor {
    double u_sup_plus_v = 0.0;   ///< [V]
    double u_sup_minus_v = 0.0;  ///< [V]
    double u_fan_v = 0.0;        ///< [V]
    double i_tec_plus_a = 0.0;   ///< [A]
    double i_tec_minus_a = 0.0;  ///< [A]
    double u_th1_v = 0.0;        ///< [V]
    double u_th2_v = 0.0;        ///< [V]
    double u_det_v = 0.0;        ///< Detector bias readback [V]
    double u_1st_v = 0.0;        ///< Preamplifier 1st stage output [V]
    double u_out_v = 0.0;        ///< Preamplifier output [V]
    double temperature_c = 0.0;  ///< Module enclosure temperature [C]
    int64_t timestamp_ms = 0;
    bool valid = false;
};

struct LabMLimits {
    double det_bias_u_min_v = 0.0;
    double det_bias_u_max_v = 0.0;
    double det_bias_i_min_a = 0.0;
    double det_bias_i_max_a = 0.0;
    double offset_min_v = 0.0;
    double offset_max_v = 0.0;
    uint32_t varactor_min = 0;
    uint32_t varactor_max = 0;
    bool valid = false;
};

struct Limits {
    double setpoint_min_k = 0.0;
    double setpoint_max_k = 0.0;
    double i_tec_max_min_a = 0.0;
    double i_tec_max_max_a = 0.0;
    bool valid = false;
};

struct DeviceIden {
    std::string type;
    std::string name;
    std::string serial;
    uint32_t firmware_version = 0;
    uint32_t hardware_version = 0;
    bool valid = false;
};

struct ModuleIden {
    std::string type;
    std::string name;
    std::string detector_name;
    std::string serial;
    std::string detector_serial;
    uint32_t cool_time_s = 0;
    bool valid = false;
};

enum class Result {
    OK = 0,
    NOT_INITIALIZED,
    OPEN_FAILED,
    IO_ERROR,
    TIMEOUT,
    BAD_RESPONSE,
    INVALID_PARAM,
    OUT_OF_RANGE,
    NOT_SUPPORTED,
    NO_DEVICE,
    PYTHON_ERROR,
};

/** Text of the last Python exception, empty when the last call succeeded. */
const std::string &lastPythonError();

class Bridge {
   public:
    Bridge() = default;
    ~Bridge();

    Bridge(const Bridge &) = delete;
    Bridge &operator=(const Bridge &) = delete;

    /** Starts the interpreter and imports rp_ptcc_bridge. */
    Result initialize();

    /** Opens a controller. Empty device scans all candidate nodes. */
    Result open(const std::string &device, uint32_t baudrate);
    void close();

    bool isOpen() const;
    std::string devicePath() const;
    Result moduleType(int &out) const;

    Result setThrottleMs(uint32_t value);
    Result setTimeoutMs(uint32_t value);

    Result readMonitor(MonitorData &out);
    Result readBasicParams(BasicParams &out, int reg);
    Result readLimits(Limits &out);
    Result readLabMMonitor(LabMMonitor &out);
    Result readLabMParams(LabMParams &out, int reg);
    Result readLabMLimits(LabMLimits &out);
    Result labMGainValues(std::vector<double> &out);
    Result readDeviceIden(DeviceIden &out);
    Result readModuleIden(ModuleIden &out);

    Result setTemperature(double kelvin);
    Result setMaxCurrent(double amperes);
    Result setCooler(int mode);
    Result setFan(int mode);
    Result setSupply(int mode, double u_plus, double u_minus);
    Result setPwm(uint32_t value);

    Result setLabMDetectorBiasVoltage(double volts);
    Result setLabMDetectorBiasCurrent(double amperes);
    Result setLabMOffset(double volts);
    Result setLabMGain(double volt_per_volt);
    Result setLabMGainCode(uint32_t code);
    Result setLabMVaractor(uint32_t code);
    Result setLabMTransimpedance(int mode);
    Result setLabMCoupling(int mode);
    Result setLabMBandwidth(int mode);

    Result startMonitoring(uint32_t period_ms);
    void stopMonitoring();
    bool isMonitoring() const { return m_polling.load(); }
    MonitorData cachedMonitor() const;

    /** Cached LAB_M sample, refreshed by the poller on LAB_M modules only. */
    LabMMonitor cachedLabMMonitor() const;

    Result errorCount(uint64_t &out) const;

    /** Newline separated list of candidate serial ports. */
    std::string listPorts() const;

    std::string statusText(int code) const;
    Result isErrorStatus(int code, bool &out) const;
    std::string protocolRevision() const;

   private:
    void pollLoop(uint32_t period_ms);

    mutable std::mutex m_cache_mutex;
    MonitorData m_cache;
    LabMMonitor m_labm_cache;

    std::thread m_poll_thread;
    std::atomic<bool> m_polling{false};
    std::condition_variable m_poll_cv;
    std::mutex m_poll_mutex;

    bool m_initialized = false;
    bool m_owns_interpreter = false;  ///< false when loaded into a running interpreter
};

const char *resultText(Result result);

}  // namespace ptcc

#endif  // PTCC_BRIDGE_H
