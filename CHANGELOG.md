# Beta version (1.04-8)

- Freq. calibration feature was added to the calibration utility that enables users to improve and flatten freq. response of 125-xx devices
- CAN decoding was added to Logic analyzer app
- STEMlab 125-10 now supports use of Logic analyzer extension module 
- Logic analyzer app now stores all settings
- Burst generator bug at higher frequencies was fixed
- Ripple on slow analog outputs is fixed
- LA trigger issue fixed
- Jupyter calibration and overflow bug fixed
- Other general improvements and updates including

# Stable version (1.04-7)

- The streaming application was improved. One of the main features is that users can now exactly know when and which samples are lost due to throughput problems or limitation
- Watchdog was implemented for Linux and WEB applications. In case of fatal error or lost connection device will always return to its previous operation state. 
- Open source FPGA code can now be compiled with Vivado 2020.1
- New OS available notification was implemented into WEB interface
other general improvements and updates
- Added application for calibrating RP

# Stable version (1.03-701)
General improvements:
- Access point working with official WIFI dongle
- Client WIFI mode fixed
- Trigger issues fixed
- Broken acquired signal issues fixed
- Jupyter slow analog inputs bug fixed
- Improved UI of the oscilloscope and spectrum analyzer
- Improved LCR application
- LA data zooming bug fix
- Shutdown button added to WEB interface
- Added console server for streaming application
- Multiple bug fixes in streaming applications
- Added functionality limiting the number of samples in streaming application
- Improved console applications. Added calibration support.
- Other stability improvements & bug fixes.


# Beta version (0.99-41)

Streaming:
- Added speed limit depending on the choice of data transfer mode

# Stable version (0.98-696)

SCPI server:
- Fixed bug with choosing the type of generated signal

# Beta version (0.98-694)

Bode analyser:
- The periods number was added: minimum periods number of the generated signal for each step

Spectrum analyzer:
- The CSV export was fixed (min/max values).

Oscilloscope:
- Some trigger bugs at the low frequency were fixed

Network manager:
- The link-local IP displays when no other IP's are available

# Beta version (0.98-693)

Spectrum analyzer:
- Signal generator is now available also in Spectrum analyzer app

Logic analyzer, Bode analyser:
- Logic analyzer & Bode analyzer data can be exported also in raw format

Oscilloscope:
- Oscilloscope app settings are restored at startup

Applications:
- Several bug fixes & stability improvements

# Beta version (0.98-685)

VNA:
- added Vector network analyser application to the ecosystem image
- now it's possible to download windows tool for VNA application here:
- VNA windows tool and VNA python script support bonjour rp-XXXXXX.local addresses

Bode analyser:
- Improved phase and amplitude calculations

OS
- Changed links to external resources

# Stable version (0.98-615)

FPGA (mercury):
- fixed bugs causing unreliable reads from oscilloscope, logic analyzer buffer
- moved burst mode finite/infinite control into period repetitions register
- restructured arbitrary signal generator, so periodic and burst mode are separate modules
- added digital loopback between generator and oscilloscope (this is for test purposes)
- added bit controlling if trigger will restart a running burst or not
- trigger holdoff was removed from oscilloscope, instead it should be placed into
  a common trigger module accepting various trigger inputs and handling timing,
  while oscilloscope and logic analyzer triggers only handle values,
  at least this is the idea

OS:
- updated to Ubuntu 16.04.3

Jupyter:
- using `ctypes` array for generator/oscilloscope data buffer

Bazaar:
- fixed some issues with network manager and update manager

# Release 0.97-RC7

Linux kernel:
- added support for BCM43143 USB Wi-Fi chipset
- updated 8192cu driver to switch from 'wext' to 'cfg80211'
  https://github.com/raspberrypi/linux/pull/1489

FPGA:
- CPU and SPI clock updates
- using old DNA regset in `logic` FPGA project to avoid Bazaar issues
FPGA (mercury):
- recoded burst mode for asg/generator, Python API changes were also needed

OS:
- added some TFT SPI display support code
- installed `console-setup` to fix some keyboard/console related systemd issues
- removed discovery
- removed wireless extensions related code
- updated `libiio` to version `0.10`

Jupyter:
- updated for latest mercury FPGA code
- recoded regset in `ctypes`
- reorginized part of the code into smaller reusable blocks
- updated calibration (it is not finished yet and not enabled by default)
- added LG/LA driver and examples

# Release 0.97-RC6


# Release 0.97

U-Boot:
- updated to upstream xilinx-v2016.4

Linux kernel:
- added support for I2C GPIO expanders (PCA953X, MCP23S08)
- added GPIO based drivers for SPI, I2C and 1-wire
- enabled reset controller support
- patched Cadence I2C controller driver to add proper HW reset
- changed kernel patching procedure, the kernel is now forked,
  and patches are commited to the fork Git repo
- updated to upstream xilinx-v2016.2
- added TFT framebuffer and touch screen drivers
- added GPIO expander and HW monitor device drivers for HAMLAB

FPGA:
- updated build procedure to create bitstreams from multiple projects
- moved device tree code to the FPGA directory
- instead of patching just append or include `*.dtsi` files
- adddd UIO device which covers the entire FPGA address space
- added UDEV code for renaming UIO devices
- added UDEV code for changing group access permissions
  xdevcfg,uio,led,gpio,spi,i2c,dialout,dma
- added the Mercury project, will be used to implement new features,
  initially only supported in Jupyter
- the `axi4lite` project is used to test AXI4 bus implementations,
  for now here is a basic AXI4-Lite slave (GPIO) and
  an integrated logic analyzer for observing bus performance and bugs

