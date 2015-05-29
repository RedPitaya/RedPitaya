# Red Pitaya specifics

## FSBL (first stage boot loader)

### Version
FSBL version is defined by the Vivado tool, version **2013.3** is used for this project.

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

    `include/phy.h`

2.  Added support for Lantiq PHY11G.

    `include/phy.h`
    `drivers/net/phy/phy.c`
    `drivers/net/phy/lantiq.c`
    `drivers/net/phy/Makefile`
    `include/linux/mii.h`

3.  Red Pitaya board added to U-Boot configuration.

    `arch/arm/cpu/armv7/zynq/Kconfig`
    `arch/arm/dts/zynq-red-pitaya.dts`
    `configs/zynq_red_pitaya_defconfig`
    `include/configs/zynq_red_pitaya.h`

4.  Boot options for Ubuntu/Debian SD card images.

    `TODO`

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

## Linux kernel

### Version
A new version of U-Boot is provided with each Vivado version, here a slightly older version was used **v14.6.02**.

### Patches
```
OS/linux/patches/autonegotiation_restart.patch
```
File drivers/net/phy/phy.c was updated to enable autonegotiation restart. The related code was also updated upstream for **v2015.1**, it is not obvious if the functionality is the same, but it is probably better to use the upstream code without changes.
```
OS/linux/patches/eeprom.patch
```
Modifies the EEPROM driver `drivers/misc/eeprom/at24.c` to use a 32 Byte page size for `24c64` instead of the default 8 Bytes. This patch might improve speed and reduce EEPROM wear. Since there are no related changes upstream this this patch will be applied to the latest **v2015.1** version. TODO, check if the same can be achieved using runtime configuration options.
```
OS/linux/patches/lantiq-PHY11G-itech.patch
```
A driver for Lantiq PHY11G is provided here. I checked the manfacturers website for an official driver, but could not find one.
```
OS/linux/patches/xdevcfg-Automatic-endian-swap-and-header-removal.patch 
```
This patch is already properly handled upstream.
```
OS/linux/patches/add_rtl8192cu.patch
```
This patch adds the out of tree driver for Broadcom rtl8192cu chip, since there are many issues with the in kernel version. This issues have not yet been handled upstream, so this or even better an updated pathch are needed for **v2015.1** also.

### Configuration

