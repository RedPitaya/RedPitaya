/* Red Pitaya C API example Generating continuous signal  
 * This application generates a specific signal */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <rp.h>

int main(int argc, char **argv){

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	rp_waveform_t wave_form = RP_WAVEFORM_SINE;
	rp_channel_t channel = RP_CH_1;

	/* Generating frequency */
	rp_GenFreq(channel, 10000.0);

	/* Generating amplitude */
	rp_GenAmp(channel, 1.0);

	/* Generating wave form */
	rp_GenWaveform(channel, wave_form);

	/* Enable channel */
	rp_GenOutEnable(channel);

	/* Releasing resources */
	rp_Release();

	return 0;
}