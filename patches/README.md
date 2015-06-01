# Red Pitaya specifics

## FSBL (first stage boot loader)

### Version
FSBL version is defined by the Vivado tool, version **2015.1** is used for this project.

### Patches
```
FPGA/patches/memtest-vivado.patch
```
This patch seems to enable status reporting after a memory test. TODO: details

## U-Boot specifics

### Version
A new version of U-Boot is provided with each Vivado version, the current version is **v2015.1**.

### Patches
There are several patches for U-Boot:

1.  Increased PHY autonegotiation timeout from 4 to 10s.

    ```lang-none
    include/phy.h
    ```

2.  Added support for Lantiq PHY11G.

    ```lang-none
    include/phy.h
    drivers/net/phy/phy.c
    drivers/net/phy/lantiq.c
    drivers/net/phy/Makefile
    include/linux/mii.h
    ```

3.  Red Pitaya board added to U-Boot configuration.

    ```lang-none
    arch/arm/cpu/armv7/zynq/Kconfig
    arch/arm/dts/zynq-red-pitaya.dts
    configs/zynq_red_pitaya_defconfig
    include/configs/zynq_red_pitaya.h
    ```

4.  Boot options for Ubuntu/Debian SD card images.

    ```lang-none
    TODO
    ```

### Configuration
There are three possible sources for the U-Boot environment: configuration header file, persistent storage (EEPROM) and scripts. If the environment is taken from persistent storage (`CONFIG_ENV_IS_IN_EEPROM` macro is defined), then the contents of the `CONFIG_EXTRA_ENV_SETTINGS` macro are ignored. So EEPROM becomes the only source for the environment. Scripts can only be called from the previously described environment sources, so currently they are not used.

#### Board header file
`modeboot` is internaly set to `sdboot`, this makes `sdboot` the default boot sequence.

#### Environment
Currently the environment is stored inside the EEPROM:
```lang-none
bootcmd=run $modeboot
bootdelay=3
baudrate=115200
ipaddr=10.10.70.102
serverip=10.10.70.101
prod_date=12/22/13
kernel_image=uImage
ramdisk_image=uramdisk.image.gz
devicetree_image=devicetree.dtb
bitstream_image=system.bit.bin
loadbit_addr=0x100000
kernel_size=0x500000
devicetree_size=0x20000
ramdisk_size=0x5E0000
fdt_high=0x20000000
initrd_high=0x20000000
sdboot=echo Copying Linux from SD to RAM... && mmcinfo && fatload mmc 0 0x3000000 ${kernel_image} && fatload mmc 0 0x2A00000 ${devicetree_image} && fatload mmc 0 0x2000000 ${ramdisk_image} && bootm 0x3000000 0x2000000 0x2A00000
ethaddr=00:26:32:F0:03:21
nav_code=4651
hw_rev=1.0
serial=140900801
```
The Linux `boot` parameters are not appropriate for running Ubuntu images, since a different filesystem configuration is more appropriate. Pavel Demin solved this issue by patching a U-Boot source file, where some variables from the environment contained in the EEPROM are overwritten.

## Linux kernel

### Version
A new version of the Linux kernel is provided with each Vivado version, the current version is **v2015.1**.

### Patches
```lang-none
arch/arm/configs/xilinx_zynq_defconfig
```
Kernel configuration update based on a generic configuration for Xilinx Zynq devices.
```lang-none
drivers/misc/eeprom/at24.c
```
Modifies the EEPROM driver to use a 32 Byte page size for `24c64` instead of the default 8 Bytes. This patch might improve speed and reduce EEPROM wear. There are no related changes upstream. TODO, check if the same can be achieved using runtime configuration options.
```lang-none
drivers/net/phy/Kconfig
drivers/net/phy/Makefile
drivers/net/phy/lantiq.c
```
A driver for Lantiq PHY11G is provided here. I checked the manfacturers website for an official driver, but could not find one.
```lang-none
drivers/net/wireless/Kconfig
drivers/net/wireless/Makefile
drivers/net/wireless/rtl8192cu/*
```
This patch adds the out of tree driver for Broadcom rtl8192cu chip, since there are many issues with the in kernel version (even latest kernels). The provided code was copied from the **Rasberry PI** kernel repository `https://github.com/raspberrypi/linux/tree/rpi-3.18.y/drivers/net/wireless/rtl8192cu`.

### Configuration
Linux kernel configuration is based on `arch/arm/configs/xilinx_zynq_defconfig`, which is patched to add support for `RT2X00` and `RTL8192CU` wireless drivers. Aditional networking features are also enabled so Red Pitaya can behave like an access point routing travic from the wireless adapter to the wired port connected to the internet.

## Device tree


## Init process
Init configuration is slit between generic components (file system mounting, wireless access point) and Red Pitaya specific components (starting Nginx and SCPI servers).

```lang-none
OS/filesystem/
OS/filesystem/sbin
OS/filesystem/sbin/os-postinstall
OS/filesystem/etc
OS/filesystem/etc/network
OS/filesystem/etc/network/interfaces
OS/filesystem/etc/network/interfaces.ap
OS/filesystem/etc/network/hostapd.conf
OS/filesystem/etc/network/dnsmasq.conf
OS/filesystem/etc/network/config
OS/filesystem/etc/network/wpa_supplicant.conf
```

```lang-none
OS/buildroot/overlay/sbin/ro
OS/buildroot/overlay/sbin/lantiq_mdio
OS/buildroot/overlay/sbin/rw
OS/buildroot/overlay/sbin/bazaar
OS/buildroot/overlay/var/log -> ../tmp/log
OS/buildroot/overlay/etc/fw_env.config
OS/buildroot/overlay/etc/ssh_host_key
OS/buildroot/overlay/etc/init.d
OS/buildroot/overlay/etc/init.d/S40network
OS/buildroot/overlay/etc/init.d/connman-config
OS/buildroot/overlay/etc/init.d/S90scpi
OS/buildroot/overlay/etc/init.d/rcK
OS/buildroot/overlay/etc/init.d/S80nginx
OS/buildroot/overlay/etc/init.d/rcS
OS/buildroot/overlay/etc/ssh_host_dsa_key
OS/buildroot/overlay/etc/fstab
OS/buildroot/overlay/etc/ssh_host_rsa_key
OS/buildroot/overlay/etc/inittab
OS/buildroot/overlay/etc/profile
OS/buildroot/overlay/etc/motd
OS/buildroot/overlay/etc/ssh_host_ed25519_key
OS/buildroot/overlay/etc/ssh_host_ecdsa_key
OS/buildroot/overlay/etc/network/interfaces
OS/buildroot/overlay/usr/share/udhcpc/default.script
```
