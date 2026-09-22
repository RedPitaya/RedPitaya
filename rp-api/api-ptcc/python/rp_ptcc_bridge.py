"""Flat facade over the upstream VIGO ptcc-library.

librp-ptcc embeds CPython and calls the functions in this module. All PTCC
protocol work is done by the upstream package
(https://gitlab.com/vigophotonics/ptcc-library); nothing here re-implements it.

Everything is returned as plain dicts, floats, ints and strings so the C++ side
only has to deal with simple conversions.
"""

import os
import select
import termios
import time

# Requires Python 3.12 or newer: the upstream package uses typing.override.
from ptcc_library import (
    DeviceRegister,
    GainVoltPerVolt,
    PtccObject,
    ModuleType,
    PtccCtrl,
    PtccMessageReceiver,
    PtccObjectID,
    detect_device,
    error_messages,
    status_messages,
)
from ptcc_library.ptcc_defines import LOOKUP_VALUE_LISTS

DEFAULT_BAUDRATE = 57600

# The device stores the second stage gain as one of the codes in the upstream
# GainVoltPerVolt enum; the panel shows it in V/V. X0_5 -> 0.5, X1_5 -> 1.5.
GAIN_CODE_TO_VOLT_PER_VOLT = {member.value: float(member.name[1:].replace("_", "."))
                              for member in GainVoltPerVolt}

_BAUDRATES = {
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
    230400: termios.B230400,
}

class SerialTransport:
    """CommunicationInterface implementation on top of termios.

    Uses only the standard library so the driver has no pyserial dependency on
    the Red Pitaya image.
    """

    def __init__(self, device, baudrate=DEFAULT_BAUDRATE, timeout=0.1):
        if baudrate not in _BAUDRATES:
            raise ValueError(f"unsupported baudrate {baudrate}")

        self.timeout = timeout
        self.device = device
        self._fd = os.open(device, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)

        try:
            attributes = termios.tcgetattr(self._fd)
        except termios.error as error:
            os.close(self._fd)
            self._fd = -1
            raise IOError(f"{device} is not a serial port: {error}") from error

        iflag, oflag, cflag, lflag, ispeed, ospeed, cc = attributes

        # raw 8N1, no flow control, reader detached from the terminal
        iflag &= ~(termios.IXON | termios.IXOFF | termios.IXANY | termios.ICRNL |
                   termios.INLCR | termios.IGNCR | termios.ISTRIP | termios.INPCK |
                   termios.BRKINT)
        oflag &= ~termios.OPOST
        lflag &= ~(termios.ICANON | termios.ECHO | termios.ECHOE | termios.ISIG | termios.IEXTEN)
        cflag &= ~(termios.CSIZE | termios.PARENB | termios.CSTOPB | termios.CRTSCTS)
        cflag |= termios.CS8 | termios.CREAD | termios.CLOCAL
        cc[termios.VMIN] = 0
        cc[termios.VTIME] = 0
        ispeed = ospeed = _BAUDRATES[baudrate]

        termios.tcsetattr(self._fd, termios.TCSANOW,
                          [iflag, oflag, cflag, lflag, ispeed, ospeed, cc])
        termios.tcflush(self._fd, termios.TCIOFLUSH)

    def write(self, data):
        if self._fd < 0:
            raise IOError("port is closed")
        if isinstance(data, (list, bytearray)):
            data = bytes(data)
        written = 0
        while written < len(data):
            written += os.write(self._fd, data[written:])

    def read(self, size=1):
        if self._fd < 0:
            raise IOError("port is closed")
        deadline = time.monotonic() + self.timeout
        chunks = []
        remaining = size
        while remaining > 0:
            left = deadline - time.monotonic()
            if left <= 0:
                break
            ready, _, _ = select.select([self._fd], [], [], left)
            if not ready:
                break
            chunk = os.read(self._fd, remaining)
            if not chunk:
                break
            chunks.append(chunk)
            remaining -= len(chunk)
        return b"".join(chunks)

    def flush(self):
        if self._fd >= 0:
            termios.tcflush(self._fd, termios.TCIOFLUSH)

    def close(self):
        if self._fd >= 0:
            os.close(self._fd)
            self._fd = -1


