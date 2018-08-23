/* Red Pitaya C API example Generating continuous signal
 * This application generates a specific signal */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "redpitaya/rp.h"

int main(int argc, char **argv){

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	/* Generator reset */
	rp_GenReset();

	/* Generating frequency */
	rp_GenFreq(RP_CH_1, 0);
	rp_GenFreq(RP_CH_2, 0);

	/* Generating wave form */
	rp_GenWaveform(RP_CH_1, RP_WAVEFORM_DC);
	rp_GenWaveform(RP_CH_2, RP_WAVEFORM_DC);

	/* Generating amplitude */
	rp_GenAmp(RP_CH_1, 0.9);
	rp_GenOffset(RP_CH_1, 0);

	rp_GenAmp(RP_CH_2, 0.9);
	rp_GenOffset(RP_CH_2, 0);

	/* Enable channel */
	rp_GenOutEnable(RP_CH_1);
	rp_GenOutEnable(RP_CH_2);

	/* Releasing resources */
	rp_Release();

	return 0;
}
