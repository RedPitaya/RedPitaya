/*
 * Red Pitaya Api test application
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "../src/rp.h"



int main(int argc, char **argv){



	int ret_val = 0;
	ret_val = rp_Init();

	//rp_pinState_t high_state = RP_HIGH;
	//rp_pinState_t low_state = RP_LOW;  

	if(ret_val != RP_OK){
		//printf(stderr, "Rp library init failed");
	}
	//API Turning on one led 1
	/*
	rp_dpin_t pin = RP_LED1;

	while(1){
		//fprintf("Testing LED1 ON: %d\n",);
		rp_DpinSetState(pin, RP_HIGH);
	 	usleep(200000);
	 	//fprintf("Testing LED1 OFF: %d\n", ); 
	 	rp_DpinSetState(pin, RP_LOW);
	 	usleep(200000);
	}
	
	//API 2 Turning on leds based on p parameter
	rp_dpin_t pin;
	float p = 67;
	for(pin = RP_LED1; pin < RP_LED7; pin++){
		if(p >= (100/7)*pin){
			rp_DpinSetState(pin, high_state);
		}else{
			rp_DpinSetState(pin, low_state);
		}
		usleep(100000);
	}
	*/

	//API N Generating Signal at 125Msps
	/*
	rp_waveform_t signal_form = RP_WAVEFORM_SQUARE;
	rp_channel_t channel = RP_CH_1; //0
	//rp_trig_src_t trig_source = RP_TRIG_SRC_INTERNAL;
	
	if(rp_GenFreq(channel, 10000.0) == RP_OK){
		printf("Succesfuly set frequency.\n");
	}

	if(rp_GenWaveform(channel, signal_form) == RP_OK){
		printf("Succesfully set signal form.\n");
	}
	
	if(rp_GenAmp(channel, 1.0) == RP_OK){
		printf("Succesfuly set amplitude.\n");
	}

	if(rp_GenOutEnable(channel) == RP_OK){
		printf("Succesfuly enabled ch1.\n");
	}

	*/
	rp_acq_trig_src_t trig_source = RP_TRIG_SRC_NOW;
	uint32_t array_size = 16 * 1024;

	float *buff = (float *)malloc(array_size * sizeof(float));

	rp_AcqStart();
	rp_AcqSetTriggerSrc(trig_source);
	printf("Checking seg. fault 1\n");
	int ret = rp_AcqGetOldestDataV(channel, &array_size, buff);
	printf("Ret val: %d\n", ret);

	printf("Checking seg. fault 2\n");
	int i;
	for (i = 0; i < array_size; ++i)
	{
		printf("Data: %f\n", buff[i]);
	}

	if(rp_GenOutEnable(channel) == RP_OK){
		printf("Succesfuly enabled ch1.\n");
	}

	ret_val = rp_Release();
}

 
