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
| Bazaar       | Nginx server with dependencies, Red Pitaya Bazaar module & application controller module loader.
| fpga         | FPGA design for the inital set of Red Pitaya applications.
| OS/buildroot | GNU/Linux operating system components
| patches      | Directory containing red pitaya patches
| scpi-server  | Scpi server directory, containing red pitaya core scpi server
| Test         | Command line utilities (acquire, generate, ...).
| shared       | libredpitaya.so API source code

## BUILD PROCESS ##

### Ecosystem structure ###
- Fpga + Devicetree
- api
- Apps-free
- scpi-server
- Linux kernel
- Debian

#### Installation requirements ####

You will need the following to build the RedPitaya components:

1. Xilinx FPGA development tools: ISE 14.6 and Vivado 2015.2 are now officially supported. You mileage may vary with differente versions of the tools.
2. Linaro cross compiler. Both the toolchain from Xilinx Vivado SDK and the Linaro toolchain are needed.
It can be downloaded [here](https://releases.linaro.org/14.11/components/toolchain/binaries/arm-linux-gnueabihf/gcc-linaro-4.9-2014.11-x86_)
3. GNU make autoconf, automake, ...

#### Installing required tools ####
If you are having trouble installing the required tools, take a look at [Red Pitaya OS](http://wiki.redpitaya.com/index.php?title=Red_Pitaya_OS) wiki page, for more information.

### Fpga and Device tree ###
- Both fpga and devicetree README can be found [here](fpga/README.md)

### Api ###

Export PATH to include your local linaro bin directory and export CROSS_COMPILE. Lets assume, linaro is installed in /opt directory.
```bash
export PATH=$PATH:/opt/linaro/bin
export CROSS_COMPILE=arm-linux-gnueabihf-
```

Navigate to the ./api/rpbase folder and run:
```bash
make
```

The output of this process is the Red Pitaya librp.so library in ./api/lib directory.

### Apps-free ###

To build apps free, follow the instructions given at apps-free [README.md](apps-free/README.md) file.

### Scpi-server ###
Scpi server README can be found [here](scpi-server/README.md)

### Linux Kernel ###
[Linux kernel]

### U-boot ###
[U-boot]

### Debian ###
Debian README can be found [here](OS/debian/README.md)
