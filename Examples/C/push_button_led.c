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
	rp_pinState_t stat = RP_LOW;

	rp_DpinSetDirection(pin, direction);

	/* You can set a timeout */
	//int i = 0;
	while(1){
		printf("Getting pin state.\n");
		rp_DpinGetState(pin, &stat);
		printf("Setting pin state.\n");
		if(stat == RP_LOW){
			rp_DpinSetState(RP_LED5, RP_HIGH);
			printf("Setting pin state: HIGH\n");
		}else{
			rp_DpinSetState(RP_LED5, RP_LOW);
			printf("Setting pin state: LOW\n");
		}
		//i++;
	}

	rp_Release();

	return 0;
}