#include <stdint.h>
#include <stdio.h>

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include "rp_log.h"
#include "spi-helper.h"

int read_spi_configuration(int fd, spi_config_t *config)
{
	uint8_t  u8;
	uint32_t u32;

	if (ioctl(fd, SPI_IOC_RD_MODE, &u8) < 0) {
		ERROR_LOG("Set SPI_IOC_RD_MODE");
		return RP_HW_ESGS;
	}
	config->raw_value = u8;
	config->spi_mode = (rp_spi_mode_t)(u8 & SPI_MODE_X_MASK);
	config->spi_ready = ((u8 & SPI_READY) ? RP_SPI_STATE_READY : RP_SPI_STATE_NOT);
	config->cs_mode = ((u8 & SPI_CS_HIGH) ? RP_SPI_CS_HIGH : RP_SPI_CS_NORMAL);

	if (ioctl(fd, SPI_IOC_RD_LSB_FIRST, &u8) < 0) {
		ERROR_LOG("Set SPI_IOC_RD_LSB_FIRST");
		return RP_HW_ESGS;
	}
	config->lsb_first = (u8 == SPI_LSB_FIRST ? RP_SPI_ORDER_BIT_LSB
											 : RP_SPI_ORDER_BIT_MSB);

	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &u8) < 0) {
		ERROR_LOG("Set SPI_IOC_RD_BITS_PER_WORD");
		return RP_HW_ESGS;
	}
	config->bits_per_word = (u8 == 0 ? 8 : u8);

	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &u32) < 0) {
		ERROR_LOG("Set SPI_IOC_RD_MAX_SPEED_HZ");
		return RP_HW_ESGS;
	}
	config->spi_speed = u32;

	return RP_HW_OK;
}



int write_spi_configuration(int fd, spi_config_t *config)
{
	uint8_t  u8;
	uint32_t u32;
	u8 = config->raw_value;
	u8 = (u8 & ~SPI_MODE_X_MASK) | (uint8_t)config->spi_mode;
	u8 = (u8 & ~SPI_READY) | (config->spi_ready == RP_SPI_STATE_READY ? SPI_READY : 0);
	u8 = (u8 & ~SPI_CS_HIGH) | (config->cs_mode == RP_SPI_CS_HIGH ? SPI_CS_HIGH : 0);

	if (ioctl(fd, SPI_IOC_WR_MODE, &u8) < 0) {
		ERROR_LOG("Set SPI_IOC_WR_MODE");
		return RP_HW_ESSS;
	}

	u8 = (config->lsb_first == RP_SPI_ORDER_BIT_LSB ? SPI_LSB_FIRST : 0);
	if (ioctl(fd, SPI_IOC_WR_LSB_FIRST, &u8) < 0) {
		ERROR_LOG("Set SPI_IOC_WR_LSB_FIRST");
		return RP_HW_ESSS;
	}

	u8 = config->bits_per_word;
	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &u8) < 0) {
		ERROR_LOG("Set SPI_IOC_WR_BITS_PER_WORD");
		return RP_HW_ESSS;
	}

	u32 = config->spi_speed;
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &u32) < 0) {
		ERROR_LOG("Set SPI_IOC_WR_MAX_SPEED_HZ");
		return RP_HW_ESSS;
	}

	return RP_HW_OK;
}

int read_write_spi_buffers(int fd, spi_data_t *data)
{
	// struct spi_ioc_transfer transfer = {
	// 	.tx_buf        = 0,
	// 	.rx_buf        = 0,
	// 	.len           = 0,
	// 	.delay_usecs   = 0,
	// 	.speed_hz      = 0,
	// 	.bits_per_word = 0,
	// };

	if (!data) {
		ERROR_LOG("Message for SPI not init");
		return RP_HW_ESMI;
	}

	struct spi_ioc_transfer *messages = calloc(data->size, sizeof(struct spi_ioc_transfer));

	if (!messages){
		ERROR_LOG("Can't allocate memory for spi_ioc_transfer");
		return RP_HW_EAL;
	}

	for(size_t i = 0; i < data->size; i++){
		memset(&messages[i], 0, sizeof (struct spi_ioc_transfer));
		messages[i].rx_buf = (unsigned long)data->messages[i].rx_buffer;
		messages[i].tx_buf = (unsigned long)data->messages[i].tx_buffer;
		messages[i].len = data->messages[i].size;
		messages[i].cs_change = (data->messages[i].cs_change ? 1 : 0);
	}


	if (ioctl(fd, SPI_IOC_MESSAGE(data->size), messages) < 0)
		return RP_HW_EST;

	// printf("Read size %d\n",transfer.len);
	return RP_HW_OK;
}
