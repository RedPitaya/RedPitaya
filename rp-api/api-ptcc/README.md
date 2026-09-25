# api-ptcc

C++ driver for the VIGO Photonics PTCC-01 TEC controller, built as
`librp-ptcc.so` / `librp-ptcc.a` with a C API in `include/rp_ptcc.h`.

The controller appears as a serial port with no extra kernel driver: units that
enumerate as USB CDC-ACM show up as `/dev/ttyACM0`, while those with an FTDI
bridge show up as `/dev/ttyUSB0` (a PTCC-01-BAS on the bench carries an FT232).
Both are probed, in that order.

## Architecture

The library is a **wrapper, not a port**. All protocol work (framing, CRC, TLV
objects, SI scaling, value ranges, status strings) stays in the upstream
Python package at https://gitlab.com/vigophotonics/ptcc-library. The C++ side
embeds CPython, imports `rp_ptcc_bridge`, holds the GIL around every call and
converts the returned dictionaries into C structs.

```
rp_ptcc.h  (C API)
   |
rp_ptcc.cpp        argument checks, struct conversion, global state
   |
ptcc_bridge.cpp    embedded interpreter, GIL, PyObject -> C++
   |
python/rp_ptcc_bridge.py   flat facade + stdlib termios transport
   |
ptcc_library (upstream)    protocol, single source of truth
```

Nothing in C++ re-implements the protocol, so a fix released upstream reaches
the driver by bumping `PTCC_UPSTREAM_TAG` and rebuilding.

Two notes on the upstream package:

* it uses `typing.override`, so Python 3.12 or newer is required. Red Pitaya OS
  ships 3.12.3, so the package runs unmodified.
* it has no serial dependency. `rp_ptcc_bridge.SerialTransport` implements the
  `CommunicationInterface` contract with `termios` from the standard library,
  so pyserial is not needed on the image.

## What it covers

The controller half (`rp_PtccReadMonitor`, `rp_PtccGetParams`,
`rp_PtccSetSetpoint`, `rp_PtccSetMaxCurrent`, module supply, TEC PWM, cooler
and fan control) works with every module type. The detection module half (`rp_Ptcc*LabM*`) reads and
writes the LAB_M amplifier: detector bias voltage and current compensation,
output DC offset, preamplifier gain, varactor compensation, 1st stage
transimpedance, signal coupling and bandwidth, plus the LAB_M monitor
container (supply and TEC rails, both thermistors, detector bias, 1st stage
and output voltages, enclosure temperature).

## Build

```
cmake -B build -DINSTALL_DIR=/opt/redpitaya -DVERSION=1.0.0 -DREVISION=$(git rev-parse --short HEAD)
cmake --build build -j4
cmake --install build
```

`BUILD_SHARED`, `BUILD_STATIC` and `BUILD_PYTHON_MODULE` (SWIG) follow the same
convention as the other Red Pitaya API libraries. ARM compile flags are applied
only when `CMAKE_SYSTEM_PROCESSOR` is arm or aarch64, so the driver can be
built and tested on a host machine.

CMake clones the upstream package and installs it next to the bridge module:

| Option | Default | Meaning |
| --- | --- | --- |
| `PTCC_FETCH_UPSTREAM` | `ON` | Clone ptcc-library and install its package |
| `PTCC_UPSTREAM_URL` | GitLab HTTPS URL | Git remote |
| `PTCC_UPSTREAM_TAG` | `v0.2.1` | Tag, branch or commit to pin |
| `PTCC_PYTHON_DIR` | `${INSTALL_DIR}/lib/python` | Directory the bridge adds to `sys.path` |
| `BUILD_UNIT_TESTS` | `OFF` | Build and run the GoogleTest suite |

With `PTCC_FETCH_UPSTREAM=OFF` the package is expected to be already present in
`PTCC_PYTHON_DIR` or on `PYTHONPATH`.

Installed layout:

```
${INSTALL_DIR}/lib/librp-ptcc.so
${INSTALL_DIR}/include/rp_ptcc.h
${INSTALL_DIR}/lib/python/rp_ptcc_bridge.py
${INSTALL_DIR}/lib/python/ptcc_library/...
```

## Usage

```c
#include "rp_ptcc.h"

if (rp_PtccInit() != RP_PTCC_OK) {
    return -1;
}

rp_ptcc_monitor_t monitor;
rp_PtccReadMonitor(&monitor);
printf("%.3f K, status: %s\n", monitor.t_det, rp_PtccGetStatusText(monitor.status));

rp_PtccRelease();
```

