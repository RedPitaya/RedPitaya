/**
 * @file ptcc_bridge.cpp
 */

#include "ptcc_bridge.h"

#include <Python.h>

#include <chrono>

#include "rp_log.h"

#ifndef PTCC_PYTHON_DIR
#define PTCC_PYTHON_DIR "/opt/redpitaya/lib/python"
#endif

/* Where the bridge ends up on a board. PTCC_PYTHON_DIR is baked in at build
 * time and points at the staging tree when the image is built elsewhere
 * (/workspace/build/... on the build host), which does not exist on the
 * target, so the installed location is always tried as well. */
#define PTCC_RUNTIME_PYTHON_DIR "/opt/redpitaya/lib/python"

namespace ptcc {

namespace {

PyObject *g_module = nullptr;
std::mutex g_python_mutex;
std::string g_last_error;

/** ModuleType.LAB_M upstream, RP_PTCC_MODULE_LAB_M in the public header. */
constexpr int LAB_M_MODULE = 3;

int64_t nowMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

/** Holds the GIL for the lifetime of the object. */
class Gil {
   public:
    Gil() : m_state(PyGILState_Ensure()) {}
    ~Gil() { PyGILState_Release(m_state); }

    Gil(const Gil &) = delete;
    Gil &operator=(const Gil &) = delete;

   private:
    PyGILState_STATE m_state;
};

/** Moves the current Python exception into g_last_error and maps it. */
Result consumeError(Result fallback) {
    if (!PyErr_Occurred()) {
        return fallback;
    }

    PyObject *type = nullptr;
    PyObject *value = nullptr;
    PyObject *traceback = nullptr;
    PyErr_Fetch(&type, &value, &traceback);
    PyErr_NormalizeException(&type, &value, &traceback);

    std::string message;
    if (value != nullptr) {
        PyObject *text = PyObject_Str(value);
        if (text != nullptr) {
            const char *utf8 = PyUnicode_AsUTF8(text);
            if (utf8 != nullptr) {
                message = utf8;
            }
            Py_DECREF(text);
        }
    }

    Result result = fallback;
    if (type != nullptr) {
        if (PyErr_GivenExceptionMatches(type, PyExc_TimeoutError)) {
            result = Result::TIMEOUT;
        } else if (PyErr_GivenExceptionMatches(type, PyExc_NotImplementedError)) {
            result = Result::NOT_SUPPORTED;
        } else if (PyErr_GivenExceptionMatches(type, PyExc_LookupError)) {
            /* rp_ptcc_bridge.FieldError: the device did not report a field. */
            result = Result::BAD_RESPONSE;
        } else if (PyErr_GivenExceptionMatches(type, PyExc_ValueError)) {
            /* Range checks live in the upstream library. */
            result = Result::OUT_OF_RANGE;
        } else if (PyErr_GivenExceptionMatches(type, PyExc_TypeError)) {
            result = Result::NOT_SUPPORTED;
        } else if (PyErr_GivenExceptionMatches(type, PyExc_OSError)) {
            result = Result::IO_ERROR;
        }
    }

    g_last_error = message.empty() ? "unknown Python error" : message;
    ERROR_LOG("%s", g_last_error.c_str());

    Py_XDECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);
    return result;
}

/* The bridge guarantees every documented key, so a missing one is a bug or a
 * truncated response, never a zero. All readers below report failure instead
 * of substituting a default. */
bool dictDouble(PyObject *dict, const char *key, double &out) {
    PyObject *item = PyDict_GetItemString(dict, key);
    if (item == nullptr) {
        g_last_error = std::string("field missing from the bridge response: ") + key;
        return false;
    }
    const double value = PyFloat_AsDouble(item);
    if (value == -1.0 && PyErr_Occurred()) {
        PyErr_Clear();
        g_last_error = std::string("field is not a number: ") + key;
        return false;
    }
    out = value;
    return true;
}

bool dictLong(PyObject *dict, const char *key, long &out) {
    PyObject *item = PyDict_GetItemString(dict, key);
    if (item == nullptr) {
        g_last_error = std::string("field missing from the bridge response: ") + key;
        return false;
    }
    const long value = PyLong_AsLong(item);
    if (value == -1 && PyErr_Occurred()) {
        PyErr_Clear();
        g_last_error = std::string("field is not an integer: ") + key;
        return false;
    }
    out = value;
    return true;
}

bool dictBool(PyObject *dict, const char *key, bool &out) {
    PyObject *item = PyDict_GetItemString(dict, key);
    if (item == nullptr) {
        g_last_error = std::string("field missing from the bridge response: ") + key;
        return false;
    }
    out = PyObject_IsTrue(item) == 1;
    return true;
}

bool dictString(PyObject *dict, const char *key, std::string &out) {
    PyObject *item = PyDict_GetItemString(dict, key);
    if (item == nullptr) {
        g_last_error = std::string("field missing from the bridge response: ") + key;
        return false;
    }
    if (!PyUnicode_Check(item)) {
        PyObject *text = PyObject_Str(item);
        if (text == nullptr) {
            PyErr_Clear();
            g_last_error = std::string("field is not a string: ") + key;
            return false;
        }
        const char *utf8 = PyUnicode_AsUTF8(text);
        out = utf8 != nullptr ? utf8 : "";
        Py_DECREF(text);
        return true;
    }
    const char *utf8 = PyUnicode_AsUTF8(item);
    out = utf8 != nullptr ? utf8 : "";
    return true;
}

/* Collects the outcome of a group of reads. */
struct Reader {
    bool ok = true;

