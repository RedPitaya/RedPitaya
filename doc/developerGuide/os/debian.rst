.. os

#############
Red Pitaya OS
#############

For instructions on how to build the ecosystem, go to :ref:`ecosystem <ecosystem>`.

************
Dependencies
************

Ubuntu 2016.04.2 was used to build Debian/Ubuntu SD card images for Red Pitaya.

The next two packages need to be installed on the host PC:

.. code-block:: shell-session

   $ sudo apt-get install debootstrap qemu-user-static

*****************************
SD card image build Procedure
*****************************

Multiple steps are needed to prepare a proper SD card image.

1. Bootstrap Debian system with network configuration and Red Pitaya specifics.
2. Add Red Pitaya ecosystem ZIP.

================
Ubuntu bootstrap
================

.. |image.sh| replace:: ``image.sh``
.. _image.sh: /OS/debian/image.sh

.. |image-update.sh| replace:: ``image-update.sh``
.. _image-update.sh: /OS/debian/image-update.sh

.. |image-fsck.sh| replace:: ``image-fsck.sh``
.. _image-fsck.sh: /OS/debian/image-fsck.sh

.. |ubuntu.sh| replace:: ``ubuntu.sh``
.. _ubuntu.sh: /OS/debian/ubuntu.sh

.. |network.sh| replace:: ``network.sh``
.. _network.sh: /OS/debian/network.sh

.. |redpitaya.sh| replace:: ``redpitaya.sh``
.. _redpitaya.sh: /OS/debian/redpitaya.sh

.. |jupyter.sh| replace:: ``jupyter.sh``
.. _jupyter.sh: /OS/debian/jupyter.sh

.. |tft.sh| replace:: ``tft.sh``
.. _tft.sh: /OS/debian/tft.sh

Run the next command inside the project root directory. Root or ``sudo`` privileges are needed.

.. code-block:: shell-session

   $ sudo bash
   # OS/debian/image.sh
   # exit

:source:`OS/debian/image.sh` will create an SD card image with a name containing the current date and time.
Two partitions are created a 128MB FAT32 partition and a alightly less then 4GB Ext4 partition.

|image.sh|_ will call |ubuntu.sh|_ which installs the base system and some additional packages.
It also configures APT (Debian packaging system), locales, hostname, timezone,
file system table, U-boot and users (access to UART console).

|ubuntu.sh|_ also executes |network.sh|_ which creates a
``systemd-networkd`` based wired and wireless network setup.
And it executes |redpitaya.sh|_ which installs additional
Debian packages (mostly libraries) needed by Red Pitaya applications.
|redpitaya.sh|_ also extracts ``ecosystem*.zip``
(if one exists in the current directory) into the FAT partition.

Optionally (code can be commented out) |ubuntu.sh|_ also executes
|jupyter.sh|_ and |tft.sh|_ which provide additional functionality.

The generated image can be written to a SD card
using the ``dd`` command or the ``Disks`` tool (Restore Disk Image).

.. code-block:: shell-session

   $ dd bs=4M if=debian_armhf_*.img of=/dev/sd?

.. note::

   To get the correct destination storage device,
   read the output of ``dmesg`` after you insert the SD card.
   If the wrong device is specified, the content of another
   drive may be overwritten, causing permanent loose of user data.

===============================
Red Pitaya ecosystem extraction
===============================

In case an ``ecosystem*.zip`` file was not available for the previous step,
it can be extracted later to the FAT partition (128MB) of the SD card.
In addition to Red Pitaya tools, this ecosystem ZIP file contains a boot image (containing FPGA code),
a boot script (``u-boot.scr``) and the Linux kernel.

A script |image-update.sh|_ is provided for updating an existing image
to a newer ecosystem zippfile without making modifications to the ``ext4`` partition.

The script should be run with the image and ecosystem files as arguments:

.. code-block:: shell-session

   # ./image-update.sh redpitaya_ubuntu_*.img ecosystem*.zip

=================
File system check
=================

If the image creation involved multiple steps performed by the user,
for example some installation/setup procedure performed on a live Red Pitaya,
there is a possibility a file system might be corrupted.
The |image-fsck.sh|_ script performs a file system check without changing anything.

Use this script on an image before releasing it.

.. code-block:: shell-session

   # ./image-fsck.sh redpitaya_ubuntu_*.img

===================
Reducing image size
===================

A cleanup can be performed to reduce the image size. Various things can be done to reduce the image size:

* remove unused software (this could be software which was needed to compile applications)
* remove unused source files (remove source repositories used to compile applications)
* remove temporary files
* zero out empty space on the partition

The next code only removes APT temporary files and zeros out the filesystem empty space.

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
