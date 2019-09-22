######################
General considerations
######################

#. follow best practices
#. no default passwords
#. no hardcoded passwords
#. user settings similar to Ubuntu (use ``sudo``, disabled ``root`` user)
#. avoid running applications as root
#. HW access requires group membership

Main issues:
#. backward compatibility
#. Fat32 does not provide necessary file attributes

###
UIO
###

http://elinux.org/images/b/b0/Uio080417celfelc08.pdf
https://lwn.net/Articles/232575/


###########################
Hardware access permissions
###########################

Debugging UDEV rules

.. code-block:: shell-session

   udevadm info -a /dev/xdevcfg
   udevadm info -a /dev/uio0
   udevadm info -a /sys/devices/soc0/led-user/leds/led0
   udevadm info -a /sys/devices/soc0/amba/e000a000.gpio/gpio
   udevadm info -a /dev/spidev1.0
   udevadm info -a /dev/i2c-0
   udevadm info -a /sys/bus/i2c/devices/0-0050
   udevadm info -a /dev/ttyPS1
   udevadm info -a /dev/iio\:device0
   udevadm info -a /dev/iio\:device1
   udevadm info -a /dev/rprx

#####
Users
#####

=====================================
System users for running applications
=====================================

==========================
``redpitaya`` default user
==========================

=============
``root`` user
=============

Aftert the first boot the ``root`` user does not have a password
and is therefore disabled.
Setting ``root`` user password enables access to the account:

.. code-block:: shell-session

   $ sudo passwd root
