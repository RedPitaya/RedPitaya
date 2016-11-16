*************
Debug console
*************

The debug console can be used to follow the boot process:

1. FSBL (if debug mode is enabled)

   The serial console can also be used to see the output
   of other bare metal applications, for example the memory test.

2. U-Boot

   During the boot process U-Boot will show status and debug information.

   After FSBL starts U-Boot, there is a 3 second delay
   before U-Boot starts the linux kernel.
   If during this time a key is pressed,
   U-boot will stop the boot process
   and give the user access to its shell.

3. Linux console

   During the boot process Linux will show status and debug information.

   When ``systemd`` reaches ``multi-user.target`` a login prompt will appear.

      User name: ``root``
      Password: ``root``

==============
Hardware setup
==============

.. note::

   For STEMLab 125-14 you need additional USB to microUSB cable,
   for STEMLab 125-10 additional serial to USB adapter.

.. image:: console-connector.png

Connect your Red Pitaya and PC with micro USB B to USB A cable and follow the instructions for your OS.

.. image:: pitaya-USB-connection-300x164.png

-------
Windows
-------

Download and install the `FTD driver <http://www.ftdichip.com/Drivers/VCP.htm>`_ to your PC. After installation, a new
COM port will appear in the Device Manager you can use in Hyperterminal or another terminal utility to connect to Red 
Pitaya.

-----
Linux
-----

.. code-block:: shell-session

   $ sudo screen /dev/ttyUSB1 115200 cs8 ixoff

=======================
Reference boot sequence
=======================

You can compare this reference boot sequences agains yours.

------
U-Boot
------

.. code-block:: shell-session

   U-Boot 2016.01 (Nov 16 2016 - 12:23:28 +0100), Build: jenkins-redpitaya-master-156
   
   Model: Red Pitaya Board
   Board: Xilinx Zynq
   I2C:   ready
   DRAM:  ECC disabled 480 MiB
   I2C:EEPROM selection failed
   MMC:   sdhci@e0100000: 0
   In:    serial@e0000000
   Out:   serial@e0000000
   Err:   serial@e0000000
   Model: Red Pitaya Board
   Board: Xilinx Zynq
   Net:   ZYNQ GEM: e000b000, phyaddr 1, interface rgmii-id
   eth0: ethernet@e000b000
   Hit any key to stop autoboot:  0
   Running script from SD...
   Device: sdhci@e0100000
   Manufacturer ID: 19
   OEM: 4459
   Name: 00000
   Tran Speed: 25000000
   Rd Block Len: 512
   SD version 1.0   
   High Capacity: Yes
   Capacity: 3.7 GiB
   Bus Width: 4-bit 
   Erase Group Size: 512 Bytes
   reading u-boot.scr
   1203 bytes read in 17 ms (68.4 KiB/s)
   ## Executing script at 02000000
   Set devicetree and ramdisk high loading address to 0x20000000
   Loading from SD card (FAT file system) to memory
   Device: sdhci@e0100000
   Manufacturer ID: 19
   OEM: 4459
   Name: 00000
   Tran Speed: 25000000
   Rd Block Len: 512
   SD version 1.0   
   High Capacity: Yes
   Capacity: 3.7 GiB
   Bus Width: 4-bit 
   Erase Group Size: 512 Bytes
   reading u-boot.scr
   1203 bytes read in 17 ms (68.4 KiB/s)
   ## Executing script at 02000000
   Set devicetree and ramdisk high loading address to 0x20000000
   Loading from SD card (FAT file system) to memory
   Device: sdhci@e0100000
   Manufacturer ID: 19
   OEM: 4459
   Name: 00000
   Tran Speed: 25000000
   Rd Block Len: 512
   SD version 1.0   
   High Capacity: Yes
   Capacity: 3.7 GiB
   Bus Width: 4-bit 
   Erase Group Size: 512 Bytes
   reading uImage   
   4590664 bytes read in 404 ms (10.8 MiB/s)
   reading devicetree.dtb
   17342 bytes read in 19 ms (890.6 KiB/s)
   Booting Linux kernel with ramdisk and devicetree
   ## Booting kernel from Legacy Image at 02004000 ...
      Image Name:   Linux-4.4.0-xilinx
      Image Type:   ARM Linux Kernel Image (uncompressed)
      Data Size:    4590600 Bytes = 4.4 MiB
      Load Address: 00008000
      Entry Point:  00008000
      Verifying Checksum ... OK
   ## Flattened Device Tree blob at 04000000
      Booting using the fdt blob at 0x4000000
      Loading Kernel Image ... OK
      Loading Device Tree to 1d33c000, end 1d3433bd ... OK
 