For a web backend, start the background poller once and read the cache, which
never blocks and never touches the interpreter's slow path:

```c
rp_PtccStartMonitoring(1000);
...
rp_ptcc_monitor_t monitor;
rp_PtccGetMonitor(&monitor);
```

## Constraints

* The firmware needs about 0.55 s between commands, enforced upstream by
  `ThrottledCommunication`. Blocking calls wait for that interval, giving a
  ceiling of roughly 1.8 commands per second. Never call the API from an
  acquisition or rendering loop.
* `rp_PtccSetSetpoint()` and `rp_PtccSetMaxCurrent()` write to the module
  EEPROM. Debounce them in the UI and issue one write per confirmed user
  action.
* Value ranges come from two places, both of them the device's own. The
  upstream library knows the protocol range (100 to 400 K for the setpoint),
  and the module carries its own USER_MIN / USER_MAX, which is much narrower
  (180 to 300 K on the bench module). Vigo confirmed the firmware **silently
  clamps** a value outside the module range: the write is accepted, nothing is
  reported, and the module stores its own limit instead. So a write that would
  be clamped is refused here, as `RP_PTCC_ERANGE`, rather than spent on the
  EEPROM and reported as a success. `rp_PtccGetLimits()` and
  `rp_PtccGetLabMLimits()` expose the same ranges for a UI to bound its
  controls with.
* A module that reports the same USER_MIN and USER_MAX for a field does not
  let that field be changed at all: the bench module fixes its supply rails at
  9 V and its writable PWM at 0. A panel should read the range and disable the
  control rather than assume a setting is editable.
* `rp_PtccSetSetpoint()` takes Kelvins as a float. The protocol encodes the
  setpoint with three decimals, so fractions reach the device; the UI
  requirement of one decimal is well inside that.
* A field the device did not report is `RP_PTCC_ERESP`, not a zero. Zeros from
  this API are always measurements.
* The cooler only cools. Asking for a setpoint above ambient will not be
  reached and the device reports "detector overheat" after 120 s.
* Errors cannot be cleared from software. Vigo confirmed that only a power
  cycle clears the state in `PTCC_MONITOR_STATUS`, and the device does not work
  properly until it is cleared, so an application should show the error and ask
  the user to power cycle the module.
* `LAB_M` modules take the temperature setpoint like any other: the controller
  panel is the same for every module, only the amplifier settings differ.
* The detection module calls (`rp_Ptcc*LabM*`) need a LAB_M module and answer
  `RP_PTCC_ENOTSUP` on anything else, checked against the reported module type
  rather than left to time out.
* Detector bias, bias current compensation and the output offset are stored by
  the module as 0..256 codes mapped to 0..1 V, 0..10 mA and +1..-1 V, so a
  written value comes back quantised to about 3.9 mV, 39 uA and 7.8 mV.
* The preamplifier gain is one of eleven documented steps
  (`rp_PtccGetLabMGainValues()`); anything else is `RP_PTCC_ERANGE` rather
  than a silent nearest match. A device reporting a code outside that set
  comes back with `gain_known` false and the raw code in `gain_code`, which
  `rp_PtccSetLabMGainCode()` can write back.
* `rp_PtccStartMonitoring()` polls the LAB_M monitor as well on a LAB_M
  module, so the panel costs two command intervals per period there.
* `rp_PtccSetSupply()` takes the control mode and both rails together: the
  protocol carries them in one message, so a UI changing one field has to send
  the other two as they are. The firmware accepts 3 to 15 V on the positive
  rail and -15 to -3 V on the negative one.
* `rp_PtccSetPwm()` goes through the generic basic parameter message, since
  upstream has no dedicated one, and takes the raw 0..65535 setting.
* The library embeds one interpreter per process. Calls are serialised behind a
  mutex and the GIL, so the API is thread safe but not concurrent.

## Testing

```
cmake -B build -DINSTALL_DIR=/opt/redpitaya -DBUILD_UNIT_TESTS=ON
cmake --build build -j4      # builds, then runs ctest
```

`tests/ptcc_emulator.py` serves a pty and builds its answers with the upstream
library, so the fixture and the driver never share a private encoder: an
encoding regression shows up as a decode failure instead of cancelling out. It
can also answer with a broken CRC, stay silent, or present a NOMEM/LAB_M
module, which is how the timeout, rejected-frame and unsupported-command paths
are covered without hardware.

