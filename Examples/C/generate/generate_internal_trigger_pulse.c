/* Red Pitaya C API example Generating signal pulse on an external trigger 
 * This application generates a specific signal */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"

int main(int argc, char **argv){

	/* Burst count */
	//int n = 10;
	int i;

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	//rp_acq_trig_src_t trig_source = RP_TRIG_SRC_NOW; //External trigger on positive edge
	rp_channel_t channel = RP_CH_1;
	rp_waveform_t wave_form = RP_WAVEFORM_SINE;
	rp_gen_mode_t gen_mode = RP_GEN_MODE_BURST;

	rp_GenFreq(channel, 100);
	rp_GenAmp(channel, 1.0);
	rp_GenWaveform(channel, wave_form);

	/* Enable output channel */
	rp_GenOutEnable(channel);

	/* Commented for loop */
	for(i = 0; i < 100; i++){
		usleep(200000);
		rp_GenBurstCount(channel, 1);
		usleep(200000);
		rp_GenMode(channel, gen_mode);
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
