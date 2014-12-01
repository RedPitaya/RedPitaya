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
hw_monitor::hw_monitor(){

	fd_control = -1;
	/* Page variables used to calculate correct mapping addresses for temperature control */
	char *page_ptr_led, *page_ptr_temp;
	long page_addr_led, page_addr_temp, page_off_led, page_off_temp, page_size = sysconf(_SC_PAGESIZE);

	/* Open file descriptor */
	fd_control = open("/dev/mem", O_RDWR | O_SYNC);

	/* Error check */
	if(fd_control < 0) {
		cout << " Error opening (/dev/mem)";
	}

	/* Led mmap */
	page_addr_led = LED_BASE_ADDR & (~(page_size-1));
	page_off_led  = LED_BASE_ADDR - page_addr_led;

	page_ptr_led = (char*)mmap(NULL, LED_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_control, page_addr_led);
	page_ptr_led = page_ptr_led + page_off_led;

	char *c_led = (char *)&led_struct;
	c_led = page_ptr_led;

	led_struct = (led_control_t*)c_led;

	/* Temp mmap */
	page_addr_temp = TEMP_BASE_ADDR & (~(page_size-1));
	page_off_temp = TEMP_BASE_ADDR - page_addr_temp;

	page_ptr_temp = (char*)mmap(NULL, TEMP_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_control, page_addr_temp);
	page_ptr_temp = page_ptr_temp + page_off_temp;

	char *c_temp= (char *)&temp_struct;
	c_temp = page_ptr_temp;

	temp_struct = (temp_struct_t*)c_temp;

}


/* Deconstructor */
hw_monitor::~hw_monitor(){
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
/*
void hw_monitor::usage(){
	char argv_max[] = "Too many arguments. Hw_control takes 0 arguments.\n\n"
			 	 	 "Usage: \n"
					 "\t- Argument 1: Led control\n"
			 	 	 "\t- Argument 2: Temperature control.\n";

	cout << argv_max << "\n";
}
*/

/* Sample function, can be turned into anything */
void hw_monitor::power_led(int l){
	led_struct->led_control = pow(2, l);
}

float hw_monitor::temp_control(){
	float celsius;
	float kelvin = (float)((temp_struct->temp_control)*503.975) / ADC_FULL_RANGE_CNT;
	cout << "Temperature: " << kelvin-275.15 << "°C | " << kelvin << " K" << endl;

	celsius = kelvin - 275.15;

	return (celsius);
}