    void take(bool result) { ok = ok && result; }
};

/** Calls a module level function, returns a new reference or nullptr. */
PyObject *call(const char *name, PyObject *args) {
    if (g_module == nullptr) {
        return nullptr;
    }

    PyObject *function = PyObject_GetAttrString(g_module, name);
    if (function == nullptr) {
        return nullptr;
    }

    PyObject *empty = args != nullptr ? nullptr : PyTuple_New(0);
    PyObject *result = PyObject_CallObject(function, args != nullptr ? args : empty);
    Py_XDECREF(empty);
    Py_DECREF(function);
    return result;
}

/** Calls a function that returns nothing useful. */
Result callVoid(const char *name, PyObject *args, Result fallback) {
    PyObject *result = call(name, args);
    Py_XDECREF(args);
    if (result == nullptr) {
        return consumeError(fallback);
    }
    Py_DECREF(result);
    g_last_error.clear();
    return Result::OK;
}

}  // namespace

const std::string &lastPythonError() { return g_last_error; }

const char *resultText(Result result) {
    switch (result) {
        case Result::OK:
            return "OK";
        case Result::NOT_INITIALIZED:
            return "library not initialized";
        case Result::OPEN_FAILED:
            return "cannot open serial port";
        case Result::IO_ERROR:
            return "serial I/O error";
        case Result::TIMEOUT:
            return "timed out waiting for a response";
        case Result::BAD_RESPONSE:
            return "unexpected or malformed response";
        case Result::INVALID_PARAM:
            return "invalid parameter";
        case Result::OUT_OF_RANGE:
            return "value out of the allowed range";
        case Result::NOT_SUPPORTED:
            return "not supported by the attached module";
        case Result::NO_DEVICE:
            return "no PTCC device found";
        case Result::PYTHON_ERROR:
            return "python bridge error";
        default:
            return "unknown error";
    }
}

Bridge::~Bridge() { close(); }

