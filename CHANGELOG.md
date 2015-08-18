# Release 0.93

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


# Release 0.94

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

### Other applications
* Wyliodrin was ported to Debian on Red Pitaya

### APIs
* the prefered API for XADC is now the Linux IIO driver
* the prefered API for LED[9:8] is now the Linux LED driver

## Known issues

### FPGA
* possible timing issues
* poorly written test benches

### U-Boot
* there is no simple method to remove 3 second boot wait

### Linux kernel, device tree
* Linux kernel could not be updated to the latest Xilinx version 2015.2.01, due to changes to the Ethernet and USB subsystems, which caused issues

### OS
* Wireless is broken on Buildroot, and will probably only be futher maintained on Debian/Ubuntu

### Other applications
* SCPI server is poorly tested and the documentation is old
