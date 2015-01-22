/* Red Pitaya C API example Acquiring a signal from a buffer  
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>

#include <rp.h>

int main(int argc, char **argv){

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

	rp_channel_t channel = RP_CH_1;
	rp_acq_trig_src_t trig_source = RP_TRIG_SRC_NOW;
	uint32_t array_size = 16 * 1024; //Current buffer size. 

	float *buff = (float *)malloc(array_size * sizeof(float));

	/* Starts acquisition */
	rp_AcqStart();
	rp_AcqSetTriggerSrc(trig_source);

	/* Get the whole buffer into buf */
	rp_AcqGetOldestDataV(channel, &array_size, buff);

	int i;
	for (i = 0; i < array_size; ++i)
	{
		printf("Data: %f\n", buff[i]);
	}

	/* Releasing resources */
	rp_Release();
}