Result Bridge::initialize() {
    std::lock_guard<std::mutex> guard(g_python_mutex);

    if (m_initialized) {
        return Result::OK;
    }

    if (!Py_IsInitialized()) {
        Py_InitializeEx(0);
        /* Releases the GIL acquired by Py_Initialize so worker threads can take it. */
        PyEval_SaveThread();
        m_owns_interpreter = true;
    }

    Gil gil;

    PyObject *sys_path = PySys_GetObject("path");
    if (sys_path != nullptr) {
        const char *directories[] = {PTCC_PYTHON_DIR, PTCC_PYTHON_DIR "/ptcc_library",
                                    PTCC_RUNTIME_PYTHON_DIR,
                                    PTCC_RUNTIME_PYTHON_DIR "/ptcc_library"};
        for (const char *directory : directories) {
            PyObject *entry = PyUnicode_FromString(directory);
            if (entry != nullptr) {
                // Appended, not inserted: PYTHONPATH and virtualenvs keep
                // priority, so a development tree can override the installed
                // bridge without reinstalling.
                if (PySequence_Contains(sys_path, entry) == 0) {
                    PyList_Append(sys_path, entry);
                }
                Py_DECREF(entry);
            }
        }
    }

    g_module = PyImport_ImportModule("rp_ptcc_bridge");
    if (g_module == nullptr) {
        return consumeError(Result::PYTHON_ERROR);
    }

    m_initialized = true;
    g_last_error.clear();
    return Result::OK;
}

Result Bridge::open(const std::string &device, uint32_t baudrate) {
    const Result ready = initialize();
    if (ready != Result::OK) {
        return ready;
    }

    stopMonitoring();

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *args = Py_BuildValue("(sI)", device.empty() ? nullptr : device.c_str(), baudrate);
    if (args == nullptr) {
        return consumeError(Result::PYTHON_ERROR);
    }

    PyObject *result = call("open_device", args);
    Py_DECREF(args);
    if (result == nullptr) {
        const Result error = consumeError(Result::OPEN_FAILED);
        return error == Result::IO_ERROR ? (device.empty() ? Result::NO_DEVICE : Result::OPEN_FAILED) : error;
    }

    Py_DECREF(result);
    g_last_error.clear();
    return Result::OK;
}

void Bridge::close() {
    stopMonitoring();

    std::lock_guard<std::mutex> guard(g_python_mutex);
    // During interpreter shutdown there is nothing left to call into, and
    // taking the GIL then would crash.
    if (!m_initialized || !Py_IsInitialized()) {
        return;
    }

    Gil gil;
    PyObject *result = call("close_device", nullptr);
    Py_XDECREF(result);
    PyErr_Clear();
}

