/* Red Pitaya C API example Generating continuous signal
 * This application generates a specific signal */

#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <signal.h>
#include <fstream> 
#include <functional>
#include <algorithm>
#include <numeric>

#include "rp.h"


char* getCmdOption(char ** begin, char ** end, const std::string & option,int index = 0){
    char ** itr = std::find(begin, end, option);
    while(itr != end && ++itr != end){
        if (index <= 0)
            return *itr;
        index--;
    };    
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option){
    return std::find(begin, end, option) != end;
}

bool CheckMissing(const char* val,const char* Message){
    if (val == NULL) {
        std::cout << "Missing parameters: " << Message << std::endl;
        return true;
    }
    return false;
}

void UsingArgs(char const* progName){
    printf("Usage with file: %s -d1|-d2|-s1|-s2|-g\n",progName);
	printf("\t-d1 = DC signal 0.45V\n");
	printf("\t-d2 = DC signal 1.0V\n");
	printf("\t-s1 = Sine signal 0.45V\n");
	printf("\t-s2 = Sine signal 4.0V\n");
    exit(-1);
}

int main(int argc, char **argv){

	bool   dc_045     = cmdOptionExists(argv, argv + argc, "-d1");
	bool   dc_09      = cmdOptionExists(argv, argv + argc, "-d2");

	bool   ac_045     = cmdOptionExists(argv, argv + argc, "-s1");
	bool   ac_09      = cmdOptionExists(argv, argv + argc, "-s2");


	if (!(dc_045 ^ dc_09 ^ ac_045 ^ ac_09)) {
            UsingArgs(argv[0]);
        }


	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}
	float amp = 0;

	/* Generator reset */
	rp_GenReset();

	if (dc_045){
		amp = 0.45;
		/* Generating frequency */
		rp_GenFreq(RP_CH_1, 0);
		rp_GenFreq(RP_CH_2, 0);
		/* Generating wave form */
		rp_GenWaveform(RP_CH_1, RP_WAVEFORM_DC);
		rp_GenWaveform(RP_CH_2, RP_WAVEFORM_DC);

	}

	if (dc_09){
		amp = 1;
		/* Generating frequency */
		rp_GenFreq(RP_CH_1, 0);
		rp_GenFreq(RP_CH_2, 0);
		/* Generating wave form */
		rp_GenWaveform(RP_CH_1, RP_WAVEFORM_DC);
		rp_GenWaveform(RP_CH_2, RP_WAVEFORM_DC);
	}

	if (ac_045){
		amp = 0.45;
		/* Generating frequency */
		rp_GenFreq(RP_CH_1, 1000);
		rp_GenFreq(RP_CH_2, 1000);
		/* Generating wave form */
		rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
		rp_GenWaveform(RP_CH_2, RP_WAVEFORM_SINE);
	}

	if (ac_09){
		amp = 1;
		/* Generating frequency */
		rp_GenFreq(RP_CH_1, 1000);
		rp_GenFreq(RP_CH_2, 1000);
		/* Generating wave form */
		rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
		rp_GenWaveform(RP_CH_2, RP_WAVEFORM_SINE);
	}


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
