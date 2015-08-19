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
=============
- Fpga + Devicetree
- Api
- Apps-free
- Linux kernel
- Debian

Fpga and Device tree
--------------------
- [fpga](fpga/README.md)
- [devicetree]
