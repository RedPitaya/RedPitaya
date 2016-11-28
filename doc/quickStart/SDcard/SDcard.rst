###############
Prepare SD card
###############

**********
Background
**********

A Red Pitaya SD card contains two partitions:

1. 128MB FAT contains the **ecosystem**

   * boot files: FSBL, FPGA images, U-Boot, Linux kernel
   * Red Pitaya API libraries and header files
   * Red Pitaya web applications, scripts, tools
   * customized Nginx web server

2. ~4GB Ext4 contains the **OS**

   * Ubuntu/Debian OS
   * various libraries
   * network setup customization
   * systemd services customization

Most of Red Pitaya source code translates into the ecosystem,
Therefore this is updated more often.
The OS is changed less frequently.

.. note::

   You can find older and development Red Pitaya OS images and Ecosystem zipfiles
   on our `download server <http://downloads.redpitaya.com/downloads/>`_.

   http://downloads.redpitaya.com/downloads/

.. note::

   A list of new features, bugfixes and known bugs for each Red Pitaya release
   can be found in our `CHANGELOG <https://github.com/RedPitaya/RedPitaya/blob/master/CHANGELOG.md>`_.

**********************************
Download and install SD card image
**********************************

The next procedure will create a clean SD card.

#. Download the Red Pitaya SD card image
   `stable <http://downloads.redpitaya.com/downloads/red_pitaya_OS-stable.img.zip>`_ or
   `beta <http://downloads.redpitaya.com/downloads/red_pitaya_OS-beta.img.zip>`_.

   Stable: http://downloads.redpitaya.com/downloads/red_pitaya_OS-stable.img.zip
   Beta: http://downloads.redpitaya.com/downloads/red_pitaya_OS-beta.img.zip
    
   .. image:: microSDcard-RP.png
      :width: 10%

#. Unzip the SD card image.

#. Write the image onto a SD card. Instructions are available for various operating systems:

.. contents::
    :local:
    :backlinks: none
    :depth: 1

   This instructions are based on similar instructions for
   `Raspberry Pi <https://www.raspberrypi.org/documentation/installation/installing-images/>`_.

#. Insert the SD card into Red Pitaya.

   .. image:: pitaya-quick-start-insert-sd-card.png


=======
Windows
=======

#. Insert SD card into your PC or SD card reader.

   .. image:: SDcard_insert.jpg

#. Download `Win32 Disk Imager <https://sourceforge.net/projects/win32diskimager/>`_ and extract it.

   .. image:: SDcard_Win_Win32DiskImager.png

#. Open unzipped folder, right-click on the ``WinDisk32Imager``, and select **Run as Administrator**.

   .. image:: SDcard_Win_RunAsAdmin.png

#. Under image file box select unzipped Red Pitaya image file.

   .. image:: SDcard_Win_SelectImg.png

#. Under device box select the drive letter of the SD card.

   .. image:: SDcard_Win_SelectDrive.png

   .. note::

      Be careful to select the correct drive.
      If you choose the wrong one you risk erasing data
      from the computer's hard disk!
      You can easily see the drive letter (for example E:)
      by looking in the left column of Windows Explorer.

   .. image:: SDcard_Win_DriveLetter.png

#. Click Write and wait for the write to complete.

   .. image:: SDcard_Win_Write.png

#. Exit the Imager.

   .. image:: SDcard_Win_Exit.png

=====
Linux
=====

.. _linux_gui:

-------------------------
Ubuntu using Image Writer
-------------------------

#. Right click on the extracted SD card image and select **Open With > Disk Image Writer**.

#. In the **Restore Disk Image** window select your SD card in the **Destination** pull down menu.
   Be carefull to select the correct device, use the size for orientation (for example 4GB SD card).

#. You will be asked to confirm your choice and enter a password.
   Additiona dialog windows will again show the selected destination drive,
   take the oportunity to think again if you choose the right device.


.. _linux_cli:

------------
Command line
------------

.. note::
   Please note that the use of the ``dd`` tool can overwrite any partition of your machine.
   If you specify the wrong device in the instructions below, you could delete your primary Linux partition.
   Please be careful.

#. Insert SD card into your PC or SD card reader.

   .. image:: SDcard_insert.jpg 

#. Open the Terminal and check the available disks with ``df -h``.
   Our SD card is 4GB, it is named ``/dev/sdx`` and
   divided into two partitions ``/dev/sdx1`` and ``/dev/sdx2``.
   The drive mounted at ``/`` is your main drive,
   be carefull not to use it.

   .. code-block:: shell-session

      $ df -h
      Filesystem      Size  Used Avail Use% Mounted on
      /dev/sdx1       118M   27M   92M  23% /media/somebody/CAD5-1E3D
      /dev/sdx2       3.2G 1013M  2.1G  33% /media/somebody/7b2d3ba8-95ed-4bf4-bd67-eb52fe65df55

