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
#include <sys/select.h>
#include <termios.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <queue>

#include "rp.h"

int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}

void flush_kbhit(){
		tcflush(0, TCIFLUSH); 
}

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
    printf("Usage with file: %s -C1|-C2 [-A20]\n",progName);
	printf("\t-C1 = for calibrate chennel 1\n");
	printf("\t-C2 = for calibrate chennel 2\n");
	printf("\t-A20 = enable attenuator 1:20\n");
    exit(-1);
}

float sumQueue(std::queue<float> q)
{
	float sum = 0;
	while (!q.empty()){
		sum += q.front();
		q.pop();
	}
	return sum;
}


int main(int argc, char **argv){

	bool   ch1_flag        = cmdOptionExists(argv, argv + argc, "-C1");
	bool   ch2_flag        = cmdOptionExists(argv, argv + argc, "-C2");
	bool   attenuator_flag = cmdOptionExists(argv, argv + argc, "-A20");
	std::queue<float>  q_area_avg;
	std::queue<float>  q_deviation_avg;
	int skip_print = 0;
	float min_deviation=1e6;
	float max_deviation=0;

	if (!(ch1_flag ^ ch2_flag)) {
            UsingArgs(argv[0]);
        }

	rp_channel_t channel = RP_CH_1;
	rp_acq_trig_src_t trigger_source = RP_TRIG_SRC_CHA_PE;
	if (ch2_flag) {
		channel = RP_CH_2;
		trigger_source = RP_TRIG_SRC_CHB_PE;
	}

	/* Print error, if rp_Init() function failed */
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Rp api init failed!\n");
	}

		// Need for setup Test board 

	rp_DpinSetDirection(RP_DIO7_N,RP_OUT);
	rp_DpinSetState(RP_DIO7_N,RP_HIGH);

	rp_GenReset();
	rp_GenFreq(channel, 1000.0);
	rp_GenAmp(channel, 1.0);
	rp_GenWaveform(channel, RP_WAVEFORM_SQUARE);
	rp_GenOutEnable(channel);
	
	uint32_t buff_size = 16384;
	float *buff = (float *)malloc(buff_size * sizeof(float));
	rp_AcqReset();
	rp_AcqSetAC_DC(channel,RP_AC);
	if (attenuator_flag){
		rp_AcqSetGain(channel,RP_HIGH);}
	else{
		rp_AcqSetGain(channel,RP_LOW);}
	
	rp_AcqSetDecimation(RP_DEC_8);
	rp_AcqSetTriggerLevel((rp_channel_trigger_t)channel,0.25);
	rp_AcqStart();

	/* After acquisition is started some time delay is needed in order to acquire fresh samples in to buffer*/
	/* Here we have used time delay of one second but you can calculate exact value taking in to account buffer*/
	/*length and smaling rate*/

	sleep(1);
	rp_AcqSetTriggerSrc(trigger_source);
	rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
	float filtred_deviation = 0;
	float max_buff = 0;
	float min_buff = 0;
	while(1){
			rp_AcqGetTriggerState(&state);
			if(state == RP_TRIG_STATE_TRIGGERED){
			//break;
				rp_AcqStop();
				rp_AcqGetOldestDataV(channel, &buff_size, buff);
				int i;
				float area  = 0;
				float area_avg = 0;
				float count = 0;
				float deviation = 0;

				for(i = 0; i < buff_size; i++){
						if (buff[i]>0){
							area += buff[i];
							count++;
						}
						if (max_buff < buff[i]) max_buff = buff[i];
						if (min_buff > buff[i]) min_buff = buff[i];
				}

				if (count == 0) count = 1;
				area_avg = area / count;
				for(i = 0; i < buff_size; i++){
						if (buff[i]>0){
							deviation += fabs(area_avg - buff[i]);
						}
				}
				q_area_avg.push(area_avg);
				q_deviation_avg.push(deviation);
				float filtred_area = sumQueue(q_area_avg)/q_area_avg.size();
				filtred_deviation  = sumQueue(q_deviation_avg)/q_deviation_avg.size();

				if (q_area_avg.size() > 100){
					q_area_avg.pop();
				}

				if (q_deviation_avg.size() > 100){
					q_deviation_avg.pop();
					if (min_deviation > filtred_deviation)
						min_deviation = filtred_deviation;
					if (max_deviation < filtred_deviation)
						max_deviation = filtred_deviation;
				}

				if (skip_print>10){
					if (max_deviation != 0)
						printf("avg of maximum value %6.3f deviation %6.3f (MIN %6.3f) - (MAX %6.3f) maximum V %6.3f minimum V %6.3f\r", filtred_area , filtred_deviation,
																	min_deviation,max_deviation,max_buff,min_buff);
					else
						printf("avg of maximum value %6.3f deviation %6.3f\r", filtred_area , filtred_deviation);
					fflush(stdout);
					skip_print=0;
				}
				skip_print++;
				rp_AcqStart();
				rp_AcqSetTriggerSrc(trigger_source);
				//state = RP_TRIG_STATE_TRIGGERED;
				
			}
			if (_kbhit()) {
				flush_kbhit();
				break;
			}
			
	}

	
	/* Releasing resources */
	free(buff);


	/* Releasing resources */
	rp_Release();
	
	std::ofstream file;
  	file.open ("/tmp/calib_test_result.txt");
  	file << filtred_deviation;
 	file.close();

	return 0;
}
