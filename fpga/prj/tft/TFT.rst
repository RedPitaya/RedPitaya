#######################################
Interfacing SPI TFT displays with touch
#######################################

This document describes how to connect a SPI interface based
TFT display with touch support.
The given setup has advantages and drawbacks.

**PROS:**

* It uses only ``MIO`` signals so it can be used with any FPGA image.
* Only extension connector **E2** is used.
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

It is possible to reconfigure **Zynq** MIO signals using the ``pinctrl`` kernel driver.
This TFT display setup takes advantage of this by repurposing SPI, I2C and UART signals
on the E2 connector as SPI and GPIO signals which are required by the TFT display interface.
The reconfiguration is performed by loading a *device tree*.

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

**************
Software setup
**************

Instructions for starting XFCE on the TFT display.

1. download OS image:
http://downloads.redpitaya.com/downloads/red_pitaya_OS-beta.img.zip

2. copy the image onto the SD card

3. remove all files from FAT partition on the SD card

4. extract ecosystem onto the SD card:

5. power up the board

6. SSH into the board and run the next code to install the graphical interface

.. code-block:: shell-session

   apt-get -y install fake-hwclock \
     python3 python3-numpy build-essential libfftw3-dev python3-scipy \
     xfonts-base tightvncserver xfce4-panel xfce4-session xfwm4 xfdesktop4 \
     xfce4-terminal thunar gnome-icon-theme \
     xserver-xorg xinit xserver-xorg-video-fbdev

   X11DIR=$root_dir/usr/share/X11/xorg.conf.d
   mkdir -p $X11DIR
   cat << EOF_FBDEV > $X11DIR/99-fbdev.conf
   Section "Device"  
     Identifier "myfb"
     Driver "fbdev"
     Option "fbdev" "/dev/fb0"
   EndSection

7. over SSH start the X server

.. code-block:: shell-session

   startx

************************
Tested/Supported devices
************************

For now a single device was tested.
As we try more devices, this will grow into a table of device tree files
each supporting a set of displays depending on the used TFT and touch drivers.

https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf


+--------------------------------+-------------------------------+-----------------------------------+
|                                | specifications                | technical details                 |
+--------------------------------+------+------------+-----------+----------------+------------------+
| name                           | size | resolution | touch     | TFT controller | touch controller |
+================================+======+============+===========+================+==================+
| MI0283QT Adapter Rev 1.5       | 2.8" | 240x320    |           | TI ADS7846     | TI ADS7846       |
+--------------------------------+------+------------+-----------+----------------+------------------+
| Adafruit PiTFT 3.5" (original) | 3.5" | 480x320    | resistive |                |                  |
+--------------------------------+------+------------+-----------+----------------+------------------+

========================
MI0283QT Adapter Rev 1.5
========================

Vendor        - http://www.watterott.com/de/MI0283QT-2-Adapter
Documentation - https://github.com/watterott/MI0283QT-Adapter

==============================
Adafruit PiTFT 3.5" (original)
==============================

`Adafruit PiTFT 3.5" Touch Screen for Raspberry Pi <https://learn.adafruit.com/adafruit-pitft-3-dot-5-touch-screen-for-raspberry-pi>`_

Instructions for: 

.. code-block:: shell-session

   sudo mkdir /etc/X11/xorg.conf.d

   sudo nano /etc/X11/xorg.conf.d/99-calibration.conf

   Section "InputClass"
   	Identifier      "calibration"
   	MatchProduct    "stmpe-ts"
   	Option  "Calibration"   "3800 120 200 3900"
   	Option  "SwapAxes"      "1"
   EndSection