class Bridge:
    """Owns one open controller and serialises access to it."""

    def __init__(self):
        self.transport = None
        self.device = None
        self.receiver = None
        self.timeout = 2.0
        self.errors = 0
        self._captured = {}
        self._registered = set()
        self._limits = None
        self._lab_m_limits = None

    # -- connection ------------------------------------------------------

    def open(self, device=None, baudrate=DEFAULT_BAUDRATE):
        self.close()

        candidates = [device] if device else list_ports()
        if not candidates:
            raise IOError("no ttyACM or ttyUSB device nodes found")

        last_error = None
        for candidate in candidates:
            try:
                transport = SerialTransport(candidate, baudrate,
                                            timeout=max(self.timeout / 256.0, 0.005))
            except (OSError, IOError, ValueError) as error:
                last_error = error
                continue

            try:
                receiver = PtccMessageReceiver()
                found = detect_device(comm=transport, receiver=receiver)
                transport.timeout = 0.1
                self.transport = transport
                self.device = found
                self.receiver = receiver
                self._captured = {}
                self._registered = set()
                self._limits = None
                self._lab_m_limits = None
                return {"port": candidate, "module_type": int(found.module_type.value)}
            except (IOError, ValueError) as error:
                last_error = error
                transport.close()

        raise IOError(f"no PTCC controller found ({last_error})")

    def close(self):
        if self.transport is not None:
            self.transport.close()
        self.transport = None
        self.device = None
        self.receiver = None
        self._limits = None
        self._lab_m_limits = None

    def is_open(self):
        return self.device is not None

    def port(self):
        return self.transport.device if self.transport else ""

    def module_type(self):
        return int(self.device.module_type.value) if self.device else int(ModuleType.NONE.value)

    def set_throttle(self, seconds):
        if self.device is not None:
            self.device.comm.min_delay = seconds

    def set_timeout(self, seconds):
        self.timeout = seconds

    def error_count(self):
        return self.errors + (len(self.receiver.errors) if self.receiver else 0)

    # -- transport pump --------------------------------------------------

    def _require_device(self):
        if self.device is None:
            raise IOError("library not initialized")

    def _ensure_callback(self, wanted_id):
        """The receiver clears its object list on every decode, so responses can
        only be observed through callbacks."""
        if wanted_id in self._registered:
            return

        captured = self._captured

        def capture(children, object_id):
            captured[object_id] = children

        self.receiver.register_callback(wanted_id, capture, user_data=wanted_id)
        self._registered.add(wanted_id)

    def _collect(self, wanted_id):
        """Returns {object_id: value} for the first response carrying wanted_id."""
        deadline = time.monotonic() + self.timeout
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                break

            # Never block past the configured timeout.
            self.transport.timeout = min(remaining, 0.1)
            chunk = self.transport.read(256)
            if chunk:
                self.receiver.add_bytes(chunk)

            children = self._captured.pop(wanted_id, None)
            if children is not None:
                return _flatten(children)

        self.errors += 1
        raise TimeoutError("timed out waiting for a response")

    def _request(self, writer, wanted_id, *args, **kwargs):
        self._require_device()
        # Not receiver.reset(): upstream clears the callback registry there too.
        self.receiver.clear_all()
        self._ensure_callback(wanted_id)
        self._captured.pop(wanted_id, None)
        self.transport.flush()
        writer(*args, **kwargs)
        return self._collect(wanted_id)

    # -- reads -----------------------------------------------------------

    def read_monitor(self):
        values = self._request(self.device.write_msg_get_monitor,
                               PtccObjectID.PTCC_MONITOR.value)
        return {
            "t_det": _number(values, PtccObjectID.PTCC_MONITOR_T_DET),
            "t_int": _number(values, PtccObjectID.PTCC_MONITOR_T_INT),
            "i_tec": _number(values, PtccObjectID.PTCC_MONITOR_I_TEC),
            "u_tec": _number(values, PtccObjectID.PTCC_MONITOR_U_TEC),
            "i_sup_plus": _number(values, PtccObjectID.PTCC_MONITOR_I_SUP_PLUS),
            "i_sup_minus": _number(values, PtccObjectID.PTCC_MONITOR_I_SUP_MINUS),
            "u_sup_plus": _number(values, PtccObjectID.PTCC_MONITOR_U_SUP_PLUS),
            "u_sup_minus": _number(values, PtccObjectID.PTCC_MONITOR_U_SUP_MINUS),
            "i_fan": _number(values, PtccObjectID.PTCC_MONITOR_I_FAN_PLUS),
            "th_resistance": _number(values, PtccObjectID.PTCC_MONITOR_TH_ADC),
            "pwm": int(_number(values, PtccObjectID.PTCC_MONITOR_PWM)),
            "status": int(_number(values, PtccObjectID.PTCC_MONITOR_STATUS)),
            "supply_on": bool(values.get(PtccObjectID.PTCC_MONITOR_SUP_ON.value, False)),
            "fan_on": bool(values.get(PtccObjectID.PTCC_MONITOR_FAN_ON.value, False)),
            "timestamp": int(time.monotonic() * 1000.0),
        }

    def read_params(self, register=1):
        self._require_device()
        values = self._request(self.device.write_msg_get_basic_params,
                               PtccObjectID.MODULE_BASIC_PARAMS.value,
                               target=DeviceRegister(register))
        return {
            "setpoint": _number(values, PtccObjectID.MODULE_BASIC_PARAMS_T_DET),
            "i_tec_max": _number(values, PtccObjectID.MODULE_BASIC_PARAMS_I_TEC_MAX),
            "u_sup_plus": _number(values, PtccObjectID.MODULE_BASIC_PARAMS_U_SUP_PLUS),
            "u_sup_minus": _number(values, PtccObjectID.MODULE_BASIC_PARAMS_U_SUP_MINUS),
            "pwm": int(_number(values, PtccObjectID.MODULE_BASIC_PARAMS_PWM)),
            "supply_ctrl": _ctrl(values, PtccObjectID.MODULE_BASIC_PARAMS_SUP_CTRL),
            "fan_ctrl": _ctrl(values, PtccObjectID.MODULE_BASIC_PARAMS_FAN_CTRL),
            "tec_ctrl": _ctrl(values, PtccObjectID.MODULE_BASIC_PARAMS_TEC_CTRL),
        }

    def limits(self):
        """Per-module USER_MIN / USER_MAX, read once and cached.

        These are the ranges writes are checked against. The upstream library
        only knows the protocol range, which is far wider: Vigo confirmed the
        firmware silently clamps anything outside the module's own USER_MIN /
        USER_MAX instead of reporting an error, so a write that would be
        clamped is refused here rather than spent on the EEPROM.
        """
        if self._limits is None:
            try:
                self._limits = (self.read_params(int(DeviceRegister.USER_MIN.value)),
                                self.read_params(int(DeviceRegister.USER_MAX.value)))
            except (IOError, TimeoutError, ValueError):
                self._limits = ({}, {})
        return self._limits

    def read_device_iden(self):
        values = self._request(self.device.write_msg_get_device_iden,
                               PtccObjectID.DEVICE_IDEN.value)
        return {
            "type": _text(values, PtccObjectID.DEVICE_IDEN_TYPE),
            "name": _text(values, PtccObjectID.DEVICE_IDEN_NAME),
            "serial": _text(values, PtccObjectID.DEVICE_IDEN_SERIAL),
            "firmware_version": int(_number(values, PtccObjectID.DEVICE_IDEN_FIRM_VER)),
            "hardware_version": int(_number(values, PtccObjectID.DEVICE_IDEN_HARD_VER)),
        }

    def read_module_iden(self):
        self._require_device()
        if self.device.module_type == ModuleType.NONE:
            raise NotImplementedError("no module attached")

        values = self._request(self.device.write_msg_get_module_iden,
                               PtccObjectID.MODULE_IDEN.value)
        return {
            "type": _text(values, PtccObjectID.MODULE_IDEN_TYPE),
            "name": _text(values, PtccObjectID.MODULE_IDEN_NAME),
            "detector_name": _text(values, PtccObjectID.MODULE_IDEN_DET_NAME),
            "serial": _text(values, PtccObjectID.MODULE_IDEN_SERIAL),
            "detector_serial": _text(values, PtccObjectID.MODULE_IDEN_DET_SERIAL),
            "cool_time": int(_number(values, PtccObjectID.MODULE_IDEN_COOL_TIME)),
        }

    # -- LAB_M module ----------------------------------------------------

    def _check_range(self, value, low, high, what, unit):
        """Refuses a value the module would clamp.

        The device accepts such a write and quietly stores its own limit
        instead, so without this the caller would be told the write succeeded
        while the module runs at a different value.
        """
        if low is None or high is None:
            return

        bottom, top = min(low, high), max(low, high)
        if bottom == top and value == bottom:
            return
        # A tenth of a millikelvin of slack, so a value read back from the
        # device and written again is never refused by rounding alone.
        slack = max(abs(top - bottom), 1.0) * 1e-6
        if bottom - slack <= value <= top + slack:
            return

        suffix = f" {unit}" if unit else ""
        raise ValueError(f"{what} {value}{suffix} is outside the range this module accepts, "
                         f"{bottom} to {top}{suffix}")

    def _check_basic_range(self, value, field, what, unit):
        low, high = self.limits()
        self._check_range(value, low.get(field), high.get(field), what, unit)

    def _check_lab_m_range(self, value, field, what, unit):
        low, high = self.lab_m_limits()
        self._check_range(value, low.get(field), high.get(field), what, unit)

    def _require_lab_m(self):
        """LAB_M containers exist only on LAB_M modules.

        Checked here rather than left to upstream: a MEM module answers the
        LAB_M parameter query with a frame of its own, so without this the
        caller would get a decode failure instead of a plain "not supported".
        """
        self._require_device()
        if self.device.module_type is not ModuleType.LAB_M:
            raise TypeError("the attached module does not support LAB_M parameters")

    def read_lab_m_monitor(self):
        self._require_lab_m()
        values = self._request(self.device.write_msg_get_lab_m_monitor,
                               PtccObjectID.MODULE_LAB_M_MONITOR.value)
        return {
            "u_sup_plus": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_SUP_PLUS),
            "u_sup_minus": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_SUP_MINUS),
            "u_fan": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_FAN_PLUS),
            "i_tec_plus": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_TEC_PLUS),
            "i_tec_minus": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_TEC_MINUS),
            "u_th1": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_TH1),
            "u_th2": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_TH2),
            "u_det": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_U_DET),
            "u_1st": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_U_1ST),
            "u_out": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_U_OUT),
            "temperature": _number(values, PtccObjectID.MODULE_LAB_M_MONITOR_TEMP),
            "timestamp": int(time.monotonic() * 1000.0),
        }

    def read_lab_m_params(self, register=1):
        self._require_lab_m()
        values = self._request(self.device.write_msg_get_lab_m_params,
                               PtccObjectID.MODULE_LAB_M_PARAMS.value,
                               target=DeviceRegister(register))
        gain_code = int(_number(values, PtccObjectID.MODULE_LAB_M_PARAMS_GAIN))
        return {
            # DET_U, DET_I and OFFSET are linear mapped upstream, so these are
            # already Volts and Amperes rather than raw codes.
            "det_bias_u": _number(values, PtccObjectID.MODULE_LAB_M_PARAMS_DET_U),
            "det_bias_i": _number(values, PtccObjectID.MODULE_LAB_M_PARAMS_DET_I),
            "offset": _number(values, PtccObjectID.MODULE_LAB_M_PARAMS_OFFSET),
            "gain_code": gain_code,
            "gain": GAIN_CODE_TO_VOLT_PER_VOLT.get(gain_code, 0.0),
            "gain_known": gain_code in GAIN_CODE_TO_VOLT_PER_VOLT,
            "varactor": int(_number(values, PtccObjectID.MODULE_LAB_M_PARAMS_VARACTOR)),
            "transimpedance": _lookup(values, PtccObjectID.MODULE_LAB_M_PARAMS_TRANS),
            "coupling": _lookup(values, PtccObjectID.MODULE_LAB_M_PARAMS_ACDC),
            "bandwidth": _lookup(values, PtccObjectID.MODULE_LAB_M_PARAMS_BW),
        }

    def lab_m_limits(self):
        """Per-module USER_MIN / USER_MAX of the LAB_M parameters, cached.

        Writes are checked against these, see limits().
        """
        if self._lab_m_limits is None:
            try:
                self._lab_m_limits = (self.read_lab_m_params(int(DeviceRegister.USER_MIN.value)),
                                      self.read_lab_m_params(int(DeviceRegister.USER_MAX.value)))
            except (IOError, TimeoutError, ValueError, TypeError, LookupError):
                self._lab_m_limits = ({}, {})
        return self._lab_m_limits

    # -- writes ----------------------------------------------------------

    def set_temperature(self, kelvin):
        self._require_device()
        self._check_basic_range(float(kelvin), "setpoint", "setpoint", "K")
        return self._request(self.device.write_msg_set_temperature,
                             PtccObjectID.MODULE_BASIC_PARAMS.value,
                             value_in_kelvins=float(kelvin))

    def set_max_current(self, amperes):
        self._require_device()
        self._check_basic_range(float(amperes), "i_tec_max", "TEC current limit", "A")
        return self._request(self.device.write_msg_set_max_current,
                             PtccObjectID.MODULE_BASIC_PARAMS.value,
                             value_in_amperes=float(amperes))

    def set_cooler(self, mode):
        self._require_device()
        writers = {
            int(PtccCtrl.AUTO.value): self.device.write_msg_set_cooler_auto,
            int(PtccCtrl.OFF.value): self.device.write_msg_set_cooler_disabled,
            int(PtccCtrl.ON.value): self.device.write_msg_set_cooler_enabled,
        }
        if mode not in writers:
            raise ValueError(f"invalid cooler mode {mode}")
        return self._request(writers[mode], PtccObjectID.MODULE_BASIC_PARAMS.value)

    def set_fan(self, mode):
        self._require_device()
        return self._request(self.device.write_msg_set_fan,
                             PtccObjectID.MODULE_BASIC_PARAMS.value,
                             mode=PtccCtrl(mode))

    def set_supply(self, mode, u_plus, u_minus):
        """The protocol carries the control mode and both rails in one message,
        so they are written together rather than one field at a time."""
        self._require_device()
        self._check_basic_range(float(u_plus), "u_sup_plus", "positive supply rail", "V")
        self._check_basic_range(float(u_minus), "u_sup_minus", "negative supply rail", "V")
        return self._request(self.device.write_msg_set_supply_voltage,
                             PtccObjectID.MODULE_BASIC_PARAMS.value,
                             supp_ctrl_mode=PtccCtrl(mode),
                             supply_voltage_positive=float(u_plus),
                             supply_voltage_negative=float(u_minus))

    def set_pwm(self, value):
        """PWM has no dedicated message upstream, so it goes through the
        generic basic parameter writer. The value is a raw 0..65535 setting."""
        self._require_device()
        self._check_basic_range(int(value), "pwm", "PWM", "")
        parameter = PtccObject(obj_id=PtccObjectID.MODULE_BASIC_PARAMS_PWM,
                               data_value=int(value))
        return self._request(self.device.write_msg_set_module_param,
                             PtccObjectID.MODULE_BASIC_PARAMS.value,
                             ptcc_object=parameter)

    def _lab_m_write(self, writer, *args, **kwargs):
        self._require_lab_m()
        return self._request(writer, PtccObjectID.MODULE_LAB_M_PARAMS.value, *args, **kwargs)

    def set_lab_m_detector_bias_voltage(self, volts):
        self._check_lab_m_range(float(volts), "det_bias_u", "detector bias", "V")
        return self._lab_m_write(self.device.write_msg_set_module_lab_m_detector_voltage_bias,
                                 bias_value_in_volts=float(volts))

    def set_lab_m_detector_bias_current(self, amperes):
        self._check_lab_m_range(float(amperes), "det_bias_i", "bias current compensation", "A")
        return self._lab_m_write(
            self.device.write_msg_set_module_lab_m_detector_current_bias_compensation,
            bias_value_in_ampers=float(amperes))

    def set_lab_m_offset(self, volts):
        self._check_lab_m_range(float(volts), "offset", "output DC offset", "V")
        return self._lab_m_write(self.device.write_msg_set_module_lab_m_offset,
                                 offset_value_in_volts=float(volts))

    def set_lab_m_gain(self, volt_per_volt):
        """Takes the gain the panel shows, in V/V.

        Only the values the device defines are accepted; picking the nearest
        one silently would report a gain the amplifier is not running at.
        """
        for code, value in GAIN_CODE_TO_VOLT_PER_VOLT.items():
            if abs(value - float(volt_per_volt)) < 1e-6:
                return self.set_lab_m_gain_code(code)
        allowed = ", ".join(f"{value:g}" for value in sorted(GAIN_CODE_TO_VOLT_PER_VOLT.values()))
        raise ValueError(f"gain {volt_per_volt} V/V is not one of: {allowed}")

    def set_lab_m_gain_code(self, code):
        return self._lab_m_write(self.device.write_msg_set_module_lab_m_gain, gain=int(code))

    def set_lab_m_varactor(self, code):
        self._check_lab_m_range(int(code), "varactor", "varactor compensation", "")
        return self._lab_m_write(self.device.write_msg_set_module_lab_m_varactor,
                                 compensation=int(code))

    def set_lab_m_transimpedance(self, mode):
        writers = (self.device.write_msg_set_module_lab_m_transimpedance_low,
                   self.device.write_msg_set_module_lab_m_transimpedance_high)
        return self._lab_m_write(_pick(writers, mode, "transimpedance"))

    def set_lab_m_coupling(self, mode):
        writers = (self.device.write_msg_set_module_lab_m_coupling_ac,
                   self.device.write_msg_set_module_lab_m_coupling_dc)
        return self._lab_m_write(_pick(writers, mode, "coupling"))

    def set_lab_m_bandwidth(self, mode):
        writers = (self.device.write_msg_set_module_lab_m_bandwidth_low,
                   self.device.write_msg_set_module_lab_m_bandwidth_mid,
                   self.device.write_msg_set_module_lab_m_bandwidth_high)
        return self._lab_m_write(_pick(writers, mode, "bandwidth"))


