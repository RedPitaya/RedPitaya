Instructions for starting XFCE on the TFT display.

1. download OS image:
http://downloads.redpitaya.com/downloads/red_pitaya_OS-beta.img.zip

2. copy the image onto the SD card

3. remove all files from FAT partition on the SD card

4. extract ecosystem onto the SD card:

5. power up the board

6. SSH into the board and run the next code to install the graphical interface
```bash
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
```

7. over SSH start the X server
```bash
startx
```

+----------------+----------+--------+--------+----------+----------------------+
|                | function |    pin |  pin   | function |                      |
+================+==========+========+========+==========+======================+
|                | GND      | ``26`` | ``25`` | GND      |                      |
+----------------+----------+--------+--------+----------+----------------------+
|                | ADC_CLK- | ``24`` | ``23`` | ADC_CLK+ |                      |
+----------------+----------+--------+--------+----------+----------------------+
|                | GND      | ``22`` | ``21`` | GND      |                      |
+----------------+----------+--------+--------+----------+----------------------+
|                | AO[3]    | ``20`` | ``19`` | AO[2]    |                      |
+----------------+----------+--------+--------+----------+----------------------+
|                | AO[1]    | ``18`` | ``17`` | AO[0]    |            TFT_reset |
+----------------+----------+--------+--------+----------+----------------------+
|                | AI[3]    | ``16`` | ``15`` | AI[2]    |                      |
+----------------+----------+--------+--------+----------+----------------------+
|                | AI[1]    | ``14`` | ``13`` | AI[0]    |                      |
+----------------+----------+--------+--------+----------+----------------------+
|                | I2C_GND  | ``12`` | ``11`` | common   |                      |
+----------------+----------+--------+--------+----------+----------------------+
| unused         | I2C SDA  | ``10`` |  ``9`` | I2C_SCK  | SPI_SS[1], touch SS  |
+----------------+----------+--------+--------+----------+----------------------+
| touch pendown  | UART_RX  |  ``8`` |  ``7`` | UART_TX  |            TFT DC    |
+----------------+----------+--------+--------+----------+----------------------+
| SPI_SS[0]      | SPI_CS   |  ``6`` |  ``5`` | SPI_CLK  | SPI_CLK              |
+----------------+----------+--------+--------+----------+----------------------+
| SPI_MISO       | SPI_MISO |  ``4`` |  ``3`` | SPI_MOSI | SPI_MOSI             |
+----------------+----------+--------+--------+----------+----------------------+
|                | -4V      |  ``2`` |  ``1`` | +5V      |                      |
+----------------+----------+--------+--------+----------+----------------------+

TFT_reset
TFT_dc
touch SPI_SS
touch pendown
backlight