#. Unmount all SD card partitions with ``umount /dev/sdxN``
   (make sure you replace N with the right numbers).

   .. code-block:: shell-session

      $ sudo umount /dev/sdx1 /dev/sdx2

#. Write the image to the SD card with the following command.
   Replace the ``red_pitaya_image_file.img`` with
   the name of the unzipped Red Pitaya SD Card Image
   and replace ``/dev/device_name`` with the path to the SD card.

   .. code-block:: shell-session

      $ sudo dd bs=1M if=red_pitaya_image_file.img of=/dev/device_name

#. Wait until the process has finished.


=====
macOS
=====

.. _macos_gui:

-------------------
Using ApplePi-Baker
-------------------
    
#. Insert SD card into your PC or SD card reader.

   .. image:: SDcard_insert.jpg

#. Download `ApplePi-Baker <http://www.tweaking4all.com/software/macosx-software/macosx-apple-pi-baker/>`_ and extract it.

   .. image:: SDcard_macOS_ApplePi-Baker.png

#. Press **crtl** key and click on *ApplePi-Baker* icon, then click *Open* in order to run it.

   .. image:: SDcard_macOS_open.png

#. Enter your admin password and click OK.

   .. image:: SDcard_macOS_password.png

#. Select SD card drive. This can be recognized by the size of the card that is 4GB.

   .. image:: SDcard_macOS_ApplePi-Baker_drive.png

#. Select Red Pitaya OS image file.

   .. image:: SDcard_macOS_ApplePi-Baker_image.png

#. Click "Restore Backup" button in order to write image to SD card.

   .. image:: SDcard_macOS_ApplePi-Baker_restore.png

#. It's coffee time, application will show you Estimated Time for Accomplishment.

   .. image:: SDcard_macOS_ApplePi-Baker_wait.png

#. When operation is completed click "OK" and quit ApplePi-Baker.

   .. image:: SDcard_macOS_ApplePi-Baker_quit.png


.. _macos_cli:

------------
Command line
------------

#. Insert SD card into your PC or SD card reader.

   .. image:: SDcard_insert.jpg

#. Click **cmd + space**, type **Disk Utility** into the search box and press enter.
   From the menu select your SD card and click on **Erase** button (be careful not to delete your disk!).

   .. image:: SDcard_macOS_DiskUtility.png

#. Click **cmd + space**, type in **Terminal** and press enter.
   In the terminal window type: ``cd``, press enter,
   then type: ``cd Desktop`` and press enter again.

#. Unmount the partition so that you will be allowed to overwrite the disk.
   In Terminal type: ``diskutil list`` and press enter.
   This will show you the list of all memory devices.

   .. image:: Screen-Shot-2015-08-07-at-16.59.50.png

   Unmount with: ``diskutil UnmountDisk /dev/diskn``
   (insert the number ``n`` of your disk correctly!)

   .. image:: Screen-Shot-2015-08-07-at-17.14.34.png

#. Type in: ``sudo dd bs=1m if=path_of_your_image.img of=/dev/rdiskn``
   (Remember to replace ``n`` with the number that you noted before!)
   (notice there is letter ``r`` in front of the disk name, use that as well!)

   .. image:: Screen-Shot-2015-08-07-at-17.14.45.png

#. Type in your password and wait a few minutes for the image to be written.

#. When the image is written, type: ``diskutil eject /dev/diskn`` and press enter.

#. Safely eject the SD card.


#############################
Upgrading Red Pitaya software
#############################

Instead of writing the whole SD card image,
it is possible to upgrade only the ecosystem.

=====================
Web interface upgrade
=====================

At boot Red Pitaya checks for software updates,
and alerts the user if a new release is available.
Users can also check for updates manually.

#. Open Red Pitaya desktop using your WEB browser.

#. Click on the **upgrade alert** or on the **ecosystem version label** in bottom right corner.

   .. image:: SDcard_upgrade_desktop.png

#. Delect ecosystem version and start OS updater

   .. image:: SDcard_update_manager.png

#. Follow the steps in the OS updater app in order to install new OS.
   Notice that OS upgrade might cause your Red Pitaya desktop to freeze for a few minutes.

==============
Manual upgrade
==============

A manual upgrade allows you to fix a corrupted SD card image
(if only the FAT partition is corrupted) or to install
older, newer or custom ecosystem zip files.

#. Download a zip file from our `download server <http://downloads.redpitaya.com/downloads/>`_.

#. Insert SD card into card reader.

#. Delete all files from the FAT partition.
   Use ``Shift + Delete`` to avoid placing files
   into a trash bin on the same partition.

#. Extract the ecosystem zip file contents onto the now empty partition.

If you wish too keep wireless settings skip deleting the next files:

* ``wpa_supplicant.conf``
* ``hostapd.conf``