def list_ports():
    """Candidate device nodes, ttyACM before ttyUSB."""
    try:
        names = os.listdir("/dev")
    except OSError:
        return []
    return sorted("/dev/" + name for name in names
                  if name.startswith("ttyACM") or name.startswith("ttyUSB"))


def status_text(code):
    if code in status_messages:
        return status_messages[code]
    if code in error_messages:
        return error_messages[code]
    return "unknown status code"


def is_error_status(code):
    """True when the upstream table lists this code as an error."""
    return code in error_messages


def protocol_revision():
    import ptcc_library

    return getattr(ptcc_library, "__version__", "unknown")


class FieldError(LookupError):
    """A field is missing from the response or cannot be decoded.

    Raised instead of substituting a default: a zero that never came from the
    device is indistinguishable from a real measurement.
    """


def _flatten(children):
    """Maps the objects of a decoded container to their SI values."""
    values = {}
    for item in children:
        try:
            values[item.obj_id] = item.value
        except (ValueError, KeyError, TypeError):
            continue
    return values


def _require(values, object_id):
    if object_id.value not in values:
        raise FieldError(f"{object_id.name} missing from the response")
    return values[object_id.value]


def _number(values, object_id):
    value = _require(values, object_id)
    try:
        return float(value)
    except (TypeError, ValueError) as error:
        raise FieldError(f"{object_id.name} is not numeric: {value!r}") from error


