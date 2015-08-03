Directory structure:
hsi/ - "Hardware Software Interface" contains FSBL (First Stage Boot Loader) and DTS (Design Tree) builds
rtl/ - Verilog (SystemVerilog) "Register-Transfer Level"
sdc/ - "Synopsys Design Constraints" contains Xilinx design constraints
sim/ - simulation scripts
syn/ - synthesys using Vivado
tbn/ - Verilog (SystemVerilog) "test bench"

# Signal mapping

## XADC input mapping

|E2 con | schematic | ZYNQ p/n | XADC in | IIO filename     |
|-------|-----------|----------|---------|------------------|
|AI0    | AIFP0     | B19/A20  | AD8     | in_voltage11_raw |
|AI1    | AIFP1     | C20/B20  | AD0     | in_voltage9_raw  |
|AI2    | AIFP2     | E17/D18  | AD1     | in_voltage10_raw |
|AI3    | AIFP3     | E18/E19  | AD9     | in_voltage12_raw |
