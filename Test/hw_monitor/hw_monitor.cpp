/*
 * led.cpp
 *
 *  Created on: Nov 27, 2014
 *      Author: infused
 */

#include "hw_monitor.h"

#include <iostream>
using namespace std;

/* User defined commands*/
int main(int argc, char *argv[]){

	/* Create a hw monitor object */
	hw_monitor object;

	if(argc > 2){
		object.usage();
	}

	object.led_map();
	object.power_led();
}
