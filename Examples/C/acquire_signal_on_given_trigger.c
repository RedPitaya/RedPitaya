/* Red Pitaya API Example of sigal acquisition on given trigger */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"

int main(int argc, char **argv){

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	uint32_t buff_size = 16384;

	float *buff = (float *)malloc(buff_size * sizeof(float));

	rp_AcqSetDecimation(8);
	rp_AcqSetTriggerLevel(0.01); //Trig level is set in Volts while in SCPI is set in mV
	rp_AcqSetTriggerDelayNs(0);

	rp_AcqStart();

	rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);

	rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

	while(1){
		rp_AcqGetTriggerState(&state);
		if(state == RP_TRIG_STATE_TRIGGERED){
			break;
		}
	}
	rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);

	int i;
	for(i = 0; i < buff_size; i++){
		printf("%f\n", buff[i]);
	}
	/* Releasing resources */
	free(buff);
	rp_Release();

	return 0;
}