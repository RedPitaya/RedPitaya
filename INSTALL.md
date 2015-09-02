You will need the follwing to build the RedPitaya components:

1. Xilinx FPGA development tools: ISE 14.6 and Vivado 2013.3 are officially supported. Your mileage may vary with different versions of the tools.
2. ARM cross-compiler, both the toolchain from Xilinx Vivado SDK and the Linaro toolchain are needed
   https://releases.linaro.org/14.11/components/toolchain/binaries/arm-linux-gnueabihf/gcc-linaro-4.9-2014.11-x86_64_arm-linux-gnueabihf.tar.xz
3. GNU make, autoconf, automake, ...

To build the OS/Ecosystem, perform the steps described in `settings.sh` (it might need editing for correct tool paths) and run make.
```bash
. settings.sh
make
```

For more details regarding Red Pitaya development, please visit
http://wiki.redpitaya.com.