```

#
# IO Schedulers
#
CONFIG_IP_ADVANCED_ROUTER=y

CONFIG_WIRELESS_EXT=y
CONFIG_WEXT_CORE=y
CONFIG_WEXT_PROC=y
CONFIG_WEXT_SPY=y
CONFIG_WEXT_PRIV=y
CONFIG_CFG80211=y
# CONFIG_NL80211_TESTMODE is not set
# CONFIG_CFG80211_DEVELOPER_WARNINGS is not set
# CONFIG_CFG80211_REG_DEBUG is not set
# CONFIG_CFG80211_CERTIFICATION_ONUS is not set
# CONFIG_CFG80211_DEFAULT_PS is not set
# CONFIG_CFG80211_DEBUGFS is not set
# CONFIG_CFG80211_INTERNAL_REGDB is not set
# CONFIG_CFG80211_WEXT is not set
CONFIG_LIB80211=m
CONFIG_LIB80211_CRYPT_WEP=m
CONFIG_LIB80211_CRYPT_CCMP=m
CONFIG_LIB80211_CRYPT_TKIP=m
# CONFIG_LIB80211_DEBUG is not set
# CONFIG_MAC80211 is not set

CONFIG_BLK_DEV_RAM_SIZE=32768

CONFIG_LANTIQ_PHY=y

#
# USB Network Adapters
#
# CONFIG_USB_CATC is not set
# CONFIG_USB_KAWETH is not set
# CONFIG_USB_PEGASUS is not set
# CONFIG_USB_RTL8150 is not set
# CONFIG_USB_USBNET is not set
# CONFIG_USB_IPHETH is not set
CONFIG_WLAN=y
# CONFIG_ATMEL is not set
# CONFIG_PRISM54 is not set
# CONFIG_USB_ZD1201 is not set
# CONFIG_USB_NET_RNDIS_WLAN is not set
# CONFIG_ATH_CARDS is not set
# CONFIG_BRCMFMAC is not set
# CONFIG_HOSTAP is not set
CONFIG_IPW2100=m
# CONFIG_IPW2100_MONITOR is not set
# CONFIG_IPW2100_DEBUG is not set
CONFIG_LIBIPW=m
# CONFIG_LIBIPW_DEBUG is not set
# CONFIG_LIBERTAS is not set
CONFIG_RTL8192CU=y
CONFIG_RTL8192C_COMMON=m
# CONFIG_WL_TI is not set
# CONFIG_MWIFIEX is not set

#
# SPI Protocol Masters
#
CONFIG_SPI_SPIDEV=y

#
# Crypto core or helper
#
CONFIG_CRYPTO_ALGAPI=y
CONFIG_CRYPTO_ALGAPI2=y
CONFIG_CRYPTO_AEAD2=y
CONFIG_CRYPTO_BLKCIPHER=m
CONFIG_CRYPTO_BLKCIPHER2=y
CONFIG_CRYPTO_HASH=y
CONFIG_CRYPTO_HASH2=y
CONFIG_CRYPTO_RNG=m
CONFIG_CRYPTO_RNG2=y
CONFIG_CRYPTO_PCOMP2=y
CONFIG_CRYPTO_MANAGER=m
CONFIG_CRYPTO_MANAGER2=y
# CONFIG_CRYPTO_USER is not set
CONFIG_CRYPTO_MANAGER_DISABLE_TESTS=y
# CONFIG_CRYPTO_GF128MUL is not set
# CONFIG_CRYPTO_NULL is not set
# CONFIG_CRYPTO_PCRYPT is not set
CONFIG_CRYPTO_WORKQUEUE=y
# CONFIG_CRYPTO_CRYPTD is not set
# CONFIG_CRYPTO_AUTHENC is not set
# CONFIG_CRYPTO_TEST is not set

#
# Block modes
#
# CONFIG_CRYPTO_CBC is not set
# CONFIG_CRYPTO_CTR is not set
# CONFIG_CRYPTO_CTS is not set
CONFIG_CRYPTO_ECB=m
# CONFIG_CRYPTO_LRW is not set
# CONFIG_CRYPTO_PCBC is not set
# CONFIG_CRYPTO_XTS is not set

#
# Digest
#
CONFIG_CRYPTO_CRC32C=y
# CONFIG_CRYPTO_CRC32 is not set
# CONFIG_CRYPTO_GHASH is not set
# CONFIG_CRYPTO_MD4 is not set
# CONFIG_CRYPTO_MD5 is not set
CONFIG_CRYPTO_MICHAEL_MIC=m
# CONFIG_CRYPTO_RMD128 is not set
# CONFIG_CRYPTO_RMD160 is not set
# CONFIG_CRYPTO_RMD256 is not set
# CONFIG_CRYPTO_RMD320 is not set
# CONFIG_CRYPTO_SHA1 is not set
# CONFIG_CRYPTO_SHA1_ARM is not set
# CONFIG_CRYPTO_SHA256 is not set
# CONFIG_CRYPTO_SHA512 is not set
# CONFIG_CRYPTO_TGR192 is not set
# CONFIG_CRYPTO_WP512 is not set

#
# Ciphers
#
CONFIG_CRYPTO_AES=y
# CONFIG_CRYPTO_AES_ARM is not set
# CONFIG_CRYPTO_ANUBIS is not set
CONFIG_CRYPTO_ARC4=m
# CONFIG_CRYPTO_BLOWFISH is not set
# CONFIG_CRYPTO_CAMELLIA is not set
# CONFIG_CRYPTO_CAST5 is not set
# CONFIG_CRYPTO_CAST6 is not set
# CONFIG_CRYPTO_DES is not set
# CONFIG_CRYPTO_FCRYPT is not set
# CONFIG_CRYPTO_KHAZAD is not set
# CONFIG_CRYPTO_SALSA20 is not set
# CONFIG_CRYPTO_SEED is not set
# CONFIG_CRYPTO_SERPENT is not set
# CONFIG_CRYPTO_TEA is not set
# CONFIG_CRYPTO_TWOFISH is not set

```




## Device tree


## Init process

