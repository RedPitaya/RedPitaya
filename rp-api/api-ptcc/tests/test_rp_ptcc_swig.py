#!/usr/bin/env python3
"""Unit tests for the SWIG-generated `rp_ptcc` Python module.

These cover the same API as the C++ GoogleTest suite, but going through the
SWIG typemaps in src/rp_ptcc.i instead of calling into C++ directly. The two
suites fail for different reasons: the C++ one for driver logic, this one for
the marshalling layer.

Run via CTest (see tests/CMakeLists.txt), or standalone once the module is
built and on PYTHONPATH:

    PYTHONPATH=/path/to/build/output:/path/to/build python3 -m unittest test_rp_ptcc_swig -v
"""

import os
import subprocess
import sys
import time
import unittest

import rp_ptcc

EMULATOR = os.environ.get("PTCC_EMULATOR",
                          os.path.join(os.path.dirname(__file__), "ptcc_emulator.py"))


class EmulatedDevice:
    """Starts ptcc_emulator.py and exposes the pty it serves."""

    def __init__(self, *args):
        self.process = subprocess.Popen(
            [sys.executable, "-u", EMULATOR, *args],
            stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, text=True)
        self.port = self.process.stdout.readline().strip()
        self.process.stdout.readline()  # pid, only needed by the C++ harness

    def close(self):
        self.process.terminate()
        self.process.wait(timeout=5)
        self.process.stdout.close()


class LookupTests(unittest.TestCase):
    """Calls that work without an open device."""

    def test_version_strings_are_present(self):
        self.assertTrue(rp_ptcc.rp_PtccGetVersion())
        self.assertTrue(rp_ptcc.rp_PtccGetProtocolRevision())

    def test_error_texts_are_distinct(self):
        codes = [rp_ptcc.RP_PTCC_OK, rp_ptcc.RP_PTCC_ENOINIT, rp_ptcc.RP_PTCC_EOPEN,
                 rp_ptcc.RP_PTCC_EIO, rp_ptcc.RP_PTCC_ETIMEOUT, rp_ptcc.RP_PTCC_ERESP,
                 rp_ptcc.RP_PTCC_EIP, rp_ptcc.RP_PTCC_ERANGE, rp_ptcc.RP_PTCC_ENOTSUP,
                 rp_ptcc.RP_PTCC_ENODEV, rp_ptcc.RP_PTCC_EPYTHON]
        texts = {rp_ptcc.rp_PtccGetErrorText(code) for code in codes}
        self.assertEqual(len(texts), len(codes))

    def test_status_text_comes_from_the_upstream_tables(self):
        self.assertNotEqual(rp_ptcc.rp_PtccGetStatusText(0), "unknown status code")
        self.assertEqual(rp_ptcc.rp_PtccGetStatusText(200), "unknown status code")

        result, is_error = rp_ptcc.rp_PtccIsErrorStatus(128)
        self.assertEqual(result, rp_ptcc.RP_PTCC_OK)
        self.assertTrue(is_error)

        result, is_error = rp_ptcc.rp_PtccIsErrorStatus(0)
        self.assertEqual(result, rp_ptcc.RP_PTCC_OK)
        self.assertFalse(is_error)

    def test_list_ports_returns_a_count_and_a_buffer(self):
        result, ports, count = rp_ptcc.rp_PtccListPorts(1024)
        self.assertEqual(result, rp_ptcc.RP_PTCC_OK)
        self.assertEqual(count == 0, ports.strip() == "")

    def test_ranges_are_left_to_the_upstream_library(self):
        # No device, so nothing reaches the value tables yet.
        self.assertEqual(rp_ptcc.rp_PtccSetSetpoint(50), rp_ptcc.RP_PTCC_ENOINIT)
        self.assertEqual(rp_ptcc.rp_PtccSetSetpoint(230), rp_ptcc.RP_PTCC_ENOINIT)


