/**
 * @file rp_ptcc.h
 * @brief VIGO Photonics PTCC-01 TEC controller API.
 *
 * The controller enumerates as USB CDC-ACM (/dev/ttyACM*) at 57600 baud. The
 * library is a wrapper: it embeds CPython and delegates all protocol work to
 * the upstream package at https://gitlab.com/vigophotonics/ptcc-library.
 *
 * @warning The firmware needs ~0.55 s between commands. Every call here is
 *          synchronous and may block for that long. Use a worker thread.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef RP_PTCC_H
#define RP_PTCC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    RP_PTCC_OK = 0,
    RP_PTCC_ENOINIT = 1,   ///< Library not initialized
    RP_PTCC_EOPEN = 2,     ///< Cannot open the serial port
    RP_PTCC_EIO = 3,       ///< Serial I/O error
    RP_PTCC_ETIMEOUT = 4,  ///< No response in time
    RP_PTCC_ERESP = 5,     ///< Missing field, or malformed response
    RP_PTCC_EIP = 6,       ///< Invalid parameter
    RP_PTCC_ERANGE = 7,    ///< Value out of range
    RP_PTCC_ENOTSUP = 8,   ///< Not supported by the attached module
    RP_PTCC_ENODEV = 9,    ///< No PTCC device found
    RP_PTCC_EPYTHON = 10   ///< Embedded Python bridge failed
} rp_ptcc_error;

#define RP_PTCC_DEFAULT_BAUDRATE 57600
#define RP_PTCC_THROTTLE_MS 550
#define RP_PTCC_TIMEOUT_MS 2000
#define RP_PTCC_STR_LEN 64

typedef enum {
    RP_PTCC_MODULE_NONE = 0,
    RP_PTCC_MODULE_NOMEM = 1,  ///< IR module without EEPROM
    RP_PTCC_MODULE_MEM = 2,    ///< IR module with EEPROM
    RP_PTCC_MODULE_LAB_M = 3   ///< LAB_M module
} rp_ptcc_module_t;

typedef enum { RP_PTCC_CTRL_AUTO = 0, RP_PTCC_CTRL_OFF = 1, RP_PTCC_CTRL_ON = 2 } rp_ptcc_ctrl_t;

typedef enum {
    RP_PTCC_REG_DEFAULT = 0,
    RP_PTCC_REG_USER_SET = 1,
    RP_PTCC_REG_USER_MIN = 2,
    RP_PTCC_REG_USER_MAX = 3
} rp_ptcc_register_t;

typedef struct {
    float t_det;          ///< Detector temperature [K]
    float t_int;          ///< Controller internal temperature [C]
    float i_tec;          ///< TEC current [A]
    float u_tec;          ///< TEC voltage [V]
    float i_sup_plus;     ///< [A]
    float i_sup_minus;    ///< [A]
    float u_sup_plus;     ///< [V]
    float u_sup_minus;    ///< [V]
    float i_fan;          ///< [A]
    float th_resistance;  ///< Thermistor resistance [Ohm]
    uint32_t pwm;
    uint8_t status;     ///< 0-3 status, 128+ error
    bool supply_on;
    bool fan_on;
    int64_t timestamp;  ///< Monotonic sample time [ms]
    bool valid;
} rp_ptcc_monitor_t;

typedef struct {
    float setpoint;    ///< Detector temperature setpoint [K]
    float i_tec_max;   ///< TEC current limit [A]
    float u_sup_plus;  ///< [V]
    float u_sup_minus; ///< [V]
    uint32_t pwm;
    rp_ptcc_ctrl_t supply_ctrl;
    rp_ptcc_ctrl_t fan_ctrl;
    rp_ptcc_ctrl_t tec_ctrl;
    bool valid;
} rp_ptcc_params_t;

/** Range the attached module actually accepts, from its USER_MIN/USER_MAX
 *  registers. Much narrower than the protocol limits: typically 180 to 300 K. */
typedef struct {
    float setpoint_min;   ///< [K]
    float setpoint_max;   ///< [K]
    float i_tec_max_min;  ///< [A]
    float i_tec_max_max;  ///< [A]
    bool valid;
} rp_ptcc_limits_t;

typedef struct {
    char type[RP_PTCC_STR_LEN];
    char name[RP_PTCC_STR_LEN];
    char serial[RP_PTCC_STR_LEN];
    uint32_t firmware_version;
    uint32_t hardware_version;
    bool valid;
} rp_ptcc_device_iden_t;

