## Quick release building

If there are no changes needed to the Debian system, but a new ecosystem is available, then there is no need to bootstrap Debian and install Wyliodrin. Instead it is enough to delete all files from the FAT partition and extract `ecosystem*.zip` into the partition.
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
```bash
head -c 3670016000 debian_armhf_*_wyliodrin.img > debian_armhf_*_wyliodrin-short.img
```
10. compress the image using zip
11. upload image to download server
```bash
scp red_pitaya_OS_v0.94-RC??_?date?.img.zip uname@downloads.redpitaya.com/var/www/html/downloads/
```
12. make symbolic link to beta or stable
```bash
cd /var/www/html/downloads/
ln -sf red_pitaya_OS_v0.94-RC??_?date?.img.zip red_pitaya_OS-beta.img.zip
ln -sf red_pitaya_OS_v0.94-RC??_?date?.img.zip red_pitaya_OS-stable.img.zip
```

# Dependencies

Ubuntu 2015.04 was used to build Debian/Ubuntu SD card images for Red Pitaya.

The next two packages need to be installed:
```bash
sudo apt-get install debootstrap qemu-user-static
```

# Image build Procedure

Multiple steps are needed to prepare a proper SD card image.
1. Bootstrap Debian system with network configuration and Red Pitaya specifics.
2. Add Red Pitaya ecosystem ZIP.
3. Optionally install Wyliodrin.

## Debian bootstrap

Run the next command inside the project root directory. Root or `sudo` privileges are needed.
```bash
sudo OS/debian/image.sh
```
This will create an image with a name containing the current date and time. Two scripts `debian.sh` and `redpitaya.sh` will also be called from `image.sh`.

`debian.sh` bootstraps a Debian system into the EXT4 partition. It also updates packages and adds a working network configuration for Red Pitaya.
`redpitaya.sh` extracts `ecosystem*.zip` (if one exists in the current directory) into the FAT partition. It also configures some Red Pitaya Systemd services.

The generated image can be written to a SD card using the `dd` command or the `Disks` tool (Restore Disk Image).
```bash
dd bs=4M if=debian_armhf_*.img of=/dev/sd?
```
**NOTE:** To get the correct destination storage device, read the output of `dmesg` after you insert the SD card. If the wrong device is specified, the content of another
drive may be overwritten, causing permanent loose of user data.

## Red Pitaya ecosystem extraction

In case `ecosystem*.zip` was not available for the previous step, it can be extracted later to the FAT partition (128MB) of the SD card. In addition to Red Pitaya tools, this ecosystem ZIP file contains a boot image (containing FPGA code), a boot script (`u-boot.scr`) and the Linux kernel.

## Wyliodrin

Unfortunately there are issues with Wyliodrin install process inside a virtualized environment. Therefore the provided script `wiliodrin.sh` must be run from a shell on a running Red Pitaya board. The script can be copied to the FAT partition and executed from the `/root/` directory. Some code which is meant to be executed on the development machine, should be comment out (everything outside the `chroot`, including the `chroot` lines themselves).
```bash
cd /root
. /opt/redpitaya/wyliodrin.sh
```

The Wyliodrin team provided the initial support for Red Pitaya inside the `libwyliodrin` library. We are using a fork of the library which includes a few bug fixes and new features. Please have a look at the commit history for details. It would make sense to ask the Wyliodrin team to accept this changes into upstream.

https://github.com/RedPitaya/libwyliodrin

Some effort was made to port the newer C based `wyliodrin-server` instead of using the current `node.js` based server. Most of the effort was spent on attempts to replace compiling dependencies from source with packages provided in Debian. The unfinished branch can be provided to interested developers.


## Reducing image size

A cleanup can be performed to reduce the image size. Various things can be done to reduce the image size:
- remove unused software (this could be software which was needed to compile applications)
- remove unused source files (remove source repositories used to compile applications)
- remove temporary files
- zero out empty space on the partition

The next code only removes APT temporary files and zeros out the filesystem empty space.
```bash
apt-get clean
cat /dev/zero > zero.file
sync
rm -f zero.file
history -c
```


## Creating a SD card image

Since Wiliodrin and maybe the ecosystem ZIP are not part of the original SD card image. The updated SD card contents should be copied into an image using `dd` or the `Disks` tool (Create Disk Image).
```bash
dd bs=4M if=/dev/sd? of=debian_armhf_*_wyliodrin.img
```

Initially the SD card image was designed to be about 3.7GB in size, so it would fit all 4GB SD cards. If the image is created from a larger card, it will contain empty space at the end. To remove the empty space from the SD card image do:
```bash
head -c 3670016000 debian_armhf_*_wyliodrin.img > debian_armhf_*_wyliodrin-short.img
mv debian_armhf_*_wyliodrin-short.img debian_armhf_*_wyliodrin.im
```

The image size can be further reduced by compressing it. Zip is used, since it is also available by default on MS Windows.
```bash
zip debian_armhf_*_wyliodrin.img > debian_armhf_*_wyliodrin.img.zip
```


# Debian Usage

## Systemd

Systemd is used as the init system and services are used to start/stop Red Pitaya applications/servers. Service files are located in `OS/debian/overlay/etc/systemd/system/*.service`.

| service               | description
|-----------------------|-------------------------------------------------------
| `redpitaya_wyliodrin` | Wyliodrin server, is running by default
| `redpitaya_scpi`      | SCPI server, is disabled by default, since it conflicts with WEB applications
| `redpitaya_discovery` | Device discovery, is run once after boot to send Ethernet MAC and IP address to a discovery server
| `redpitaya_nginx`     | Nginx based server, serving WEB based applications

To start/stop a service, do one of the following:
```bash
systemctl start service_name
systemctl stop service_name
```

To enable/disable a service, so to determine if it will start at powerup, do one of the following:
```bash
systemctl enable service_name
systemctl disable service_name
```

To see the status of a specific service run:
```bash
systemctl
```

### debugging

```
systemd-analyze plot > /opt/redpitaya/www/apps/systemd-plot.svg
systemd-analyze dot | dot -Tsvg > /opt/redpitaya/www/apps/systemd-dot.svg

```

## WiFi

```
wpa_passphrase MyNetwork SuperSecretPassphrase > /etc/wpa_supplicant/wpa_supplicant-wlan0.conf
```