bool Bridge::isOpen() const {
    if (!m_initialized || !Py_IsInitialized()) {
        return false;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("is_open", nullptr);
    if (result == nullptr) {
        PyErr_Clear();
        return false;
    }

    const bool value = PyObject_IsTrue(result) == 1;
    Py_DECREF(result);
    return value;
}

std::string Bridge::devicePath() const {
    if (!m_initialized || !Py_IsInitialized()) {
        return std::string();
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("port", nullptr);
    if (result == nullptr) {
        PyErr_Clear();
        return std::string();
    }

    const char *utf8 = PyUnicode_AsUTF8(result);
    std::string value = utf8 != nullptr ? utf8 : "";
    Py_DECREF(result);
    return value;
}

/* 0 is a valid module type (NONE), so a failure cannot be reported as a
 * value: it goes through the return code. */
Result Bridge::moduleType(int &out) const {
    if (!m_initialized || !Py_IsInitialized()) {
        return Result::NOT_INITIALIZED;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("module_type", nullptr);
    if (result == nullptr) {
        return consumeError(Result::PYTHON_ERROR);
    }

    const long value = PyLong_AsLong(result);
    Py_DECREF(result);
    if (value == -1 && PyErr_Occurred()) {
        return consumeError(Result::BAD_RESPONSE);
    }

    out = static_cast<int>(value);
    return Result::OK;
}

Result Bridge::setThrottleMs(uint32_t value) {
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_throttle", Py_BuildValue("(d)", static_cast<double>(value) / 1000.0),
                    Result::PYTHON_ERROR);
}

Result Bridge::setTimeoutMs(uint32_t value) {
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_timeout", Py_BuildValue("(d)", static_cast<double>(value) / 1000.0),
                    Result::PYTHON_ERROR);
}

Result Bridge::readMonitor(MonitorData &out) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }

    {
        std::lock_guard<std::mutex> guard(g_python_mutex);
        Gil gil;

        PyObject *result = call("read_monitor", nullptr);
        if (result == nullptr) {
            return consumeError(Result::IO_ERROR);
        }

        Reader reader;
        long pwm = 0;
        long status = 0;
        reader.take(dictDouble(result, "t_det", out.t_det_k));
        reader.take(dictDouble(result, "t_int", out.t_int_c));
        reader.take(dictDouble(result, "i_tec", out.i_tec_a));
        reader.take(dictDouble(result, "u_tec", out.u_tec_v));
        reader.take(dictDouble(result, "i_sup_plus", out.i_sup_plus_a));
        reader.take(dictDouble(result, "i_sup_minus", out.i_sup_minus_a));
        reader.take(dictDouble(result, "u_sup_plus", out.u_sup_plus_v));
        reader.take(dictDouble(result, "u_sup_minus", out.u_sup_minus_v));
        reader.take(dictDouble(result, "i_fan", out.i_fan_a));
        reader.take(dictDouble(result, "th_resistance", out.th_resistance));
        reader.take(dictLong(result, "pwm", pwm));
        reader.take(dictLong(result, "status", status));
        reader.take(dictBool(result, "supply_on", out.supply_on));
        reader.take(dictBool(result, "fan_on", out.fan_on));
        Py_DECREF(result);

        if (!reader.ok) {
            out = MonitorData();
            return Result::BAD_RESPONSE;
        }

        out.pwm = static_cast<uint32_t>(pwm);
        out.status = static_cast<uint8_t>(status);
        out.timestamp_ms = nowMs();
        out.valid = true;
        g_last_error.clear();
    }

    std::lock_guard<std::mutex> cache_guard(m_cache_mutex);
    m_cache = out;
    return Result::OK;
}

Result Bridge::readBasicParams(BasicParams &out, int reg) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *args = Py_BuildValue("(i)", reg);
    PyObject *result = call("read_params", args);
    Py_XDECREF(args);
    if (result == nullptr) {
        return consumeError(Result::IO_ERROR);
    }

    Reader reader;
    long pwm = 0;
    long supply_ctrl = 0;
    long fan_ctrl = 0;
    long tec_ctrl = 0;
    reader.take(dictDouble(result, "setpoint", out.setpoint_k));
    reader.take(dictDouble(result, "i_tec_max", out.i_tec_max_a));
    reader.take(dictDouble(result, "u_sup_plus", out.u_sup_plus_v));
    reader.take(dictDouble(result, "u_sup_minus", out.u_sup_minus_v));
    reader.take(dictLong(result, "pwm", pwm));
    reader.take(dictLong(result, "supply_ctrl", supply_ctrl));
    reader.take(dictLong(result, "fan_ctrl", fan_ctrl));
    reader.take(dictLong(result, "tec_ctrl", tec_ctrl));
    Py_DECREF(result);

    if (!reader.ok) {
        out = BasicParams();
        return Result::BAD_RESPONSE;
    }

    out.pwm = static_cast<uint32_t>(pwm);
    out.supply_ctrl = static_cast<int>(supply_ctrl);
    out.fan_ctrl = static_cast<int>(fan_ctrl);
    out.tec_ctrl = static_cast<int>(tec_ctrl);
    out.valid = true;
    g_last_error.clear();
    return Result::OK;
}

