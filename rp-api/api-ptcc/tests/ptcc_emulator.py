#!/usr/bin/env python3
"""PTY based PTCC-01 emulator used as the device under test.

Responses are built with the upstream ptcc-library itself, so the fixture and
the driver never share a private encoder: a regression in either one shows up
as a decode failure instead of cancelling out.

Prints the slave pty path and its own pid on stdout, then serves requests
until killed. The emulator keeps the slave fd open so the driver can reopen
the port, which also means it never sees EOF: the test harness terminates it.

    ptcc_emulator.py [--module MEM|NOMEM|LAB_M|NONE] [--t-det 253.15]
"""

import argparse
import os
import pty
import sys

from ptcc_library import PtccObjectID as OID
from ptcc_library.communication.ptcc_protocol import calculate_ptcc_crc
from ptcc_library.ptcc_object import PtccObject
from ptcc_library.ptcc_defines import LOOKUP_VALUE_LISTS, ValType
from ptcc_library.ptcc_utils import to_bytes

MODULE_TYPES = {"NONE": 0, "NOMEM": 1, "MEM": 2, "LAB_M": 3}


def raw_leaf(object_id, payload):
    return PtccObject(raw_object=to_bytes(ValType.UINT16, object_id.value) +
                      to_bytes(ValType.UINT16, 4 + len(payload)) + payload)


def leaf(object_id, value):
    """Builds one object.

    PtccObject(value=...) covers the numeric types. Two cases it cannot encode
    are assembled from raw bytes: CSTR, and SERIAL_NUMBER, for which upstream
    to_bytes() raises NotImplementedError (it only decodes that type).
    """
    value_type = ValType(object_id.value & 0x0F)
    if value_type is ValType.CSTR:
        return raw_leaf(object_id, to_bytes(ValType.CSTR, value))
    if value_type is ValType.SERIAL_NUMBER:
        return raw_leaf(object_id, to_bytes(ValType.UINT32, int(value)))
    return PtccObject(obj_id=object_id, value=value)


def frame(container_id, items):
    """Encodes a device response container.

    Every step is the upstream library's own: PtccObject for the leaves,
    PtccObject.pack_container for the container header and length, and
    calculate_ptcc_crc plus the hex wrapping used by create_set_ptcc_message.
    A device response is a bare container rather than the nested SET command
    that create_set_ptcc_message builds, which is why it is assembled here
    instead of being called directly.
    """
    objects = [leaf(object_id, value) for object_id, value in items]

    container = PtccObject(raw_object=to_bytes(ValType.UINT16, container_id.value) +
                           to_bytes(ValType.UINT16, 4))
    raw = container.pack_container(objects)
    raw += to_bytes(ValType.UINT16, calculate_ptcc_crc(raw))

    encoded = []
    for byte in raw:
        pair = format(byte, "02x").upper()
        encoded.append(ord(pair[0]))
        encoded.append(ord(pair[1]))

    return bytes([ord("$")] + encoded + [ord("#")])


