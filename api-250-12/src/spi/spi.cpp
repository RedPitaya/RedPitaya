#include "spi.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>

#define MSG_A(args...) fprintf(stderr,args);
static char mode = SPI_MODE_0;

int write_to_spi(const char* spi_dev_path,char *buffer_header,int header_length, unsigned char spi_val_to_write){
    int spi_dev_node = 0;
	int ret_val = 0;

	/* Open the device node for the SPI adapter of bus 4 */
	spi_dev_node = open(spi_dev_path, O_RDWR);
	if (spi_dev_node < 0) {
		MSG_A("[rp_spi] Unable to open device node.\n");
		return -1;
	}

	/* spi mode */
	ret_val = ioctl(spi_dev_node, SPI_IOC_WR_MODE, &mode);
	if (ret_val < 0) {
		MSG_A("[rp_spi] Can't set spi mode.\n");
		return -1;
	}

	ret_val = ioctl(spi_dev_node, SPI_IOC_RD_MODE, &mode);
	if (ret_val < 0) {
		MSG_A("[rp_spi] Can't get spi mode.\n");
		return -1;
	}
	
	struct spi_ioc_transfer mesg[2] = { 0, };

    mesg[0].tx_buf = (unsigned long)buffer_header;
    mesg[0].rx_buf = (unsigned long)NULL;
    mesg[0].len = header_length;
    mesg[0].cs_change = 0;

    mesg[1].tx_buf = (unsigned long)&spi_val_to_write;
    mesg[1].rx_buf = (unsigned long)NULL;
    mesg[1].len = 1;
 
	ret_val = ioctl(spi_dev_node, SPI_IOC_MESSAGE(2), mesg);

    if (ret_val < 0) {
		MSG_A("[rp_spi] SPI Write Operation failed.\n");
		return -1;
	}          
	return 0;
}


int read_from_spi(const char* spi_dev_path,char *buffer_header,int header_length, char &value){
    int spi_dev_node = 0;
	int ret_val = 0;
	
	/* Open the device node for the SPI adapter of bus 4 */
	spi_dev_node = open(spi_dev_path, O_RDWR);
	if (spi_dev_node < 0) {
		MSG_A("[rp_spi] Unable to open device node.\n");
		return -1;
	}

	/* spi mode */
	ret_val = ioctl(spi_dev_node, SPI_IOC_WR_MODE, &mode);
	if (ret_val < 0) {
		MSG_A("[rp_spi] Can't set spi mode.\n");
		return -1;
	}

	ret_val = ioctl(spi_dev_node, SPI_IOC_RD_MODE, &mode);
	if (ret_val < 0) {
		MSG_A("[rp_spi] Can't get spi mode.\n");
		return -1;
	}
	
	struct spi_ioc_transfer mesg[2] = { 0, };

    mesg[0].tx_buf = (unsigned long)buffer_header;
    mesg[0].rx_buf = (unsigned long)NULL;
    mesg[0].len = header_length;
    mesg[0].cs_change = 0;

    mesg[1].tx_buf = (unsigned long)NULL;
	mesg[1].rx_buf = (unsigned long)value;
    mesg[1].len = 1;
 
	ret_val = ioctl(spi_dev_node, SPI_IOC_MESSAGE(2), mesg);

    if (ret_val < 0) {
		MSG_A("[rp_spi] SPI Read Operation failed.\n");
		return -1;
	}          
	return 0;
}

