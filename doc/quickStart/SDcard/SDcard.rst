###############
Prepare SD card
###############

This instructions are based on similar instructions for `Raspberry Pi <https://www.raspberrypi.org/documentation/installation/installing-images/>`_.

1. Download the Red Pitaya Image File
    
   .. image:: microSDcard-RP.png
      :width: 10%
   
   `Download <http://blog.redpitaya.com/quick-start/Login>`_
   
   - FAQ: `Where can I find more about Red Pitaya OS releases? <http://blog.redpitaya.com/faq-page/#Software|32740>`_
   - FAQ: `Where can I find old Red Pitaya OS & application relases? <http://blog.redpitaya.com/faq-page/#Software|25467>`_
    
2. Unzip

3. Select your operating system and follow the instructions:

   * :ref:`Windows <windows>`,
   * :ref:`Linux <linux>`,
   * :ref:`macOS <macos>`.

4. Insert SD card into Red Pitaya

   .. image:: pitaya-quick-start-insert-sd-card.png


.. _windows:

*******
Windows
*******

1. Insert SD card into your PC or SD card reader.

.. image:: SDcard_insert.jpg

2. Download `Win32 Disk Imager <https://sourceforge.net/projects/win32diskimager/>`_ and extract it.

.. image:: SDcard_Win_Win32DiskImager.png

3. Open unzipped folder, right-click on the ``WinDisk32Imager``, and select **Run as Administrator**.

.. image:: SDcard_Win_RunAsAdmin.png

4. Under image file box select unzipped Red Pitaya image file.

.. image:: SDcard_Win_SelectImg.png

5. Under device box select the drive letter of the SD card.

.. image:: SDcard_Win_SelectDrive.png

.. note::

   Be careful to select the correct drive; if you choose the wrong one you risk erasing data from the 
   computer's hard disk! You can easily see the drive letter (for example E:) by looking in the left column 
   of Windows Explorer.

.. image:: SDcard_Win_DriveLetter.png

6. Click Write and wait for the write to complete.

.. image:: SDcard_Win_Write.png

7. Exit the Imager.

.. image:: SDcard_Win_Exit.png


.. _linux:

*****
Linux
*****

=========================
Ubuntu using Image Writer
=========================

============
Command line
============

.. note::
   Please note that the use of the ``dd`` tool can overwrite any partition of your machine.
   If you specify the wrong device in the instructions below, you could delete your primary Linux partition.
   Please be careful.

1. Insert SD card into your PC or SD card reader.

.. image:: SDcard_insert.jpg 

3. Open the Terminal and check the available disks with ``df -h``.
   Our SD card is 4GB, it is named ``/dev/sdx`` and
   divided into two partitions ``/dev/sdx1`` and ``/dev/sdx2``.
   The drive mounted at ``/`` is your main drive,
   be carefull not to use it.

.. code-block:: shell-session

   $ df -h
   Filesystem      Size  Used Avail Use% Mounted on
   /dev/sdx1       118M   27M   92M  23% /media/somebody/CAD5-1E3D
   /dev/sdx2       3.2G 1013M  2.1G  33% /media/somebody/7b2d3ba8-95ed-4bf4-bd67-eb52fe65df55

4. Unmount all SD card partitions with ``umount /dev/sdxN``
   (make sure you replace N with the right numbers).

.. code-block:: shell-session

   $ sudo umount /dev/sdx1 /dev/sdx2

5. Write the image to the SD card with the following command.
   Replace the ``red_pitaya_image_file.img`` with
   the name of the unzipped Red Pitaya SD Card Image
   and replace ``/dev/device_name`` with the path to the SD card.

.. code-block:: shell-session

   $ sudo dd bs=1M if=red_pitaya_image_file.img of=/dev/device_name

6. Wait until the process has finished.

*****
macOS
*****

===================
Using ApplePi-Baker
===================
    
1. Insert SD card into your PC or SD card reader.

.. image:: SDcard_insert.jpg

2. Download `ApplePi-Baker <http://www.tweaking4all.com/software/macosx-software/macosx-apple-pi-baker/>`_ and extract it.

.. image:: DScard_macOS_ApplePi-Baker.png

3. Press **crtl** key and click on *ApplePi-Baker* icon, then click *Open* in order to run it.

.. image:: DScard_macOS_open.png

4. Enter your admin password and click OK.

.. image:: DScard_macOS_password.png

5. Select SD card drive. This can be recognized by the size of the card that is 4GB.

.. image:: 52.png

6. Select Red Pitaya OS image file.

.. image:: 62.png

7. Click "Restore Backup" button in order to write image to SD card.

.. image:: 71.png

8. It's coffee time, application will show you Estimated Time for Accomplishment.

.. image:: 8.png

9. When operation is completed click "OK" and quit ApplePi-Baker.

.. image:: 9.png

************
Command line
************

FAQ: `How to install Red Pitaya OS on MAC not using ApplePiBaker? <http://blog.redpitaya.com/faq-page/#QuickStart|23547>`_
