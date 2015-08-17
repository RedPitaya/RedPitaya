# Directory structure

|  path           | contents
|-----------------|----------------------------------------------------------------
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

Run the following to generate a bit file and reports:
```bash
make
```

Run the following to generate and open a project file using Vivado GUI:
```bash
make project
```

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

