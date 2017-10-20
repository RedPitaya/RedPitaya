.. _os:

#############
Red Pitaya OS
#############

********
Overview
********

Executable scripts:

+---------------------+------------------------------------------------------------------------------+
| script              | description                                                                  |
+=====================+==============================================================================+
| ``image.sh``        | full SD card image build procedure (creates and formats partitions)          |
+---------------------+------------------------------------------------------------------------------+
| ``image-update.sh`` | update existing SD card image with new ``ecosystem_*.zip``                   |
+---------------------+------------------------------------------------------------------------------+
| ``image-fsck.sh``   | run FSCK on SD card image partitions (for images created from used DS cards) |
+---------------------+------------------------------------------------------------------------------+
| ``image-clean.sh``  | deprecated                                                                   |
+---------------------+------------------------------------------------------------------------------+

Scripts to be used in a ``chroot`` environment only:

.. note::

   If this scripts are executed on the host OS directly, they can cause serious damage.

+---------------------+-----------------------------------------------------------------------------------------------------+
| script              | description                                                                                         |
+=====================+=====================================================================================================+
| ``ubuntu.sh``       | Ubuntu bootstrap, locale, apt configuration, timezone, fake HW clock)                               |
+---------------------+-----------------------------------------------------------------------------------------------------+
| ``debian.sh``       | Debian bootstrap (**experimental**, WEB applications are not working)                               |
+---------------------+-----------------------------------------------------------------------------------------------------+
| ``tools.sh``        | tools for compiling software                                                                        |
+---------------------+-----------------------------------------------------------------------------------------------------+
| ``zynq.sh``         | HW support for ZYNQ chip (U-Boot, I2C, EEPROM, dtc, IIO, NE10?, GPIO, groups with HW access rights) |
+---------------------+-----------------------------------------------------------------------------------------------------+
| ``network.sh``      | systemd-networkd based wired/wireless network configuration and required tools (hostAP, supplicant) |
+---------------------+-----------------------------------------------------------------------------------------------------+
| ``redpitaya.sh``    | libraries required by ecosystem applications (boost, jpeg, json), install and enable services       |
+---------------------+-----------------------------------------------------------------------------------------------------+
| ``jupyter.sh``      | Jupyter with NumPy and SciPy                                                                        |
+---------------------+-----------------------------------------------------------------------------------------------------+
| ``tft.sh``          | X-server and XFCE                                                                                   |
+---------------------+-----------------------------------------------------------------------------------------------------+

The ``overlay`` directory contains configuration files which are individually installed onto the OS by scripts.

*************
Bootstrapping
*************

A short list of SD card image contents:

1. Debian/Ubuntu OS (Ext4 partition):
   - base operating system files
   - additional operating system applications and libraries
   - systemd services
   - most network configuration files
   - Jupyter work space
2. Ecosystem (Fat32 partition):
   1. Bare metal:
      - ``boot.bin`` file containing FSBL, FPGA bitstream, U-Boot
      - Linux kernel image, device tree files
      - alternative FPGA bitstreams and corresponding device tree overlays
   2. User space
      - Bazaar server (Nginx) and WEB applications
      - Red Pitaya API library
      - SCPI server

To build a functional *OS image* the *ecosystem* is required,
since without the ``boot.bin`` and the Linux kernel, the system will not start.
And to build the *ecosystem* the *OS image* is required,
since the user space applications are built inside a ``chroot`` environment
with an emulated ARM CPU.

Therefore the procedure for the first build is as follows:

1. Build the OS image without the ecosystem.
   This will create a ``redpitaya_OS_*.img`` SD card image, but without the ecosystem and therefore non functional.
   It will also create a ``redpitaya_OS_*.tar.gz`` file, to be used in the ``chroot`` environment.
2. Build the ``ecosystem_*.zip`` inside the ``chroot`` environment.
3. Combine ``redpitaya_OS_*.img`` with ``ecosystem_*.zip`` using:

   .. code-block:: shell-session

      OS/debian/image-update.sh redpitaya_OS_*.img ecosystem_*.zip

After finishing the bootstrapping procedure, either the ecosystem or the OS image can be built as needed.
The more common procedure would be to build a new ecosystem using an existing ``chroot`` environment,
and then replace the ecosystem in an existing SD card image with the new one.
The build procedure for a new SD card OS image can now be done in one step.
If an existing ``ecosystem_*.zip`` file is present in the project root while building the OS image,
it will be integrated and the result will be a fully functional SD card image.

************
Dependencies
************

Ubuntu 2016.04.2 was used to build Debian/Ubuntu SD card images for Red Pitaya.

The next two packages need to be installed on the host PC:

.. code-block:: shell-session

   $ sudo apt-get install debootstrap qemu-user-static

================
Ubuntu bootstrap
================

The next steps should be executed in the root directory of the Red Pitaya Git repository.

.. code-block:: shell-session

   $ git clone https://github.com/RedPitaya/RedPitaya.git
   $ cd RedPitaya

