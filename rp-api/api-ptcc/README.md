# api-ptcc

C++ driver for the VIGO Photonics PTCC-01 TEC controller, built as
`librp-ptcc.so` / `librp-ptcc.a` with a C API in `include/rp_ptcc.h`.

The controller enumerates as USB CDC-ACM, so it appears as `/dev/ttyACM0` on
Red Pitaya OS with no extra kernel driver.

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
* Value ranges are enforced by the upstream library and by the device, never
  by a copy of their tables in this driver. An out of range setpoint comes back
  as `RP_PTCC_ERANGE` with the upstream message attached
  (`rp_PtccGetLastPythonError()`). `rp_PtccGetLimits()` reads the module's
  USER_MIN / USER_MAX registers so a UI can bound its controls, but it is
  informational only.
* `rp_PtccSetSetpoint()` takes whole Kelvins, matching the upstream signature.
* A field the device did not report is `RP_PTCC_ERESP`, not a zero. Zeros from
  this API are always measurements.
* The cooler only cools. Asking for a setpoint above ambient will not be
  reached and the device reports "detector overheat" after 120 s.
* `LAB_M` modules take the temperature setpoint like any other: the controller
  panel is the same for every module, only the amplifier settings differ.
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
```
