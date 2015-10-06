FPGA:
- replace undocumented system bus with AXI4 Lite
- move CPU accessible registers out from processing modules
- separate the code into smaller modules connected over AXI4-Stream
- rethink generator and osciloscope SW interface sequences
- sestructure registers into a hierarchy
- generalize trigger modes
- use DMA from Xilinx
- write configurable PLL
- write streaming benches

Scope:
- continuous mode
- trigger mode

Linux:
- upgrade to 3.19, there are Ethernet and USB driver issues
- write IIO drivers

Bazaar:
- review the Nginx current patch, try to remove it and instead update the config file
- update Nginx to newer version
- update Buildroot to newer version

API:
- move type definitins (structures, constants) to header files
- remove middle API layer
- avoid using read modify write access to registers

Applications:
- remove library sources from GIT, use sources from Buildroot instead
- replace kiss FFT with NE10 library
- replace libjpeg with turbo-jpeg from Buildroot

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

