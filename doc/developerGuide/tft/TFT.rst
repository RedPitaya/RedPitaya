#######################################
Interfacing SPI TFT displays with touch
#######################################

This document describes how to connect a
SPI interface based TFT display with touch support
to the :ref:`E2 <E2>` connector, without the need for specific FPGA code.
The given setup has advantages and drawbacks.

**PROS:**

* It uses only ``MIO`` signals so it can be used with any FPGA image.
* Only extension connector :ref:`E2 <E2>` is used.
* SPI is not wired through the FPGA so maximum clock speeds can be used.

**CONS:**

* MIO signals shared with SPI, I2C and UART are consumed.
  So this interfaces can not be used for other purposes.
* On board I2C EEPROM cna not be accessed.
  This might cause issues in programs which store
  calibration data in the EEPROM.
* Backlight control is not supported.

**************
Hardware setup
**************

=======
pinctrl
=======

It is possible to reconfigure **Zynq** MIO signals using the ``pinctrl`` kernel driver.
This TFT display setup takes advantage of this by repurposing SPI, I2C and UART signals
on the :ref:`E2 <E2>` connector as SPI and GPIO signals which are required by the TFT display interface.

.. |tft-E2| replace:: ``tft-E2.dtsi``
.. _tft-E2: dts/tft/tft-E2.dtsi

The reconfiguration is performed by including the |tft-E2|_ device tree.

+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
| SPI TFT+touch   | MIO | function |    pin |  pin   | function | MIO | SPI TFT+touch     |
+=================+=====+==========+========+========+==========+=====+===================+
|                 |     | GND      | ``26`` | ``25`` | GND      |     | GND               |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
|                 |     | ADC_CLK- | ``24`` | ``23`` | ADC_CLK+ |     |                   |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
|                 |     | GND      | ``22`` | ``21`` | GND      |     |                   |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
|                 |     | AO[3]    | ``20`` | ``19`` | AO[2]    |     |                   |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
|                 |     | AO[1]    | ``18`` | ``17`` | AO[0]    |     |                   |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
|                 |     | AI[3]    | ``16`` | ``15`` | AI[2]    |     |                   |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
|                 |     | AI[1]    | ``14`` | ``13`` | AI[0]    |     |                   |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
|                 |     | I2C_GND  | ``12`` | ``11`` | common   |     |                   |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
| TFT RESETn      | 51  | I2C SDA  | ``10`` |  ``9`` | I2C_SCK  | 50  | SPI_SSs[1], touch |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
| touch pendown   | 9   | UART_RX  |  ``8`` |  ``7`` | UART_TX  | 8   | TFT D/C           |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
| SPI_SSn[0], TFT | 13  | SPI_CS   |  ``6`` |  ``5`` | SPI_CLK  | 12  | SPI_SCLK          |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
| SPI_MISO        | 11  | SPI_MISO |  ``4`` |  ``3`` | SPI_MOSI | 10  | SPI_MOSI          |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+
|                 |     | -4V      |  ``2`` |  ``1`` | +5V      |     | +5V               |
+-----------------+-----+----------+--------+--------+----------+-----+-------------------+

Since some of the signals share the I2C bus which already contains an EEPROM,
there is a posibility there will be functional conflicts.
Although the probability of the I2C EEPROM going into an active state are low.
I2C devices only react after an I2C start condition is present on the bus.
The start condition requires both SDA and SCL signals to be low at the same time.
Here it is assumed TFT display RESETn (active low) will not be active
at the same time as the touch controller SPI SSn (active low) signal.

Attempst to access the I2C EEPROM will not interfere with the display,
but they will return a timeout.
This might (probably will) cause issues with applications
using the I2C EEPROM, for example calibration access from Osciloscope app.

There is no MIO pin left for backlight control,
the easiest solution is to hardwire the display backlight pin to VCC.

===============
SPI clock speed
===============

.. |ug585-Zynq-7000-TRM.pdf| replace:: Zynq TRM
.. _ug585-Zynq-7000-TRM.pdf: https://www.xilinx.com/support/documentation/user_guides/ug585-Zynq-7000-TRM.pdf

Only a limited set of SPI clock speeds can be set depending on
the clock driving the SPI controller.
The SPI controller itself provides only power of 2 clock divider options.
See the `Zynq TRM <ug585-Zynq-7000-TRM.pdf_>`_
(section *B.30 SPI Controller (SPI)* register ``BAUD_RATE_DIV``) for details.

The next table provides available frequencies for two SPI controller clock settings.
The maximum clock speed for this SPI controller is 50MHz.

+----------------------+------+------+------+------+-------+-------+-------+
| SPI controller clock | f/4  | f/8  | f/16 | f/32 | f/64  | f/128 | f/256 |
+======================+======+======+======+======+=======+=======+=======+
|             166.6MHz | 41.6 | 20.8 | 10.4 | 5.21 | 2.60  | 1.30  | 0.63  |
+----------------------+------+------+------+------+-------+-------+-------+
|             166.6MHz | 41.6 | 20.8 | 10.4 | 5.21 | 2.60  | 1.30  | 0.63  |
+----------------------+------+------+------+------+-------+-------+-------+
|             200.0MHz | 50.0 | 25.0 | 12.5 | 6.25 | 3.125 | 1.56  | 0.781 |
+----------------------+------+------+------+------+-------+-------+-------+

