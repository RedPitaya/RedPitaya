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

typedef enum { RP_PTCC_TRANS_LOW = 0, RP_PTCC_TRANS_HIGH = 1 } rp_ptcc_trans_t;

typedef enum { RP_PTCC_COUPLING_AC = 0, RP_PTCC_COUPLING_DC = 1 } rp_ptcc_coupling_t;

typedef enum { RP_PTCC_BW_LOW = 0, RP_PTCC_BW_MID = 1, RP_PTCC_BW_HIGH = 2 } rp_ptcc_bw_t;

/**
 * Detection module (LAB_M) settings, the right hand panel of the application.
 * Only LAB_M modules have these; every other module answers RP_PTCC_ENOTSUP.
 */
typedef struct {
    float det_bias_u;   ///< Detector bias voltage [V]
    float det_bias_i;   ///< Detector bias current compensation [A]
    float offset;       ///< Output DC offset [V]
    float gain;         ///< Preamplifier gain [V/V], 0 when gain_known is false
    uint32_t gain_code; ///< Gain as the device stores it
    bool gain_known;    ///< False when the device reports a code outside the documented set
    uint32_t varactor;  ///< Preamplifier 1st stage frequency compensation, 0..4095
    rp_ptcc_trans_t transimpedance;  ///< 1st stage transimpedance
    rp_ptcc_coupling_t coupling;     ///< Signal coupling
    rp_ptcc_bw_t bandwidth;          ///< Preamplifier bandwidth
    bool valid;
} rp_ptcc_labm_params_t;

/** Detection module readings, the Monitor section of the panel. */
typedef struct {
    float u_sup_plus;   ///< [V]
    float u_sup_minus;  ///< [V]
    float u_fan;        ///< [V]
    float i_tec_plus;   ///< [A]
    float i_tec_minus;  ///< [A]
    float u_th1;        ///< [V]
    float u_th2;        ///< [V]
    float u_det;        ///< Detector bias voltage [V]
    float u_1st;        ///< Preamplifier 1st stage voltage [V]
    float u_out;        ///< Preamplifier output voltage [V]
    float temperature;  ///< Module enclosure temperature [C]
    int64_t timestamp;  ///< Monotonic sample time [ms]
    bool valid;
} rp_ptcc_labm_monitor_t;

/** Range the attached LAB_M module accepts, from its USER_MIN/USER_MAX
 *  registers. Informational, like rp_ptcc_limits_t. */
typedef struct {
    float det_bias_u_min;  ///< [V]
    float det_bias_u_max;  ///< [V]
    float det_bias_i_min;  ///< [A]
    float det_bias_i_max;  ///< [A]
    float offset_min;      ///< [V]
    float offset_max;      ///< [V]
    uint32_t varactor_min;
    uint32_t varactor_max;
    bool valid;
} rp_ptcc_labm_limits_t;

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
 * @param kelvin Detector temperature setpoint [K]. The protocol carries the
 *        setpoint with three decimals, so fractions are preserved; the value
 *        is rounded to 1 mK on the wire. The valid range is enforced by the
 *        library and the device; use rp_PtccGetLimits() to populate a UI.
 * @warning Stored in the module EEPROM. Call only on a confirmed user action,
 *          never on every slider event.
 */
rp_ptcc_error rp_PtccSetSetpoint(float kelvin);

/** @warning Stored in EEPROM, see rp_PtccSetSetpoint(). */
rp_ptcc_error rp_PtccSetMaxCurrent(float amperes);

rp_ptcc_error rp_PtccSetCooler(rp_ptcc_ctrl_t mode);

rp_ptcc_error rp_PtccSetFan(rp_ptcc_ctrl_t mode);

/**
 * Module supply: control mode and both rails.
 *
 * The protocol carries the three in one message, so they are written together
 * and a UI has to send the values it is not changing as they are.
 *
 * @param u_plus positive rail [V], 3 to 15 on the current firmware.
 * @param u_minus negative rail [V], -15 to -3. Out of range values come back
 *        as RP_PTCC_ERANGE from the upstream tables.
 * @warning Stored in EEPROM, see rp_PtccSetSetpoint().
 */
rp_ptcc_error rp_PtccSetSupply(rp_ptcc_ctrl_t mode, float u_plus, float u_minus);

/**
 * TEC PWM setting, 0 to 65535 as the module stores it.
 * @warning Stored in EEPROM, see rp_PtccSetSetpoint().
 */
rp_ptcc_error rp_PtccSetPwm(uint32_t value);

/* -- Detection module (LAB_M) ---------------------------------------------
 *
 * Every call here needs a LAB_M module attached and returns RP_PTCC_ENOTSUP
 * otherwise. The setters write to the module EEPROM, exactly like
 * rp_PtccSetSetpoint(): one write per confirmed user action.
 */

/** Reads the detection module monitor container from the device. Blocks. */
rp_ptcc_error rp_PtccReadLabMMonitor(rp_ptcc_labm_monitor_t *_out_value);

/** Returns the cached sample without I/O. The poller started by
 *  rp_PtccStartMonitoring() refreshes it on LAB_M modules. */
rp_ptcc_error rp_PtccGetLabMMonitor(rp_ptcc_labm_monitor_t *_out_value);

rp_ptcc_error rp_PtccGetLabMParams(rp_ptcc_register_t target, rp_ptcc_labm_params_t *_out_value);

/** Reads the range this module accepts. Cached after the first call. */
rp_ptcc_error rp_PtccGetLabMLimits(rp_ptcc_labm_limits_t *_out_value);

/**
 * The gains the device defines, in V/V, ascending, for a UI to offer.
 * @param size number of elements _out_values can hold.
 * @param _out_count number of gains available, set even when size is smaller.
 */
rp_ptcc_error rp_PtccGetLabMGainValues(float *_out_values, size_t size, uint32_t *_out_count);

rp_ptcc_error rp_PtccSetLabMDetectorBiasVoltage(float volts);

rp_ptcc_error rp_PtccSetLabMDetectorBiasCurrent(float amperes);

rp_ptcc_error rp_PtccSetLabMOffset(float volts);

/** @param volt_per_volt one of the values rp_PtccGetLabMGainValues() returns;
 *         anything else is RP_PTCC_ERANGE rather than a silent nearest match. */
rp_ptcc_error rp_PtccSetLabMGain(float volt_per_volt);

/** Writes the raw gain code, for a UI that read one back with
 *  rp_ptcc_labm_params_t::gain_known false. */
rp_ptcc_error rp_PtccSetLabMGainCode(uint32_t code);

rp_ptcc_error rp_PtccSetLabMVaractor(uint32_t code);

rp_ptcc_error rp_PtccSetLabMTransimpedance(rp_ptcc_trans_t mode);

rp_ptcc_error rp_PtccSetLabMCoupling(rp_ptcc_coupling_t mode);

rp_ptcc_error rp_PtccSetLabMBandwidth(rp_ptcc_bw_t mode);

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
