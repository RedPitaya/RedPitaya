/* Red Pitaya C API example one --TODO-- add exact example name */

#include <stdio.h>
#include <stdlib.h>

#include <rp.h>

int main(int argc, char **argv){

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	rp_dpin_t pin = RP_DIO5_N;
	rp_pinDirection_t direction = RP_IN;
	rp_pinState_t *stat = NULL;

	rp_DpinSetDirection(pin, direction);

	rp_dpin_t led_pin = RP_LED5;

	int i = 0;
	while(i++ < 1000){
		rp_DpinGetState(pin, stat);
		
		if(stat == RP_LOW){
			rp_DpinSetState(led_pin, RP_HIGH);
		}else{
			rp_DpinSetState(led_pin, RP_LOW);
		}
	}

	rp_Release();

	return 0;
}