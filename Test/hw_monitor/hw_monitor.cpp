/*
 * led.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: infused
 */

#include "hw_monitor.h"

#include <iostream>
using namespace std;


/* Constructor */
hw_monitor::hw_monitor(){}

/* Deconstructor */
hw_monitor::~hw_monitor(){}


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