typedef struct {
    char type[RP_PTCC_STR_LEN];
    char name[RP_PTCC_STR_LEN];
    char detector_name[RP_PTCC_STR_LEN];
    char serial[RP_PTCC_STR_LEN];
    char detector_serial[RP_PTCC_STR_LEN];
    uint32_t cool_time;  ///< Nominal cool down time [s]
    bool valid;
} rp_ptcc_module_iden_t;

#ifdef __cplusplus
extern "C" {
#endif

/** Opens the first controller found on /dev/ttyACM* or /dev/ttyUSB*. */
rp_ptcc_error rp_PtccInit();

/** Opens a controller on a specific device node. */
rp_ptcc_error rp_PtccInitDevice(const char *device);

/** Opens a controller, device may be NULL to scan. */
rp_ptcc_error rp_PtccInitEx(const char *device, uint32_t baudrate);

/** Stops polling, closes the port, releases resources. */
rp_ptcc_error rp_PtccRelease();

rp_ptcc_error rp_PtccIsConnected(bool *_out_value);

rp_ptcc_error rp_PtccGetDevicePath(char *_out_value, size_t size);

rp_ptcc_error rp_PtccGetModuleType(rp_ptcc_module_t *_out_value);

/** Returns a newline separated list of candidate serial ports. */
rp_ptcc_error rp_PtccListPorts(char *_out_value, size_t size, uint32_t *_out_count);

/** Overrides the inter command interval. Below RP_PTCC_THROTTLE_MS commands are lost. */
rp_ptcc_error rp_PtccSetThrottle(uint32_t milliseconds);

rp_ptcc_error rp_PtccSetTimeout(uint32_t milliseconds);

/** Reads the monitor container from the device. Blocks. */
rp_ptcc_error rp_PtccReadMonitor(rp_ptcc_monitor_t *_out_value);

/** Returns the cached sample without I/O. Use with rp_PtccStartMonitoring(). */
rp_ptcc_error rp_PtccGetMonitor(rp_ptcc_monitor_t *_out_value);

/** Starts a background thread refreshing the cache, period clamped to the throttle. */
rp_ptcc_error rp_PtccStartMonitoring(uint32_t period_ms);

rp_ptcc_error rp_PtccStopMonitoring();

rp_ptcc_error rp_PtccGetTemperature(float *_out_value);

rp_ptcc_error rp_PtccGetParams(rp_ptcc_register_t target, rp_ptcc_params_t *_out_value);

rp_ptcc_error rp_PtccGetSetpoint(float *_out_value);

/** Reads the range this module accepts. Cached after the first call. */
rp_ptcc_error rp_PtccGetLimits(rp_ptcc_limits_t *_out_value);

/**
 * @param kelvin Whole Kelvins, as accepted by the upstream library. The valid
 *        range is enforced by the library and the device; use
 *        rp_PtccGetLimits() to populate a UI.
 * @warning Stored in the module EEPROM. Call only on a confirmed user action,
 *          never on every slider event.
 */
rp_ptcc_error rp_PtccSetSetpoint(int32_t kelvin);

/** @warning Stored in EEPROM, see rp_PtccSetSetpoint(). */
rp_ptcc_error rp_PtccSetMaxCurrent(float amperes);

rp_ptcc_error rp_PtccSetCooler(rp_ptcc_ctrl_t mode);

rp_ptcc_error rp_PtccSetFan(rp_ptcc_ctrl_t mode);

rp_ptcc_error rp_PtccGetDeviceIden(rp_ptcc_device_iden_t *_out_value);

rp_ptcc_error rp_PtccGetModuleIden(rp_ptcc_module_iden_t *_out_value);

/** Number of frames rejected on CRC or parsing since init. */
rp_ptcc_error rp_PtccGetErrorCount(uint64_t *_out_value);

const char *rp_PtccGetStatusText(uint8_t status);

/** Classification comes from the upstream error table, not from the code range. */
rp_ptcc_error rp_PtccIsErrorStatus(uint8_t status, bool *_out_value);

const char *rp_PtccGetErrorText(rp_ptcc_error error);

/** Text of the last exception raised inside the Python bridge. */
const char *rp_PtccGetLastPythonError();

const char *rp_PtccGetVersion();

/** Upstream ptcc-library revision the protocol defines were generated from. */
const char *rp_PtccGetProtocolRevision();

#ifdef __cplusplus
}
#endif

#endif  // RP_PTCC_H