OS:
- Update to Ubuntu 16.04.2
- OS build script cleanup, mostly to organize the code into topics
- partially removed IPv6 support, it was causing issues with zeroconf
- installed more libraries (libiio v0.9, ...)

API:
- changed API to map /dev/uio/
- added cleaner API1 just used by Jupyter apps now, it is far from stable

Documentation:
- recoded most documentation into reStructuredText, so it can be published on
  http://redpitaya.readthedocs.io/en/latest/

Wyliodrin:
- removed

Applications:
- added bode analyzer

Jupyter:
- added Jupyter for interactive Python notebooks
- added a few basic exampleds

# Release 0.96 RC?

Application changes:
- partial cleanup of copied JS/CSS code and images

# Release 0.96 RC1

System Changes:
- recoded Bazaar related Makefiles, instead of cross compilation, a native
  compiler root system is used, running inside an ARM QEMU virtualized
  environment, this avoids library version issues, but emulation is slow
- top Makefiles are now split into a system Makefile.x86, which requires x86
  to run tools like Vivado and a user space Makefile used to compile
  applications (Bazaar, web apps, SCPI, API, ...)
- updated Nginx from version 1.5.3 to the latest 1.10.0
- removed NGINX patches
- updated Lua Nginx from version v0.8.7 to the latest v0.10.2
- updated Websocket++ from version 0.5.0 to a slightly newer 0.7.0,
  and provided a patch for an issue triggered by kernel update to 4.4
- removed Buildroot, lately it was only used to provide libraries at compile
  time, now Debian libraries are used directly
- removing SDK, since it was not maintained, now it is possible to compile all
  user space code directly on Red Pitaya
- removed some deprecated/duplicated source code
- Vivado/U-Boot/Linux were updated to xilins-v2016.2
- the Linaro compiler is no longer needed, ARMHF compiler is now part of Vivado SDK

OS changes:
- the OS was switched from Debian Jessie to Ubintu 16.04,
  the main reason for the switch is access to Launchpad PPA
- OS image building scripts are now a bit cleaner with separation into
  generic Ubuntu settings, network and Red Pitaya specific settings

Network:
- the network setup was entirely recoded using systemd specific tools
- zeroconf support was added
- the network setup is now properly documented in NETWORK.md

FPGA2:
- `fpga2` project was added, which is used by `api2` and the LA application
- the project is rewritten in SystemVerilog mostly to make the code shorter
- instead of storing samples in a short buffer, Xilinx DMA is used to copy data into the main memory
- more thought is (should be) given to modules with Linux kernel drivers available

API2:
- UIO is used to divide the memory space into separate segments each controlled by a separate driver
- device tree overlays are used to handle loading various FPGA images, this is not yet ready for public use

Application changes:
- Makefile changes so applications can be compiled directly on Red Pitaya
- removing 'libjpeg' sources from spectrum analyzer applications and instead
  using the 'libjpeg-turbo' Debian system library

Network manager
/SCPI/Wyliodrin managers

# Release 0.95

New applications:
- LCR meter application - accurately measures capacitors, inductors and resistors using the test frequencies of 100Hz, 1 kHz, 10kHz and 100kHz.

Application improvements:
-SCPI server:
  - can now be started directly from WEB browser via Remote control app
- Oscilloscope:
  - navigation was improved (time offset can now be set by mouse dragging, t/div can be changed by scrolling)
  - user is now able to retrieve signal acquired into ADC buffer even if acquisition is stopped
  - stability of data transfer was improved together with network performance indication
  - gain setting is now remembered when quitting app
  - trigger status is now properly updated at high frequencies
  - we fixed few normal trigger issues
- Spectrum analyzer:
  - acquire algorithm fix that fixes spectrum issues

General system improvements:
- discovery service & market access problems are now solved
- user is now able to upgrade Red Pitaya OS directly from the WEB browser
  rewriting complete SD card image is now only necesary in case of Debian/Ubuntu changes (usually systemd and network related)
- Red Pitaya desktop was improved so it is more simple to use and also provides better support for tablets and smartphones
- browser detection was added that suggest users to install or upgrade to recommended version
- online/offline detection was added so that user knows if Red Pitaya can access the internet / if it is possible use some features that requires internet connection
- switching between contributed and official applications doesn’t causes problems anymore
- system crashes reports with debug information can now be sent to Red Pitaya team on user request
- Feedback app - enables user to send feedback our support with attached information about Red Pitaya OS, computer OS & browser
- Analytics is sent to Red Pitaya team on user request in order to track system events that can help improve user experience

Other:
- access to wiki page was added, but the main news is that we also updated wiki documentation

# Release 0.94 RC24

Changes:
- libstdc++.so was removed from /opt/redpitaya/lib, this was made possible
  by fixing C++ application Makefile-s, some were using g++ while also linking
  stdc++, which caused conflicts, linking stdc++ was removed
- small updates to free applications

**Known issues** (at least major ones) are same as in 0.94 RC23.

# Release 0.94 RC23

Changes:
- migration to Vivado 2015.4 and related U-Boot, Linux kernel and devicetree versions
- changes to ARM clocking so the CPU runs at 666MHz instead of 500MHz (Ulrich Habel)
- SPI userspace access seems to be fixed, `/dev/spidev1.0` is present, but
  kernel log still does not mention SPI or QSPI drivers

## Known issues

### FPGA
+ possible timing issues
+ poorly written test benches
+ generator burst mode is buggy

### U-Boot
+ there is no simple method to remove 3 second boot wait

### Linux kernel, device tree
+ Warnings are reported for Lantiq Ethernet PHY devicetree

### OS
+ wireless is broken on Buildroot, and will probably only be futher maintained on Debian/Ubuntu

### Other applications
+ SCPI server is poorly tested and the documentation is old

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
