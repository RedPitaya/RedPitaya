/*
 * led.h
 *
 *  Created on: Nov 27, 2014
 *      Author: infused
 */

#ifndef HW_MONITOR_H_
#define HW_MONITOR_H_

#include <iostream>
using namespace std;

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <cmath>
#include <stddef.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/** Base Housekeeping address */
const int LED_BASE_ADDR = 0x40000000;
const int LED_BASE_SIZE = 0x30;

int fd_control = -1;

/* Structure declaration */
typedef struct led_control_s{
	unsigned long int id;
	unsigned long int dna_part1;
	unsigned long int dna_part2;
	unsigned long int reserved_1;
	unsigned long int ex_cd_p;
	unsigned long int ex_cd_n;
	unsigned long int ex_co_p;
	unsigned long int ex_co;
	unsigned long int ex_ci_p;
	unsigned long int ex_ci_n;
	unsigned long int reserved_2;
	unsigned long int reserved_3;
	unsigned long int led_control;
} led_control_t;

/* Address structure set to null */
led_control_t *led_struct;

class hw_monitor {
/*Function declarations */
public:
	hw_monitor();
	~hw_monitor();
	void usage();
	void led_map();
	void power_led();
};

/* Constructor */
hw_monitor::hw_monitor(){}

/* Usage fuction */

void hw_monitor::led_map(){

	/* Page variables used to calculate correct mapping addresses for temperature control */
	char *page_ptr;
	long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

	/* Open file descriptor */
	fd_control = open("/dev/mem", O_RDWR | O_SYNC);

	/* Error check */
	if(fd_control < 0) {
		cout << " Error opening (/dev/mem)";
	}

	page_addr = LED_BASE_ADDR & (~(page_size-1));
	page_off  = LED_BASE_ADDR - page_addr;

	page_ptr = (char*)mmap(NULL, LED_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_control, page_addr);
	page_ptr = page_ptr + page_off;

	char *c_led = (char *)&led_struct;
	c_led = page_ptr;

	led_struct = (led_control_t*)c_led;
}

/* Deconstructor */
hw_monitor::~hw_monitor(){}

void hw_monitor::usage(){
	char argv_max[] = "Too many arguments. Hw_control takes 0 arguments.\n\n"
			 	 	 "Usage: \n"
					 "\t- Argument 1: Led control\n"
			 	 	 "\t- Argument 2: Temperature control.\n";

	cout << argv_max << "\n";
}


/* Sample function, can be turned into anything */
void hw_monitor::power_led(){

	/* Turining on all the leds */
	for(int i = 1; i < 9; i++){
		led_struct->led_control = pow(2, i);
		usleep(100000);
	}

	/* Powering off all the leds */
	for(int j = 9; j >= 0; j--){
		led_struct->led_control = pow(2, j);
		usleep(100000);
	}
}


#endif /* HW_MONITOR_H_ */
