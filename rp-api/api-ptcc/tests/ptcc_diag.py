#!/usr/bin/env python3
"""Diagnoses a failing temperature setpoint on real hardware.

Bisects the stack: talks to the controller with the upstream ptcc-library
directly, bypassing librp-ptcc entirely. If the setpoint fails here too, the
problem is between the library and the device, not in the wrapper.

    PYTHONPATH=/opt/redpitaya/lib/python python3 ptcc_diag.py [-d /dev/ttyACM0] [-t 230]

Nothing is written unless --set is given.
"""

import argparse
import sys
import time

sys.path.insert(0, "/opt/redpitaya/lib/python")

from ptcc_library import PtccObjectID, PtccMessageReceiver, detect_device  # noqa: E402
from ptcc_library.communication.ptcc_protocol import generate_msg_set_temperature  # noqa: E402

import rp_ptcc_bridge  # noqa: E402


def dump(title, values):
    print(f"\n--- {title} ---")
    for key, value in values.items():
        print(f"  {key:20s} = {value}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("-d", "--device", default=None)
    parser.add_argument("-t", "--temperature", type=int, default=230)
    parser.add_argument("--set", action="store_true", help="actually write the setpoint (EEPROM)")
    args = parser.parse_args()

    bridge = rp_ptcc_bridge.bridge()

    print("1. opening the controller")
    info = bridge.open(args.device)
    print(f"   port        = {info['port']}")
    print(f"   module type = {info['module_type']} ({bridge.device.module_type.name})")
    print(f"   device class= {type(bridge.device).__name__}")

    print("\n2. current state")
    monitor = bridge.read_monitor()
    dump("monitor", monitor)
    print(f"   status text = {rp_ptcc_bridge.status_text(monitor['status'])}")

    for register, name in ((1, "USER_SET"), (2, "USER_MIN"), (3, "USER_MAX"), (0, "DEFAULT")):
        try:
            dump(f"params {name}", bridge.read_params(register))
        except Exception as error:  # noqa: BLE001
            print(f"\n--- params {name} ---\n  failed: {type(error).__name__}: {error}")

    print("\n3. the exact frame that would be sent")
    try:
        frame = generate_msg_set_temperature(bridge.device.module_type, args.temperature)
        print(f"   {bytes(frame).decode()}")
    except Exception as error:  # noqa: BLE001
        print(f"   cannot build the frame: {type(error).__name__}: {error}")
        return 1

    if not args.set:
        print("\nNothing was written. Re-run with --set to attempt the write.")
        return 0

    print(f"\n4. writing setpoint {args.temperature} K")
    receiver = bridge.receiver
    receiver.clear_all()
    captured = {}

    def capture(children, object_id):
        captured[object_id] = children

    for object_id in (PtccObjectID.MODULE_BASIC_PARAMS.value,
                      PtccObjectID.MODULE_BASIC_PARAMS_T_DET.value,
                      PtccObjectID.PTCC_MONITOR.value):
        receiver.register_callback(object_id, capture, user_data=object_id)

    bridge.transport.flush()
    bridge.device.write_msg_set_temperature(value_in_kelvins=args.temperature)

    deadline = time.monotonic() + 3.0
    raw = b""
    while time.monotonic() < deadline:
        bridge.transport.timeout = 0.2
        chunk = bridge.transport.read(256)
        if chunk:
            raw += chunk
            receiver.add_bytes(chunk)

    print(f"   raw response      = {raw!r}")
    print(f"   containers seen   = {[hex(key) for key in captured]}")
    print(f"   receiver errors   = {receiver.errors}")

    print("\n5. reading the setpoint back")
    try:
        dump("params USER_SET", bridge.read_params(1))
    except Exception as error:  # noqa: BLE001
        print(f"  failed: {type(error).__name__}: {error}")

    bridge.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
