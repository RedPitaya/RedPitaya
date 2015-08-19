# RED PITAYA ECOSYSTEM AND APPLICATIONS 

Here you will find the sources of various software components of the
Red Pitaya system. The components are mainly contained in dedicated
directories, however, due to the nature of the Xilinx SoC "All 
Programmable" paradigm and the way several components are interrelated,
some components might be spread across many directories or found at
different places one would expect.


| directories  | contents
|--------------|----------------------------------------------------------------
| api          | librp.so API source code
| Applications | Red Pitaya applications (controller modules & GUI clients).
| apps-free    | Red Pitaya application for the old environment (also with controler modules & GUI clients).
| Bazaar       | Nginx server with dependencies, Red Pitaya Bazaar module &
|              | application controller module loader.
| fpga         | FPGA design for the inital set of Red Pitaya applications.
| OS           | GNU/Linux operating system components including:
|              | - Linux kernel config & patches
|              | - U-Boot config & patches
|              | - Staged ramdisk
|              | - Red pitaya IP discovery client
| Test         | Command line utilities (acquire, generate, ...).
| shared       | libredpitaya.so API source code

BUILD PROCESS
-------------

Ecosystem structure
-------------------
- Fpga + Devicetree
- api
- Apps-free
- Linux kernel
- Debian

Installation requirements
-------------------------

You will need the following to build the RedPitaya components:

1. Xilinx FPGA development tools: ISE 14.6 and Vivado 2015.2 are now officially supported. You mileage may vary with differente versions of the tools.
2. Linaro cross compiler. Both the toolchain from Xilinx Vivado SDK and the Linaro toolchain are needed.
It can be downloaded [here](https://releases.linaro.org/14.11/components/toolchain/binaries/arm-linux-gnueabihf/gcc-linaro-4.9-2014.11-x86_)
3. GNU make autoconf, automake, ...

Fpga and Device tree
--------------------
- [fpga](fpga/README.md)
- [devicetree]

api
---
Source folder: ./api/rpbase
Navigate to the source folder and run
```bash
make
```
Give the fact, that you followed the instructions given at Installation requirements, you will be successfully be able to build the Red
Pitaya library.
