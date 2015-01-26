/* Red Pitaya C API example Bar Graph With Leds 
 * This application turns one one led */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <rp.h>

int main(int arc, char **argv){

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}
	rp_dpin_t pin;

	float p = 67;

	/* Turning on leds based on parameter p */
	for(pin = RP_LED1; pin < RP_LED7; pin++){
		if(p >= (100/7)*pin){
			rp_DpinSetState(pin, RP_HIGH);
		}else{
			rp_DpinSetState(pin, RP_LOW);
		}
		usleep(100000);
	}

	/* Turning off the leds with a short delay */
	for(pin = RP_LED1; pin < RP_LED7; pin++){
		rp_DpinSetState(pin, RP_LOW);
	}

	/* Releasing resources */
	rp_Release();

	return 0;
}