/* Read analog voltage on slow analog input */

#include <stdio.h>
#include <stdlib.h>

#include <rp.h>

int main(int argc, char **argv){

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	float volts;
	rp_ApinGetValue(RP_AIN3, &volts);
	printf("Volts: %f\n", volts);

	rp_dpin_t led_pin = 0;

	float p = volts*(100/3.3);

	int i;
	for(i = 1; i < 7; i++){
		if(p >= (100/7) * i){
			rp_DpinSetState(led_pin + i, RP_HIGH);
		}else{
			rp_DpinSetState(led_pin + i, RP_LOW);
		}
	}

	rp_Release();
	
	return 0;
	
}