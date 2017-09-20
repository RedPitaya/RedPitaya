~~~~~~~~~~~~~~~~~~~~~~
General purpose inputs
~~~~~~~~~~~~~~~~~~~~~~

==============
GPIOs and LEDs
==============

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


========================
Linux access to GPIO/LED
========================

************
SYSFS access
************

This document is used as reference:
`Linux+GPIO+Driver <http://www.wiki.xilinx.com/Linux+GPIO+Driver>`_

There are 54+64=118 GPIO provided by ZYNQ PS, MIO provides 54 GPIO,
and EMIO provide additional 64 GPIO.

The next formula is used to calculate the ``gpio_base`` index.

.. code-block:: none

   base_gpio = ARCH_NR_GPIOS - ZYNQ_GPIO_NR_GPIOS = 1024 - 118 = 906

Values for the used macros can be found in the kernel sources.

.. code-block:: shell-session

   $ grep ZYNQ_GPIO_NR_GPIOS drivers/gpio/gpio-zynq.c
   #define	ZYNQ_GPIO_NR_GPIOS	118
   $ grep -r CONFIG_ARCH_NR_GPIO tmp/linux-xlnx-xilinx-v2017.2
   tmp/linux-xlnx-xilinx-v2017.2/.config:CONFIG_ARCH_NR_GPIO=1024

Another way to find the `gpio_base` index is to check the given name inside `sysfs`.

.. code-block:: shell-session

   # find /sys/class/gpio/ -name gpiochip*
   /sys/class/gpio/gpiochip906

GPIOs are accessible at the ``sysfs`` index.

Example for writing to and reading gpio value for EMIO[15: 8] and EMIO[23:16] pins on exp_n_io and exp_p_io on pins from 968 to 983.

.. code-block:: shell-session

   #export pin 968
   $ echo "968" > /sys/class/gpio/export
   #set direction to output
   $ echo "out" > /sys/class/gpio/gpio968/direction
   #set pin to LOW
   $ echo "0" > /sys/class/gpio/gpio968/value
   #set pin to HIGH
   $ echo "1" > /sys/class/gpio/gpio968/value
   #set pin direction to input
   $ echo "in" > /sys/class/gpio/gpio968/direction
   #output pin value
   $ cat /sys/class/gpio/gpio968/value
   #when done with pin you should unexport it with
   $ echo 968 > /sys/class/gpio/unexport
   
***********************
Character device access
***********************

References:

http://elinux.org/images/9/9b/GPIO_for_Engineers_and_Makers.pdf
https://www.youtube.com/watch?v=lQRCDl0tFiQ

The Linux kernel contains GPIO utilities in its ``tools`` directory.

https://github.com/torvalds/linux/tree/master/tools/gpio

We isolated the sources and created a library from ``gpio-utils.c`` and
executables from other source files.

https://github.com/RedPitaya/gpio-utils
