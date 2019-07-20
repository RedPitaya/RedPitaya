
/* This code is sued for starting continous ranging on SRF02 sensor
 * 
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>



#define I2C_SLAVE_FORCE 		   0x0706
#define I2C_SLAVE    			   0x0703    /* Change slave address            */
#define I2C_FUNCS    			   0x0705    /* Get the adapter functionality */
#define I2C_RDWR    			   0x0707    /* Combined R/W transfer (one stop only)*/
 
 

int main(int argc, char **argv)
{
	printf("SRF02 Ranging program\n");

	int fd;										// File descrition
	char *fileName = "/dev/i2c-0";							// Name of the port we will be using
	int  address = 0x70;								// Address of the SRF08 shifted right 1 bit
	unsigned char buf[10];								// Buffer for data being read/ written on the i2c bus
	if ((fd = open(fileName, O_RDWR)) < 0) {					// Open port for reading and writing
		printf("Cannot open i2c port\n");
		exit(1);
	}

	if (ioctl(fd, I2C_SLAVE_FORCE, address) < 0) {					// Set the port options and set the address of the device we wish to speak to
		printf("Unable to get bus access to talk to slave\n");
		exit(1);
	}
while(1)
{
	buf[0] = 0;									// Commands for performing a ranging on the SRF02 in centimeters
	buf[1] = 81;

	if ((write(fd, buf, 2)) != 2) {							// Write commands to the i2c port
		printf("Error writing to i2c slave\n");
		exit(1);
	}

	usleep(70000);									// this sleep waits for the ping to come back. It should be greater than 65mS

	buf[0] = 0;									// This is the register we wish to read from

	if ((write(fd, buf, 1)) != 1) {							// Send register to read from
		printf("Error writing to i2c slave\n");
		exit(1);
	}

	if (read(fd, buf, 4) != 4) {							// Read back data into buffer
		printf("Unable to read from slave\n");
		exit(1);
	}
	else {
		unsigned char highByte = buf[2];
		unsigned char lowByte = buf[3];
		unsigned int result = (highByte << 8) + lowByte;			// Calculate range
		printf("Software v: %u \n", buf[0]);
		printf("Range was: %u\n", result);
		
	}
        usleep(1000000);                                                                //Wait for 1 second to start next ranging
        }


    return EXIT_SUCCESS;
}
