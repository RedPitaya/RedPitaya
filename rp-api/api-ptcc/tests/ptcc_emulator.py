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

MODULE_TYPES = {"NONE": 0, "NOMEM": 1, "MEM": 2, "LAB_M": 3}


def frame(container_id, items):
    payload = []
    for object_id, value in items:
        payload += PtccObject(obj_id=object_id, value=value).raw_object

    raw = [(container_id.value >> 8) & 0xFF, container_id.value & 0xFF,
           ((len(payload) + 4) >> 8) & 0xFF, (len(payload) + 4) & 0xFF] + payload
    crc = calculate_ptcc_crc(raw)
    raw += [(crc >> 8) & 0xFF, crc & 0xFF]
    return b"$" + "".join(f"{byte:02X}" for byte in raw).encode() + b"#"


def build_responses(args):
    module = MODULE_TYPES[args.module]

    monitor = frame(OID.PTCC_MONITOR, [
        (OID.PTCC_MONITOR_T_DET, args.t_det),
        (OID.PTCC_MONITOR_T_INT, 31.5),
        (OID.PTCC_MONITOR_I_TEC, 0.842),
        (OID.PTCC_MONITOR_U_TEC, 1.973),
        (OID.PTCC_MONITOR_I_FAN_PLUS, 0.11),
        (OID.PTCC_MONITOR_U_SUP_PLUS, 5.02),
        (OID.PTCC_MONITOR_U_SUP_MINUS, -5.01),
        (OID.PTCC_MONITOR_PWM, 12800),
        (OID.PTCC_MONITOR_STATUS, args.status),
        (OID.PTCC_MONITOR_MODULE_TYPE, module),
        (OID.PTCC_MONITOR_SUP_ON, True),
        (OID.PTCC_MONITOR_FAN_ON, True),
    ])

    params = frame(OID.MODULE_BASIC_PARAMS, [
        (OID.MODULE_BASIC_PARAMS_T_DET, args.setpoint),
        (OID.MODULE_BASIC_PARAMS_I_TEC_MAX, 1.5),
        (OID.MODULE_BASIC_PARAMS_TEC_CTRL, 2),
        (OID.MODULE_BASIC_PARAMS_FAN_CTRL, 0),
        (OID.MODULE_BASIC_PARAMS_PWM, 1000),
    ])

    device_iden = frame(OID.DEVICE_IDEN, [
        (OID.DEVICE_IDEN_FIRM_VER, 1234),
        (OID.DEVICE_IDEN_HARD_VER, 11),
    ])

    module_iden = frame(OID.MODULE_IDEN, [
        (OID.MODULE_IDEN_COOL_TIME, 300),
    ])

    responses = {
        OID.GET_PTCC_MONITOR.value: monitor,
        OID.GET_DEVICE_IDEN.value: device_iden,
        OID.GET_MODULE_IDEN.value: module_iden,
        OID.GET_PTCC_MOD_NO_MEM_IDEN.value: module_iden,
    }
    for command in (OID.GET_MODULE_USER_SET, OID.GET_MODULE_USER_MIN, OID.GET_MODULE_USER_MAX,
                    OID.GET_MODULE_DEFAULT, OID.SET_MODULE_USER_SET,
                    OID.GET_PTCC_MOD_NO_MEM_USER_SET, OID.GET_PTCC_MOD_NO_MEM_USER_MIN,
                    OID.GET_PTCC_MOD_NO_MEM_USER_MAX, OID.GET_PTCC_MOD_NO_MEM_DEFAULT,
                    OID.SET_PTCC_MOD_NO_MEM_USER_SET):
        responses[command.value] = params

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
    parser.add_argument("--status", type=int, default=2)
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
