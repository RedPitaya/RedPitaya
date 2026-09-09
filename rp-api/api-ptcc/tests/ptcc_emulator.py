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
from ptcc_library.ptcc_defines import ValType
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


def build_responses(args):
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

    def basic_params(setpoint, current_limit):
        return frame(OID.MODULE_BASIC_PARAMS, [
            (OID.MODULE_BASIC_PARAMS_T_DET, setpoint),
            (OID.MODULE_BASIC_PARAMS_I_TEC_MAX, current_limit),
            (OID.MODULE_BASIC_PARAMS_U_SUP_PLUS, 9.0),
            (OID.MODULE_BASIC_PARAMS_U_SUP_MINUS, -9.0),
            (OID.MODULE_BASIC_PARAMS_PWM, 1000),
            (OID.MODULE_BASIC_PARAMS_SUP_CTRL, 0),
            (OID.MODULE_BASIC_PARAMS_FAN_CTRL, 0),
            (OID.MODULE_BASIC_PARAMS_TEC_CTRL, 2),
        ])

    params = basic_params(args.setpoint, 1.5)

    # Real modules restrict the setpoint far more than the protocol does;
    # the user guide quotes 180 to 300 K.
    params_min = basic_params(args.min_setpoint, 0.0)
    params_max = basic_params(args.max_setpoint, 2.0)

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

    return responses


def serve(master, responses, corrupt_crc):
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

            response = responses.get(int.from_bytes(raw[0:2], "big"))
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
    parser.add_argument("--setpoint", type=int, default=250)
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

    responses = {} if args.silent else build_responses(args)

    master, slave = pty.openpty()
    print(os.ttyname(slave), flush=True)
    print(os.getpid(), flush=True)
    sys.stdout.flush()

    try:
        serve(master, responses, args.corrupt_crc)
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