def _text(values, object_id):
    value = _require(values, object_id)
    return value if isinstance(value, str) else str(value)


def _ctrl(values, object_id):
    """Maps a control value back to the 0/1/2 AUTO/OFF/ON encoding."""
    value = _require(values, object_id)
    if isinstance(value, str):
        for member in PtccCtrl:
            if member.name.lower() == value.strip().lower():
                return int(member.value)
        raise FieldError(f"{object_id.name} has an unknown mode {value!r}")
    try:
        return int(value)
    except (TypeError, ValueError) as error:
        raise FieldError(f"{object_id.name} is not a control mode: {value!r}") from error


def _lookup(values, object_id):
    """Maps a looked up value back to its index in the upstream value list.

    Upstream decodes these fields to their label ("LOW", "AC", "MID"), while
    the C API carries the index, which is what the device stores.
    """
    value = _require(values, object_id)
    if isinstance(value, str):
        labels = LOOKUP_VALUE_LISTS[object_id]
        for index, label in enumerate(labels):
            if label.strip().lower() == value.strip().lower():
                return index
        raise FieldError(f"{object_id.name} has an unknown value {value!r}")
    try:
        return int(value)
    except (TypeError, ValueError) as error:
        raise FieldError(f"{object_id.name} is not an index: {value!r}") from error


