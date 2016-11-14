###############
Troubleshooting
###############

#. Problems after upgrading to new OS.

   * licensing is not working
   * status always shows offline
   * OS version information is not updated
   * other malfunctions of the OS and applications

   Solution:

      Force refresh of the Red Pitaya application page

   What if this doesn't help?

      Clear your browser cache and history.

#. I can’t connect to Red Pitaya.

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

#. WIFI connection is too slow

   FAQ

#. I cannot upgrade OS, access marketplace or unlock applications.

   Make sure your Red Pitaya has access to the internet.