def build_responses(args, basic, lab_m):
    module = MODULE_TYPES[args.module]

    monitor_fields = [
        (OID.PTCC_MONITOR_T_DET, args.t_det),
        (OID.PTCC_MONITOR_T_INT, 31.5),
        (OID.PTCC_MONITOR_I_TEC, 0.842),
        (OID.PTCC_MONITOR_U_TEC, 1.973),
        (OID.PTCC_MONITOR_I_SUP_PLUS, 0.0655),
        (OID.PTCC_MONITOR_I_SUP_MINUS, -0.0732),
        (OID.PTCC_MONITOR_I_FAN_PLUS, 0.11),
        (OID.PTCC_MONITOR_U_SUP_PLUS, 5.02),
        (OID.PTCC_MONITOR_U_SUP_MINUS, -5.01),
        (OID.PTCC_MONITOR_TH_ADC, 524288),
        (OID.PTCC_MONITOR_PWM, 12800),
        (OID.PTCC_MONITOR_STATUS, args.status),
        (OID.PTCC_MONITOR_MODULE_TYPE, module),
        (OID.PTCC_MONITOR_SUP_ON, True),
        (OID.PTCC_MONITOR_FAN_ON, True),
    ]

    if args.drop_field:
        monitor_fields = [item for item in monitor_fields
                          if item[0] is not OID.PTCC_MONITOR_I_TEC]

    monitor = frame(OID.PTCC_MONITOR, monitor_fields)

    def basic_params(values):
        return frame(OID.MODULE_BASIC_PARAMS, [
            (OID.MODULE_BASIC_PARAMS_T_DET, values["setpoint"]),
            (OID.MODULE_BASIC_PARAMS_I_TEC_MAX, values["current_limit"]),
            (OID.MODULE_BASIC_PARAMS_U_SUP_PLUS, values["u_sup_plus"]),
            (OID.MODULE_BASIC_PARAMS_U_SUP_MINUS, values["u_sup_minus"]),
            (OID.MODULE_BASIC_PARAMS_PWM, values["pwm"]),
            (OID.MODULE_BASIC_PARAMS_SUP_CTRL, values["supply_ctrl"]),
            (OID.MODULE_BASIC_PARAMS_FAN_CTRL, values["fan_ctrl"]),
            (OID.MODULE_BASIC_PARAMS_TEC_CTRL, values["tec_ctrl"]),
        ])

    params = basic_params(basic)

    # Real modules restrict the setpoint far more than the protocol does;
    # the user guide quotes 180 to 300 K.
    params_min = basic_params(dict(basic, setpoint=args.min_setpoint, current_limit=0.0,
                                   u_sup_plus=3.0, u_sup_minus=-15.0, pwm=0))
    params_max = basic_params(dict(basic, setpoint=args.max_setpoint, current_limit=2.0,
                                   u_sup_plus=15.0, u_sup_minus=-3.0, pwm=65535))

    device_iden = frame(OID.DEVICE_IDEN, [
        (OID.DEVICE_IDEN_TYPE, 1),
        (OID.DEVICE_IDEN_NAME, "SmartTEC-ADV"),
        (OID.DEVICE_IDEN_SERIAL, 20250609),
        (OID.DEVICE_IDEN_FIRM_VER, 1234),
        (OID.DEVICE_IDEN_HARD_VER, 11),
    ])

    module_iden = frame(OID.MODULE_IDEN, [
        (OID.MODULE_IDEN_TYPE, module),
        (OID.MODULE_IDEN_NAME, "LabM-I-10.6"),
        (OID.MODULE_IDEN_DET_NAME, "PVMI-4TE-10.6-1x1"),
        (OID.MODULE_IDEN_SERIAL, 444161139),
        (OID.MODULE_IDEN_DET_SERIAL, 143339),
        (OID.MODULE_IDEN_COOL_TIME, 300),
    ])

    responses = {
        OID.GET_PTCC_MONITOR.value: monitor,
        OID.GET_DEVICE_IDEN.value: device_iden,
        OID.GET_MODULE_IDEN.value: module_iden,
        OID.GET_PTCC_MOD_NO_MEM_IDEN.value: module_iden,
    }
    for command in (OID.GET_MODULE_USER_SET, OID.GET_MODULE_DEFAULT, OID.SET_MODULE_USER_SET,
                    OID.GET_PTCC_MOD_NO_MEM_USER_SET, OID.GET_PTCC_MOD_NO_MEM_DEFAULT,
                    OID.SET_PTCC_MOD_NO_MEM_USER_SET):
        responses[command.value] = params

    for command in (OID.GET_MODULE_USER_MIN, OID.GET_PTCC_MOD_NO_MEM_USER_MIN):
        responses[command.value] = params_min

    for command in (OID.GET_MODULE_USER_MAX, OID.GET_PTCC_MOD_NO_MEM_USER_MAX):
        responses[command.value] = params_max

    if module == MODULE_TYPES["LAB_M"]:
        responses.update(lab_m_responses(lab_m))

    return responses


