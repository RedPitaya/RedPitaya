************
Register map 
************

Red Pitaya HDL design has multiple functions, which are configured by registers. It also uses memory locations to store capture data and generate output signals. All of this are described in this document. Memory location is written in a way that is seen by SW. 

The table describes address space partitioning implemented on FPGA via AXI GP0 interface. All registers have offsets aligned to 4 bytes and are 32-bit wide. Granularity is 32-bit, meaning that minimum transfer size is 4 bytes. The organization is little-endian.
The memory block is divided into 8 parts. Each part is occupied by individual IP core. Address space of individual application is described in the subsection below. The size of each IP core address space is 4MByte. 
For additional information and better understanding check other documents (schematics, specifications...).

.. tabularcolumns:: |p{15mm}|p{22mm}|p{22mm}|p{55mm}|

+--------+-------------+------------+----------------------------------+
|        |    Start    | End        | Module Name                      |
+========+=============+============+==================================+
| CS[0]  | 0x40000000  | 0x400FFFFF | Housekeeping                     |
+--------+-------------+------------+----------------------------------+
| CS[1]  | 0x40100000  | 0x401FFFFF | Oscilloscope                     |
+--------+-------------+------------+----------------------------------+
| CS[2]  | 0x40200000  | 0x402FFFFF | Arbitrary signal generator (ASG) |
+--------+-------------+------------+----------------------------------+
| CS[3]  | 0x40300000  | 0x403FFFFF | PID controller                   |
+--------+-------------+------------+----------------------------------+
| CS[4]  | 0x40400000  | 0x404FFFFF | Analog mixed signals (AMS)       |
+--------+-------------+------------+----------------------------------+
| CS[5]  | 0x40500000  | 0x405FFFFF | Daisy chain                      |
+--------+-------------+------------+----------------------------------+
| CS[6]  | 0x40600000  | 0x406FFFFF | FREE                             |
+--------+-------------+------------+----------------------------------+
| CS[7]  | 0x40700000  | 0x407FFFFF | Power test                       |
+--------+-------------+------------+----------------------------------+

==================
Red Pitaya Modules
==================

Here are described submodules used in Red Pitaya FPGA logic.

.. tabs::

   .. tab:: 12X-XX

      .. include:: regset.rst

   .. tab:: 250-12

      .. include:: regset250_12.rst

