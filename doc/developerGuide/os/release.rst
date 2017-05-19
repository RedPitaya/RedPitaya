***********************************************
Quick release procedure (for internal use only)
***********************************************

If there are no changes needed to the Debian system, but a new ecosystem is available,
then there is no need to bootstrap Debian.
Instead it is enough to delete all files from the FAT partition
and extract ``ecosystem*.zip`` into the partition.
Start with an existing release image:

1. load Red Pitaya OS image onto a SD card of at least 4GB
2. insert the card into a PC
3. on Linux EXT4 partition will also be mounted, unmount it to avoid corruption
4. remove all contents from FAT partition, take care to delete the files not to move them into a recycle bin (`SHFT+DEL`)
5. extract `ecosystem*.zip` into the FAT partition
6. unmount FAT partition
7. make an image of the SD card
8. remove SD card from PC
9. shorten the image so it fits on all 4GB sd cards

   .. code-block:: shell-session

      $ head -c 3670016000 debian_armhf_*.img > debian_armhf_*-short.img

10. compress the image using zip
11. upload image to download server

   .. code-block:: shell-session

      $ scp red_pitaya_OS_v0.94-RC??_?date?.img.zip uname@downloads.redpitaya.com/var/www/html/downloads/

12. make symbolic link to beta or stable

   .. code-block:: shell-session

      $ cd /var/www/html/downloads/
      $ ln -sf red_pitaya_OS_v0.94-RC??_?date?.img.zip red_pitaya_OS-beta.img.zip
      $ ln -sf red_pitaya_OS_v0.94-RC??_?date?.img.zip red_pitaya_OS-stable.img.zip

