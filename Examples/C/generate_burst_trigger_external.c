/* Red Pitaya external trigger pulse generation Example */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"


int main(int argc, char **argv){

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	rp_GenReset();
	
	rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
	rp_GenFreq(RP_CH_1, 200);
	rp_GenAmp(RP_CH_1, 1);

	rp_GenBurstCount(RP_CH_1, 1);
	rp_GenMode(RP_CH_1, RP_GEN_MODE_BURST);
	rp_GenTriggerSource(RP_CH_1, RP_GEN_TRIG_SRC_EXT_PE);

	rp_GenOutEnable(RP_CH_1);
	rp_GenTriggerOnly(RP_CH_1);

	/* Release rp resources */
	rp_Release();

	return 0;
}
