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
    ModuleType,
    PtccCtrl,
    PtccLabMDevice,
    PtccMessageReceiver,
    PtccObjectID,
    detect_device,
    error_messages,
    status_messages,
)

DEFAULT_BAUDRATE = 57600

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

    # -- writes ----------------------------------------------------------

    def set_temperature(self, kelvin):
        self._require_device()
        if isinstance(self.device, PtccLabMDevice):
            raise NotImplementedError("LAB_M modules do not accept a temperature setpoint")
        return self._request(self.device.write_msg_set_temperature,
                             PtccObjectID.MODULE_BASIC_PARAMS.value,
                             value_in_kelvins=int(round(kelvin)))

    def set_max_current(self, amperes):
        self._require_device()
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
    return code in error_messages


def protocol_revision():
    import ptcc_library

    return getattr(ptcc_library, "__version__", "unknown")


def _flatten(children):
    """Maps the objects of a decoded container to their SI values."""
    values = {}
    for item in children:
        try:
            values[item.obj_id] = item.value
        except (ValueError, KeyError, TypeError):
            continue
    return values


def _number(values, object_id):
    value = values.get(object_id.value, 0)
    try:
        return float(value)
    except (TypeError, ValueError):
        return 0.0


def _text(values, object_id):
    value = values.get(object_id.value, "")
    return value if isinstance(value, str) else str(value)


def _ctrl(values, object_id):
    """Maps a control value back to the 0/1/2 AUTO/OFF/ON encoding."""
    value = values.get(object_id.value)
    if isinstance(value, str):
        for member in PtccCtrl:
            if member.name.lower() == value.strip().lower():
                return int(member.value)
        return int(PtccCtrl.AUTO.value)
    try:
        return int(value)
    except (TypeError, ValueError):
        return int(PtccCtrl.AUTO.value)


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
