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

#define MSG(...) if (g_enable_verbous) fprintf(stdout,__VA_ARGS__);
#define MSG_A(...) fprintf(stdout,__VA_ARGS__);

#define MAP_SIZE 4096UL
//#define FPGA_SPI_ADDR 0x40000000
#define MAP_MASK (MAP_SIZE - 1)

int write_to_fpga_spi(const char* _path,unsigned int fpga_address,unsigned short dev_address, char a_addr, unsigned char spi_val_to_write){
	int fd = -1;
	int retval = 0;
	char* map_base = (char*)(-1);

	if((fd = open(_path, O_RDWR | O_SYNC)) == -1) return -1;

	/* Read from command line */
	//unsigned long addr;

	/* Map one page */
	map_base = (char*)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, fpga_address);
	if(map_base == (char *) -1) retval = -1;

	void* virt_addr = map_base + (dev_address & MAP_MASK);
	*((unsigned long *) virt_addr) = (unsigned long)a_addr;
	virt_addr = map_base	   + ((dev_address + 0x0004) & MAP_MASK);
	usleep(1000);
	*((unsigned long *) virt_addr) = (unsigned long)spi_val_to_write;
	if (map_base != (char*)(-1)) {
		if(munmap(map_base, MAP_SIZE) == -1) retval = -1;
		map_base = (char*)(-1);
	}
	if (fd != -1) {
		close(fd);
	}
	return retval;
}

int read_from_fpga_spi(const char* _path,unsigned int fpga_address,unsigned short dev_address,char a_addr, char &value){
		int fd = -1;
	int retval = 0;
	char* map_base = (char*)(-1);

	if((fd = open(_path, O_RDWR | O_SYNC)) == -1) return -1;

	/* Map one page */
	map_base = (char*)mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, fpga_address);
	if(map_base == (char *) -1) retval = -1;
	void* virt_addr = map_base + (dev_address & MAP_MASK);
	*((unsigned long *) virt_addr) = (unsigned long)a_addr + 0x8000;
	virt_addr = map_base	   + ((dev_address + 0x0004) & MAP_MASK);
	*((unsigned long *) virt_addr) = (unsigned long)0x44;
	virt_addr = map_base	   + ((dev_address + 0x0008) & MAP_MASK);
	usleep(1000);
	value = (char)(*((uint32_t *) virt_addr));

	if (map_base != (char*)(-1)) {
		if(munmap(map_base, MAP_SIZE) == -1) retval = -1;
		map_base = (char*)(-1);
	}
	if (fd != -1) {
		close(fd);
	}
	return retval;
}
