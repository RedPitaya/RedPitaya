#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"

#define Istrue(e) ((e) != 0) //Helper function. Int > 0 is true, else == 0 is false

int main(int argc, char **argv){

	/* Plink Hello message */
	fprintf(stderr, "HELLO RED PITAYA!\n");
	usleep(30000);

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	/* Set rp_dpin_t to led 1 */
	rp_dpin_t pin = RP_LED1;

	int retries = 1000; //ms
	while(Istrue(retries--)){
		/* Setting pin to 1 */
		rp_DpinSetState(pin, RP_HIGH);
		usleep(20000);
		rp_DpinSetState(pin, RP_LOW);
		usleep(20000);
	}

	/* Releasing resources */
	rp_Release();
	printf("SUCCSESFULLY RAN PROGRAM BLINK DIODE.\n");
	
	return 0;
}
