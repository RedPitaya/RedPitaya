/* Red Pitaya C API example one --TODO-- add exact example name 
 * This application turns one one led */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"

#define Istrue(e) ((e) != 0) //Helper function. Int > 0 is true, while 0 = false

const rp_pinState_t high_state = RP_HIGH;
const rp_pinState_t low_state = RP_LOW;

int main(int argc, char **argv){
	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	/* Set rp_dpin_t to led 1 */
	rp_dpin_t pin = RP_LED1;

	int retries = 1000; //ms
	while(Istrue(retries--)){
		/* Setting pin to 1 */
		rp_DpinSetState(pin, high_state);
		usleep(20000);
		rp_DpinSetState(pin, low_state);
		usleep(20000);
	}

	return 0;
}