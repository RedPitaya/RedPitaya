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

namespace ptcc {

namespace {

PyObject *g_module = nullptr;
std::mutex g_python_mutex;
std::string g_last_error;

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
        } else if (PyErr_GivenExceptionMatches(type, PyExc_ValueError)) {
            result = Result::OUT_OF_RANGE;
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

double dictDouble(PyObject *dict, const char *key) {
    PyObject *item = PyDict_GetItemString(dict, key);
    if (item == nullptr) {
        return 0.0;
    }
    const double value = PyFloat_AsDouble(item);
    if (value == -1.0 && PyErr_Occurred()) {
        PyErr_Clear();
        return 0.0;
    }
    return value;
}

long dictLong(PyObject *dict, const char *key) {
    PyObject *item = PyDict_GetItemString(dict, key);
    if (item == nullptr) {
        return 0;
    }
    const long value = PyLong_AsLong(item);
    if (value == -1 && PyErr_Occurred()) {
        PyErr_Clear();
        return 0;
    }
    return value;
}

bool dictBool(PyObject *dict, const char *key) {
    PyObject *item = PyDict_GetItemString(dict, key);
    return item != nullptr && PyObject_IsTrue(item) == 1;
}

std::string dictString(PyObject *dict, const char *key) {
    PyObject *item = PyDict_GetItemString(dict, key);
    if (item == nullptr || !PyUnicode_Check(item)) {
        return std::string();
    }
    const char *utf8 = PyUnicode_AsUTF8(item);
    return utf8 != nullptr ? std::string(utf8) : std::string();
}

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
        const char *directories[] = {PTCC_PYTHON_DIR, PTCC_PYTHON_DIR "/ptcc_library"};
        for (const char *directory : directories) {
            PyObject *entry = PyUnicode_FromString(directory);
            if (entry != nullptr) {
                if (PySequence_Contains(sys_path, entry) == 0) {
                    PyList_Insert(sys_path, 0, entry);
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

int Bridge::moduleType() const {
    if (!m_initialized || !Py_IsInitialized()) {
        return 0;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("module_type", nullptr);
    if (result == nullptr) {
        PyErr_Clear();
        return 0;
    }

    const int value = static_cast<int>(PyLong_AsLong(result));
    Py_DECREF(result);
    return value;
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

        out.t_det_k = dictDouble(result, "t_det");
        out.t_int_c = dictDouble(result, "t_int");
        out.i_tec_a = dictDouble(result, "i_tec");
        out.u_tec_v = dictDouble(result, "u_tec");
        out.i_sup_plus_a = dictDouble(result, "i_sup_plus");
        out.i_sup_minus_a = dictDouble(result, "i_sup_minus");
        out.u_sup_plus_v = dictDouble(result, "u_sup_plus");
        out.u_sup_minus_v = dictDouble(result, "u_sup_minus");
        out.i_fan_a = dictDouble(result, "i_fan");
        out.th_resistance = dictDouble(result, "th_resistance");
        out.pwm = static_cast<uint32_t>(dictLong(result, "pwm"));
        out.status = static_cast<uint8_t>(dictLong(result, "status"));
        out.supply_on = dictBool(result, "supply_on");
        out.fan_on = dictBool(result, "fan_on");
        out.timestamp_ms = nowMs();
        out.valid = true;

        Py_DECREF(result);
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

    out.setpoint_k = dictDouble(result, "setpoint");
    out.i_tec_max_a = dictDouble(result, "i_tec_max");
    out.u_sup_plus_v = dictDouble(result, "u_sup_plus");
    out.u_sup_minus_v = dictDouble(result, "u_sup_minus");
    out.pwm = static_cast<uint32_t>(dictLong(result, "pwm"));
    out.supply_ctrl = static_cast<int>(dictLong(result, "supply_ctrl"));
    out.fan_ctrl = static_cast<int>(dictLong(result, "fan_ctrl"));
    out.tec_ctrl = static_cast<int>(dictLong(result, "tec_ctrl"));
    out.valid = true;

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

    out.type = dictString(result, "type");
    out.name = dictString(result, "name");
    out.serial = dictString(result, "serial");
    out.firmware_version = static_cast<uint32_t>(dictLong(result, "firmware_version"));
    out.hardware_version = static_cast<uint32_t>(dictLong(result, "hardware_version"));
    out.valid = true;

    Py_DECREF(result);
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

    out.type = dictString(result, "type");
    out.name = dictString(result, "name");
    out.detector_name = dictString(result, "detector_name");
    out.serial = dictString(result, "serial");
    out.detector_serial = dictString(result, "detector_serial");
    out.cool_time_s = static_cast<uint32_t>(dictLong(result, "cool_time"));
    out.valid = true;

    Py_DECREF(result);
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

uint64_t Bridge::errorCount() const {
    if (!m_initialized || !Py_IsInitialized()) {
        return 0;
    }

    std::lock_guard<std::mutex> guard(g_python_mutex);
    Gil gil;

    PyObject *result = call("error_count", nullptr);
    if (result == nullptr) {
        PyErr_Clear();
        return 0;
    }

    const uint64_t value = static_cast<uint64_t>(PyLong_AsUnsignedLongLong(result));
    Py_DECREF(result);
    PyErr_Clear();
    return value;
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

bool Bridge::isErrorStatus(int code) const {
    std::lock_guard<std::mutex> guard(g_python_mutex);
    if (g_module == nullptr || !Py_IsInitialized()) {
        return code >= 128;
    }

    Gil gil;
    PyObject *args = Py_BuildValue("(i)", code);
    PyObject *result = call("is_error_status", args);
    Py_XDECREF(args);
    if (result == nullptr) {
        PyErr_Clear();
        return code >= 128;
    }

    const bool value = PyObject_IsTrue(result) == 1;
    Py_DECREF(result);
    return value;
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

        std::unique_lock<std::mutex> lock(m_poll_mutex);
        m_poll_cv.wait_for(lock, std::chrono::milliseconds(period_ms),
                           [this] { return !m_polling.load(); });
    }
}

}  // namespace ptcc
