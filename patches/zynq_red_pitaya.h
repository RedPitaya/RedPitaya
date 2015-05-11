#ifndef __CONFIG_ZYNQ_RED_PITAYA_H
#define __CONFIG_ZYNQ_RED_PITAYA_H

#define CONFIG_SYS_SDRAM_SIZE		(480 * 1024 * 1024)

#define CONFIG_ZYNQ_SERIAL_UART0
#define CONFIG_ZYNQ_GEM0
#define CONFIG_ZYNQ_GEM_PHY_ADDR0	1

#define CONFIG_SYS_NO_FLASH

#define CONFIG_ZYNQ_SDHCI0
#define CONFIG_ZYNQ_QSPI
#define CONFIG_ZYNQ_EEPROM
#define CONFIG_ZYNQ_BOOT_FREEBSD

#include <configs/zynq-common.h>

#undef CONFIG_PHY_MARVELL

#undef CONFIG_SYS_I2C_EEPROM_ADDR_LEN
#undef CONFIG_SYS_I2C_EEPROM_ADDR
#undef CONFIG_SYS_EEPROM_PAGE_WRITE_BITS
#undef CONFIG_SYS_EEPROM_SIZE

#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_IS_IN_SPI_FLASH
#undef CONFIG_ENV_OFFSET

#define CONFIG_PHY_LANTIQ

#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		2
#define CONFIG_SYS_I2C_EEPROM_ADDR		0x50
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	5
#define CONFIG_SYS_EEPROM_SIZE			8192 /* Bytes */

#define CONFIG_ENV_IS_IN_EEPROM
#define CONFIG_ENV_SIZE		1024 /* Total Size of Environment Sector */
#define CONFIG_ENV_OFFSET	(2048*3) /* WP area starts at last 1/4 of 8k eeprom */

#define CONFIG_EXTRA_ENV_SETTINGS      \
	"ethaddr=00:26:33:14:50:00\0"   \
	"hw_rev=B\0"    \
	"prod_date=12/22/13\0"  \
	"serial=00000000\0"     \
	"kernel_image=uImage\0"    \
	"ramdisk_image=uramdisk.image.gz\0"     \
	"devicetree_image=devicetree.dtb\0"     \
	"bitstream_image=system.bit.bin\0"      \
	"loadbit_addr=0x100000\0"       \
	"kernel_size=0x500000\0"        \
	"devicetree_size=0x20000\0"     \
	"ramdisk_size=0x5E0000\0"       \
	"fdt_high=0x20000000\0" \
	"initrd_high=0x20000000\0"      \
	"sdboot=echo Copying Linux from SD to RAM... && " \
		"mmcinfo && " \
		"fatload mmc 0 0x3000000 ${kernel_image} && " \
		"fatload mmc 0 0x2A00000 ${devicetree_image} && " \
		"fatload mmc 0 0x2000000 ${ramdisk_image} && " \
		"bootm 0x3000000 0x2000000 0x2A00000\0"

#endif /* __CONFIG_ZYNQ_RED_PITAYA_H */
