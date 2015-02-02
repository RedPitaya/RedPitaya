/* Red Pitaya C API example Generating signal pulse on an external trigger 
 * This application generates a specific signal */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"

int main(int argc, char **argv){

	/* Burst count */
	int i;

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}
	
	rp_GenFreq(RP_CH_1, 100);
	rp_GenAmp(RP_CH_1, 1.0);
	rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);

	/* Enable output channel */
	rp_GenOutEnable(RP_CH_1);

	for(i = 0; i < 100; i++){
		usleep(200000);
		rp_GenBurstCount(RP_CH_1, 1);
		usleep(200000);
		rp_GenMode(RP_CH_1, RP_GEN_MODE_BURST);
	} 

	/*
	usleep(50000000);
	rp_GenBurstCount(channel, 1);
	rp_GenMode(channel, gen_mode);
	usleep(5000000);
	*/

	/* Releasing resources */
	rp_Release();
}
