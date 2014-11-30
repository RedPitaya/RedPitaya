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
int main(){

	/* Create a hw monitor object */
	hw_monitor object;

	/* Power on leds */
	object.power_led();

	/* Cout temperature */
	object.temp_control();
}
