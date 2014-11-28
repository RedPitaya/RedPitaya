/*
 * led.h
 *
 *  Created on: Nov 27, 2014
 *      Author: infused
 */

#ifndef LED_H_
#define LED_H_

#include <iostream>
using namespace std;

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

class led_init {
/*Function declarations */
public:
	led_init();
	~led_init();
	void usage(void);
	void led_map();
};

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


/* The constructor initialized the map process */
led_init::led_init(){}

void led_init::led_map(){

	/* Address structure set to null */
	led_control_t *led_struct = NULL;

	/* Page variables used to calculate correct mapping addresses for temperature control */
	void *page_ptr;
	long page_addr, page_off, page_size = sysconf(_SC_PAGESIZE);

	/* Open file descriptor */
	fd_control = open("/dev/mem", O_RDWR | O_SYNC);

	/* Error check */
	if(fd_control < 0) {
		cout << " Error opening (/dev/mem)";
	}

	page_addr = LED_BASE_ADDR & (~(page_size-1));
	page_off  = LED_BASE_ADDR - page_addr;

	page_ptr = mmap(NULL, LED_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_control, page_addr);


	//led_struct = (void*)(static_cast<char *>(page_ptr) + page_off);

}

led_init::~led_init(){}

void led_init::usage(void){
	char argv_max[] = "Too many arguments! LCR control takes exactly 1 argument."
			 	 	 "Usage: \n"
					 "\t-Argument 1: Led control\n"
			 	 	 "\t-Argument 2: Temperature control.\n";

	cout << argv_max << "\n";
}

#endif /* LED_H_ */
