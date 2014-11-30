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

/* Base AMS address */
const int TEMP_BASE_ADDR = 0x40400000;
const int TEMP_BASE_SIZE = 0x31;

const int ADC_FULL_RANGE_CNT =  0xfff;

/* File descriptor */
int fd_control = -1;

/* Led structure declaration */
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

/* Temperature structure declaration */
typedef struct temp_struct_s {
	unsigned long int xadc_aif0;
	unsigned long int xadc_aif1;
	unsigned long int xadc_aif2;
	unsigned long int xadc_aif3;
	unsigned long int xadc_aif4;
	unsigned long int reserved_1;
	unsigned long int reserved_2;
	unsigned long int reserved_3;
	unsigned long int pwm_dac0;
	unsigned long int pwm_dac1;
	unsigned long int pwm_dac2;
	unsigned long int pwm_dac3;
	unsigned long int temp_control;
}temp_struct_t;

/* Address structure set to null */
led_control_t *led_struct = NULL;
temp_struct_t *temp_struct = NULL;

class hw_monitor {

/*Function declarations */
public:
	hw_monitor();
	~hw_monitor();
	void usage();
	int init(int argv);
	int exit();
	void power_led(int l);
	void temp_control();
};

/* Constructor */
hw_monitor::hw_monitor(){}

int hw_monitor::init(int argv){

	/* Page variables used to calculate correct mapping addresses for temperature control */
	char *page_ptr;
	long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

	/* Open file descriptor */
	fd_control = open("/dev/mem", O_RDWR | O_SYNC);

	/* Error check */
	if(fd_control < 0) {
		cout << " Error opening (/dev/mem)";
	}

	if(argv == 1){
		page_addr = LED_BASE_ADDR & (~(page_size-1));
		page_off  = LED_BASE_ADDR - page_addr;

		page_ptr = (char*)mmap(NULL, LED_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_control, page_addr);
		page_ptr = page_ptr + page_off;

		char *c_led = (char *)&led_struct;
		c_led = page_ptr;

		led_struct = (led_control_t*)c_led;
	}else{
		page_addr = TEMP_BASE_ADDR & (~(page_size-1));
		page_off = TEMP_BASE_ADDR - page_addr;

		page_ptr = (char*)mmap(NULL, TEMP_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_control, page_addr);
		page_ptr = page_ptr + page_off;

		char *c_temp= (char *)&temp_struct;
		c_temp = page_ptr;

		temp_struct = (temp_struct_t*)c_temp;

	}
	return(0);
}

 int hw_monitor::exit(){
	 if(led_struct){
		 if(munmap(led_struct, LED_BASE_SIZE) < 0){
			 cout << "munmap failed." << endl;
		 }
		 led_struct = NULL;
	 }
	 if(temp_struct){
		 if(munmap(temp_struct, TEMP_BASE_SIZE) < 0){
			 cout << "munmap failed." << endl;
		 }
		 temp_struct = NULL;
	 }
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
void hw_monitor::power_led(int l){
	/* Initialize mmap process for led map address */
	this->init(1);

	/* Turining on all the leds
	for(int i = 1; i < 9; i++){
		led_struct->led_control = pow(2, i);
		usleep(100000);
	}

	/* Powering off all the leds
	for(int j = 9; j >= 0; j--){
		led_struct->led_control = pow(2, j);
		usleep(100000);
	}
	*/

	led_struct->led_control = pow(2, l);

	hw_monitor::exit();
}

void hw_monitor::temp_control(){
	/* Initialize mmap process for AMS address */
	this->init(2);

	float kelvin = (float)((temp_struct->temp_control)*503.975) / ADC_FULL_RANGE_CNT;
	cout << "Temperature: " << kelvin-275.15 << "°C | " << kelvin << " K" << endl;
	hw_monitor::exit();
}


#endif /* HW_MONITOR_H_ */