def _pick(writers, mode, name):
    """Selects the upstream writer for a discrete setting."""
    try:
        index = int(mode)
    except (TypeError, ValueError) as error:
        raise ValueError(f"invalid {name} mode {mode!r}") from error
    if not 0 <= index < len(writers):
        raise ValueError(f"invalid {name} mode {mode}")
    return writers[index]


_BRIDGE = Bridge()


def bridge():
    return _BRIDGE


# Module level entry points called from C++.
def open_device(device=None, baudrate=DEFAULT_BAUDRATE):
    return _BRIDGE.open(device, baudrate)


def close_device():
    _BRIDGE.close()


def is_open():
    return _BRIDGE.is_open()


def port():
    return _BRIDGE.port()


def module_type():
    return _BRIDGE.module_type()


def set_throttle(seconds):
    _BRIDGE.set_throttle(seconds)


def set_timeout(seconds):
    _BRIDGE.set_timeout(seconds)


def error_count():
    return _BRIDGE.error_count()


def read_monitor():
    return _BRIDGE.read_monitor()


def read_params(register=1):
    return _BRIDGE.read_params(register)


def limits():
    low, high = _BRIDGE.limits()
    return {
        "setpoint_min": low.get("setpoint", 0.0),
        "setpoint_max": high.get("setpoint", 0.0),
        "i_tec_max_min": low.get("i_tec_max", 0.0),
        "i_tec_max_max": high.get("i_tec_max", 0.0),
    }


