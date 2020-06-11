/* Red Pitaya C API example Generating continuous signal
 * This application generates a specific signal */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "rp.h"

#ifdef Z20_250_12
#include "rp-spi.h"
#include "rp-i2c-max7311.h"
#include "rp-gpio-power.h"
#endif


int main(int argc, char **argv){

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}
	float amp = 0.9;

	/* Generator reset */
	rp_GenReset();

	/* Generating frequency */
	rp_GenFreq(RP_CH_1, 0);
	rp_GenFreq(RP_CH_2, 0);

	/* Generating wave form */
	rp_GenWaveform(RP_CH_1, RP_WAVEFORM_DC);
	rp_GenWaveform(RP_CH_2, RP_WAVEFORM_DC);

#ifdef Z20_250_12
	rp_GenSetGainOut(RP_CH_1,RP_GAIN_5X);
	rp_GenSetGainOut(RP_CH_2,RP_GAIN_5X);
#endif

	/* Generating amplitude */
	rp_GenAmp(RP_CH_1, amp);
	rp_GenOffset(RP_CH_1, 0);

	rp_GenAmp(RP_CH_2, amp);
	rp_GenOffset(RP_CH_2, 0);

	/* Enable channel */
	rp_GenOutEnable(RP_CH_1);
	rp_GenOutEnable(RP_CH_2);

	/* Releasing resources */
	rp_Release();

	return 0;
}
