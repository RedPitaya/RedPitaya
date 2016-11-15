*************
Debug console
*************

This type of connection is used for direct USB serial console connectivity.
Here user access STEMLab board via console/terminal.
This is useful for the developers and more demanding users.
Connecting to the STEMLab board via USB will open Linux Terminal
where user can control STEMLab board completely using command line tools.

.. note::

   For STEMLab 125-14 you need additional USB to microUSB cable,
   for STEMLab 125-10 additional serial to USB adapter.

.. image:: connect-17.png

1. Connect your Red Pitaya and PC with micro USB B to USB A cable and follow the instructions for your OS.

.. image:: pitaya-USB-connection-300x164.png

Serial console connection is independent from the Ethernet connection.
Use a Micro USB cable to connect your computer with Red Pitaya.
Connection instructions will be given for Windows, Linux and OS X users separately.

Serial port configuration:

.. image:: Selection_002.png

**User name and password to login are “root”.**

=============
Windows users
=============

Download and install the `FTD driver <http://www.ftdichip.com/Drivers/VCP.htm>`_ to your PC. After installation, a new
COM port will appear in the Device Manager you can use in Hyperterminal or another terminal utility to connect to Red 
Pitaya.

`FAQ <http://redpitaya.com/faq/>`_    
