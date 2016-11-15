###############
Troubleshooting
###############

**********************************
Problems after upgrading to new OS
**********************************

* licensing is not working
* status always shows offline
* OS version information is not updated
* other malfunctions of the OS and applications

Solution:

   Force refresh of the Red Pitaya application page

What if this doesn't help?

   Clear your browser cache and history.

*********************************
Problems connecting to Red Pitaya
*********************************

#. First check the LEDs:


   image:
   If Green LED is not ON or it is blinking.
   Seems like something is wrong with the power supply or maybe it’s USB cable.
   Make sure that:

      * you have plugged the USB cable into the right USB connector on Red Pitaya
      * your power supply is 5V/2A
      * try to replace USB cable and also USB power supply

   If Green LED is ON, but Blue LED is not.
   In this case there is an error while loading Red Pitaya system from the SD card. Make sure that:

      * you have properly inserted Red Pitaya SD card and that it has properly installed Red Pitaya OS
      * try to use another SD card

   If Green and blue LEDs are on, but red and orange LEDs are not blinking.
   Red LED is indicating CPU heartbeat, while orange LED indicates access to SD card.
   Notice that this two LEDs always starts blinking 10s after green and blue LEDs are turned ON.

#. Make sure your Red Pitaya and computer are connected to same local network


#. If you are Windows users make sure you have installed bonjur.

**********************************
Problems with slow WIFI connection
**********************************

If your wireless connection with Red Pitaya works very slowly and
all the applications seems very unresponsive and are not running smoothly,
please check the following:

* check the wifi signal strength on your PC/tablet/smartphone
* check the wifi signal strength of your Red Pitaya.

   #. Connect to your Red Pitaya via SSH connection. `How? <>`_

   #. Enter ``cat /proc/net/wireless`` command in order to get
      information about link quality and signal strength.

      .. image:: Screen-Shot-2015-09-26-at-20.28.27.png

      Link quality measures the number of packet errors that occur.
      The lower the number of packet errors, the higher this will be.
      Link quality goes from 0-100%.

      Level or signal strength is a simple measure of the amplitude of the signal that is received.
      The closer you are to the access point, the higher this will be.

* If you are in the area with many routers around you
  it might happen that more of them operate at the same wifi channel
  which drastically decreases data throughput and slows down connection.
  Here are the instructions how to
  `change your wifi router channel in order to optimize your wireless signal
  <http://www.howtogeek.com/howto/21132/change-your-wi-fi-router-channel-to-optimize-your-wireless-signal/>`_.
  For MAC users we recommend using diagnosed using Scan feature of
  `Wireless diagnostic <http://www.howtogeek.com/211034/troubleshoot-and-analyze-your-mac%E2%80%99s-wi-fi-with-the-wireless-diagnostics-tool/>`_
  tool in order to find best wifi channel.

**********************************************************************
Problems with upgradin OS, marketplace access or application unlocking
**********************************************************************

Make sure your Red Pitaya has access to the internet.
