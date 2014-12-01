/*
 * led.h
 *
 *  Created on: Nov 27, 2014
 *      Author: infused
 */

#ifndef HW_MONITOR_HW_MONITOR_H_
#define HW_MONITOR_HW_MONITOR_H_

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
const int TEMP_BASE_SIZE = 0x30;

const int ADC_FULL_RANGE_CNT =  0xfff;



class hw_monitor {

public:
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

#endif /* HW_MONITOR_HW_MONITOR_H_ */