**************
Software setup
**************

Instructions for starting XFCE on the TFT display.

A set of Ubuntu/Debian packages should be installed:

.. code-block:: shell-session

   apt-get -y install \
     python3 python3-numpy build-essential libfftw3-dev python3-scipy \
     xfonts-base tightvncserver xfce4-panel xfce4-session xfwm4 xfdesktop4 \
     xfce4-terminal thunar gnome-icon-theme \
     xserver-xorg xinit xserver-xorg-video-fbdev

.. |99-fbdev.conf| replace:: ``/usr/share/X11/xorg.conf.d/99-fbdev.conf``
.. _99-fbdev.conf: /OS/debian/overlay/usr/share/X11/xorg.conf.d/99-fbdev.conf

An X11 configuration file should be added to the system :source:|99-fbdev.conf|_:

.. literalinclude:: /OS/debian/overlay/etc/udev/rules.d/95-ads7846.rules

Over SSH start the X server:

.. code-block:: shell-session

   startx

************************
Tested/Supported devices
************************

The next table lists supported devices
and coresponding of device tree files
each supporting a set of displays depending on the used TFT and touch drivers.

+---------------+-------------------------------+-----------------------------------+-------------------------+
|               | specifications                | technical details                 | devicetree              |
|               +------+------------+-----------+----------------+------------------+                         |
| screen name   | size | resolution | touch     | TFT controller | touch controller |                         |
+===============+======+============+===========+================+==================+=========================+
| |MI0283QT-2|_ | 2.8" | 240x320    |           | |ILI9341|_     | |ADS7846|_       | |tft-ili9341-ads7846|_  |
+---------------+------+------------+-----------+----------------+------------------+-------------------------+
| |PiTFT-35|_   | 3.5" | 480x320    | resistive | |HX8357D|_     | |STMPE610|_      | |tft-hx8357d-stmpe601|_ |
+---------------+------+------------+-----------+----------------+------------------+-------------------------+

========================
MI0283QT Adapter Rev 1.5
========================

.. |MI0283QT-2| replace:: MI0283QT Adapter Rev 1.5
.. _MI0283QT-2: https://github.com/watterott/MI0283QT-Adapter

.. |ILI9341| replace:: ILI9341
.. _ILI9341: https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf

.. |ADS7846| replace:: ADS7846
.. _ADS7846: http://www.ti.com/lit/ds/symlink/ads7846.pdf

.. |tft-ili9341-ads7846| replace:: ``tft-ili9341-ads7846.dtsi``
.. _tft-ili9341-ads7846: /fpga/dts/tft/tft-ili9341-ads7846.dtsi

The device is powered by **+5V**,
and it generates 3.3V using an onboard LDO.
Therefore all IO are 3.3V, so there are no conflicts.

Connector pinout based on the |MI0283QT-2|_
`schematic <https://github.com/watterott/MI0283QT-Adapter/blob/master/hardware/MI0283QT_v15.pdf>`_.

+-------------------+-----------+--------+--------+-----------+-------------------+
| SPI TFT+touch     |           |    pin |  pin   |           | SPI TFT+touch     |
+===================+===========+========+========+===========+===================+
|                   | ADS_VREF  | ``16`` | ``15`` | ADS_VBAT  |                   |
+-------------------+-----------+--------+--------+-----------+-------------------+
|                   | ADS_AUX   | ``14`` | ``13`` | ADS_IRQ   | touch pendown     |
+-------------------+-----------+--------+--------+-----------+-------------------+
| TFT D/C           | BUSY-RS   | ``12`` | ``11`` | A-ADS_CS  | SPI_SSs[1], touch |
+-------------------+-----------+--------+--------+-----------+-------------------+
| SPI_SCLK          | A-SCL     | ``10`` |  ``9`` | SDO       | SPI_MISO          |
+-------------------+-----------+--------+--------+-----------+-------------------+
| SPI_MOSI          | A-SDI     |  ``8`` |  ``7`` | A-LCD_CS  | SPI_SSn[0], TFT   |
+-------------------+-----------+--------+--------+-----------+-------------------+
| TFT RESETn        | A-LCD_RST |  ``6`` |  ``5`` | LCD_LED   | backlight         |
+-------------------+-----------+--------+--------+-----------+-------------------+
| +5V               | VCC       |  ``4`` |  ``3`` | VCC       |                   |
+-------------------+-----------+--------+--------+-----------+-------------------+
| GND               | GND       |  ``2`` |  ``1`` | GND       |                   |
+-------------------+-----------+--------+--------+-----------+-------------------+

Backlight control is not available on the :ref:`E2 <E2>` connector.
A simple solution is to connect the **LCD_LED** signal
to +5V VCC, this can be done with a simple jumper
between the two display connector pins.
Otherwise it would be possible to repurpose a LED on Red Pitaya.