Result Bridge::readLimits(Limits &out) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("limits", nullptr);
    if (result == nullptr) {
        return consumeError(Result::IO_ERROR);
    }

    Reader reader;
    reader.take(dictDouble(result, "setpoint_min", out.setpoint_min_k));
    reader.take(dictDouble(result, "setpoint_max", out.setpoint_max_k));
    reader.take(dictDouble(result, "i_tec_max_min", out.i_tec_max_min_a));
    reader.take(dictDouble(result, "i_tec_max_max", out.i_tec_max_max_a));
    Py_DECREF(result);

    if (!reader.ok) {
        out = Limits();
        return Result::BAD_RESPONSE;
    }

    out.valid = out.setpoint_max_k > out.setpoint_min_k;
    g_last_error.clear();
    return Result::OK;
}

Result Bridge::readLabMMonitor(LabMMonitor &out) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }

    {
        std::lock_guard<std::mutex> guard(g_python_mutex);
        Gil gil;

        PyObject *result = call("read_lab_m_monitor", nullptr);
        if (result == nullptr) {
            return consumeError(Result::IO_ERROR);
        }

        Reader reader;
        reader.take(dictDouble(result, "u_sup_plus", out.u_sup_plus_v));
        reader.take(dictDouble(result, "u_sup_minus", out.u_sup_minus_v));
        reader.take(dictDouble(result, "u_fan", out.u_fan_v));
        reader.take(dictDouble(result, "i_tec_plus", out.i_tec_plus_a));
        reader.take(dictDouble(result, "i_tec_minus", out.i_tec_minus_a));
        reader.take(dictDouble(result, "u_th1", out.u_th1_v));
        reader.take(dictDouble(result, "u_th2", out.u_th2_v));
        reader.take(dictDouble(result, "u_det", out.u_det_v));
        reader.take(dictDouble(result, "u_1st", out.u_1st_v));
        reader.take(dictDouble(result, "u_out", out.u_out_v));
        reader.take(dictDouble(result, "temperature", out.temperature_c));
        Py_DECREF(result);

        if (!reader.ok) {
            out = LabMMonitor();
            return Result::BAD_RESPONSE;
        }

        out.timestamp_ms = nowMs();
        out.valid = true;
        g_last_error.clear();
    }

    std::lock_guard<std::mutex> cache_guard(m_cache_mutex);
    m_labm_cache = out;
    return Result::OK;
}

Result Bridge::readLabMParams(LabMParams &out, int reg) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *args = Py_BuildValue("(i)", reg);
    PyObject *result = call("read_lab_m_params", args);
    Py_XDECREF(args);
    if (result == nullptr) {
        return consumeError(Result::IO_ERROR);
    }

    Reader reader;
    long gain_code = 0;
    long varactor = 0;
    long transimpedance = 0;
    long coupling = 0;
    long bandwidth = 0;
    reader.take(dictDouble(result, "det_bias_u", out.det_bias_u_v));
    reader.take(dictDouble(result, "det_bias_i", out.det_bias_i_a));
    reader.take(dictDouble(result, "offset", out.offset_v));
    reader.take(dictDouble(result, "gain", out.gain));
    reader.take(dictLong(result, "gain_code", gain_code));
    reader.take(dictBool(result, "gain_known", out.gain_known));
    reader.take(dictLong(result, "varactor", varactor));
    reader.take(dictLong(result, "transimpedance", transimpedance));
    reader.take(dictLong(result, "coupling", coupling));
    reader.take(dictLong(result, "bandwidth", bandwidth));
    Py_DECREF(result);

    if (!reader.ok) {
        out = LabMParams();
        return Result::BAD_RESPONSE;
    }

    out.gain_code = static_cast<uint32_t>(gain_code);
    out.varactor = static_cast<uint32_t>(varactor);
    out.transimpedance = static_cast<int>(transimpedance);
    out.coupling = static_cast<int>(coupling);
    out.bandwidth = static_cast<int>(bandwidth);
    out.valid = true;
    g_last_error.clear();
    return Result::OK;
}

