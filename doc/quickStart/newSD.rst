###############
Prepare SD card
###############

1. Download the Red Pitaya Image File
    
    .. image:: microSDcard-RP.png
       :width: 10%
    
    `Download <http://blog.redpitaya.com/quick-start/Login>`_
    
    - FAQ: `Where can I find more about Red Pitaya OS releases? <http://blog.redpitaya.com/faq-page/#Software|32740>`_
    - FAQ: `Where can I find old Red Pitaya OS & application relases? <http://blog.redpitaya.com/faq-page/#Software|25467>`_
    
2. Unzip

3. Select your operating system and follow the instructions:  

*******
Windows
*******


    1. Insert SD card into your PC or SD card reader.
    
    .. image:: SDcard_Win_insert.jpg
    
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
    
    7.  Exit the Imager.
    
    .. image:: SDcard_Win_Exit.png

*****
Linux
*****

    1. Insert SD card into your PC or SD card reader.
    
    .. image:: 1.jpg 
    
    2. Run Disks application to format the SD card.
    
    .. image:: ubuntu_disk_format.jpg
    
    3. Open the Terminal and check the available disks with "df -h". Our SD card is 4GB and mounted to /dev/sdb

    .. image:: ubuntu_sdprepare1.png
    
    4. Unmount the SD card with "umount /dev/sdbN" (make sure you replace N with the right number).
    
    .. image:: ubuntu_sdprepare3.png
    
    5. Write the image to the SD card with the following command : dd if=red_pitaya_image_file of=/dev/sdb bs=1M
    
    .. note::
    
        Replace the red_pitaya_image_file with the name of the unzipped Red Pitaya SD Card Image and
        /dev/device_name is replaced with the path to the SD Card, usually it will be /dev/sdb.
    
    .. image:: 51.png
    
    
    6. Wait until the process has finished.
    
    .. image:: 61.png

MacOS
*****
    
    1. Insert SD card into your PC or SD card reader.
    
    .. image:: 1.jpg
    
    2. Download Apple Pi Baker and unzip it.
    
    .. image:: 21.png


    3. Press "crtl" key and click on ApplePi-Baker icon, then click Open in order to run it.
    
    .. image:: 3-1.png


    4. Enter your admin password and click OK.
    
    .. image:: 41.png


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
    
    FAQ: `How to install Red Pitaya OS on MAC not using ApplePiBaker? <http://blog.redpitaya.com/faq-page/#QuickStart|23547>`_

4.  Insert SD card into Red Pitaya

    .. image:: pitaya-quick-start-insert-sd-card.png
    
