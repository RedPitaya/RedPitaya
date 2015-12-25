# Release 0.94 RC22

System changes:
- fixed slow analog output API
- fixed generator code in many old applications, so they can use the latest
  FPGA bitstream (0.94), this should reduce issues with applications, which
  do not load their own bitstream (Test, SCPI)

Application changes:
- system monitor was added under (Settings -> SYS INFO) in order to monitor
  network performances, refresh rate and RP CPU, RAM status
- refresh rate is auto-adjusted to network performance and user is also
  informed if network performance is too low and can affect user experience
- transferred data is now compressed

**Known issues** (at least major ones) are same as in 0.94 RC12.
+ generator burst mode is buggy
+ SPI interface is not working (seems to be a kernel configuration issue)

# Release 0.94 RC21

Changes:
- fixed copy-paste error in GPIO API (https://github.com/jjmz)
- added fallback to static IP if DHCP fails (Pavel Demin)
- a fix for IMM trigger for generator
- removing some backup files
- added support for LCR meter addon board to lcr executable
- come calibration related changes to scopegenpro UI

**Known issues** (at least major ones) are same as in 0.94 RC12.
+ generator burst mode is buggy
+ SPI interface is not working (seems to be a kernel configuration issue)

# Release 0.94 RC19

Changes:
- fixed licensing related regression

**Known issues** (at least major ones) are same as in 0.94 RC12.
+ generator burst mode is buggy
+ SPI interface is not working (seems to be a kernel configuration issue)

# Release 0.94 RC18

Changes:
- fix for WiFi driver switching (Pavel Demin)
- fixed command prompt issues (Pavel Demin)
- some SCPI fixes (arbitrary generator sequence support, ...)
- some cleanup of SCPI and API examples
- some building documentation updates

**Known issues** (at least major ones) are same as in 0.94 RC12.
+ generator burst mode is buggy
+ SPI interface is not working (seems to be a kernel configuration issue)

# Release 0.94 RC17

Changes:
- fixed missing configuration file for WiFi access point mode (Pavel Demin)
- cleanup of analog output PWM and digital IO API code, it is now reduced to a single layer
- API header file was moved from other API sources, to avoid header file name conflicts

**Known issues** (at least major ones) are same as in 0.94 RC12.
+ SPI interface is not working (seems to be a kernel configuration issue)

# Release 0.94 RC16

Changes:
- move to a newer version of SCPI parser (between v2.1 and Git master) some patches were ported some removed
- fixed a SCPI acquisition issue, where buffer was overwritten by incoming data (involuntary continuous acquisition mode)
- removed continuous calibration related I2C EEPROM reads while running scopegenpro, this improved stability
- removed rebug option from application builds, thus improving performance
- updated build process (Makefile) mostly for the public release
- some cleanup of Examples

**Known issues** (at least major ones) are same as in 0.94 RC12.
+ SPI interface is not working (seems to be a kernel configuration issue)

# Release 0.94 RC15

Changes:
- on extension board analog input divider was removed, so in libwyliodrin analog input range was changed from 10V to 7V

**Known issues** (at least major ones) are same as in 0.94 RC12.

# Release 0.94 RC14

Changes:
- removed superfluous debug messages in Nginx debug log
- updated visual programing WEB URL

**Known issues** (at least major ones) are same as in 0.94 RC12.

# Release 0.94 RC13

Changes:
- spectrum analyzer was very slow on slow SD cards, a tmpfs /tmp/ram was created for waterfall JPEG images
- removed XADC code from monitor, added Examples/xadc/xadc.sh covering the same functionality (currently missing proper voltage scaling)

**Known issues** (at least major ones) are same as in 0.94 RC12.

# Release 0.94 RC12

RC12 is the first public release in the series.

## Changes

### FPGA
* reorganized `fpga` directory structure
* removed Vivado project, instead added non project mode TCL scripts
* added Vivado TCL scripts for creating a project (used for migrating to newer tool versions)
* updated Vivado to newest version 2015.2
* fixed an ASG issue which could result in a slightly wrong frequency and slightly distorted signals
* restructured `rtl/red_pitaya_analog.v` to move IO and PLL related code (device specific code) to the top module
* connected XADC to AXI using the Xilinx provided Wizard, which allows us to use the IIO Linux driver with it
* LED[0] is now a normal software controllable output
* test benches were updated, so they compile now, and some were also run and used, there were no significant changes to functionality

### Device tree
* updated to Xilinx version 2015.1
* changed patch structure, to take advantage of device tree language features
* added nodes for XADC
* added nodes for `LED8` and `LED9`

### U-Boot
* updated to Xilinx version 2015.2
* patched so it executes additional code (from script files) after I2C EEPROM configuration is executed and before running the Linux kernel (porting done by *Pavel Demin*)
* scripts were added to run either Buildroot from initramfs or Debian from EXT4

### Linux kernel
* updated to Xilinx version 2015.1 (porting done by *Pavel Demin*)
* updated Wireless driver, copied from Raspberry PI

### OS
* in addition to Buildroot, Debian Jessie OS is now supported (porting done by *Pavel Demin*)
* discovery was rewritten from lengthy C code into a short BASH script

### Toolchain
* for bare metal applications the toolchain from Vivado 2015.2 SDK is used
* for Linux user space Linaro 2014.11 is used
* ARMHF ABI is used for better compatibility with modern Linux distributions, breaking compatibility with older ARMEL applications

### Web applications
* existing free applications were recompiled for ARMHF, and are still also compatible with ARMEL for older ecosystems
* new Oscilloscope+Generator and Spectrum analyzer were added
* fixed support for applications loading their own FPGA bitstream images

### Other applications
* Wyliodrin was ported to Debian on Red Pitaya

### APIs
* the prefered API for XADC is now the Linux IIO driver
* the prefered API for LED[9:8] is now the Linux LED driver
* removed some dependencies to `libredpitaya`, will probably be deprecated, since most functionality is present in `librp`

## Known issues

### FPGA
* possible timing issues
* poorly written test benches

### U-Boot
* there is no simple method to remove 3 second boot wait

### Linux kernel, device tree
* Linux kernel could not be updated to the latest Xilinx version 2015.2.01, due to changes to the Ethernet and USB subsystems, which caused issues

### OS
* wireless is broken on Buildroot, and will probably only be futher maintained on Debian/Ubuntu
* The scrip for detecting the WiFi driver and reconfiguring the AP config file is not able to write the file, so only one chip is supported without manually modifying `hostapd.conf`

### Other applications
* SCPI server is poorly tested and the documentation is old



# Release 0.93 (30 April 2015)

## Changes

### FPGA
* added burst mode to ASG (arbitrary signal generator)
* added DMA for writing oscilloscope streams directly into the main memory (512MB)
* removed all (most) references to the unsupported tool Ahead (ISE), from now on only Vivado should be used
* partial RTL whitespace cleanup
* CDC (clock domain crossing) moved from `bus_clk_bridge.v` into PS (processing system)
* registered read data bus for all slaves on the custom system bus
* removed unconnected SPI interface from PS
* removed bench support from Vivado project

### Interfaces UART/I2C/SPI
- removed unconnected SPI interface
- added a dummy SPI device to the device tree, so `/dev/spidev1.0` is now available
- added interface usage examples to `Examples/Communication/C/`

## Known issues

### FPGA
- the test bench is broken, mostly because the removal of CDC from modules
- the plan is to use SystemVerilog in the test environment, this is not supported by the Vivado simulator



# Release 0.92 (29 May 2014)

Release 0.92 is a bug-fix release, not introducing any new features. The bug-fixes resolve mainly the following issues:

 1. Several AUTO button related issues.
 2. Several zoom related issues.
 3. Several Measure panel improvements (refresh rate, readability, accuracy, relevance).
 4. Manual range controls.
 5. Artifacts in signal traces at long time ranges (> 1 s).
 6. Long delays at incremental acquisitions and long time ranges (> 1 s).
 7. Trigger level visibility in plot area based on trigger mode & source.
 8. Trigger level dependency on gain settings.
 9. Arbitrary waveform generator file upload & immediate new waveform activation.
10. Signal generation on EXT trigger.
11. Acquire uses correct FPGA filter coefficients.
12. Problems with double GET/POST requests sent from browser for single click.
13. Several general SW stability issues, crashing applications at certain conditions.
14. U-Boot reports correct amount of memory.
15. Ecosystem fixes:
    - Linux OS built completely from sources, including the ramdisk.
    - Using buildroot for the ramdisk, making it possible to add (almost) any SW.
    - Using nginx dependency libraries from buildroot.
    - Versions embedded to all major Red Pitaya binaries & scripts for traceability.
    - WiFi (wlan0) and wired (eth0) routing conflict when WiFi is used.
16. Cosmetics: improved logging, unique app icons...
17. Cleanup of obsolete code.


# Release 0.90 (27 February 2014)

Initial release.
