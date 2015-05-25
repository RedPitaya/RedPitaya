# Red Pitaya specifics

## FSBL (first stage boot loader) specifics
### Version
FSBL version is defined by the Vivado tool, version **2013.3** is used for this project.
### Patches
```
FPGA/patches/memtest-vivado.patch
```
This patch seems to enable status reporting after a memory test. TODO: details

## U-Boot specifics
### Version
A new version of U-Boot is provided with each Vivado version, here a slightly older version was used **v14.6.01**.
### Patches
There are several patched for U-Boot
```
OS/u-boot/patches/0001-Changed-serial-from-uart1-to-uart0-and-phy-addr-1.patch
```
Changed-serial-from-uart1-to-uart0 and phy addr=1. But it modifies a file for a different board `include/configs/zynq_zed.h` This settings ere moved to `include/configs/zynq_red_pitaya.h` for **v2015.1**.
```
OS/u-boot/patches/0002-Increased-PHY-autonegotiation-timeout-from-4-to-10s.patch
```
Increased-PHY-autonegotiation-timeout-from-4-to-10s. This patch was transfered to **v2015.1** without changes.
```
OS/u-boot/patches/0003-Changed-serial-clock-from-50-to-100MHz-CPU_FREQ-800M.patch
```
Changed-serial-clock-from-50-to-100MHz, CPU_FREQ=800MHz. Changes are applied to `include/configs/zynq_common.h`, they should instead be overriding this macros in `include/configs/zynq_red_pitaya.h`. Regardless `CONFIG_ZYNQ_SERIAL_CLOCK[01]` are not present in version **v2015.1**. Aniway the usefulness of this patch in unclear.
```
OS/u-boot/patches/0004-Unmasked-PLL-bit-from-boot-mode-register-check-tempo-USB.patch
```
Bootmode and USB reset fixes (for a bug in Linux). **NOTE:** almost certainly fixed upstream.
```
OS/u-boot/patches/0005-Added-Lantiq-PHY11G-support-for-LED-settings.patch
OS/u-boot/patches/0006-Lantiq-PHY11G-support-for-LED-configuration.patch
OS/u-boot/patches/0007-Lantiq-copyright-changed.patch
OS/u-boot/patches/0008-Fixed-10BASE-T-FULL-led-blink-speed-setting.patch
```
This patches were combined for version **v2015.1**. Instead of modifying the generic `include/configs/zynq_common.h`, Lantiq Ethernet PHY is enabled in `include/configs/zynq_red_pitaya.h`.
```
OS/u-boot/patches/0009-Enabled-dhcp-command.patch
```
Instead of modifying the generic `include/configs/zynq_common.h` DHCP should be enabled in the board specific `include/configs/zynq_red_pitaya.h`.
```
OS/u-boot/patches/0010-Added-Red-Pitaya-configuration-based-on-modified-zed.patch
```
This patch modified `boards.cfg` which is deprecated, instead boards are now defined in `Kconfig`. There is also a MAC address change in `include/configs/zynq_common.h` for an unknown purpose, this was not forwarded to **v2015.1**.
```
OS/u-boot/patches/0011-Added-Red-Pitaya-configuration-header-file-based-on-.patch
OS/u-boot/patches/0012-Red-Pitaya-env-par-added-wp-eeprom-area-is-now-in-us.patch
```
This file is a board specific configuration file, but instead of properly including `include/configs/zynq_common.h` (or `include/configs/zynq-common.h` in **v2015.1**), it copies its contents. At the same time this means all patches to `zynq_common.h` become meaningless. This path is ignored for **v2015.1**, instead a much shorter config file is written. EEPROM setings from this patch were used, but there are many other variables I am unsure about.
```
OS/u-boot/patches/0013-Version-String-Added.patch
```
This patch adds a version string to Linux tools for handling U-Boot environment variables, since this is not very usefull, this patch was removed for **v2015.1**.
```
OS/u-boot/patches/0014-Fix-half-of-memory-reported.patch
```
This is for an issue reporting memory size if 16bit memory chip is used insted of a 32bit chip. This issue is fixed in **v2015.1**, so the pach is not needed.

### Configuration

There are three possible sources for the U-Boot environment: configuration header file, persistent storage (EEPROM) and scripts. If the environment is taken from persistent storage (`CONFIG_ENV_IS_IN_EEPROM` macro is defined), then the contents of the `CONFIG_EXTRA_ENV_SETTINGS` macro are ignored. So EEPROM becomes the only source for the environment. Scripts can only be called from the previously described environment sources, so currently they are not used.

#### Board header file

`modeboot` is internaly set to `sdboot`, this makes `sdboot` the default boot sequence.

#### Environment

Currently the environment is stored inside the EEPROM:

```
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


## Linux kernel specifics


## Device tree specifics


## Init process specifics

