####
FPGA
####

*************
Prerequisites
*************

1. *Libraries used by ModelSim-Altera*

Install libraries:

.. code-block:: shell-session

    # apt-get install libxft2 libxft2:i386 lib32ncurses5

2. *Xilinx Vivado 2016.4 (including SDK)* 

*******************
Directory structure
*******************

There are multiple FPGA projects, some with generic functionality, some with specific functionality for an application.
Common code for all projects is placed directly into the ``fpga`` directory. Common code are mostly reusable modules.
Project specific code is placed inside the ``fpga/prj/name/`` directories and is similarly organized as common code.

.. tabularcolumns:: |p{30mm}|p{120mm}|

+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
|  path             | contents                                                                                                                                                                 |
+===================+==========================================================================================================================================================================+
| ``fpga/Makefile`` | main Makefile, used to run FPGA related tools                                                                                                                            |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/*.tcl``    | TCL scripts to be run inside FPGA tools                                                                                                                                  |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/archive/`` | archive of XZ compressed FPGA bit files                                                                                                                                  |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/doc/``     | documentation (block diagrams, address space, ...)                                                                                                                       |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/brd/``     | board files `Vivado System-Level Design Entry (ug895) <http://www.xilinx.com/support/documentation/sw_manuals/xilinx2016_2/ug895-vivado-system-level-design-entry.pdf>`_ |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/ip/``      | third party IP, for now Zynq block diagrams                                                                                                                              |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/rtl/``     | Verilog (SystemVerilog) *Register-Transfer Level*                                                                                                                        |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/sdc/``     | *Synopsys Design Constraints* contains Xilinx design constraints                                                                                                         |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/sim/``     | simulation scripts                                                                                                                                                       |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/tbn/``     | Verilog (SystemVerilog) *test bench*                                                                                                                                     |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/dts/``     | device tree source include files                                                                                                                                         |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/prj/name`` | project `name` specific code                                                                                                                                             |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| ``fpga/hsi/``     | *Hardware Software Interface* contains                                                                                                                                   |
|                   | FSBL (First Stage Boot Loader) and                                                                                                                                       |
|                   | DTS (Design Tree) builds                                                                                                                                                 |
+-------------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

****************
Building process
****************
    
If Xilinx Vivado is installed at the default location, then the next command will properly configure system variables:

.. code-block:: shell-session

   $ . /opt/Xilinx/Vivado/2016.2/settings64.sh

The default mode for building the FPGA is to run a TCL script inside Vivado.
Non project mode is used, to avoid the generation of project files,
which are too many and difficult to handle.
This allows us to only place source files and scripts under version control.

The next scripts perform various tasks:

.. tabularcolumns:: |p{60mm}|p{60mm}|

+-----------------------------------+------------------------------------------------+
| TCL script                        | action                                         |
+===================================+================================================+
| ``red_pitaya_vivado.tcl``         | creates the bitstream and reports              |
+-----------------------------------+------------------------------------------------+
| ``red_pitaya_vivado_project.tcl`` | creates a Vivado project for graphical editing |
+-----------------------------------+------------------------------------------------+
| ``red_pitaya_hsi_fsbl.tcl``       | creates FSBL executable binary                 |
+-----------------------------------+------------------------------------------------+
| ``red_pitaya_hsi_dts.tcl``        | creates device tree sources                    |
+-----------------------------------+------------------------------------------------+

To generate a bit file, reports, device tree and FSBL, run (replace ``name`` with project name):

.. code-block:: shell-session

   $ make PRJ=name

To generate and open a Vivado project using GUI, run:

.. code-block:: shell-session

   $ make project PRJ=name

**********
Simulation    
**********

Our scripts are configured for Alteras ModelSim, which can be obtained from Alteras site for free.
Scripts expect the default install location.
On Ubuntu the install process fails to create an appropriate path to executable files,
so this path must be created manually:

.. code-block:: shell-session

   $ ln -s $HOME/altera/16.0/modelsim_ase/linux $HOME/altera/16.0/modelsim_ase/linux_rh60

To run simulation, Vivado tools have to be installed.
There is no need to source ``settings.sh``.
For now the path to the ModelSim simulator is hard coded into the simulation ``Makefile``.

.. code-block:: shell-session

   $ cd fpga/sim

Simulations can be run by running ``make`` with the bench file name as target:

.. code-block:: shell-session

   $ make top_tb

Some simulations have a waveform window configuration script like ``top_tb.tcl``
which will prepare an organized waveform window.

.. code-block:: shell-session

   $ make top_tb WAV=1
   
***********
Device tree
***********

Device tree is used by Linux to describe features and address space of memory mapped hardware attached to the CPU.

Running ``make`` inside this directory will create a device tree source and some include files:

+------------------+------------------------------------------------------------------------+
| device tree file | contents                                                               |
+==================+========================================================================+
| `zynq-7000.dtsi` | description of peripherals inside PS (processing system)               |
+------------------+------------------------------------------------------------------------+
| `pl.dtsi`        | description of AXI attached peripherals inside PL (programmable logic) |
+------------------+------------------------------------------------------------------------+
| `system.dts`     | description of all peripherals, includes the above ``*.dtsi`` files    |
+------------------+------------------------------------------------------------------------+

To enable some Linux drivers (Ethernet, XADC, I2C EEPROM, SPI, GPIO and LED) additional configuration files.
Generic device tree files can be found in ``fpga/dts`` while project specific code is in ``fpga/prj/name/dts/``.

**************
Signal mapping
**************

===========
XADC inputs
===========

XADC input data can be accessed through the Linux IIO (Industrial IO) driver interface.

+--------+-----------+----------+---------+------------------+--------------------+-------+
| E2 con | schematic | ZYNQ p/n | XADC in | IIO filename     | measurement target | range |
+========+===========+==========+=========+==================+====================+=======+
| AI0    | AIF[PN]0  | B19/A20  | AD8     | in_voltage11_raw | general purpose    | 7.01V |
+--------+-----------+----------+---------+------------------+--------------------+-------+
| AI1    | AIF[PN]1  | C20/B20  | AD0     | in_voltage9_raw  | general purpose    | 7.01V |
+--------+-----------+----------+---------+------------------+--------------------+-------+
| AI2    | AIF[PN]2  | E17/D18  | AD1     | in_voltage10_raw | general purpose    | 7.01V |
+--------+-----------+----------+---------+------------------+--------------------+-------+
| AI3    | AIF[PN]3  | E18/E19  | AD9     | in_voltage12_raw | general purpose    | 7.01V |
+--------+-----------+----------+---------+------------------+--------------------+-------+
|        | AIF[PN]4  | K9 /L10  | AD      | in_voltage0_raw  | 5V power supply    | 12.2V |
+--------+-----------+----------+---------+------------------+--------------------+-------+

-----------
Input range
-----------

The default mounting intends for unipolar XADC inputs,
which allow for observing only positive signals with a saturation range of *0V ~ 1V*.
There are additional voltage dividers use to extend this range up to the power supply voltage.
It is possible to configure XADC inputs into a bipolar mode with a range of *-0.5V ~ +0.5V*,
but it requires removing R273 and providing a *0.5V ~ 1V* common voltage on the E2 connector.

.. note::

   Unfortunately there is a design error,
   where the XADC input range in unipolar mode was thought to be *0V ~ 0.5V*.
   Consequently the voltage dividers were miss designed for a range of double the supply voltage.

~~~~~~~~~~~~~~~
5V power supply
~~~~~~~~~~~~~~~

.. code-block:: none

                           -------------------0  Vout
             ------------  |  ------------
   Vin  0----| 56.0kOHM |-----| 4.99kOHM |----0  GND
             ------------     ------------

Ratio: 4.99/(56.0+4.99)=0.0818
Range: 1V / ratio = 12.2V

~~~~~~~~~~~~~~~~~~~~~~
General purpose inputs
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: none

                           -------------------0  Vout
             ------------  |  ------------
   Vin  0----| 30.0kOHM |-----| 4.99kOHM |----0  GND
             ------------     ------------
   
Ratio: 4.99/(30.0+4.99)=0.143
Range: 1V / ratio = 7.01

=========
GPIO LEDs
=========

+-----------+--------+-------------------+-------------------------------+
| LED       | color  | SW driver         | dedicated meaning             |
+===========+========+===================+===============================+
| ``[7:0]`` | yellow | RP API            | user defined                  |
+-----------+--------+-------------------+-------------------------------+
|   ``[8]`` | yellow | kernel ``MIO[0]`` | CPU heartbeat (user defined)  |
+-----------+--------+-------------------+-------------------------------+
|   ``[9]`` | reg    | kernel ``MIO[7]`` | SD card access (user defined) |
+-----------+--------+-------------------+-------------------------------+
|  ``[10]`` | green  | none              | *Power Good* status           |
+-----------+--------+-------------------+-------------------------------+
|  ``[11]`` | blue   | none              | FPGA programming *DONE*       |
+-----------+--------+-------------------+-------------------------------+

For now only LED8 and LED9 are accessible using a kernel driver. LED [7:0] are not driven by a kernel driver, since the Linux GPIO/LED subsystem does not allow access to multiple pins simultaneously.
.. TODO preveri z Iztokom ali je to res?

========================
Linux access to GPIO/LED
========================

This document is used as reference: `Linux+GPIO+Driver <http://www.wiki.xilinx.com/Linux+GPIO+Driver>`_

There are 54+64=118 GPIO provided by ZYNQ PS, MIO provides 54 GPIO,
and EMIO provide additional 64 GPIO.

The next formula is used to calculate the ``gpio_base`` index.

.. code-block:: none

   base_gpio = ZYNQ_GPIO_NR_GPIOS - ARCH_NR_GPIOS = 1024 - 118 = -906

Values for the used macros can be found in the kernel sources.

.. code-block:: shell-session

   $ grep ZYNQ_GPIO_NR_GPIOS drivers/gpio/gpio-zynq.c
   #define	ZYNQ_GPIO_NR_GPIOS	118
   $ grep -r CONFIG_ARCH_NR_GPIO tmp/linux-xlnx-xilinx-v2016.1
   tmp/linux-xlnx-xilinx-v2016.1/.config:CONFIG_ARCH_NR_GPIO=1024

Another way to find the `gpio_base` index is to check the given name inside `sysfs`.

.. code-block:: shell-session

   # find /sys/class/gpio/ -name gpiochip*
   /sys/class/gpio/gpiochip906

The default pin assignment for GPIO is described in the next table.

+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| FPGA   | connector  | GPIO               | MIO/EMIO index   | ``sysfs`` index              | color, dedicated meaning                  |
+========+============+====================+==================+==============================+===========================================+
|        |            | ``exp_p_io [7:0]`` | ``EMIO[15: 8]``  | ``906+54+[15: 8]=[975:968]`` |                                           |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
|        |            | ``exp_n_io [7:0]`` | ``EMIO[23:16]``  | ``906+54+[23:16]=[983:976]`` |                                           |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
|        |            | LED ``[7:0]``      | ``EMIO[ 7: 0]``  | ``906+54+[ 7: 0]=[967:960]`` | yellow                                    |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
|        |            | LED ``  [8]``      |  ``MIO[ 0]``     | ``906+   [ 0]   = 906``      | yellow = CPU heartbeat (user defined)     |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
|        |            | LED ``  [9]``      |  ``MIO[ 7]``     | ``906+   [ 7]   = 913``      | red    = SD card access (user defined)    |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| ``D5`` | ``E2[ 7]`` | UART1_TX           |  ``MIO[ 8]``     | ``906+   [ 8]   = 914``      | output only                               |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| ``B5`` | ``E2[ 8]`` | UART1_RX           |  ``MIO[ 9]``     | ``906+   [ 9]   = 915``      | requires ``pinctrl`` changes to be active |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| ``E9`` | ``E2[ 3]`` | SPI1_MOSI          |  ``MIO[10]``     | ``906+   [10]   = 916``      | requires ``pinctrl`` changes to be active |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| ``C6`` | ``E2[ 4]`` | SPI1_MISO          |  ``MIO[11]``     | ``906+   [11]   = 917``      | requires ``pinctrl`` changes to be active |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| ``D9`` | ``E2[ 5]`` | SPI1_SCK           |  ``MIO[12]``     | ``906+   [12]   = 918``      | requires ``pinctrl`` changes to be active |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| ``E8`` | ``E2[ 6]`` | SPI1_CS#           |  ``MIO[13]``     | ``906+   [13]   = 919``      | requires ``pinctrl`` changes to be active |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| ``B13``| ``E2[ 9]`` | I2C0_SCL           |  ``MIO[50]``     | ``906+   [50]   = 956``      | requires ``pinctrl`` changes to be active |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| ``B9`` | ``E2[10]`` | I2C0_SDA           |  ``MIO[51]``     | ``906+   [51]   = 957``      | requires ``pinctrl`` changes to be active |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+

GPIOs are accessible at the ``sysfs`` index.
The next example will light up ``LED[0]``, and read back its value.

.. code-block:: shell-session

   $ export INDEX=960
   $ echo $INDEX > /sys/class/gpio/export
   $ echo out    > /sys/class/gpio/gpio$INDEX/direction
   $ echo 1      > /sys/class/gpio/gpio$INDEX/value
   $ cat           /sys/class/gpio/gpio$INDEX/value

.. note::

   | A new user space ABI for GPIO is coming in kernel v4.8, ioctl will be used instead of ``sysfs``.
   | `link <https://git.kernel.org/cgit/linux/kernel/git/linusw/linux-gpio.git/tree/include/uapi/linux/gpio.h?h=for-next>`_

   
===================
Linux access to LED
===================

This document is used as reference: http://www.wiki.xilinx.com/Linux+GPIO+Driver

By providing GPIO/LED details in the device tree, it is possible to access LEDs using a dedicated kernel interface.

To show CPU load on LED 9 use:

.. code-block:: shell-session

   $ echo heartbeat > /sys/class/leds/led0/trigger

To switch LED 8 ON use:

.. code-block:: shell-session

   $ echo 1 > /sys/class/leds/led0/brightness
   
==============================
PS ``pinctrl`` for MIO signals
==============================

+--------------------+--------------------------------+
| dts                | description                    |
+====================+================================+
| ``spi2gpio.dtsi``  | E2 connector, SPI1 signals     |
+--------------------+--------------------------------+
| ``i2c2gpio.dtsi``  | E2 connector, I2C0 signals     |
+--------------------+--------------------------------+
| ``uart2gpio.dtsi`` | E2 connector, UART1 signals    |
+--------------------+--------------------------------+
| ``miso2gpio.dtsi`` | E2 connector, SPI1 MISO signal |
+--------------------+--------------------------------+
   
************
Register map 
************

.. TODO preveri z iztokom kaj je s id.rst konflit z Housekeeping?

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

------------
Housekeeping
------------

.. tabularcolumns:: |p{15mm}|p{105mm}|p{15mm}|p{15mm}|

+----------+------------------------------------------------+------+-----+
| offset   | description                                    | bits | R/W |
+==========+================================================+======+=====+
| **0x0**  | **ID**                                         |      |     |
+----------+------------------------------------------------+------+-----+
|          | Reserved                                       | 31:4 | R   | 
+----------+------------------------------------------------+------+-----+
|          | Design ID                                      |  3:0 | R   |
+----------+------------------------------------------------+------+-----+
|          |    0 -prototype                                |      |     |
+----------+------------------------------------------------+------+-----+
|          |    1 -release                                  |      |     |
+----------+------------------------------------------------+------+-----+
| **0x4**  | **DNA part 1**                                 |      |     |
+----------+------------------------------------------------+------+-----+
|          | DNA[31:0]                                      | 31:0 | R   |
+----------+------------------------------------------------+------+-----+
| **0x8**  | **DNA part 2**                                 |      |     |
+----------+------------------------------------------------+------+-----+
|          | Reserved                                       | 31:25| R   |
+----------+------------------------------------------------+------+-----+
|          | DNA[56:32]                                     | 24:0 | R   |
+----------+------------------------------------------------+------+-----+
| **0xC**  | **Digital Loopback**                           |      |     |
+----------+------------------------------------------------+------+-----+
|          | Reserved                                       | 31:1 | R   |
+----------+------------------------------------------------+------+-----+
|          | digital_loop                                   |    0 | R/W |
+----------+------------------------------------------------+------+-----+
| **0x10** | **Expansion connector direction P**            |      |     |
+----------+------------------------------------------------+------+-----+
|          | Reserved                                       | 31:8 | R   |
+----------+------------------------------------------------+------+-----+
|          | Direction for P lines                          |  7:0 | R/W |
+----------+------------------------------------------------+------+-----+
|          | 1-out                                          |      |     |
+----------+------------------------------------------------+------+-----+
|          | 0-in                                           |      |     |
+----------+------------------------------------------------+------+-----+
| **0x14** | **Expansion connector direction N**            |      |     |
+----------+------------------------------------------------+------+-----+
|          | Reserved                                       | 31:8 | R   |
+----------+------------------------------------------------+------+-----+
|          | Direction for N lines                          | 7:0  | R/W |
+----------+------------------------------------------------+------+-----+
|          | 1-out                                          |      |     |
+----------+------------------------------------------------+------+-----+
|          | 0-in                                           |      |     |
+----------+------------------------------------------------+------+-----+
| **0x18** | **Expansion connector output P**               |      |     |
+----------+------------------------------------------------+------+-----+
|          | Reserved                                       | 31:8 | R   |
+----------+------------------------------------------------+------+-----+
|          | P pins output                                  | 7:0  | R/W |
+----------+------------------------------------------------+------+-----+
| **0x1C** | **Expansion connector output N**               |      |     |
+----------+------------------------------------------------+------+-----+
|          | Reserved                                       | 31:8 | R   |
+----------+------------------------------------------------+------+-----+
|          | N pins output                                  | 7:0  | R/W |
+----------+------------------------------------------------+------+-----+
| **0x20** | **Expansion connector input P**                |      |     |
+----------+------------------------------------------------+------+-----+
|          | Reserved                                       | 31:8 | R   |
+----------+------------------------------------------------+------+-----+
|          | P pins input                                   | 7:0  | R   |
+----------+------------------------------------------------+------+-----+
| **0x24** | **Expansion connector input N**                |      |     |
+----------+------------------------------------------------+------+-----+
|          | Reserved                                       | 31:8 | R   |
+----------+------------------------------------------------+------+-----+
|          |  N pins input                                  |  7:0 | R   |
+----------+------------------------------------------------+------+-----+
| **0x30** |  **LED control**                               |      |     |
+----------+------------------------------------------------+------+-----+
|          |  Reserved                                      |  31:8| R   |
+----------+------------------------------------------------+------+-----+
|          |  LEDs 7-0                                      |  7:0 | R/W |
+----------+------------------------------------------------+------+-----+

------------
Oscilloscope
------------

.. tabularcolumns:: |p{15mm}|p{105mm}|p{15mm}|p{15mm}|

+----------+----------------------------------------------------+------+-----+
| offset   | description                                        | bits | R/W |
+==========+====================================================+======+=====+
| **0x0**  | **Configuration**                                  |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           |  31:3|   R |
+----------+----------------------------------------------------+------+-----+
|          | | Trigger status before acquire ends,              |     2|   R |
|          | | 0 – pre trigger                                  |      |     |
|          | | 1 – post trigger                                 |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reset write state machine                          |     1|   W |
+----------+----------------------------------------------------+------+-----+
|          | Start writing data into memory (ARM trigger).      |     0|   W |
+----------+----------------------------------------------------+------+-----+
| **0x4**  | **Trigger source**                                 |      |     |
+----------+----------------------------------------------------+------+-----+
|          |  Selects trigger source for data capture. When     |      |     |
|          |  trigger delay is ended value goes to 0.           |      |     |
+----------+----------------------------------------------------+------+-----+
|          |  Reserved                                          |  31:4|   R |
+----------+----------------------------------------------------+------+-----+
|          | | Trigger source                                   |  3:0 | R/W |
|          | | 1 - trig immediately                             |      |     |
|          | | 2 - ch A threshold positive edge                 |      |     |
|          | | 3 - ch A threshold negative edge                 |      |     |
|          | | 4 - ch B threshold positive edge                 |      |     |
|          | | 5 - ch B threshold negative edge                 |      |     |
|          | | 6 - external trigger positive edge - DIO0_P pin  |      |     |
|          | | 7 - external trigger negative edge               |      |     |
|          | | 8 - arbitrary wave generator application       \ |      |     |
|          |       positive edge                                |      |     |
|          | | 9 - arbitrary wave generator application         |      |     |
|          |       negative edge                             \  |      |     |
+----------+----------------------------------------------------+------+-----+
| **0x8**  | **Ch A threshold**                                 |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:14| R   |
+----------+----------------------------------------------------+------+-----+
|          | Ch A threshold, makes trigger when ADC value       | 13:0 | R/W |
+----------+----------------------------------------------------+------+-----+
|          | cross this value                                   |      |     |
+----------+----------------------------------------------------+------+-----+
| **0xC**  | **Ch B threshold**                                 |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:14| R   |
+----------+----------------------------------------------------+------+-----+
|          | Ch B threshold, makes trigger when ADC value       | 13:0 | R/W |
|          | cross this value                                   |      |     |
+----------+----------------------------------------------------+------+-----+
| **0x10** | **Delay after trigger**                            |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Number of decimated data after trigger written     | 31:0 | R/W |
|          | into memory                                        |      |     |
+----------+----------------------------------------------------+------+-----+
| **0x14** | **Data decimation**                                |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Decimate input data, uses data average             |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:17| R   |
+----------+----------------------------------------------------+------+-----+
|          | Data decimation, supports only this values: 1,     | 16:0 | R/W |
|          | 8, 64,1024,8192,65536. If other value is           |      |     |
|          | written data will NOT be correct.                  |      |     |
+----------+----------------------------------------------------+------+-----+
| **0x18** | **Write pointer - current**                        |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:14| R   |
+----------+----------------------------------------------------+------+-----+
|          | Current write pointer                              | 13:0 | R   |
+----------+----------------------------------------------------+------+-----+
| **0x1C** | **Write pointer - trigger**                        |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:14| R   |
+----------+----------------------------------------------------+------+-----+
|          | Write pointer at time when trigger arrived         | 13:0 | R   |
+----------+----------------------------------------------------+------+-----+
| **0x20** | **Ch A hysteresis**                                |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:14| R   |
+----------+----------------------------------------------------+------+-----+
|          | Ch A threshold hysteresis. Value must be outside   | 13:0 | R/W |
|          | to enable trigger again.                           |      |     |
+----------+----------------------------------------------------+------+-----+
| **0x24** | **Ch B hysteresis**                                |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:14| R   |
+----------+----------------------------------------------------+------+-----+
|          | Ch B threshold hysteresis. Value must be outside   | 13:0 | R/W |
|          | to enable trigger again.                           |      |     |
+----------+----------------------------------------------------+------+-----+
| **0x28** | **Other**                                          |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:1 | R   |
|          | Enable signal average at decimation                | 0    | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x2C** | **PreTrigger Counter**                             |      |     |
+----------+----------------------------------------------------+------+-----+
|          | This unsigned counter holds the number of samples  | 31:0 | R   |
|          | captured between the start of acquire and trigger. |      |     |
|          | The value does not overflow, instead it stops      |      |     |
|          | incrementing at 0xffffffff.                        |      |     |
+----------+----------------------------------------------------+------+-----+
| **0x30** | **CH A Equalization filter**                       |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:18| R   |
+----------+----------------------------------------------------+------+-----+
|          | AA Coefficient                                     | 17:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x34** | **CH A Equalization filter**                       |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:25| R   |
+----------+----------------------------------------------------+------+-----+
|          | BB Coefficient                                     | 24:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x38** | **CH A Equalization filter**                       |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:25| R   |
+----------+----------------------------------------------------+------+-----+
|          | KK Coefficient                                     | 24:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x3C** | **CH A Equalization filter**                       |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:25| R   |
+----------+----------------------------------------------------+------+-----+
|          | PP Coefficient                                     | 24:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x40** | **CH B Equalization filter**                       |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:18| R   |
+----------+----------------------------------------------------+------+-----+
|          | AA Coefficient                                     | 17:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x44** | **CH B Equalization filter**                       |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:25| R   |
+----------+----------------------------------------------------+------+-----+
|          | BB Coefficient                                     | 24:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x48** | **CH B Equalization filter**                       |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:25| R   |
+----------+----------------------------------------------------+------+-----+
|          | KK Coefficient                                     | 24:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x4C** | **CH B Equalization filter**                       |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:25| R   |
+----------+----------------------------------------------------+------+-----+
|          | PP Coefficient                                     | 24:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x50** | **CH A AXI lower address**                         |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Starting writing address                           | 31:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x54** | **CH A AXI upper address**                         |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Address where it jumps to lower                    | 31:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x58** | **CH A AXI delay after trigger**                   |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Number of decimated data after trigger written     | 31:0 | R/W |
|          | into memory                                        |      |     |
+----------+----------------------------------------------------+------+-----+
| **0x5C** | **CH A AXI enable master**                         |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:1 | R   |
+----------+----------------------------------------------------+------+-----+
|          | Enable AXI master                                  | 0    | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x60** | **CH A AXI write pointer - trigger**               |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Write pointer at time when trigger arrived         | 31:0 | R   |
+----------+----------------------------------------------------+------+-----+
| **0x64** | **CH A AXI write pointer - current**               |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Current write pointer                              | 31:0 | R   |
+----------+----------------------------------------------------+------+-----+
| **0x70** | **CH B AXI lower address**                         |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Starting writing address                           | 31:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x74** | **CH B AXI upper address**                         |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Address where it jumps to lower                    | 31:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x78** | **CH B AXI delay after trigger**                   |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Number of decimated data after trigger written     | 31:0 | R/W |
|          | into memory                                        |      |     |
+----------+----------------------------------------------------+------+-----+
| **0x7C** | **CH B AXI enable master**                         |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:1 | R   |
+----------+----------------------------------------------------+------+-----+
|          | Enable AXI master                                  | 0    | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x80** | **CH B AXI write pointer - trigger**               |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Write pointer at time when trigger arrived         | 31:0 | R   |
+----------+----------------------------------------------------+------+-----+
| **0x84** | **CH B AXI write pointer - current**               |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Current write pointer                              | 31:0 | R   |
+----------+----------------------------------------------------+------+-----+
| **0x90** | **Trigger debouncer time**                         |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Number of ADC clock periods trigger is disabled    | 19:0 | R/W |
|          | after activation reset value is decimal 62500 or   |      |     |
|          | equivalent to 0.5ms                                |      |     |
+----------+----------------------------------------------------+------+-----+
| **0xA0** | **Accumulator data sequence length**               |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:14| R   |
+----------+----------------------------------------------------+------+-----+
| **0xA4** | **Accumulator data offset corection ChA**          |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:14| R   |
+----------+----------------------------------------------------+------+-----+
|          | signed offset value                                | 13:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0xA8** | **Accumulator data offset corection ChB**          |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:14| R   |
+----------+----------------------------------------------------+------+-----+
|          | signed offset value                                | 13:0 | R/W |
+----------+----------------------------------------------------+------+-----+
| **0x10000| **Memory data (16k samples)**                      |      |     |
| to       |                                                    |      |     |
| 0x1FFFC**|                                                    |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:16| R   |
+----------+----------------------------------------------------+------+-----+    
|          | Captured data for ch A                             | 15:0 | R   |
+----------+----------------------------------------------------+------+-----+    
| **0x20000| **Memory data (16k samples)**                      |      |     |
| to       |                                                    |      |     |
| 0x2FFFC**|                                                    |      |     |
+----------+----------------------------------------------------+------+-----+
|          | Reserved                                           | 31:16| R   |
+----------+----------------------------------------------------+------+-----+    
|          | Captured data for ch B                             | 15:0 | R   |
+----------+----------------------------------------------------+------+-----+    

--------------------------------
Arbitrary Signal Generator (ASG)
--------------------------------

.. tabularcolumns:: |p{15mm}|p{105mm}|p{15mm}|p{15mm}|

+----------+----------------------------------------------------+------+-----+    
| offset   | description                                        | bits | R/W |
+==========+====================================================+======+=====+
| **0x0**  |  **Configuration**                                 |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:25| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  ch B external gated repetitions                   | 24   | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  ch B set output to 0                              | 23   | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  ch B SM reset                                     | 22   | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 21   | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  ch B SM wrap pointer (if disabled starts at       | 20   | R/W |
|          |  address0 )                                        |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | | ch B trigger selector: (don't change when SM is  | 19:16| R/W |
|          | | active)                                          |      |     |
|          | | 1-trig immediately                               |      |     |
|          | | 2-external trigger positive edge - DIO0_P pin    |      |     |
|          | | 3-external trigger negative edge                 |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 15:9 | R   |
+----------+----------------------------------------------------+------+-----+    
|          |  ch A external gated bursts                        | 8    | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  ch A set output to 0                              | 7    | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  ch A SM reset                                     | 6    | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 5    | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  ch A SM wrap pointer (if disabled starts at       | 4    | R/W |
|          |  address 0)                                        |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | | ch A trigger selector: (don't change when SM is  | 3:0  | R/W |
|          | | active)                                          |      |     |
|          | | 1-trig immediately                               |      |     |
|          | | 2-external trigger positive edge - DIO0_P pin    |      |     |
|          | | 3-external trigger negative edge                 |      |     |
+----------+----------------------------------------------------+------+-----+    
| **0x4**  |  **Ch A amplitude scale and offset**               |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  out  = (data*scale)/0x2000 + offset               |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:30| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Amplitude offset                                  | 29:16| R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 15:14| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Amplitude scale. 0x2000 == multiply by 1. Unsigned| 13:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x8**  |  **Ch A counter wrap**                             |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:30| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Value where counter wraps around. Depends on SM   | 29:0 | R/W |
|          |  wrap setting. If it is 1 new value is  get by     |      |     |
|          |  wrap, if value is 0 counter goes to offset value. |      |     |
|          |  16 bits for decimals.                             |      |     |
+----------+----------------------------------------------------+------+-----+    
| **0xC**  |  **Ch A start offset**                             |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:30| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Counter start offset. Start offset when trigger   | 29:0 | R/W |
|          |  arrives. 16 bits for decimals.                    |      |     |
+----------+----------------------------------------------------+------+-----+    
| **0x10** |   **Ch A counter step**                            |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:30| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Counter step. 16 bits for decimals.               | 29:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x14** |   **Ch A buffer current read pointer**             |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:16| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Read pointer                                      | 15:2 | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 1:0  | R   |
+----------+----------------------------------------------------+------+-----+    
| **0x18** |   **Ch A number of read cycles in one burst**      |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:16| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Number of repeats of table readout. 0=infinite    | 15:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x1C** |   **Ch A number of burst repetitions**             |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:16| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Number of repetitions. 0=disabled                 | 15:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x20** |   **Ch A delay between burst repetitions**         |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Delay between repetitions. Granularity=1us        | 31:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x24** |   **Ch B amplitude scale and offset**              |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  out  = (data*scale)/0x2000 + offset               |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:30| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Amplitude offset                                  | 29:16| R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 15:14| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Amplitude scale. 0x2000 == multiply by 1. Unsigned| 13:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x28** |   **Ch B counter wrap**                            |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:30| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Value where counter wraps around. Depends on SM   | 29:0 | R/W |
|          |  wrap setting. If it is 1 new value is  get by     |      |     |
|          |  wrap, if value is 0 counter goes to offset value. |      |     |
|          |  16 bits for decimals.                             |      |     |
+----------+----------------------------------------------------+------+-----+    
| **0x2C** |   **Ch B start offset**                            |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:30| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Counter start offset. Start offset when trigger   | 29:0 | R/W |
|          |  arrives. 16 bits for decimals.                    |      |     |
+----------+----------------------------------------------------+------+-----+    
| **0x30** |   **Ch B counter step**                            |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:30| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Counter step. 16 bits for decimals.               | 29:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x34** |   **Ch B buffer current read pointer**             |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:16| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Read pointer                                      | 15:2 | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 1:0  | R   |
+----------+----------------------------------------------------+------+-----+    
| **0x38** |   **Ch B number of read cycles in one burst**      |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:16| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Number of repeats of table readout. 0=infinite    | 15:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x3C** |   **Ch B number of burst repetitions**             |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:16| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Number of repetitions. 0=disabled                 | 15:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x40** |   **Ch B delay between burst repetitions**         |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Delay between repetitions. Granularity=1us        | 31:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x10000|  Ch A memory data (16k samples)                    |      |     |
| to       |                                                    |      |     |
| 0x1FFFC**|                                                    |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:14| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  ch A data                                         | 13:0 | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x20000|  Ch B memory data (16k samples)                    |      |     |
| to       |                                                    |      |     |
| 0x2FFFC**|                                                    |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:14| R   |
+----------+----------------------------------------------------+------+-----+    
|          |  ch B data                                         | 13:0 | R/W |
+----------+----------------------------------------------------+------+-----+    

--------------
PID Controller
--------------

.. tabularcolumns:: |p{15mm}|p{105mm}|p{15mm}|p{15mm}|

+----------+----------------------------------------------------+------+-----+    
| offset   | description                                        | bits | R/W |
+==========+====================================================+======+=====+
| **0x0**  | **Configuration**                                  |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:4 | R   |
+----------+----------------------------------------------------+------+-----+    
|          | PID22 integrator reset                             | 3    | R/W |
+----------+----------------------------------------------------+------+-----+    
|          | PID21 integrator reset                             | 2    | R/W |
+----------+----------------------------------------------------+------+-----+    
|          | PID12 integrator reset                             | 1    | R/W |
+----------+----------------------------------------------------+------+-----+    
|          | PID11 integrator reset                             | 0    | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x10** | **PID11 set point**                                |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID11 set point                                    | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x14** | **PID11 proportional coefficient**                 |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID11 Kp                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x18** | **PID11 integral coefficient**                     |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID11 Ki                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x1C** | **PID11 derivative coefficient**                   |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID11 Kd                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x20** | **PID12 set point**                                |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID12 set point                                    | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x24** | **PID12 proportional coefficient**                 |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID12 Kp                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x28** | **PID12 integral coefficient**                     |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID12 Ki                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x2C** | **PID12 derivative coefficient**                   |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID12 Kd                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x30** | **PID21 set point**                                |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID21 set point                                    | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x34** | **PID21 proportional coefficient**                 |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID21 Kp                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x38** | **PID21 integral coefficient**                     |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID21 Ki                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x3C** | **PID21 derivative coefficient**                   |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID21 Kd                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x40** | **PID22 set point**                                |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID22 set point                                    | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x44** | **PID22 proportional coefficient**                 |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID22 Kp                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x48** | **PID22 integral coefficient**                     |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID22 Ki                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    
| **0x4C** | **PID22 derivative coefficient**                   |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:14|  R  |
+----------+----------------------------------------------------+------+-----+    
|          | PID22 Kd                                           | 13:0 |  R/W|
+----------+----------------------------------------------------+------+-----+    

--------------------------
Analog Mixed Signals (AMS)
--------------------------

.. tabularcolumns:: |p{15mm}|p{105mm}|p{15mm}|p{15mm}|

+----------+-----------------------------------------------------+------+-----+    
| offset   | description                                         | bits | R/W |
+==========+=====================================================+======+=====+
| **0x0**  | **XADC AIF0**                                       |      |     |
+----------+-----------------------------------------------------+------+-----+    
|          | Reserved                                            | 31:12| R   |
+----------+-----------------------------------------------------+------+-----+    
|          | AIF0 value                                          | 11:0 | R   |
+----------+-----------------------------------------------------+------+-----+    
| **0x4**  | **XADC AIF1**                                       |      |     |
+----------+-----------------------------------------------------+------+-----+    
|          | Reserved                                            | 31:12| R   |
+----------+-----------------------------------------------------+------+-----+    
|          | AIF1 value                                          | 11:0 | R   |
+----------+-----------------------------------------------------+------+-----+    
| **0x8**  | **XADC AIF2**                                       |      |     |
+----------+-----------------------------------------------------+------+-----+    
|          | Reserved                                            | 31:12| R   |
+----------+-----------------------------------------------------+------+-----+    
|          | AIF2 value                                          | 11:0 | R   |
+----------+-----------------------------------------------------+------+-----+    
| **0xC**  | **XADC AIF3**                                       |      |     |
+----------+-----------------------------------------------------+------+-----+    
|          | Reserved                                            | 31:12| R   |
+----------+-----------------------------------------------------+------+-----+    
|          | AIF3 value                                          | 11:0 | R   |
+----------+-----------------------------------------------------+------+-----+    
| **0x10** | **XADC AIF4**                                       |      |     |
+----------+-----------------------------------------------------+------+-----+    
|          | Reserved                                            | 31:12| R   |
+----------+-----------------------------------------------------+------+-----+    
|          | AIF4 value (5V power supply)                        | 11:0 | R   |
+----------+-----------------------------------------------------+------+-----+    
| **0x20** | **PWM DAC0**                                        |      |     |
+----------+-----------------------------------------------------+------+-----+    
|          | Reserved                                            | 31:24| R   |
+----------+-----------------------------------------------------+------+-----+    
|          | PWM value (100% == 156)                             | 23:16| R/W |
+----------+-----------------------------------------------------+------+-----+    
|          | Bit select for PWM repetition which have value PWM+1| 15:0 | R/W |
+----------+-----------------------------------------------------+------+-----+    
| **0x24** | **PWM DAC1**                                        |      |     |
+----------+-----------------------------------------------------+------+-----+    
|          | Reserved                                            | 31:24| R   |
+----------+-----------------------------------------------------+------+-----+    
|          | PWM value (100% == 156)                             | 23:16| R/W |
+----------+-----------------------------------------------------+------+-----+    
|          | Bit select for PWM repetition which have value PWM+1| 15:0 | R/W |
+----------+-----------------------------------------------------+------+-----+    
| **0x28** | **PWM DAC2**                                        |      |     |
+----------+-----------------------------------------------------+------+-----+    
|          | Reserved                                            | 31:24| R   |
+----------+-----------------------------------------------------+------+-----+    
|          | PWM value (100% == 156)                             | 23:16| R/W |
+----------+-----------------------------------------------------+------+-----+    
|          | Bit select for PWM repetition which have value PWM+1| 15:0 | R/W |
+----------+-----------------------------------------------------+------+-----+    
| **0x2C** | **PWM DAC3**                                        |      |     |
+----------+-----------------------------------------------------+------+-----+    
|          | Reserved                                            | 31:24| R   |
+----------+-----------------------------------------------------+------+-----+    
|          | PWM value (100% == 156)                             | 23:16| R/W |
+----------+-----------------------------------------------------+------+-----+    
|          | Bit select for PWM repetition which have value PWM+1| 15:0 | R/W |
+----------+-----------------------------------------------------+------+-----+    

-----------
Daisy Chain
-----------

.. tabularcolumns:: |p{15mm}|p{105mm}|p{15mm}|p{15mm}|

+----------+----------------------------------------------------+------+-----+    
| offset   | description                                        | bits | R/W |
+==========+====================================================+======+=====+
| **0x0**  | **Control**                                        |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 31:2 | R   |
+----------+----------------------------------------------------+------+-----+    
|          |  RX enable                                         | 1    | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  TX enable                                         | 0    | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x4**  | **Transmitter data selector**                      |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Custom data                                       | 31:1 | R/W |
+----------+----------------------------------------------------+------+-----+    
|          |  Reserved                                          | 15:8 | R   |
+----------+----------------------------------------------------+------+-----+    
|          |  | Data source                                     | 3:0  | R/W |
|          |  | 0 - data is 0                                   |      |     |
|          |  | 1 - user data (from logic)                      |      |     |
|          |  | 2 - custom data (from this register)            |      |     |
|          |  | 3 - training data (0x00FF)                      |      |     |
|          |  | 4 - transmit received data (loop back)          |      |     |
|          |  | 5 - random data (for testing)                   |      |     |
+----------+----------------------------------------------------+------+-----+    
| **0x8**  | **Receiver training**                              |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:2 | R   |
+----------+----------------------------------------------------+------+-----+    
|          | Training successful                                | 1    | R   |
+----------+----------------------------------------------------+------+-----+    
|          | Enable training                                    | 0    | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0xC**  | **Received data**                                  |      |     |
+----------+----------------------------------------------------+------+-----+    
|          |  Received data which is different than 0           | 31:1 | R   |
+----------+----------------------------------------------------+------+-----+    
|          |  Received raw data                                 | 15:0 | R   |
+----------+----------------------------------------------------+------+-----+    
| **0x10** | **Testing control**                                |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:1 | R   |
+----------+----------------------------------------------------+------+-----+    
|          | Reset testing counters (error & data)              | 0    | R/W |
+----------+----------------------------------------------------+------+-----+    
| **0x14** | **Testing error counter**                          |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Error increases if received data is not the        | 31:0 | R   |
|          | same as transmitted testing data                   |      |     |
+----------+----------------------------------------------------+------+-----+    
| **0x18** | **Testing data counter**                           |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Counter increases when value different as          | 31:0 | R   |
|          | 0 is received                                      |      |     |
+----------+----------------------------------------------------+------+-----+    

----------
Power Test
----------

.. tabularcolumns:: |p{15mm}|p{105mm}|p{15mm}|p{15mm}|

+----------+----------------------------------------------------+------+-----+    
| offset   | description                                        | bits | R/W |
+==========+====================================================+======+=====+
| **0x0**  | **Control**                                        |      |     |
+----------+----------------------------------------------------+------+-----+    
|          | Reserved                                           | 31:1 | R   |
+----------+----------------------------------------------------+------+-----+    
|          | Enable module                                      | 0    | R/W |
+----------+----------------------------------------------------+------+-----+    