`ptcc_control` is the command line front end used to verify the link without
the web application. It exercises every function of the public API:

```
-d <dev>      device node, otherwise all ttyACM and ttyUSB nodes are probed
-b <baud>     line rate, 57600 by default
-T <ms>       minimum interval between commands, 550 by default
-O <ms>       response timeout, 2000 by default
-l            list candidate serial ports
-i            controller and module identification
-e            link state and rejected frame counter
-m            read the monitor container once
-q            print the detector temperature only
-w[=ms]       watch through the background poller
-p[=Register] dump a parameter register: Set, Default, Min, Max
-t[=K]        get or set the temperature setpoint
-x[=A]        get or set the TEC current limit
-c[=State]    cooler control: Off, On, Auto
-f[=State]    fan control: Off, On, Auto
-s            print the status code only
-j            JSON output
-v            library version and protocol revision

--supply-mode[=State] module supply control
--supply-plus[=V]     positive rail, 3 to 15 V
--supply-minus[=V]    negative rail, -15 to -3 V
--pwm[=value]         TEC PWM, 0 to 65535
--labm-monitor        detection module readings
--labm-params[=Reg]   detection module register: Set, Default, Min, Max
--gains               the gains the device accepts
--bias-voltage[=V]    detector bias voltage
--bias-current[=A]    detector bias current compensation
--offset[=V]          output DC offset
--gain[=V/V]          preamplifier gain
--varactor[=code]     preamp 1st stage frequency compensation
--transimpedance[=Low|High]
--coupling[=AC|DC]
--bandwidth[=Low|Mid|High]
```

These print the current value when given without one. The supply options are
sent as one message, so the fields left out keep what the module already holds:

```
ptcc_control --labm-params
ptcc_control --gain=10 --coupling=DC
ptcc_control --supply-mode=On --supply-plus=12 --supply-minus=-12
```

## Service mode

`ptcc_control --service[=port]` keeps the controller open and exposes it over a
websocket built on `librp-websocket`, the same one the other Red Pitaya tools
use. The client is the **application**, not the browser: `main.cpp` connects
with `rp_websocket::CWEBClient` and republishes what it needs as ordinary
application parameters, so the page never talks to this service directly.

The controller lives in its own process because the driver embeds CPython,
while an application `.so` is loaded and unloaded by the nginx worker with
`dlopen`/`dlclose`, which an embedded interpreter does not survive. It also
keeps the 0.55 s command throttle out of the request path and survives a
restart of the application.

```
ptcc_control --service            # port 50001, monitor polled once per second
ptcc_control --service=50001 --period=600 -d /dev/ttyUSB0
```

Messages are the `{"KEY": {"type": ..., "value": ...}}` objects `librp-websocket`
encodes, batched: everything that changed in one round goes out as a single
message, and a full snapshot is one message of about eighty-five keys. The
service publishes a value only when it changes, so a client that has just
connected sends `PTCC_REFRESH` and gets the whole state back from memory,
answered on the websocket thread in a few milliseconds rather than behind a
device read.

```cpp
auto ptcc = std::make_shared<rp_websocket::CWEBClient>();
ptcc->receiveDouble.connect([](auto key, auto value) { /* PTCC_T_DET, ... */ });
ptcc->connected.connect([&]() { ptcc->send("PTCC_REFRESH", 1); });
ptcc->start("127.0.0.1", PTCC_SERVICE_PORT);
...
ptcc->send("PTCC_SET_SETPOINT", 230.5F);
```

`PTCC_UPTIME` arrives every poll period. It is the sign of a live service, and
it is the only traffic when nothing else changes.

Published by the service:

