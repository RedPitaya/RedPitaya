#include "gpio.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>


#define VALUE_MAX 40
#define MAX_PATH 64
#define LOW  0
#define HIGH 1

#define MSG_A(args...) fprintf(stderr,args);

int gpio_write(int pin, int value){
	char path[VALUE_MAX];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	// get pin value file descrptor
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		MSG_A("[rp_gpio] Unable to to open sysfs pins value file %s for writing\n",path);
		return -1;
	}
	if(value==LOW){
		//write low
		if (1 != write(fd, "0", 1)) {
			MSG_A("[rp_gpio] Unable to write value\n");
			return -1;
		}
	}
        else if(value==HIGH){
		//write high
		if (1 != write(fd, "1", 1)) {
                	MSG_A("[rp_gpio] Unable to write value\n");
                	return -1;
		}
	}else MSG_A("[rp_gpio] Nonvalid pin value requested\n");

	//close file
	close(fd);
	return 0;
}

int gpio_read(int pin){

	char path[VALUE_MAX];
	char value_str[3];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);

	// get pin file descriptor for reading its state
	fd = open(path, O_RDONLY);
	if (-1 == fd) {
		MSG_A("[rp_gpio] Unable to open gpio sysfs pin value file %s for reading\n",path);
		return -1;
	}

	// read value
	if (-1 == read(fd, value_str, 3)) {
		MSG_A("[rp_gpio] Unable to read value\n");
		close(fd);
		return -1;
	}

	// close file
	close(fd);

	// return integar value
	return atoi(value_str);
}


int gpio_pin_direction(int pin, int value){
	char path[VALUE_MAX];
	int fd;

	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	// get pin value file descrptor
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		MSG_A("[rp_gpio] Unable to to open sysfs pins value file %s for writing direction\n",path);
		return -1;
	}
	if(value == RP_GPIO_IN){
		//write in
		if (2 != write(fd, "in", 2)) {
			MSG_A("[rp_gpio] Unable to write direction value\n");
			close(fd);
			return -1;
		}
	}
        else if(value == RP_GPIO_OUT){
		//write out
		if (3 != write(fd, "out", 3)) {
                	MSG_A("[rp_gpio] Unable to write direction value\n");
					close(fd);
                	return -1;
		}
	}else MSG_A("[rp_gpio] Nonvalid pin direction requested\n");

	//close file
	close(fd);
	return 0;
}


int gpio_export(int pin){
	char path[VALUE_MAX];
	int fd;
	char buffer [10];

	snprintf(path, VALUE_MAX, "/sys/class/gpio/export");
	// get pin value file descrptor
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		MSG_A("[rp_gpio] Unable to to open sysfs pins value file %s for export\n",path);
		return -1;
	}
	sprintf(buffer, "%d", pin);
	int str_len = strlen(buffer);
	//write in
	if (str_len != write(fd, buffer, str_len)) {
		MSG_A("[rp_gpio] Unable to write export pin %s\n",buffer);
		close(fd);
		return -1;
	}
	
	//close file
	close(fd);
	return 0;
}


int gpio_unexport(int pin){
	char path[VALUE_MAX];
	int fd;
	char buffer [10];

	snprintf(path, VALUE_MAX, "/sys/class/gpio/unexport");
	// get pin value file descrptor
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		MSG_A("[rp_gpio] Unable to to open sysfs pins value file %s for unexport\n",path);
		return -1;
	}
   	sprintf(buffer, "%d", pin);
	int str_len = strlen(buffer);
	//write in
	if (str_len != write(fd, buffer, str_len)) {
		MSG_A("[rp_gpio] Unable to write unexport pin\n");
		close(fd);
		return -1;
	}
	
	//close file
	close(fd);
	return 0;
}