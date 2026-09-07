#!/usr/bin/python3
"""Smoke test for the rp_ptcc Python binding.

Run on the Red Pitaya with a PTCC-01 controller attached to the USB port:

    LD_LIBRARY_PATH=/opt/redpitaya/lib \
    PYTHONPATH=/opt/redpitaya/lib/python \
    python3 rp_ptcc_test.py

The test only reads from the device. Setpoint writes are commented out
because the controller stores them in the module EEPROM.
"""

import sys
import time

import rp_ptcc

print("=" * 60)
print("VIGO Photonics PTCC Controller Test")
print("=" * 60)

print(f"\nLibrary version   : {rp_ptcc.rp_PtccGetVersion()}")
print(f"Protocol revision : {rp_ptcc.rp_PtccGetProtocolRevision()}")

# List candidate serial ports
print("\n1. rp_ptcc.rp_PtccListPorts()")
res, ports, count = rp_ptcc.rp_PtccListPorts(1024)
print(f"Result: {res}, found {count} port(s)")
for port in ports.split("\n"):
    if port:
        print(f"  {port}")

# Open the first controller found
print("\n2. rp_ptcc.rp_PtccInit()")
res = rp_ptcc.rp_PtccInit()
print(f"Result: {res} ({rp_ptcc.rp_PtccGetErrorText(res)})")
if res != rp_ptcc.RP_PTCC_OK:
    print("No controller found, aborting.")
    sys.exit(1)

# Connection state
print("\n3. rp_ptcc.rp_PtccIsConnected()")
res, connected = rp_ptcc.rp_PtccIsConnected()
print(f"Result: {res}, connected = {connected}")

print("\n4. rp_ptcc.rp_PtccGetDevicePath()")
res, path = rp_ptcc.rp_PtccGetDevicePath(256)
print(f"Result: {res}, path = {path}")

print("\n5. rp_ptcc.rp_PtccGetModuleType()")
res, module = rp_ptcc.rp_PtccGetModuleType()
names = {0: "NONE", 1: "NOMEM", 2: "MEM", 3: "LAB_M"}
print(f"Result: {res}, module = {names.get(module, module)}")

# Identification
print("\n6. rp_ptcc.read_device_iden()")
try:
    for key, value in rp_ptcc.read_device_iden().items():
        print(f"  {key:20s} = {value}")
except RuntimeError as error:
    print(f"  failed: {error}")

print("\n7. rp_ptcc.read_module_iden()")
try:
    for key, value in rp_ptcc.read_module_iden().items():
        print(f"  {key:20s} = {value}")
except RuntimeError as error:
    print(f"  failed: {error}")

# Monitor
print("\n8. rp_ptcc.read_monitor()")
try:
    monitor = rp_ptcc.read_monitor()
    print(f"  T detector           = {monitor['t_det']:.3f} K")
    print(f"  T internal           = {monitor['t_int']:.1f} C")
    print(f"  I TEC                = {monitor['i_tec']:.4f} A")
    print(f"  U TEC                = {monitor['u_tec']:.3f} V")
    print(f"  PWM                  = {monitor['pwm']}")
    print(f"  Supply on            = {monitor['supply_on']}")
    print(f"  Fan on               = {monitor['fan_on']}")
    print(f"  Status               = {monitor['status']} - {monitor['status_text']}")
except RuntimeError as error:
    print(f"  failed: {error}")

# Parameters
print("\n9. rp_ptcc.read_params()")
try:
    for key, value in rp_ptcc.read_params().items():
        print(f"  {key:20s} = {value}")
except RuntimeError as error:
    print(f"  failed: {error}")

print("\n10. rp_ptcc.rp_PtccGetSetpoint()")
res, setpoint = rp_ptcc.rp_PtccGetSetpoint()
print(f"Result: {res}, setpoint = {setpoint:.3f} K")

# Background polling
print("\n11. rp_ptcc.rp_PtccStartMonitoring(1000)")
res = rp_ptcc.rp_PtccStartMonitoring(1000)
print(f"Result: {res}")
for i in range(3):
    time.sleep(1.2)
    monitor = rp_ptcc.read_monitor()
    print(f"  sample {i + 1}: T = {monitor['t_det']:.3f} K, I TEC = {monitor['i_tec']:.4f} A")

print("\n12. rp_ptcc.rp_PtccStopMonitoring()")
res = rp_ptcc.rp_PtccStopMonitoring()
print(f"Result: {res}")

# Writes are destructive for the module EEPROM, so they stay disabled.
# print("\n13. rp_ptcc.rp_PtccSetSetpoint(230.0)")
# res = rp_ptcc.rp_PtccSetSetpoint(230.0)
# print(f"Result: {res} ({rp_ptcc.rp_PtccGetErrorText(res)})")

print("\n14. rp_ptcc.rp_PtccGetErrorCount()")
res, errors = rp_ptcc.rp_PtccGetErrorCount()
print(f"Result: {res}, rejected frames = {errors}")

print("\n15. rp_ptcc.rp_PtccRelease()")
res = rp_ptcc.rp_PtccRelease()
print(f"Result: {res}")

print("\n" + "=" * 60)
print("Test finished")
print("=" * 60)
