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
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

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
    int moduleType() const;

    Result setThrottleMs(uint32_t value);
    Result setTimeoutMs(uint32_t value);

    Result readMonitor(MonitorData &out);
    Result readBasicParams(BasicParams &out, int reg);
    Result readDeviceIden(DeviceIden &out);
    Result readModuleIden(ModuleIden &out);

    Result setTemperature(double kelvin);
    Result setMaxCurrent(double amperes);
    Result setCooler(int mode);
    Result setFan(int mode);

    Result startMonitoring(uint32_t period_ms);
    void stopMonitoring();
    bool isMonitoring() const { return m_polling.load(); }
    MonitorData cachedMonitor() const;

    uint64_t errorCount() const;

    /** Newline separated list of candidate serial ports. */
    std::string listPorts() const;

    std::string statusText(int code) const;
    bool isErrorStatus(int code) const;
    std::string protocolRevision() const;

   private:
    void pollLoop(uint32_t period_ms);

    mutable std::mutex m_cache_mutex;
    MonitorData m_cache;

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