Result Bridge::readLabMLimits(LabMLimits &out) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("lab_m_limits", nullptr);
    if (result == nullptr) {
        return consumeError(Result::IO_ERROR);
    }

    Reader reader;
    long varactor_min = 0;
    long varactor_max = 0;
    reader.take(dictDouble(result, "det_bias_u_min", out.det_bias_u_min_v));
    reader.take(dictDouble(result, "det_bias_u_max", out.det_bias_u_max_v));
    reader.take(dictDouble(result, "det_bias_i_min", out.det_bias_i_min_a));
    reader.take(dictDouble(result, "det_bias_i_max", out.det_bias_i_max_a));
    reader.take(dictDouble(result, "offset_min", out.offset_min_v));
    reader.take(dictDouble(result, "offset_max", out.offset_max_v));
    reader.take(dictLong(result, "varactor_min", varactor_min));
    reader.take(dictLong(result, "varactor_max", varactor_max));
    Py_DECREF(result);

    if (!reader.ok) {
        out = LabMLimits();
        return Result::BAD_RESPONSE;
    }

    out.varactor_min = static_cast<uint32_t>(varactor_min);
    out.varactor_max = static_cast<uint32_t>(varactor_max);
    out.valid = out.varactor_max > out.varactor_min;
    g_last_error.clear();
    return Result::OK;
}

Result Bridge::labMGainValues(std::vector<double> &out) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("lab_m_gain_values", nullptr);
    if (result == nullptr) {
        return consumeError(Result::PYTHON_ERROR);
    }

    const Py_ssize_t count = PySequence_Size(result);
    if (count < 0) {
        Py_DECREF(result);
        g_last_error = "lab_m_gain_values did not return a sequence";
        return Result::BAD_RESPONSE;
    }

    out.clear();
    out.reserve(static_cast<size_t>(count));
    for (Py_ssize_t index = 0; index < count; ++index) {
        PyObject *item = PySequence_GetItem(result, index);
        if (item == nullptr) {
            continue;
        }
        const double value = PyFloat_AsDouble(item);
        Py_DECREF(item);
        if (value == -1.0 && PyErr_Occurred()) {
            PyErr_Clear();
            continue;
        }
        out.push_back(value);
    }
    Py_DECREF(result);

    g_last_error.clear();
    return Result::OK;
}

Result Bridge::readDeviceIden(DeviceIden &out) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("read_device_iden", nullptr);
    if (result == nullptr) {
        return consumeError(Result::IO_ERROR);
    }

    Reader reader;
    long firmware = 0;
    long hardware = 0;
    reader.take(dictString(result, "type", out.type));
    reader.take(dictString(result, "name", out.name));
    reader.take(dictString(result, "serial", out.serial));
    reader.take(dictLong(result, "firmware_version", firmware));
    reader.take(dictLong(result, "hardware_version", hardware));
    Py_DECREF(result);

    if (!reader.ok) {
        out = DeviceIden();
        return Result::BAD_RESPONSE;
    }

    out.firmware_version = static_cast<uint32_t>(firmware);
    out.hardware_version = static_cast<uint32_t>(hardware);
    out.valid = true;
    g_last_error.clear();
    return Result::OK;
}

Result Bridge::readModuleIden(ModuleIden &out) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("read_module_iden", nullptr);
    if (result == nullptr) {
        return consumeError(Result::IO_ERROR);
    }

    Reader reader;
    long cool_time = 0;
    reader.take(dictString(result, "type", out.type));
    reader.take(dictString(result, "name", out.name));
    reader.take(dictString(result, "detector_name", out.detector_name));
    reader.take(dictString(result, "serial", out.serial));
    reader.take(dictString(result, "detector_serial", out.detector_serial));
    reader.take(dictLong(result, "cool_time", cool_time));
    Py_DECREF(result);

    if (!reader.ok) {
        out = ModuleIden();
        return Result::BAD_RESPONSE;
    }

    out.cool_time_s = static_cast<uint32_t>(cool_time);
    out.valid = true;
    g_last_error.clear();
    return Result::OK;
}