.. |95-ads7846.rules| replace:: ``/etc/udev/rules.d/95-ads7846.rules``
.. _95-ads7846.rules: /OS/debian/overlay/etc/udev/rules.d/95-ads7846.rules

The |95-ads7846.rules|_ UDEV rule will create a symbolik link ``/dev/input/touchscreen``.

.. literalinclude:: /OS/debian/overlay/etc/udev/rules.d/95-ads7846.rules

==============================
Adafruit PiTFT 3.5" (original)
==============================

.. |PiTFT-35| replace:: Adafruit PiTFT 3.5" Touch Screen for Raspberry Pi
.. _PiTFT-35: https://learn.adafruit.com/adafruit-pitft-3-dot-5-touch-screen-for-raspberry-pi

.. |HX8357D| replace:: HX8357D
.. _HX8357D: https://cdn-shop.adafruit.com/datasheets/HX8357-D_DS_April2012.pdf

.. |STMPE610| replace:: STMPE610
.. _STMPE610: https://cdn-shop.adafruit.com/datasheets/STMPE610.pdf

.. |tft-hx8357d-stmpe601| replace:: ``tft-hx8357d-stmpe601.dtsi``
.. _tft-hx8357d-stmpe601: /fpga/dts/tft/tft-hx8357d-stmpe601.dtsi

The device is powered by **+5V**,
and it generates 3.3V using an onboard LDO.
Therefore all IO are 3.3V, so there are no conflicts.

Male connector pinout based on the |PiTFT-35|_
`schematic <https://cdn-learn.adafruit.com/assets/assets/000/019/763/original/adafruit_products_schem.png?1411058465>`__.

+-------------------+--------+--------+-------------------+
| SPI TFT+touch     |    pin |  pin   | SPI TFT+touch     |
+===================+========+========+===================+
| SPI_SSs[1], touch | ``26`` | ``25`` | GND               |
+-------------------+--------+--------+-------------------+
| SPI_SSn[0], TFT   | ``24`` | ``23`` | SPI_SCLK          |
+-------------------+--------+--------+-------------------+
| TFT D/C           | ``22`` | ``21`` | SPI_MISO          |
+-------------------+--------+--------+-------------------+
| GND               | ``20`` | ``19`` | SPI_MOSI          |
+-------------------+--------+--------+-------------------+
| touch pendown     | ``18`` | ``17`` |                   |
+-------------------+--------+--------+-------------------+
|                   | ``16`` | ``15`` |                   |
+-------------------+--------+--------+-------------------+
| GND               | ``14`` | ``13`` |                   |
+-------------------+--------+--------+-------------------+
|                   | ``12`` | ``11`` |                   |
+-------------------+--------+--------+-------------------+
|                   | ``10`` |  ``9`` | GND               |
+-------------------+--------+--------+-------------------+
|                   |  ``8`` |  ``7`` |                   |
+-------------------+--------+--------+-------------------+
| GND               |  ``6`` |  ``5`` |                   |
+-------------------+--------+--------+-------------------+
|                   |  ``4`` |  ``3`` |                   |
+-------------------+--------+--------+-------------------+
| +5V               |  ``2`` |  ``1`` |                   |
+-------------------+--------+--------+-------------------+

.. |95-stmpe.rules| replace:: ``/etc/udev/rules.d/95-stmpe.rules``
.. _95-stmpe.rules: /OS/debian/overlay/etc/udev/rules.d/95-stmpe.rules

The |95-stmpe.rules|_ UDEV rule will create a symbolik link ``/dev/input/touchscreen``.

.. literalinclude:: /OS/debian/overlay/etc/udev/rules.d/95-stmpe.rules


.. |99-calibration.conf| replace:: ``/etc/X11/xorg.conf.d/99-calibration.conf``
.. _99-calibration.conf: /OS/debian/overlay/etc/X11/xorg.conf.d/99-calibration.conf

A calibration file should be added to the system |99-calibration.conf|_:

.. literalinclude:: /OS/debian/overlay/usr/share/X11/xorg.conf.d/99-fbdev.conf

------------------------
Graphical representation
------------------------

.. figure:: img/TFT_connection.svg
  
    Graphical representation of how to connect Red Pitayas :ref:`E2 <E2>` connetor to the Adafruit PiTFT 3.5".

.. figure:: img/TFT_connection-table.svg
  
    Simplified graphical representation of Red Pitayas :ref:`E2 <E2>` connetor to the Adafruit PiTFT 3.5". For pin
    locations please look at the top picture.

*************************
Debugging/Troubleshooting
*************************

================================
``pinctrl``, GPIO and interrupts
================================

To see current ``pinctrl`` settings try:

.. code-block:: shell-session

   $ cat /sys/kernel/debug/pinctrl/pinctrl-maps

To see the status of GPIO signals try:

.. code-block:: shell-session

   $ cat /sys/kernel/debug/gpio

To see the status of interrupts try:

.. code-block:: shell-session

   $ cat /proc/interrupts

=====
Touch
=====

``evtest`` can be used to see lowlevel touch events (and keyboard/mouse):

.. code-block:: shell-session

   sudo apt-get install -y evtest
