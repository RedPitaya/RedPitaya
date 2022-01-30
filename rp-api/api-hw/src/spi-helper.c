#include <stdint.h>
#include <stdio.h>

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

#include "spi-helper.h"

int read_spi_configuration(int fd, spi_config_t *config)
{
	uint8_t  u8;
	uint32_t u32;

	if (ioctl(fd, SPI_IOC_RD_MODE, &u8) < 0) {
		MSG("[Error] SPI_IOC_RD_MODE\n");
		return RP_HW_ESGS;
	}
	config->spi_mode = (rp_spi_mode_t)(u8 & 0x03);
	config->spi_ready = ((config->spi_mode & SPI_READY) ? RP_SPI_STATE_READY 
														: RP_SPI_STATE_NOT);

	if (ioctl(fd, SPI_IOC_RD_LSB_FIRST, &u8) < 0) {
		MSG("[Error] SPI_IOC_RD_LSB_FIRST\n");
		return RP_HW_ESGS;
	}
	config->lsb_first = (u8 == SPI_LSB_FIRST ? RP_SPI_ORDER_BIT_LSB 
											 : RP_SPI_ORDER_BIT_MSB);

	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &u8) < 0) {
		MSG("[Error] SPI_IOC_RD_BITS_PER_WORD\n");
		return RP_HW_ESGS;
	}
	config->bits_per_word = (u8 == 0 ? 8 : u8);

	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &u32) < 0) {
		MSG("[Error] SPI_IOC_RD_MAX_SPEED_HZ\n");
		return RP_HW_ESGS;
	}
	config->spi_speed = u32;

	return RP_HW_OK;
}



int write_spi_configuration(int fd, spi_config_t *config)
{
	uint8_t  u8;
	uint32_t u32;
	
	u8 = (uint8_t)config->spi_mode | (config->spi_ready == RP_SPI_STATE_READY ? SPI_READY : 0);	

	if (ioctl(fd, SPI_IOC_WR_MODE, &u8) < 0) {
		MSG("[Error] SPI_IOC_WR_MODE\n");
		return RP_HW_ESSS;
	}

	u8 = (config->lsb_first == RP_SPI_ORDER_BIT_LSB ? SPI_LSB_FIRST : 0);
	if (ioctl(fd, SPI_IOC_WR_LSB_FIRST, &u8) < 0) {
		MSG("[Error] SPI_IOC_WR_LSB_FIRST\n");
		return RP_HW_ESSS;
	}

	u8 = config->bits_per_word;
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &u8) < 0) {
		MSG("[Error] SPI_IOC_WR_BITS_PER_WORD\n");
		return RP_HW_ESSS;
	}

	u32 = config->spi_speed;
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &u32) < 0) {
		MSG("[Error] SPI_IOC_WR_MAX_SPEED_HZ\n");
		return RP_HW_ESSS;
	}
	
	return RP_HW_OK;
}

int read_write_spi_buffers(int fd, void *tx_buffer, void *rx_buffer, unsigned int length)
{
	struct spi_ioc_transfer transfer = {
		.tx_buf        = 0,
		.rx_buf        = 0,
		.len           = 0,
		.delay_usecs   = 0,
		.speed_hz      = 0,
		.bits_per_word = 0,
	};

	transfer.rx_buf = (unsigned long)rx_buffer;
	transfer.tx_buf = (unsigned long)tx_buffer;
	transfer.len = length;

	if (ioctl(fd, SPI_IOC_MESSAGE(1), & transfer) < 0)
		return RP_HW_EST;

	printf("Read size %d\n",transfer.len);
	return RP_HW_OK;
}
