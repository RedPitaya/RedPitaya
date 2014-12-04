/*
 * main.cpp
 *
 *  Created on: Dec 1, 2014
 *      Author: infused
 */

#include <iostream>
#include <stdlib.h>
using namespace std;

#include "hw_monitor.h"

int main(){
	hw_monitor hw;

	while(1){
		for(int i = 1; i < 8; i++){
			hw.power_led(i);
			usleep(100000);
		}
		for(int j = 8; j >= 0; j--){
			hw.power_led(j);
			usleep(10000);
		}
	}

	/*
	float avg = 0;
	for(int k = 0; k < 100; k++){
		avg += hw.temp_control();
	}

	cout << "Average temp: " << avg/100 << endl;
	*/
}
