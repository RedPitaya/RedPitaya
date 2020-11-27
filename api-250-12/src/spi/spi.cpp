#include "spi.h"

#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <linux/types.h>
#include <linux/spi/spidev.h>

#define MSG_A(args...) fprintf(stdout,args);
static char mode = SPI_MODE_0;

#define MAP_SIZE 4096UL
//#define FPGA_SPI_ADDR 0x40000000
#define MAP_MASK (MAP_SIZE - 1)

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
		close(spi_dev_node);
		return -1;
	}

	ret_val = ioctl(spi_dev_node, SPI_IOC_RD_MODE, &mode);
	if (ret_val < 0) {
		MSG_A("[rp_spi] Can't get spi mode.\n");
		close(spi_dev_node);
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
		close(spi_dev_node);
		return -1;
	}          
	close(spi_dev_node);
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
		close(spi_dev_node);
		return -1;
	}

	ret_val = ioctl(spi_dev_node, SPI_IOC_RD_MODE, &mode);
	if (ret_val < 0) {
		MSG_A("[rp_spi] Can't get spi mode.\n");
		close(spi_dev_node);
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
		close(spi_dev_node);
		return -1;
	}          
	close(spi_dev_node);
	return 0;
}


int write_to_fpga_spi(const char* _path,unsigned int fpga_address,unsigned short dev_address, char a_addr, unsigned char spi_val_to_write){
	int fd = -1;
	int retval = 0;
	void* map_base = (void*)(-1);

	if((fd = open(_path, O_RDWR | O_SYNC)) == -1) return -1;

	/* Read from command line */
	//unsigned long addr;

	/* Map one page */
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, fpga_address);
	if(map_base == (void *) -1) retval = -1;

	void* virt_addr = map_base + (dev_address & MAP_MASK);
	*((unsigned long *) virt_addr) = (unsigned long)a_addr;
	virt_addr = map_base	   + ((dev_address + 0x0004) & MAP_MASK);
	*((unsigned long *) virt_addr) = (unsigned long)spi_val_to_write;
	if (map_base != (void*)(-1)) {
		if(munmap(map_base, MAP_SIZE) == -1) retval = -1;
		map_base = (void*)(-1);
	}
	if (fd != -1) {
		close(fd);
	}
	return retval;
}

int read_from_fpga_spi(const char* _path,unsigned int fpga_address,unsigned short dev_address,char a_addr, char &value){
		int fd = -1;
	int retval = 0;
	void* map_base = (void*)(-1);

	if((fd = open(_path, O_RDWR | O_SYNC)) == -1) return -1;

	/* Map one page */
	map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, fpga_address);
	if(map_base == (void *) -1) retval = -1;
	void* virt_addr = map_base + (dev_address & MAP_MASK);
	*((unsigned long *) virt_addr) = (unsigned long)a_addr + 0x8000;
	virt_addr = map_base	   + ((dev_address + 0x0004) & MAP_MASK);
	*((unsigned long *) virt_addr) = (unsigned long)0x44;
	virt_addr = map_base	   + ((dev_address + 0x0008) & MAP_MASK);
	usleep(100 * 1000);
	value = (char)(*((uint32_t *) virt_addr));
	
	if (map_base != (void*)(-1)) {
		if(munmap(map_base, MAP_SIZE) == -1) retval = -1;
		map_base = (void*)(-1);
	}
	if (fd != -1) {
		close(fd);
	}
	return retval;
}