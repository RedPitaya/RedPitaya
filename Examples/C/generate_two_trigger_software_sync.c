#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"


int main(int argc, char **argv) {

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK) {
		fprintf(stderr, "Rp api init failed!\n");
	}

	rp_GenSynchronise();

	rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
	rp_GenFreq(RP_CH_1, 2000);
	rp_GenAmp(RP_CH_1, 1);

	rp_GenWaveform(RP_CH_2, RP_WAVEFORM_SINE);
	rp_GenFreq(RP_CH_2, 2000);
	rp_GenAmp(RP_CH_2, 1);

	rp_GenOutEnableSync(true);
	rp_GenSynchronise();

	/* Release rp resources */
	rp_Release();

	return 0;
}
