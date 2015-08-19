# Directory structure

|  path           | contents
|-----------------|-------------------------------------------------------------
| `fpga/Makefile` | main Makefile, used to run FPGA related tools
| `fpga/*.tcl`    | TCL scripts to be run inside FPGA tools
| `fpga/archive/` | archive of XZ compressed FPGA bit files
| `fpga/doc/`     | documentation (block diagrams, address space, ...)
| `fpga/ip/`      | third party IP, for now Zynq block diagrams
| `fpga/rtl/`     | Verilog (SystemVerilog) "Register-Transfer Level"
| `fpga/sdc/`     | "Synopsys Design Constraints" contains Xilinx design constraints
| `fpga/sim/`     | simulation scripts
| `fpga/tbn/`     | Verilog (SystemVerilog) "test bench"
|                 |
| `fpga/hsi/`     | "Hardware Software Interface" contains FSBL (First Stage Boot Loader) and DTS (Design Tree) builds

# Build process

Xilinx Vivado 2015.2 (including SDK) is required. If installed at the default location, then the next command will properly configure system variables:
```bash
. /opt/Xilinx/Vivado/2015.2/settings64.sh
```

The default mode for building the FPGA is to run a TCL script inside Vivado. Non project mode is used, to avoid the generation of project files, which are too many and difficult to handle. This allows us to only place source files and scripts under version control.

The next scripts perform various tasks:
| TCL script                      | action
|---------------------------------|---------------------------------------------
| `red_pitaya_hsi_dram_test.tcl`  | should create the `zynq_dram_test` but the produced binary can not be run from a SD card
| `red_pitaya_hsi_dts.tcl`        | creates device tree sources
| `red_pitaya_hsi_fsbl.tcl`       | creates FSBL executable binary
| `red_pitaya_vivado_project.tcl` | creates a Vivado project for graphical editing
| `red_pitaya_vivado.tcl`         | creates the bitstream and reports

To generate a bit file, reports, device tree and FSBL, run:
```bash
make
```

To generate and open a Vivado project using GUI, run:
```bash
make project
```

# Device tree

Device tree is used by Linux to describe features and address space of memory mapped hardware attached to the CPU.

Running `make` inside this directory will create a device tree source and some include files:
| device tree file | contents
|------------------|------------------------------------------------------------
| `zynq-7000.dtsi` | description of peripherals inside PS (processing system)
| `pl.dtsi`        | description of AXI attached peripherals inside PL (programmable logic)
| `system.dts`     | description of all peripherals, includes the above `*.dtsi` files

To enable some Linux drivers (Ethernet, XADC, I2C EEPROM, SPI, GPIO and LED) the device tree source is patched using `../patches/devicetree.patch`.

# Signal mapping

## XADC inputs

XADC input data can be accessed through the Linux IIO (Industrial IO) driver interface.

|E2 con | schematic | ZYNQ p/n | XADC in | IIO filename     |
|-------|-----------|----------|---------|------------------|
|AI0    | AIFP0     | B19/A20  | AD8     | in_voltage11_raw |
|AI1    | AIFP1     | C20/B20  | AD0     | in_voltage9_raw  |
|AI2    | AIFP2     | E17/D18  | AD1     | in_voltage10_raw |
|AI3    | AIFP3     | E18/E19  | AD9     | in_voltage12_raw |

## GPIO LEDs

| LED     | color  | SW driver       | dedicated meaning
|---------|--------|-----------------|----------------------------------
| `[7:0]` | yellow | RP API          | user defined
| `  [8]` | yellow | kernel `MIO[0]` | user defined
| `  [9]` | reg    | kernel `MIO[7]` | user defined
| ` [10]` | green  | none            | "Power Good" status
| ` [11]` | blue   | none            | FPGA programming "DONE"

### Linux access to GPIO

This document is used as reference: http://www.wiki.xilinx.com/Linux+GPIO+Driver

The base value of `MIO` GPIOs was determined to be `906`.
```bash
redpitaya> find /sys/class/gpio/ -name gpiochip*
/sys/class/gpio/gpiochip906
```

GPIOs are accessible at base value + MIO index:
```bash
echo 906 > /sys/class/gpio/export
echo 913 > /sys/class/gpio/export
```

### Linux access to LED

This document is used as reference: http://www.wiki.xilinx.com/Linux+GPIO+Driver

By providing GPIO/LED details in the device tree, it is possible to access LEDs using a dedicated kernel interface.
NOTE: only LED 8 and LED 9 support this interface for now.

To show CPU load on LED 9 use:
```bash
echo heartbeat > /sys/class/leds/led9/trigger
```
To switch LED 8 on use:
```bash
echo 1 > /sys/class/leds/led8/brightness
```
