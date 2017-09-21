~~~~~~~~~~~~~~~~~~~~~~~~~~~~
General purpose input output
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

=====
GPIOs
=====

This document introduses handling of GPIO signals that are conected to Zynq-7000 PS EMIO block
and is accesible as general purpose input / output pins on Extension conector E1 with Linux gpio subsystem userspace interfaces.

There are two interfaces legacy sysfs interface and new character device based one.

====
PINS
====

Pins connected to the PL block require FPGA code to function. If the pin signals are wired directly (in the FPGA sources) from PS based EMIO signals to the FPGA pads, 
then they can be managed using Linux drivers intended for the PS block. This is currently done with two fpga projects: classic and mercury.

Apropriate fpga bitstream can be applied using "cat /opt/redpitaya/fpga/classic/fpga.bit > /dev/xdevcfg" bash command. 

There are 54+64=118 GPIO provided by ZYNQ PS, MIO provides 54 GPIO,
and EMIO provide additional 64 GPIO and only 16 out of those are accesible on board. 
On Extension connector E1; pins from DIO0_N to DIO7_N and DIO0_P to DIO7_P.

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

The default pin assignment for GPIO is described in the next table.

+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
| FPGA   | connector  | GPIO               | MIO/EMIO index   | ``sysfs`` index              | comments, LED color, dedicated meaning    |
+========+============+====================+==================+==============================+===========================================+
|        |            | ``exp_p_io [7:0]`` | ``EMIO[15: 8]``  | ``906+54+[15: 8]=[975:968]`` |  DIO7_P : DIO0_P                          |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+
|        |            | ``exp_n_io [7:0]`` | ``EMIO[23:16]``  | ``906+54+[23:16]=[983:976]`` |  DIO7_N : DIO0_N                          |
+--------+------------+--------------------+------------------+------------------------------+-------------------------------------------+



====================
Linux access to GPIO
====================

************
SYSFS access
************

This document is used as reference:
`Linux+GPIO+Driver <http://www.wiki.xilinx.com/Linux+GPIO+Driver>`_



Bash example for writing to and reading from gpio value for pins from 968(DIO0_P) to 983(DIO7_N).


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
   
 

SYSFS GPIO C example is available at github: https://github.com/RedPitaya/RedPitaya/tree/master/Examples/gpio_sysfs


***********************
Character device access
***********************

Character device usersace access to gpio kernel subsystem is confirmed working on kernels newer and including 4.8.

References:

http://elinux.org/images/9/9b/GPIO_for_Engineers_and_Makers.pdf
https://www.youtube.com/watch?v=lQRCDl0tFiQ

The Linux kernel contains GPIO utilities in its ``tools`` directory.

https://github.com/torvalds/linux/tree/master/tools/gpio

We isolated the sources and created a library from ``gpio-utils.c`` and
executables from other source files.

https://github.com/RedPitaya/gpio-utils