| Group | Keys |
| --- | --- |
| Link | `PTCC_CONNECTED`, `PTCC_PORT`, `PTCC_PERIOD`, `PTCC_UPTIME`, `PTCC_VERSION`, `PTCC_PROTOCOL`, `PTCC_LAST_ERROR`, `PTCC_LAST_ERROR_CODE` |
| Identification | `PTCC_MODULE_TYPE`, `PTCC_LABM_PRESENT`, `PTCC_DEV_*`, `PTCC_MOD_*`, `PTCC_DET_*`, `PTCC_COOL_TIME` |
| Controller monitor | `PTCC_T_DET`, `PTCC_T_INT`, `PTCC_I_TEC`, `PTCC_U_TEC`, `PTCC_I_SUP_P/N`, `PTCC_U_SUP_P/N`, `PTCC_I_FAN`, `PTCC_TH_RES`, `PTCC_PWM`, `PTCC_SUPPLY_ON`, `PTCC_FAN_ON`, `PTCC_STATUS`, `PTCC_STATUS_TEXT`, `PTCC_STATUS_IS_ERROR` |
| Controller settings | `PTCC_SETPOINT`, `PTCC_I_TEC_MAX`, `PTCC_SUP_U_P/N`, `PTCC_PWM_SET`, `PTCC_SUP_CTRL`, `PTCC_FAN_CTRL`, `PTCC_TEC_CTRL` |
| Ranges | `PTCC_SETPOINT_MIN/MAX`, `PTCC_I_TEC_MAX_MIN/MAX`, `PTCC_SUP_U_P_MIN/MAX`, `PTCC_SUP_U_N_MIN/MAX`, `PTCC_PWM_MIN/MAX` |
| Detection module | `PTCC_LABM_BIAS_U`, `PTCC_LABM_BIAS_I`, `PTCC_LABM_OFFSET`, `PTCC_LABM_GAIN`, `PTCC_LABM_GAIN_CODE`, `PTCC_LABM_GAIN_KNOWN`, `PTCC_LABM_GAINS`, `PTCC_LABM_VARACTOR`, `PTCC_LABM_TRANS`, `PTCC_LABM_COUPLING`, `PTCC_LABM_BW`, plus `_MIN`/`_MAX` for the analog ones |
| Detection module monitor | `PTCC_LABM_U_SUP_P/N`, `PTCC_LABM_U_FAN`, `PTCC_LABM_I_TEC_P/N`, `PTCC_LABM_TH1/TH2`, `PTCC_LABM_U_DET`, `PTCC_LABM_U_1ST`, `PTCC_LABM_U_OUT`, `PTCC_LABM_TEMP` |

Accepted from the page:

| Key | Type | Meaning |
| --- | --- | --- |
| `PTCC_SET_SETPOINT` | double | Detector temperature [K] |
| `PTCC_SET_I_TEC_MAX` | double | TEC current limit [A] |
| `PTCC_SET_COOLER`, `PTCC_SET_FAN`, `PTCC_SET_SUP_CTRL` | int | 0 Auto, 1 Off, 2 On |
| `PTCC_SET_SUP_U_P`, `PTCC_SET_SUP_U_N` | double | Supply rails [V]; the other fields of the same protocol message are filled in from the module |
| `PTCC_SET_PWM` | int | TEC PWM |
| `PTCC_SET_LABM_BIAS_U`, `_BIAS_I`, `_OFFSET`, `_GAIN` | double | Detection module analog settings; gain must be one of `PTCC_LABM_GAINS` |
| `PTCC_SET_LABM_GAIN_CODE`, `_VARACTOR` | int | Raw device codes |
| `PTCC_SET_LABM_TRANS`, `_COUPLING`, `_BW` | int | Index into Low/High, AC/DC, Low/Mid/High |
| `PTCC_REFRESH` | int | Resend the whole state from memory |
| `PTCC_RELOAD` | int | Re-read every register from the device, which takes seconds |
| `PTCC_RECONNECT` | int | Reopen the serial port |
| `PTCC_STOP` | int | Stop the service |

Every write is answered with `PTCC_LAST_ERROR_CODE` (an `rp_ptcc_error`) and
`PTCC_LAST_ERROR`, then with the values that actually changed, read back from
the device. That answer takes about 1.2 s: the write and the read back each wait
out the throttle. Writes go to the module EEPROM, so the page has to send them
on a confirmed user action and never on a slider drag.

The poll period is raised to `RP_PTCC_THROTTLE_MS` per monitor container plus
400 ms if the requested one is shorter, which is 1.5 s on a LAB_M module: two
containers at 550 ms each leave nothing for the panel's own commands otherwise.
The whole state is read once before the poller starts, so the first snapshot is
complete.

The service is not reachable from a browser and needs no nginx entry: it is
addressed on the loopback by the application.

The service keeps going after the controller disappears: the link state is
published as `PTCC_CONNECTED` and the port is reopened every five seconds until
it comes back, which is what happens when the USB cable is pulled.
