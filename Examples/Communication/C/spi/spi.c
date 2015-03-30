
/* @brief This is a simple application for testing SPI communication on a RedPitaya
 * @Author Luka Golinar <luka.golinar@redpitaya.com>
 * 
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/spi/spidev.h>

/* Inline functions definition */
static int init_spi();
static int release_spi();
static int read_spi();
static int write_spi(char *buffer, int size);

/* Constants definition */
int spi_fd = -1;

int main(void){

	char *buffer = "REDPITAYA SPI TEST";

	/* Init the spi resources */
	if(init_spi() < 0){
		printf("Initialization of SPI failed. Error: %s\n", strerror(errno));
		return -1;
	}

	if(write_spi(buffer, strlen(buffer)) < 0){
		printf("Write to SPI failed. Error: %s\n", strerror(errno));
		return -1;
	}

	if(read_spi() < 0){
		printf("ReaD from SPI failed. Error: %s\n", strerror(errno));
		return -1;
	}

	if(release_spi() < 0){
		printf("Relase of SPI resources failed, Error: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

static int init_spi(){


	/* MODES: mode |= SPI_LOOP; 
	 *        mode |= SPI_CPHA; 
	 *        mode |= SPI_CPOL; 
	 *		  mode |= SPI_LSB_FIRST; 
	 *        mode |= SPI_CS_HIGH; 
	 *        mode |= SPI_3WIRE; 
	 *        mode |= SPI_NO_CS; 
	 *        mode |= SPI_READY;
	 *
	 * multiple possibilities possible using | */

	int mode = 0;

	/* Opening file stream */
	spi_fd = open("/dev/spidev1.0", O_RDWR | O_NOCTTY);

	if(spi_fd < 0){
		printf("Error opening spidev0.1. Error: %s\n", strerror(errno));
		return -1;
	}


	if(ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0){
		printf("Error setting SPI_IOC_RD_MODE. Error: %s\n", strerror(errno));
		return -1;
	}

	int spi_speed = 1000000;

	if(ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0){
		printf("Error setting SPI_IOC_WR_MAX_SPEED_HZ. Error: %s\n", strerror(errno));
		return -1;
	}

	return 0;
}

static int release_spi(){

	close(spi_fd);

	return 0;
}

static int read_spi(){

	return 0;
}

static int write_spi(char *buffer, int size){

	int write_spi = write(spi_fd, buffer, strlen(buffer));

	if(write_spi < 0){
		printf("Failed to write to SPI. Error: %s\n", strerror(errno));
		return -1;
	}

	int i;
	for(i = 0; i < size; i++){
		printf("i: %d | Element at i: %d\n", i, buffer[i]);
	}


	return 0;
}