Run the next command to build the OS image. Root or ``sudo`` privileges are needed.
The code should be executed as the root user,
otherwise some configuration files will be placed into the wrong users home directory.

.. code-block:: shell-session

   $ sudo bash
   # OS/debian/image.sh
   # exit

:download:`image.sh <../../../OS/debian/image.sh>`  will create an SD card image with a name containing the current 
date and time. Two partitions are created a 128MB FAT32 partition for the ecosystem and a slightly less then 4GB Ext4 partition.

:download:`image.sh <../../../OS/debian/image.sh>` will call :download:`ubuntu.sh <../../../OS/debian/ubuntu.sh>`
which installs the base system and some additional packages. It also configures APT (Debian packaging system),
locales, hostname, timezone, file system table, U-boot and users (access to UART console).

:download:`ubuntu.sh <../../../OS/debian/ubuntu.sh>` also executes 
:download:`network.sh <../../../OS/debian/network.sh>` which creates a
``systemd-networkd`` based wired and wireless network setup. And it executes
:download:`redpitaya.sh <../../../OS/debian/redpitaya.sh>` which installs additional Debian packages (mostly libraries)
needed by Red Pitaya applications. :download:`redpitaya.sh <../../../OS/debian/redpitaya.sh>` also extracts 
``ecosystem*.zip`` (if one exists in the current directory) into the FAT partition.

Optionally (code can be commented out) :download:`ubuntu.sh <../../../OS/debian/ubuntu.sh>` also executes
:download:`jupyter.sh <../../../OS/debian/jupyter.sh>` and :download:`tft.sh <../../../OS/debian/tft.sh>` which provide 
additional functionality.

===========================
Red Pitaya ecosystem update
===========================

In case an ``ecosystem*.zip`` file was not available for the previous step,
it can be extracted later to the FAT partition (128MB) of the SD card.
In addition to Red Pitaya tools, this ``ecosystem_*.zip`` file contains a boot image (containing FPGA code),
a boot script (``u-boot.scr``) and the Linux kernel.

A script :download:`image-update.sh <../../../OS/debian/image-update.sh>` is provided for updating an existing image
to a newer ``ecosystem_*.zip`` file without making modifications to the ``ext4`` partition.

The script should be run with the image and ecosystem files as arguments:

.. code-block:: shell-session

   # ./image-update.sh redpitaya_ubuntu_*.img ecosystem*.zip

=================
File system check
=================

If the image creation involved multiple steps performed by the user,
for example some installation/setup procedure performed on a live Red Pitaya,
there is a possibility a file system might be corrupted.
The :download:`image-fsck.sh <../../../OS/debian/image-fsck.sh>` script performs a file system check without changing 
anything.

Use this script on an image before releasing it.

.. code-block:: shell-session

   # ./image-fsck.sh redpitaya_ubuntu_*.img

===================
Reducing image size
===================

.. note::

   This steps should only be performed on a live Red Pitaya board.
   If executed on the host OS, they can and will cause problems.

A cleanup can be performed to reduce the image size. Various things can be done to reduce the image size:

* remove unused software (this could be software which was needed to compile applications)
* remove unused source files (remove source repositories used to compile applications)
* remove temporary files
* zero out empty space on the partition

The next code only removes APT temporary files and zeros out the file system empty space.

.. code-block:: shell-session

   $ apt-get clean
   $ cat /dev/zero > zero.file
   $ sync
   $ rm -f zero.file
   $ history -c

************
Debian Usage
************

=======
Systemd
=======

Systemd is used as the init system and services are used to start/stop Red Pitaya applications/servers.
Service files are located in ``OS/debian/overlay/etc/systemd/system/*.service``.

+-------------------------+----------------------------------------------------------------------------------------------------+
| service                 | description                                                                                        |
+=========================+====================================================================================================+
| ``jupyter``             | Jupyter notebbok for Python development                                                            |
+-------------------------+----------------------------------------------------------------------------------------------------+
| ``redpitaya_scpi``      | SCPI server, is disabled by default, since it conflicts with WEB applications                      |
+-------------------------+----------------------------------------------------------------------------------------------------+
| ``redpitaya_nginx``     | Nginx based server, serving WEB based applications                                                 |
+-------------------------+----------------------------------------------------------------------------------------------------+

To start/stop a service, do one of the following:

.. code-block:: shell-session

   $ systemctl start service_name
   $ systemctl stop service_name

To enable/disable a service, so to determine if it will start at powerup, do one of the following:

.. code-block:: shell-session

   $ systemctl enable service_name
   $ systemctl disable service_name

To see the status of a specific service run:

.. code-block:: shell-session

   $ systemctl

---------
Debugging
---------

.. code-block:: shell-session

   $ systemd-analyze plot > /opt/redpitaya/www/apps/systemd-plot.svg
   $ systemd-analyze dot | dot -Tsvg > /opt/redpitaya/www/apps/systemd-dot.svg