Result Bridge::setTemperature(double kelvin) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_temperature", Py_BuildValue("(d)", kelvin), Result::IO_ERROR);
}

Result Bridge::setMaxCurrent(double amperes) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_max_current", Py_BuildValue("(d)", amperes), Result::IO_ERROR);
}

Result Bridge::setCooler(int mode) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_cooler", Py_BuildValue("(i)", mode), Result::IO_ERROR);
}

Result Bridge::setFan(int mode) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_fan", Py_BuildValue("(i)", mode), Result::IO_ERROR);
}

Result Bridge::setSupply(int mode, double u_plus, double u_minus) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_supply", Py_BuildValue("(idd)", mode, u_plus, u_minus),
                    Result::IO_ERROR);
}

Result Bridge::setPwm(uint32_t value) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_pwm", Py_BuildValue("(k)", static_cast<unsigned long>(value)),
                    Result::IO_ERROR);
}

Result Bridge::setLabMDetectorBiasVoltage(double volts) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_lab_m_detector_bias_voltage", Py_BuildValue("(d)", volts),
                    Result::IO_ERROR);
}

Result Bridge::setLabMDetectorBiasCurrent(double amperes) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_lab_m_detector_bias_current", Py_BuildValue("(d)", amperes),
                    Result::IO_ERROR);
}

Result Bridge::setLabMOffset(double volts) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_lab_m_offset", Py_BuildValue("(d)", volts), Result::IO_ERROR);
}

Result Bridge::setLabMGain(double volt_per_volt) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_lab_m_gain", Py_BuildValue("(d)", volt_per_volt), Result::IO_ERROR);
}

Result Bridge::setLabMGainCode(uint32_t code) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_lab_m_gain_code", Py_BuildValue("(k)", static_cast<unsigned long>(code)),
                    Result::IO_ERROR);
}

Result Bridge::setLabMVaractor(uint32_t code) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_lab_m_varactor", Py_BuildValue("(k)", static_cast<unsigned long>(code)),
                    Result::IO_ERROR);
}

Result Bridge::setLabMTransimpedance(int mode) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_lab_m_transimpedance", Py_BuildValue("(i)", mode), Result::IO_ERROR);
}

Result Bridge::setLabMCoupling(int mode) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_lab_m_coupling", Py_BuildValue("(i)", mode), Result::IO_ERROR);
}

Result Bridge::setLabMBandwidth(int mode) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;
    return callVoid("set_lab_m_bandwidth", Py_BuildValue("(i)", mode), Result::IO_ERROR);
}