class DeviceTests(unittest.TestCase):
    """Reads and writes against the emulator."""

    def setUp(self):
        self.device = EmulatedDevice()
        self.assertTrue(self.device.port)
        self.assertEqual(rp_ptcc.rp_PtccInitDevice(self.device.port), rp_ptcc.RP_PTCC_OK)
        rp_ptcc.rp_PtccSetThrottle(0)

    def tearDown(self):
        rp_ptcc.rp_PtccRelease()
        self.device.close()

    def test_connection_state_and_module_type(self):
        result, connected = rp_ptcc.rp_PtccIsConnected()
        self.assertEqual(result, rp_ptcc.RP_PTCC_OK)
        self.assertTrue(connected)

        result, path = rp_ptcc.rp_PtccGetDevicePath(256)
        self.assertEqual(result, rp_ptcc.RP_PTCC_OK)
        self.assertEqual(path, self.device.port)

        result, module = rp_ptcc.rp_PtccGetModuleType()
        self.assertEqual(result, rp_ptcc.RP_PTCC_OK)
        self.assertEqual(module, rp_ptcc.RP_PTCC_MODULE_MEM)

    def test_monitor_helper_returns_si_values(self):
        monitor = rp_ptcc.read_monitor()
        self.assertAlmostEqual(monitor["t_det"], 253.150, places=3)
        self.assertAlmostEqual(monitor["i_tec"], 0.842, places=4)
        self.assertEqual(monitor["pwm"], 12800)
        self.assertTrue(monitor["supply_on"])
        self.assertTrue(monitor["valid"])
        self.assertNotEqual(monitor["status_text"], "")

    def test_params_helper_returns_every_field(self):
        params = rp_ptcc.read_params()
        self.assertAlmostEqual(params["setpoint"], 250.0, places=3)
        self.assertAlmostEqual(params["i_tec_max"], 1.5, places=4)
        self.assertEqual(params["tec_ctrl"], rp_ptcc.RP_PTCC_CTRL_ON)

    def test_identification_helpers(self):
        device = rp_ptcc.read_device_iden()
        self.assertEqual(device["firmware_version"], 1234)
        self.assertEqual(device["hardware_version"], 11)

        module = rp_ptcc.read_module_iden()
        self.assertEqual(module["cool_time"], 300)

    def test_setpoint_round_trip(self):
        self.assertEqual(rp_ptcc.rp_PtccSetSetpoint(230), rp_ptcc.RP_PTCC_OK)
        self.assertEqual(rp_ptcc.rp_PtccSetSetpoint(50), rp_ptcc.RP_PTCC_ERANGE)

        result, setpoint = rp_ptcc.rp_PtccGetSetpoint()
        self.assertEqual(result, rp_ptcc.RP_PTCC_OK)
        self.assertGreater(setpoint, 0.0)

    def test_control_modes_are_accepted(self):
        for mode in (rp_ptcc.RP_PTCC_CTRL_ON, rp_ptcc.RP_PTCC_CTRL_OFF, rp_ptcc.RP_PTCC_CTRL_AUTO):
            self.assertEqual(rp_ptcc.rp_PtccSetCooler(mode), rp_ptcc.RP_PTCC_OK)
            self.assertEqual(rp_ptcc.rp_PtccSetFan(mode), rp_ptcc.RP_PTCC_OK)

    def test_background_poller_updates_the_cache(self):
        self.assertEqual(rp_ptcc.rp_PtccStartMonitoring(100), rp_ptcc.RP_PTCC_OK)
        try:
            deadline = time.monotonic() + 5.0
            sample = None
            while time.monotonic() < deadline:
                sample = rp_ptcc.read_monitor()
                if sample["valid"]:
                    break
                time.sleep(0.05)
            self.assertIsNotNone(sample)
            self.assertTrue(sample["valid"])
        finally:
            self.assertEqual(rp_ptcc.rp_PtccStopMonitoring(), rp_ptcc.RP_PTCC_OK)


if __name__ == "__main__":
    unittest.main()
