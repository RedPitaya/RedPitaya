/**
 * $Id: $
 *
 * @brief Testing procedure for Sensor Extension module
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <unistd.h>

#include "rp.h"

int main(int argc, char *argv[]){

	int j = 0;
	float sum = 0;

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}	

	/* Set PIN value */
	rp_dpin_t pin;
	for(pin = RP_DIO1_P; pin <= RP_DIO1_N; pin++){
		rp_DpinSetDirection(pin, RP_IN);
	}
	rp_DpinSetDirection(RP_DIO0_P, RP_OUT);
	rp_DpinSetState(RP_DIO0_P, 1);
	rp_DpinSetState(RP_DIO0_P, 0);

	rp_ApinSetValue(RP_AOUT0, 0.5);
	usleep(1000);
	rp_ApinSetValue(RP_AOUT1, 1);

	/* Get DIGITAL pin value */
	rp_dpin_t d_pin = RP_DIO2_P;
	while(d_pin < RP_DIO1_N){
		rp_pinState_t state;
		rp_DpinGetState(d_pin, &state);
		sum += state;
		j++;
		d_pin++;
	}
	;
	rp_apin_t a_pin = RP_AIN0;
	while(a_pin < RP_AIN3){
		float a_val;
		rp_ApinGetValue(a_pin, &a_val);
		if((a_val / 0.7) >= 0.45 && (a_val / 0.7) < 0.55){
			sum += 1;
		} 
		j++;
		a_pin++;
	}

	if(sum != 12.0){
		printf("Invalid output data: %f\n", sum);
		return -1;
	}
	return 0;
}