Result Bridge::errorCount(uint64_t &out) const {
    if (!m_initialized || !Py_IsInitialized()) {
        return Result::NOT_INITIALIZED;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("error_count", nullptr);
    if (result == nullptr) {
        return consumeError(Result::PYTHON_ERROR);
    }

    const unsigned long long value = PyLong_AsUnsignedLongLong(result);
    Py_DECREF(result);
    if (PyErr_Occurred()) {
        return consumeError(Result::BAD_RESPONSE);
    }

    out = static_cast<uint64_t>(value);
    return Result::OK;
}

std::string Bridge::listPorts() const {
    std::lock_guard<std::mutex> guard(g_python_mutex);
    if (g_module == nullptr || !Py_IsInitialized()) {
        return std::string();
    }

    Gil gil;
    PyObject *result = call("list_ports", nullptr);
    if (result == nullptr) {
        PyErr_Clear();
        return std::string();
    }

    std::string joined;
    const Py_ssize_t count = PyList_Check(result) ? PyList_Size(result) : 0;
    for (Py_ssize_t i = 0; i < count; ++i) {
        PyObject *item = PyList_GetItem(result, i);
        const char *utf8 = (item != nullptr) ? PyUnicode_AsUTF8(item) : nullptr;
        if (utf8 == nullptr) {
            continue;
        }
        if (!joined.empty()) {
            joined += "\n";
        }
        joined += utf8;
    }

    Py_DECREF(result);
    return joined;
}

std::string Bridge::statusText(int code) const {
    std::lock_guard<std::mutex> guard(g_python_mutex);
    if (g_module == nullptr || !Py_IsInitialized()) {
        return "unknown status code";
    }

    Gil gil;
    PyObject *args = Py_BuildValue("(i)", code);
    PyObject *result = call("status_text", args);
    Py_XDECREF(args);
    if (result == nullptr) {
        PyErr_Clear();
        return "unknown status code";
    }

    const char *utf8 = PyUnicode_AsUTF8(result);
    std::string value = utf8 != nullptr ? utf8 : "unknown status code";
    Py_DECREF(result);
    return value;
}

Result Bridge::isErrorStatus(int code, bool &out) const {
    std::lock_guard<std::mutex> guard(g_python_mutex);
    if (g_module == nullptr || !Py_IsInitialized()) {
        return Result::PYTHON_ERROR;
    }

    Gil gil;
    PyObject *args = Py_BuildValue("(i)", code);
    PyObject *result = call("is_error_status", args);
    Py_XDECREF(args);
    if (result == nullptr) {
        return consumeError(Result::PYTHON_ERROR);
    }

    out = PyObject_IsTrue(result) == 1;
    Py_DECREF(result);
    return Result::OK;
}

std::string Bridge::protocolRevision() const {
    std::lock_guard<std::mutex> guard(g_python_mutex);
    if (g_module == nullptr || !Py_IsInitialized()) {
        return "unknown";
    }

    Gil gil;
    PyObject *result = call("protocol_revision", nullptr);
    if (result == nullptr) {
        PyErr_Clear();
        return "unknown";
    }

    const char *utf8 = PyUnicode_AsUTF8(result);
    std::string value = utf8 != nullptr ? utf8 : "unknown";
    Py_DECREF(result);
    return value;
}

MonitorData Bridge::cachedMonitor() const {
    std::lock_guard<std::mutex> guard(m_cache_mutex);
    return m_cache;
}

LabMMonitor Bridge::cachedLabMMonitor() const {
    std::lock_guard<std::mutex> guard(m_cache_mutex);
    return m_labm_cache;
}

Result Bridge::startMonitoring(uint32_t period_ms) {
    if (!m_initialized) {
        return Result::NOT_INITIALIZED;
    }
    if (m_polling.load()) {
        return Result::OK;
    }

    m_polling.store(true);
    m_poll_thread = std::thread(&Bridge::pollLoop, this, period_ms);
    return Result::OK;
}

void Bridge::stopMonitoring() {
    if (!m_polling.load()) {
        return;
    }

    m_polling.store(false);
    m_poll_cv.notify_all();
    if (m_poll_thread.joinable()) {
        m_poll_thread.join();
    }
}

void Bridge::pollLoop(uint32_t period_ms) {
    while (m_polling.load()) {
        MonitorData sample;
        const Result result = readMonitor(sample);
        if (result != Result::OK) {
            TRACE("monitor poll failed: %s", resultText(result));
        }

        // A LAB_M module has a second monitor container the panel needs. It
        // costs one more command interval, so it is only polled where it
        // exists.
        int module = 0;
        if (moduleType(module) == Result::OK && module == LAB_M_MODULE) {
            LabMMonitor labm;
            const Result labm_result = readLabMMonitor(labm);
            if (labm_result != Result::OK) {
                TRACE("LAB_M monitor poll failed: %s", resultText(labm_result));
            }
        }

        std::unique_lock<std::mutex> lock(m_poll_mutex);
        m_poll_cv.wait_for(lock, std::chrono::milliseconds(period_ms),
                           [this] { return !m_polling.load(); });
    }
}

}  // namespace ptcc