def read_device_iden():
    return _BRIDGE.read_device_iden()


def read_module_iden():
    return _BRIDGE.read_module_iden()


def set_temperature(kelvin):
    _BRIDGE.set_temperature(kelvin)


def set_max_current(amperes):
    _BRIDGE.set_max_current(amperes)


def set_cooler(mode):
    _BRIDGE.set_cooler(mode)


def set_fan(mode):
    _BRIDGE.set_fan(mode)


def set_supply(mode, u_plus, u_minus):
    _BRIDGE.set_supply(mode, u_plus, u_minus)


def set_pwm(value):
    _BRIDGE.set_pwm(value)


def read_lab_m_monitor():
    return _BRIDGE.read_lab_m_monitor()


def read_lab_m_params(register=1):
    return _BRIDGE.read_lab_m_params(register)


def lab_m_limits():
    low, high = _BRIDGE.lab_m_limits()
    return {
        "det_bias_u_min": low.get("det_bias_u", 0.0),
        "det_bias_u_max": high.get("det_bias_u", 0.0),
        "det_bias_i_min": low.get("det_bias_i", 0.0),
        "det_bias_i_max": high.get("det_bias_i", 0.0),
        # OFFSET maps raw 0..256 to +1..-1 V, so USER_MIN carries the larger
        # voltage; report them the way a UI needs them.
        "offset_min": min(low.get("offset", 0.0), high.get("offset", 0.0)),
        "offset_max": max(low.get("offset", 0.0), high.get("offset", 0.0)),
        "varactor_min": low.get("varactor", 0),
        "varactor_max": high.get("varactor", 0),
    }


def lab_m_gain_values():
    """The gains the device accepts, in V/V, ascending."""
    return sorted(GAIN_CODE_TO_VOLT_PER_VOLT.values())


def set_lab_m_detector_bias_voltage(volts):
    _BRIDGE.set_lab_m_detector_bias_voltage(volts)


def set_lab_m_detector_bias_current(amperes):
    _BRIDGE.set_lab_m_detector_bias_current(amperes)


def set_lab_m_offset(volts):
    _BRIDGE.set_lab_m_offset(volts)


def set_lab_m_gain(volt_per_volt):
    _BRIDGE.set_lab_m_gain(volt_per_volt)


def set_lab_m_gain_code(code):
    _BRIDGE.set_lab_m_gain_code(code)


def set_lab_m_varactor(code):
    _BRIDGE.set_lab_m_varactor(code)


def set_lab_m_transimpedance(mode):
    _BRIDGE.set_lab_m_transimpedance(mode)


def set_lab_m_coupling(mode):
    _BRIDGE.set_lab_m_coupling(mode)


def set_lab_m_bandwidth(mode):
    _BRIDGE.set_lab_m_bandwidth(mode)
