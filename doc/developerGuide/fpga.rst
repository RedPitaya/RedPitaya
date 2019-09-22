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

2. *Xilinx Vivado 2017.2 (including SDK)* 

*******************
Directory structure
*******************

There are multiple FPGA projects, some with generic functionality, some with specific functionality for an application.
Common code for all projects is placed directly into the ``fpga`` directory. Common code are mostly reusable modules.
Project specific code is placed inside the ``fpga/prj/name/`` directories and is similarly organized as common code.

.. |ug895| replace:: Vivado System-Level Design Entry
.. _ug895: https://www.xilinx.com/support/documentation/sw_manuals/xilinx2017_2/ug895-vivado-system-level-design-entry.pdf

.. tabularcolumns:: |p{30mm}|p{120mm}|

+-------------------+------------------------------------------------------------------+
|  path             | contents                                                         |
+===================+==================================================================+
| ``fpga/Makefile`` | main Makefile, used to run FPGA related tools                    |
+-------------------+------------------------------------------------------------------+
| ``fpga/*.tcl``    | TCL scripts to be run inside FPGA tools                          |
+-------------------+------------------------------------------------------------------+
| ``fpga/archive/`` | archive of XZ compressed FPGA bit files                          |
+-------------------+------------------------------------------------------------------+
| ``fpga/doc/``     | documentation (block diagrams, address space, ...)               |
+-------------------+------------------------------------------------------------------+
| ``fpga/brd/``     | board files |ug895|_                                             |
+-------------------+------------------------------------------------------------------+
| ``fpga/ip/``      | third party IP, for now Zynq block diagrams                      |
+-------------------+------------------------------------------------------------------+
| ``fpga/rtl/``     | Verilog (SystemVerilog) *Register-Transfer Level*                |
+-------------------+------------------------------------------------------------------+
| ``fpga/sdc/``     | *Synopsys Design Constraints* contains Xilinx design constraints |
+-------------------+------------------------------------------------------------------+
| ``fpga/sim/``     | simulation scripts                                               |
+-------------------+------------------------------------------------------------------+
| ``fpga/tbn/``     | Verilog (SystemVerilog) *test bench*                             |
+-------------------+------------------------------------------------------------------+
| ``fpga/dts/``     | device tree source include files                                 |
+-------------------+------------------------------------------------------------------+
| ``fpga/prj/name`` | project `name` specific code                                     |
+-------------------+------------------------------------------------------------------+
| ``fpga/hsi/``     | *Hardware Software Interface* contains                           |
|                   | FSBL (First Stage Boot Loader) and                               |
|                   | DTS (Design Tree) builds                                         |
+-------------------+------------------------------------------------------------------+

*****************
FPGA sub-projects
*****************

There are multiple FPGA sub-projects they mostly contain incremental changes
on the first Red Pitaya release.

+-------------------+------------------------------------------------------------------+
| ``prj/name``      | desctiption                                                      |
+===================+==================================================================+
| 0.93              | This is the original Red Pitaya release including all bugs.      |
|                   | For deprecated application backward compatibility only.          |
+-------------------+------------------------------------------------------------------+
| 0.94              | 1. The CDC (clock domain crossing) code on the custom CPU bus    |
|                   |    was removed. Instead CDC for GP0 port already available in    |
|                   |    PS was used. This improves speed and reliability and reduces  |
|                   |    RTL complexity.                                               |
|                   | 2. A value increment bug in the generator was fixed, this should |
|                   |    improve generated frequencies near half sampling rate.        |
|                   | 3. XADC custom RTL wrapper was replaced with Xilinx AXI XADC.    |
|                   |    This enables the use of the Linux driver with IIO streaming   |
|                   |    support.                                                      |
+-------------------+------------------------------------------------------------------+
| classic           | 1. A lot of the code was rewritten in SystemVerilog.             |
|                   | 2. Removed GPIO and LED registers from housekeeping, instead the |   
|                   |    GPIO controller inside PL is used. This enables the use of    |
|                   |    Linux kernel features for GPIO (IRQ, SPI, I2C, 1-wire) and    |
|                   |    LED (triggers).                                               |
+-------------------+------------------------------------------------------------------+
| logic             | This image is used by the logic analyzer, it is using DMA to     |
|                   | transfer data to man DDR3 RAM. ADC and DAS code is unfinished.   |
+-------------------+------------------------------------------------------------------+
| axi4lite          | Image intended for testing various AXI4 bus implementations.     |
|                   | It contains a Vivado ILA (integrated logic ananlyzer) to         |
|                   | observe and review the performance of the bus implementation.    |
+-------------------+------------------------------------------------------------------+


.. _buildprocess:

****************
Building process
****************
    
If Xilinx Vivado is installed at the default location, then the next command will properly configure system variables:

.. code-block:: shell-session

   $ . /opt/Xilinx/Vivado/2017.2/settings64.sh

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


ModelSim as provided for free from Altera is used to run simulations.
Scripts expect the default install location.
On Ubuntu the inslall process fails to create an appropriate path to executable files,
so this path must be created:

.. code-block:: shell-session

   $ ln -s $HOME/intelFPGA/16.1/modelsim_ase/linux $HOME/intelFPGA/16.1/modelsim_ase/linux_rh60
   $ sudo apt install libxft2:i386

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

   
.. _devicetree:   

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
which allow for observing only positive signals with a saturation range of **0V ~ 1V**.
There are additional voltage dividers use to extend this range up to the power supply voltage.
It is possible to configure XADC inputs into a bipolar mode with a range of **-0.5V ~ +0.5V**,
but it requires removing R273 and providing a **0.5V ~ 1V** common voltage on the E2 connector.

.. note::

   Unfortunately there is a design error,
   where the XADC input range in unipolar mode was thought to be **0V ~ 0.5V**.
   Consequently the voltage dividers were miss designed for a range of double the supply voltage.

~~~~~~~~~~~~~~~
5V power supply
~~~~~~~~~~~~~~~

.. code-block:: none

                          ----------------0  Vout
             ----------  |  ----------
   Vin  0----| 56.0kΩ |-----| 4.99kΩ |----0  GND
             ----------     ----------

.. math::

   ratio = \frac{4.99 k\Omega}{56.0 k\Omega +4.99 k\Omega} = 0.0818

   range = \frac{1 V}{ratio} = 12.2 V

~~~~~~~~~~~~~~~~~~~~~~
General purpose inputs
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: none

                          ----------------0  Vout
             ----------  |  ----------
   Vin  0----| 30.0kΩ |-----| 4.99kΩ |----0  GND
             ----------     ----------
   
.. math::

   ratio = \frac{4.99 k\Omega}{30.0 k\Omega + 4.99  k\Omega} = 0.143

   range = \frac{1 V}{ratio} = 7.01 V

=============
GPIO and LEDs
=============

Handling of GPIO and LED signals depends on wether they are connected to
Zynq-7000 PS (MIO) or PL (EMIO or FPGA) block.

MIO pins signals are controlled by the PS block.
Each pin has a few multiplexed functions.
The multiplexer, slew rate, and pullup resistor enable
can be be controlled using software usually with
device tree `pinctrl` code.
Xilinx also provides Linux drivers for all PS based peripherals,
so all MIO signals can be managed using Linux drivers.

Pins connected to the PL block require FPGA code to function.
If the pin signals are wired directly (in the FPGA sources)
from PS based EMIO signals to the FPGA pads,
then they can be managed using Linux drivers
intended for the PS block.

The default pin assignment for GPIO is described in the next table.

+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| FPGA   | connector  | GPIO               | MIO/EMIO index   | ``sysfs`` index              | comments, LED color, dedicated meaning    |
+========+============+====================+==================+==============================+===========================================+
|        |            |                    |                  |                              | green, *Power Good* status                |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
|        |            |                    |                  |                              | blue, FPGA programming *DONE*             |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
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

It is possible to modify MIO pin functionality
using device tree files during Linux bootup.
The listed files should be included in the main device tree.

This files can be modified into device tree overlays,
which can be used to modify MIO functionality at runtime.

+--------------------+------------------------------------------------------+
| device tree file   | description                                          |
+====================+======================================================+
| ``spi2gpio.dtsi``  | E2 connector, SPI1 signals are repurposed as GPIO    |
+--------------------+------------------------------------------------------+
| ``i2c2gpio.dtsi``  | E2 connector, I2C0 signals are repurposed as GPIO    |
+--------------------+------------------------------------------------------+
| ``uart2gpio.dtsi`` | E2 connector, UART1 signals are repurposed as GPIO   |
+--------------------+------------------------------------------------------+
| ``miso2gpio.dtsi`` | E2 connector, SPI1 MISO signal is repurposed as GPIO |
|                    | SPI can then only be used for writing (maybe 3-wire) |
+--------------------+------------------------------------------------------+

.. include:: regset.rst
