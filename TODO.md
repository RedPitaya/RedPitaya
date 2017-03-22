Jupyter:
- terminal should run `bash`
- terminal middle mouse button paste should be fixed,
  it works well in PYNQ, since the latest Jupyter code is used,
  it might be just an upstream regression
- there is this error in the log:
  `404 GET /jupyter/nbextensions/widgets/notebook/js/extension.js`
  so some widgets are not working
- rethink start/trigger/stop synchronization
- each channel should have own UIO DT node and driver,
  so they could be used by separate applications
- UIO devices should be opened exclusively,
  to avoid conflicts between applications
- FPGA image should not be reloaded by default,
  to avoid crashing other applications already using the FPGA
- add device tree overlay and FPGA manager support

FPGA:
- replace undocumented system bus with AXI4 Lite
- move CPU accessible registers out from processing modules
- separate the code into smaller modules connected over AXI4-Stream
- rethink generator and osciloscope SW interface sequences
- sestructure registers into a hierarchy
- generalize trigger modes
- write configurable PLL
- write streaming benches
- Scope:
  - use DMA from Xilinx
  - continuous mode
  - trigger mode

U-Boot:
- Remove source file patch, which enables modifying the boot
  sequence stored in the EEPROM environment.
  A better solution would be to use a script
  to load only parts of the environment.

Linux:
- Write IIO drivers for streaming data from/to ADC/DAC.
- Update to kernel 4.6 (tag xilinx-v2016.3)
  * We already ported the simplest of our patches.
  * The Lantiq PHY driver needed an update, Pavel Did most of the work.
    There is now a patch available for the upstream kernel.
    https://www.spinics.net/lists/netdev/msg379071.html
    https://www.spinics.net/lists/netdev/msg379072.html
    But it does not work. I updated Pavel's code to be closer to
    the the upstream patch and this one works.
    I also asked the author of the patch for help, we will see.
  * The FPGA manager patch will be difficult to apply.
    It requires devicetree overlay changes only upstreamed in 4.8.
    https://lkml.org/lkml/2016/11/1/392
    https://www.spinics.net/lists/devicetree/msg147528.html
    I tried to merge upstream v4.8 into xilinx-v2016.3,
    there are some nontrivial conflicts to solve.
    For now we will wait for Xilinx to port kernel 4.8.

OS:
- Rethink user management and security.
- Create a single UIO device for FPGA v0.94 and handle access permissions
  using UDEV rules

Bazaar:
- replace current JSON libraries with precompiled ones installed
  as Debian packages
- move our code into a dynamic module, and use the system Nginx
- perhaps, recode Lua code into C, so Lua can be removed from Nginx
- make sure our module creates a response to a HTTP request,
  our log is now filled with warnings due to missing responses
- integrate redpitaya.so library, it is not used elsewhere
- use API to get Zynq DNA (there are 2 instances)
- We should use hierarchical inheritance to reduce the ammount
  of configuration lines in nginx.conf.

API:
- move type definitins (structures, constants) to header files
- remove middle API layer
- avoid using read modify write access to registers

Applications:
- replace kiss FFT with NE10 library (already available in Debian)

SCPI:
- migrate to latest upstream
- push our patches upstream


TODO

* Fix bugs.
* Decrease the amount of duplicated code and move common components
  to shared.
* Bring OS & ecosystem sources together as they compose one package.
* Comment the code extensively in Doxygen style. Provide Doxygen
  documentation.
* Simplify Test/monitor.