def lab_m_responses(state):
    """The second container pair only a LAB_M module answers.

    DET_U, DET_I and OFFSET are linear mapped upstream (raw 0..256 to 0..1 V,
    0..10 mA and +1..-1 V), so the values here are SI and the encoder does the
    mapping. GAIN is a device code, TRANS/ACDC/BW are indices into the
    upstream value lists.
    """

    def lab_m_params(values):
        return frame(OID.MODULE_LAB_M_PARAMS, [
            (OID.MODULE_LAB_M_PARAMS_DET_U, values["det_bias_u"]),
            (OID.MODULE_LAB_M_PARAMS_DET_I, values["det_bias_i"]),
            (OID.MODULE_LAB_M_PARAMS_GAIN, values["gain_code"]),
            (OID.MODULE_LAB_M_PARAMS_OFFSET, values["offset"]),
            (OID.MODULE_LAB_M_PARAMS_VARACTOR, values["varactor"]),
            (OID.MODULE_LAB_M_PARAMS_TRANS, values["transimpedance"]),
            (OID.MODULE_LAB_M_PARAMS_ACDC, values["coupling"]),
            (OID.MODULE_LAB_M_PARAMS_BW, values["bandwidth"]),
        ])

    monitor = frame(OID.MODULE_LAB_M_MONITOR, [
        (OID.MODULE_LAB_M_MONITOR_SUP_PLUS, 5.02),
        (OID.MODULE_LAB_M_MONITOR_SUP_MINUS, -5.01),
        (OID.MODULE_LAB_M_MONITOR_FAN_PLUS, 11.98),
        (OID.MODULE_LAB_M_MONITOR_TEC_PLUS, 0.842),
        (OID.MODULE_LAB_M_MONITOR_TEC_MINUS, -0.840),
        (OID.MODULE_LAB_M_MONITOR_TH1, 1.234),
        (OID.MODULE_LAB_M_MONITOR_TH2, 0.987),
        (OID.MODULE_LAB_M_MONITOR_U_DET, 0.650),
        (OID.MODULE_LAB_M_MONITOR_U_1ST, 0.112),
        (OID.MODULE_LAB_M_MONITOR_U_OUT, 1.503),
        (OID.MODULE_LAB_M_MONITOR_TEMP, 29.5),
    ])

    params = lab_m_params(state)
    # Raw 0 is +1 V for OFFSET, so USER_MIN carries the higher voltage.
    params_min = lab_m_params({"det_bias_u": 0.0, "det_bias_i": 0.0, "gain_code": 48,
                               "offset": 1.0, "varactor": 0, "transimpedance": 0,
                               "coupling": 0, "bandwidth": 0})
    params_max = lab_m_params({"det_bias_u": 1.0, "det_bias_i": 0.01, "gain_code": 111,
                               "offset": -1.0, "varactor": 4095, "transimpedance": 1,
                               "coupling": 1, "bandwidth": 2})

    return {
        OID.GET_MODULE_LAB_M_MONITOR.value: monitor,
        OID.GET_MODULE_LAB_M_USER_SET.value: params,
        OID.GET_MODULE_LAB_M_DEFAULT.value: params,
        OID.SET_MODULE_LAB_M_USER_SET.value: params,
        OID.GET_MODULE_LAB_M_USER_MIN.value: params_min,
        OID.GET_MODULE_LAB_M_USER_MAX.value: params_max,
    }


