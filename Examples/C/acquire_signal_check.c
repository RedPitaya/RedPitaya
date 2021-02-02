/* Red Pitaya C API example Acquiring a signal from a buffer and check it
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "rp.h"

#define EPS 0.05
#define EPS_F 100

const float c_osc_fpga_smpl_freq = ADC_SAMPLE_RATE;
const int c_meas_time_thr = ADC_BUFFER_SIZE / (1000000000 / ADC_SAMPLE_RATE);
const float c_meas_freq_thr = 0.05;
const float c_min_period = 19.6e-9; // 51 MHz

void filterBuffer(float *_buffer,int _size){
    float *n_b = malloc(_size * sizeof(float));
    memcpy(n_b,_buffer,_size);
    float core[] = { 1.0 / 8.0 , 1.0 / 4.0 , 1.0 / 4.0 , 1.0 / 4.0 , 1.0 / 8.0};
    for(int i = 2 ; i < _size - 2 ; i++ ){
        float sum = 0;
        for (int j = -2 ; j <= 2 ; j++ ){
            sum += core[j + 2] * _buffer [i + j];
        }
        n_b[i] = sum;
    }
    memcpy(_buffer,n_b,_size);
    free(n_b);
}

bool checkAmplitudeAndFreq(float *_buff, uint32_t _size,float _nominal,float *min, float *max,float *frequency){
        int trig_t[2] = { 0, 0 };
        int trig_cnt = 0;
        int state = 0;
        if (_size > 0){
                *min = _buff[0];
                *max = _buff[0];
                for (int i = 1 ; i < _size ; ++i){
                        if (*min > _buff[i]) *min = _buff[i];
                        if (*max < _buff[i]) *max = _buff[i];
                }

                uint32_t dec_factor = 1;
                rp_AcqGetDecimationFactor(&dec_factor);

                float acq_dur=(float)(_size)/((float) c_osc_fpga_smpl_freq) * (float) dec_factor;
                float cen = (*max + *min) / 2;
                float thr1 = cen + 0.2 * (*min - cen);
                float thr2 = cen + 0.2 * (*max - cen);
                float res_period = 0;
                for(int i = 0; i < _size; i++) {
                        float sa = _buff[i];
                        if((state == 0) && (sa < thr1)) {
                                state = 1;
                        }
                        if((state == 1) && (sa >= thr2) ) {
                                state = 0;
                                if (trig_cnt++ == 0) {
                                trig_t[0] = i;
                                } else {
                                trig_t[1] = i;
                                }
                        }
                        if ((trig_t[1] - trig_t[0]) > c_meas_time_thr) {
                                break;
                        }
                }
                if(trig_cnt >= 2) {
                        res_period = (float)(trig_t[1] - trig_t[0]) /
                                ((float)c_osc_fpga_smpl_freq * (trig_cnt - 1)) * dec_factor;
                }

                if( ((thr2 - thr1) < c_meas_freq_thr) ||
                        (res_period * 3 >= acq_dur)   ||
                        (res_period < c_min_period) ){
                        res_period = 0;
                }
                float period = res_period * 1000.f;
                period = (period == 0.f) ?  0.000001f : period;
                *frequency = (float) (1 / (period / 1000.0));
                if ((fabs(*min + _nominal) < EPS) && (fabs(*max - _nominal) < EPS))
                        return true;
                return false;
        }
        return false;
}

float trapezoidalApprox(double *data, float T, int size){
    double result = 0;
    for(int i = 0; i < size - 1; i++){
        result += data[i] + data[i+1];
    }
    result = ((T / 2.0) * result);
    return result;
}

bool isSineTester(float *data, uint32_t size)
{
        uint32_t dec_factor = 1;
        rp_AcqGetDecimationFactor(&dec_factor);
        double T = (dec_factor / ADC_SAMPLE_RATE);
        double ch_rms[size];
        double ch_avr[size];
        for(int i = 0; i < size; i++) {
                ch_rms[i] = data[i] * data[i];
                ch_avr[i] = fabs(data[i]);
        }
        double K0 = sqrtf(T * size * trapezoidalApprox(ch_rms, T, size)) / trapezoidalApprox(ch_avr, T, size);
        return ((K0 > 1.10) && (K0 < 1.12));
}


int main(int argc, char **argv){

        bool fillState = false;
	int  counter=100;
        /* Print error, if rp_Init() function failed */
        if(rp_Init() != RP_OK){
                fprintf(stderr, "Rp api init failed!\n");
        }

        rp_GenReset();
        rp_GenFreq(RP_CH_1, 20000.0);
        rp_GenAmp(RP_CH_1, 1.0);
        rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
        rp_GenOutEnable(RP_CH_1);


        uint32_t buff_size = 16384;
        float *buff = (float *)malloc(buff_size * sizeof(float));

        rp_AcqReset();
        rp_AcqSetDecimation(RP_DEC_8);
        rp_AcqSetTriggerLevel(RP_CH_1, 0);
        rp_AcqSetTriggerDelay(ADC_BUFFER_SIZE/2.0);

        while(counter--){
                fillState = false;
                rp_AcqStart();

                /* After acquisition is started some time delay is needed in order to acquire fresh samples in to buffer*/
                /* Here we have used time delay of one second but you can calculate exact value taking in to account buffer*/
                /*length and smaling rate*/

                sleep(1);
                rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
                rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;

                while(1){
                        rp_AcqGetTriggerState(&state);
                        if(state == RP_TRIG_STATE_TRIGGERED){
                        break;
                        }
                }

                while(!fillState){
                        rp_AcqGetBufferFillState(&fillState);
                }

                rp_AcqStop();
                rp_AcqGetOldestDataV(RP_CH_1, &buff_size, buff);
                filterBuffer(buff,buff_size);
                printf("Acquiring Done\n");
                float min = 0;
                float max = 0;
                float frequency = 0;
                bool  isBrokenSignal = false;
                if (checkAmplitudeAndFreq(buff,buff_size,1.0, &min , &max , &frequency)) {
                        printf("\tAmplitude is correct MIN = %0.4f , MAX = %0.4f\n",min,max);
                }else{
                        printf("\tAmplitude is not correct MIN = %0.4f , MAX = %0.4f\n",min,max);
                        isBrokenSignal = true;
                }
                if (fabs(frequency - 20000.0) < EPS_F) {
                        printf("\tFrequency is correct %0.4f\n",frequency);
                }else{
                        printf("\tFrequency is not correct %0.4f\n",frequency);
                        isBrokenSignal = true;
                }

                if (isSineTester(buff,buff_size)){
                        printf("\tSignal form is sine\n");
                }else{
                        printf("\tSignal form is not sine\n");
                        isBrokenSignal = true;
                }

                printf("Signal is %s\n\n",isBrokenSignal ? "not correct" : "correct");
        }

        /* Releasing resources */
        free(buff);
        rp_Release();
        return 0;
}