class Responder:
    """Answers requests and remembers what was written.

    A SET command updates the stored basic parameters before it is answered,
    so the next read returns the value the driver actually put on the wire.
    Only the writable fields are tracked, the rest of the answers are static.
    """

    SET_COMMANDS = (OID.SET_MODULE_USER_SET.value,
                    OID.SET_PTCC_MOD_NO_MEM_USER_SET.value,
                    OID.SET_MODULE_LAB_M_USER_SET.value)

    # Writable leaf -> the state entry it lands in.
    BASIC_FIELDS = {
        OID.MODULE_BASIC_PARAMS_T_DET.value: "setpoint",
        OID.MODULE_BASIC_PARAMS_I_TEC_MAX.value: "current_limit",
        OID.MODULE_BASIC_PARAMS_U_SUP_PLUS.value: "u_sup_plus",
        OID.MODULE_BASIC_PARAMS_U_SUP_MINUS.value: "u_sup_minus",
        OID.MODULE_BASIC_PARAMS_PWM.value: "pwm",
        OID.MODULE_BASIC_PARAMS_SUP_CTRL.value: "supply_ctrl",
        OID.MODULE_BASIC_PARAMS_FAN_CTRL.value: "fan_ctrl",
        OID.MODULE_BASIC_PARAMS_TEC_CTRL.value: "tec_ctrl",
    }
    LAB_M_FIELDS = {
        OID.MODULE_LAB_M_PARAMS_DET_U.value: "det_bias_u",
        OID.MODULE_LAB_M_PARAMS_DET_I.value: "det_bias_i",
        OID.MODULE_LAB_M_PARAMS_GAIN.value: "gain_code",
        OID.MODULE_LAB_M_PARAMS_OFFSET.value: "offset",
        OID.MODULE_LAB_M_PARAMS_VARACTOR.value: "varactor",
        OID.MODULE_LAB_M_PARAMS_TRANS.value: "transimpedance",
        OID.MODULE_LAB_M_PARAMS_ACDC.value: "coupling",
        OID.MODULE_LAB_M_PARAMS_BW.value: "bandwidth",
    }

    def __init__(self, args):
        self.args = args
        self.basic = {
            "setpoint": float(args.setpoint),
            "current_limit": 1.5,
            "u_sup_plus": 9.0,
            "u_sup_minus": -9.0,
            "pwm": 1000,
            "supply_ctrl": 0,
            "fan_ctrl": 0,
            "tec_ctrl": 2,
        }
        self.lab_m = {
            "det_bias_u": 0.5,
            "det_bias_i": 0.004,
            "gain_code": 85,     # X10
            "offset": 0.0,
            "varactor": 2048,
            "transimpedance": 1,  # HIGH
            "coupling": 1,        # DC
            "bandwidth": 2,       # HIGH
        }
        self._rebuild()

    def _rebuild(self):
        self.responses = build_responses(self.args, self.basic, self.lab_m)

    def answer(self, raw):
        command = int.from_bytes(raw[0:2], "big")
        if command in self.SET_COMMANDS:
            self._apply(raw)
        return self.responses.get(command)

    def _apply(self, raw):
        """Decodes a SET frame with the upstream parser and stores its values."""
        try:
            request = PtccObject(raw_object=raw[:-2])
        except (ValueError, KeyError):
            return

        for container in request.objects:
            for item in container.objects:
                try:
                    value = item.value
                except (ValueError, KeyError):
                    continue

                if item.obj_id in self.BASIC_FIELDS:
                    self.basic[self.BASIC_FIELDS[item.obj_id]] = _index_of(item.obj_id, value)
                elif item.obj_id in self.LAB_M_FIELDS:
                    self.lab_m[self.LAB_M_FIELDS[item.obj_id]] = _index_of(item.obj_id, value)

        self._rebuild()


def _index_of(object_id, value):
    """Turns a decoded label back into the index the frames are built from."""
    if not isinstance(value, str):
        return value
    labels = LOOKUP_VALUE_LISTS[OID(object_id)]
    for index, label in enumerate(labels):
        if label.strip().lower() == value.strip().lower():
            return index
    raise ValueError(f"unknown value {value!r} for {OID(object_id).name}")


def serve(master, responder, corrupt_crc):
    buffer = b""
    while True:
        try:
            data = os.read(master, 256)
        except OSError:
            return
        if not data:
            return

        buffer += data
        while b"#" in buffer:
            request, _, buffer = buffer.partition(b"#")
            if b"$" not in request:
                continue
            request = request[request.rfind(b"$") + 1:]

            try:
                raw = bytes.fromhex(request.decode())
            except ValueError:
                continue
            if len(raw) < 2:
                continue

            response = responder.answer(raw) if responder else None
            if response is None:
                continue

            if corrupt_crc:
                broken = bytearray(response)
                broken[-2] = ord("0") if broken[-2] != ord("0") else ord("1")
                response = bytes(broken)

            os.write(master, response)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--module", default="MEM", choices=sorted(MODULE_TYPES))
    parser.add_argument("--t-det", type=float, default=253.150)
    parser.add_argument("--setpoint", type=float, default=250)
    parser.add_argument("--min-setpoint", type=int, default=180)
    parser.add_argument("--max-setpoint", type=int, default=300)
    parser.add_argument("--status", type=int, default=2)
    parser.add_argument("--drop-field", action="store_true",
                        help="omit PTCC_MONITOR_I_TEC to exercise the missing field path")
    parser.add_argument("--corrupt-crc", action="store_true",
                        help="answer with a broken CRC to exercise the error path")
    parser.add_argument("--silent", action="store_true",
                        help="accept requests but never answer, to exercise timeouts")
    args = parser.parse_args()

    responder = None if args.silent else Responder(args)

    master, slave = pty.openpty()
    print(os.ttyname(slave), flush=True)
    print(os.getpid(), flush=True)
    sys.stdout.flush()

    try:
        serve(master, responder, args.corrupt_crc)
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
