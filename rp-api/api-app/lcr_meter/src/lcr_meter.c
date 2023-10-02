/**
* $Id: $
*
* @brief Red Pitaya application Impedance analyzer module interface
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <stdatomic.h>

#include "lcr_meter.h"
#include "utils.h"
#include "calib.h"
#include "rp.h"
#include "rp_hw-calib.h"

// /* Global variables definition */
// #ifdef Z20_250_12
// #define MIN_PERIODES 4
// #else
// #define MIN_PERIODES 8
// #endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef double data_t;

typedef struct impendace_params {
    float frequency;
    float _Complex z_out;
    float phase_out;
} impendace_params_t;

/* Thread variables */
pthread_t                  *g_lcr_thread_handler = NULL;
pthread_mutex_t             g_mutex;
volatile atomic_flag        g_thread_flag = ATOMIC_FLAG_INIT;
volatile impendace_params_t g_th_params;


// static bool                 g_isCuruitOpend            = false;
// static bool                 g_isFrequencyChange        = false;
bool                        g_isShuntAutoChange        = true;
bool                        g_isSine                   = true;


/* Init lcr params struct */
lcr_params_t main_params = {0, 0, CALIB_NONE, false, 0 , 0 , 0, true};

/* Main lcr data params */
lcr_main_data_t calc_data;


/* R shunt values definition */
const double SHUNT_TABLE[] =
{10, 1e2, 1e3, 1e4, 1e5, 1.3e6};

const int RANGE_FORMAT[] =
{1.0, 10.0, 100.0, 1000.0};

const double RANGE_UNITS[] =
{1e-9, 1e-6, 1e-3, 1, 1e3, 1e6};

double C_CALIB[6/*shunt*/][6/*freq*/] =
//100    1k     10k    100k     1M
{{1E-12,1E-12, 1E-12, 1E-12, 1E-12 , 1E-12}, // 10
 {1E-12,1E-12, 1E-12, 1E-12, 1E-12 , 1E-12}, // 100
 {1E-12,1E-12, 1E-12, 1E-12, 1E-12 , 1E-12}, // 1k
 {-1160E-12,-1160E-12, -113E-12, -13E-12, 176E-12, 176E-12}, // 10k
 {-146E-12,-146E-12, -17E-12, 175E-12, 288E-12, 288E-12},     // 100k
 {-40E-12,-40E-12, 230E-12, 290E-12, 170E-12, 170E-12} // 1M
};

void flog(char *s){
    FILE *out = fopen("/tmp/debug.log", "a+");
    fprintf(out, "%s", s);
    fclose(out);
}

int getIndex(double x)
{
    int index = -1;
    do {
        index++;
        x /= 10;
    } while(x > 5);
    return index;
}

data_t accu_buf[2][5];
int    accu_buf_size = 0;

/* Init the main API structure */
int lcr_Init()
{
    accu_buf_size = 0;
    /* Init mutex thread */
    if(pthread_mutex_init(&g_mutex, NULL)){
        RP_LOG(LOG_ERR, "Failed to initialize mutex: %s\n", strerror(errno));
        return RP_EOOR;
    }

    pthread_mutex_lock(&g_mutex);

    if(rp_Init() != RP_OK) {
        RP_LOG(LOG_ERR, "Unable to inicialize the RPI API structure "
                        "needed by impedance analyzer application: %s\n", strerror(errno));
        return RP_EOOR;
    }

    /* Set default values of the lcr structure */
    lcr_SetDefaultValues();
    rp_AcqReset();
    rp_GenReset();

    /* Set calibration values */
    FILE* f_calib = fopen("/opt/redpitaya/www/apps/lcr_meter/CAPACITOR_CALIB", "rb");
    if(f_calib) {
        int readed = fread(&C_CALIB, sizeof(C_CALIB), 1, f_calib);
        if(readed != 1){
            fclose(f_calib);
            pthread_mutex_unlock(&g_mutex);
            return RP_RCA;
        }
        fclose(f_calib);
    }
    pthread_mutex_unlock(&g_mutex);
    lcr_GenRun();
    return RP_OK;
}

/* Release resources used the main API structure */
int lcr_Release(){
    lcr_GenStop();
    pthread_mutex_lock(&g_mutex);
    rp_Release();
    RP_LOG(LOG_INFO, "Releasing Red Pitaya library resources.\n");
    pthread_mutex_unlock(&g_mutex);
    pthread_mutex_destroy(&g_mutex);
    return RP_OK;
}

/* Set default values of all rpi resources */
int lcr_Reset(){
    pthread_mutex_lock(&g_mutex);
    rp_Reset();
    /* Set default values of the lcr_params structure */
    lcr_SetDefaultValues();
    pthread_mutex_unlock(&g_mutex);
    return RP_OK;
}

int lcr_SetDefaultValues(){
    EXEC_CHECK(lcr_setRShunt(0));
    set_IIC_Shunt(0);
    EXEC_CHECK(lcr_SetFrequency(10.0));
    EXEC_CHECK(lcr_SetCalibMode(CALIB_NONE));
    EXEC_CHECK(lcr_SetMeasTolerance(0));
    EXEC_CHECK(lcr_SetMeasRangeMode(0));
    EXEC_CHECK(lcr_SetRangeFormat(0));
    EXEC_CHECK(lcr_SetRangeUnits(0));
    EXEC_CHECK(lcr_SetMeasSeries(true));
    accu_buf_size = 0;
    return RP_OK;
}

int  lcr_GenRun(){
    return lcr_SafeThreadGen(RP_CH_1, main_params.frequency);
}

int  lcr_GenStop(){
    pthread_mutex_lock(&g_mutex);
    ECHECK_APP_MUTEX(g_mutex,rp_GenOutDisable(RP_CH_1));
    ECHECK_APP_MUTEX(g_mutex,rp_GenOutDisable(RP_CH_2));
    pthread_mutex_unlock(&g_mutex);
    return RP_OK;
}

/* Generate functions  */
int lcr_SafeThreadGen(rp_channel_t channel,
                      float frequency)
{
    pthread_mutex_lock(&g_mutex);
    ECHECK_APP_MUTEX(g_mutex,rp_GenReset());
    ECHECK_APP_MUTEX(g_mutex,rp_GenAmp(channel, LCR_AMPLITUDE));
    ECHECK_APP_MUTEX(g_mutex,rp_GenOffset(channel, LCR_AMPLITUDE));
    ECHECK_APP_MUTEX(g_mutex,rp_GenWaveform(channel, RP_WAVEFORM_SINE));//RP_WAVEFORM_TRIANGLE
    ECHECK_APP_MUTEX(g_mutex,rp_GenFreq(channel, frequency));
    ECHECK_APP_MUTEX(g_mutex,rp_GenOutEnable(channel));
    ECHECK_APP_MUTEX(g_mutex,rp_GenResetTrigger(channel));
    g_th_params.frequency = frequency;
    pthread_mutex_unlock(&g_mutex);
    return RP_OK;
}


/* Main call function */
int lcr_Run(){
    if (g_lcr_thread_handler != NULL) return RP_EOOR;
    g_lcr_thread_handler = malloc(sizeof(pthread_t));
    int err;

    atomic_flag_test_and_set(&g_thread_flag);
    err = pthread_create(g_lcr_thread_handler, 0, lcr_MainThread, NULL);
    if(err != RP_OK){
        return RP_EOOR;
    }
    return RP_OK;
}

int lcr_Stop(){
    if (g_lcr_thread_handler == NULL) return RP_EOOR;
    atomic_flag_clear(&g_thread_flag);
    pthread_join(*g_lcr_thread_handler, 0);
    free(g_lcr_thread_handler);
    g_lcr_thread_handler = NULL;
    return RP_OK;
}

int lcr_CopyParams(lcr_main_data_t *params){
    pthread_mutex_lock(&g_mutex);
    if(lcr_CalculateData(g_th_params.z_out, g_th_params.phase_out) != RP_OK){
         pthread_mutex_unlock(&g_mutex);
         return RP_EOOR;
    }
    memcpy(params,&calc_data,sizeof(lcr_main_data_t));
    pthread_mutex_unlock(&g_mutex);
    return RP_OK;
}

/* Main Lcr meter thread */
void *lcr_MainThread(void *args)
{
    float **analysis_data = multiDimensionVector(ADC_BUFFER_SIZE);
    while(atomic_flag_test_and_set(&g_thread_flag)){
            /* Main lcr meter algorithm */

            pthread_mutex_lock(&g_mutex);
            uint32_t acq_size = 0;
            int ret_val = lcr_ThreadAcqData(analysis_data, &acq_size);
            if(ret_val == RP_OK) {
                if(main_params.calibration) {
                } else {
                    lcr_CheckShunt((const float **)analysis_data,acq_size);
                    lcr_getImpedance((const float **)analysis_data,acq_size);
                }
            }
            pthread_mutex_unlock(&g_mutex);

            usleep(500);
    }
    delMultiDimensionVector(analysis_data);
    //Exit thread
    //pthread_exit(0);
    return RP_OK;
}


/* Acquire functions. Callback to the API structure */
int lcr_ThreadAcqData(float **data,uint32_t *acq_size)
{
    uint32_t speed = 0;
    int ret = rp_HPGetBaseFastADCSpeedHz(&speed);
    if (ret != RP_HP_OK){
        fprintf(stderr,"[Error:lcr_ThreadAcqData] Error get ADC rate err: %d\n",ret);
        return RP_EOOR;
    }

    rp_acq_trig_state_t state;
    rp_acq_decimation_t api_decimation;

    uint32_t pos;
    bool     fillState = false;
    float rawData0[ADC_BUFFER_SIZE];
    float rawData1[ADC_BUFFER_SIZE];
    int      dec;
	*acq_size = ADC_BUFFER_SIZE;
    lcr_getDecimationValue(g_th_params.frequency, &api_decimation, &dec,speed);
    unsigned long long sleep_time = (unsigned long long)((*acq_size) * dec / (speed / 1e6));
	sleep_time = sleep_time < 1 ? 1 : sleep_time;

    //*acq_size = round((MIN_PERIODES * ADC_SAMPLE_RATE) / (g_th_params.frequency * dec));

    uint32_t acq_delay = *acq_size > ADC_BUFFER_SIZE ? ADC_BUFFER_SIZE / 2.0 : *acq_size - ADC_BUFFER_SIZE / 2.0;
    ECHECK(rp_AcqReset());
    ECHECK(rp_AcqSetDecimation(api_decimation));
    ECHECK(rp_AcqSetTriggerLevel(RP_T_CH_1, 0));
    ECHECK(rp_AcqSetTriggerDelay(acq_delay));
    ECHECK(rp_AcqStart());
    usleep(sleep_time);
    ECHECK(rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW));

    state = RP_TRIG_STATE_TRIGGERED;
    while(true){
        rp_AcqGetTriggerState(&state);
        if(state == RP_TRIG_STATE_TRIGGERED){
            break;
        }
    }

    while(!fillState){
        ECHECK(rp_AcqGetBufferFillState(&fillState));
    }

    ECHECK(rp_AcqStop());
    ECHECK(rp_AcqGetWritePointerAtTrig(&pos));
    pos++;
    buffers_t buff;
    buff.ch_f[0] = rawData0;
    buff.ch_f[1] = rawData1;
    buff.size = ADC_BUFFER_SIZE;
    ECHECK(rp_AcqGetDataV2(pos,&buff));
    for(uint32_t i = 0; i < *acq_size; ++i) {
        data[0][i] = buff.ch_f[0][i];
        data[1][i] = buff.ch_f[1][i];
    }
    //FirRectangleWindow(data[0], *acq_size, 7);
    //FirRectangleWindow(data[1], *acq_size, 7);
    //ECHECK(rp_AcqGetDataV(RP_CH_1, pos, acq_size, data[0]));
    //ECHECK(rp_AcqGetDataV(RP_CH_2, pos, acq_size, data[1]));

    return RP_OK;
}

int lcr_getImpedance(const float **data,const uint32_t acq_size)
{
    uint32_t speed = 0;
    int ret = rp_HPGetBaseFastADCSpeedHz(&speed);
    if (ret != RP_HP_OK){
        fprintf(stderr,"[Error:lcr_ThreadAcqData] Error get ADC rate err: %d\n",ret);
        return RP_EOOR;
    }

    float w_out;
    int decimation;
    rp_acq_decimation_t api_decimation;
    int r_shunt_index;
    lcr_getRShunt(&r_shunt_index);

    double r_shunt = SHUNT_TABLE[r_shunt_index];

    //Calculate output angular velocity
    w_out = g_th_params.frequency * 2 * M_PI;
    lcr_getDecimationValue(g_th_params.frequency, &api_decimation, &decimation,speed);

    return lcr_data_analysis((const float**)data, acq_size, 0,
                                r_shunt, w_out, decimation);
}

void lcr_CheckShunt(const float **data,const uint32_t acq_size)
{
    // Is AUTO or MANUAL mode is selected
    if(!g_isShuntAutoChange) {
        set_IIC_Shunt(main_params.r_shunt);
        return;
    }

    // Calculate pick to pick voltage
    float u0_max = -1E9;
    float u0_min =  1E9;
    float u1_max = -1E9;
    float u1_min =  1E9;
    for(uint32_t i = 0; i < acq_size; i++) {
        u0_max = MAX(u0_max, data[0][i]);
        u0_min = MIN(u0_min, data[0][i]);
        u1_max = MAX(u1_max, data[1][i]);
        u1_min = MIN(u1_min, data[1][i]);
    }
    float p2p0   = (u0_max - u0_min);
    float p2p1   = (u1_max - u1_min);
//    float diff   = p2p1 - p2p0;
    float maxp2p = p2p0 > p2p1 ? p2p0 : p2p1;
    float measurep2p =  p2p0 < p2p1 ? p2p0 : p2p1;

    if (maxp2p * 0.15 > measurep2p) {
            main_params.r_shunt = main_params.r_shunt < 5 ? main_params.r_shunt + 1 : 5;
            set_IIC_Shunt(main_params.r_shunt);
    }

    if (maxp2p * 0.85 < measurep2p) {
            main_params.r_shunt = main_params.r_shunt > 0 ? main_params.r_shunt - 1 : 0;
            set_IIC_Shunt(main_params.r_shunt);
    }
}



// int lcr_Correction(){
//     int start_freq 		= START_CORR_FREQ;

//     int ret_val;
//     struct impendace_params args;
//     pthread_t lcr_thread_handler;

//     float _Complex *amplitude_z =
//             malloc(CALIB_STEPS * sizeof(float _Complex));

//     calib_t calib_mode = main_params.calibration;

//     char command[100];
//     //Set system to read-write
//     strcpy(command, "rw");

//     ret_val = system(command);
//     if(ret_val != 0){
//         free(amplitude_z);
//         return RP_EOOR;
//     }

//     for(int i = 0; i < CALIB_STEPS; i++){

//         args.frequency = start_freq * powf(10, i);
//         //lcr_MainThread(&args);
//         ret_val = pthread_create(&lcr_thread_handler, 0, lcr_MainThread, &args);
//         if(ret_val != 0){
//             printf("Main thread creation failed.\n");
//             free(amplitude_z);
//             return RP_EOOR;
//         }

//         pthread_join(lcr_thread_handler, 0);
//         amplitude_z[i] = args.z_out;
//     }

//     //Store calibration
//     store_calib(calib_mode, amplitude_z);

//     free(amplitude_z);

//     //Set system to read-only
//     strcpy(command, "ro");

//     ret_val = system(command);
//     if(ret_val != 0){
//         return RP_EOOR;
//     }

//     return RP_OK;
// }

int lcr_CalculateData(float _Complex z_measured, float phase_measured)
{
    int status;
    bool calibration = false;

    //Client depended parameters
    double R_out, C_out, L_out, ESR_out;

    //Client independed
    data_t ampl_out, phase_out, Q_out, D_out;

    const char *calibrations[] =
    {"/opt/redpitaya/www/apps/lcr_meter/CALIB_OPEN",
     "/opt/redpitaya/www/apps/lcr_meter/CALIB_SHORT"};

    FILE *f_open  = fopen(calibrations[0], "r");
    FILE *f_short = fopen(calibrations[1], "r");

    //Calibration was made
    if((f_open != NULL) && (f_short != NULL)) {
        calibration = true;
    }

    float _Complex z_open[CALIB_STEPS]  = {0, 0, 0, 0};
    float _Complex z_short[CALIB_STEPS] = {0, 0, 0, 0};
    float _Complex z_final;

    /* Read calibration from files */
    if(calibration) {
        int line = 0;
        while(!feof(f_open)) {
            float z_open_imag, z_open_real;
            status = fscanf(f_open, "%f %fi", &z_open_real, &z_open_imag);
            if (!status) {
                fclose(f_open);
                fclose(f_short);
                return RP_RCA;
            }
            z_open[line] = z_open_real + z_open_imag*I;
            line++;
        }

        line = 0;
        while(!feof(f_short)){
            float z_short_imag, z_short_real;
            status = fscanf(f_short, "%f %fi", &z_short_real, &z_short_imag);
            if (!status)
            {
                fclose(f_open);
                fclose(f_short);
                return RP_RCA;
            }
            z_short[line] = z_short_real + z_short_imag*I;
            line++;
        }
    }

    /* --------------- CALCULATING OUTPUT PARAMETERS --------------- */
    int index = log10(g_th_params.frequency) - 2;

    //Calibration was made
    if(calibration) {
         z_final = z_open[index] - ((z_short[index] -
                                     z_measured) / (z_measured - z_open[index]));

    //No calibration was made
    } else
    {
        z_final = z_measured;
    }

    data_t w_out = 2 * M_PI * g_th_params.frequency;

    data_t Y = 1.0 / z_final;
    data_t G_p = creal(Y);
    data_t B_p = cimag(Y);
    data_t X_s = cimag(z_final);

    /*  mode */
    if(main_params.series){
        R_out = creal(z_final);
        C_out = -1.0 / (w_out * X_s);
        L_out = X_s / w_out;
        ESR_out = R_out;

    } else {
        /* Parallel mode */
        R_out = 1.0 / G_p;
        C_out = B_p / w_out;
        L_out = -1.0 * (w_out * B_p);
        ESR_out = 1.0; //TODO
    }

    Q_out = X_s / R_out;
    D_out = -1 / Q_out;
    ampl_out = cabs(z_final);
    phase_out = phase_measured;
    //Set output structure pointers
    calc_data.lcr_amplitude = ampl_out;
    calc_data.lcr_phase     = phase_out;
    calc_data.lcr_D 	     = D_out;
    calc_data.lcr_Q         = Q_out;
    calc_data.lcr_ESR       = ESR_out;
    calc_data.lcr_L         = L_out;
    calc_data.lcr_C         = C_out;
    calc_data.lcr_R         = R_out;

    // Update data for conosle tool

    calc_data.lcr_L_s = X_s / w_out;
    calc_data.lcr_C_s = -1.0 / (w_out * X_s);
    calc_data.lcr_R_s = creal(z_final);

    calc_data.lcr_L_p = -1.0 * (w_out * B_p);
    calc_data.lcr_C_p = B_p / w_out;
    calc_data.lcr_R_p = 1.0 / G_p;

    calc_data.lcr_Q_s = X_s / calc_data.lcr_R_s;
    calc_data.lcr_D_s = -1.0 / calc_data.lcr_Q_s;
    calc_data.lcr_Q_p = X_s / calc_data.lcr_R_p;
    calc_data.lcr_D_p = -1.0 / calc_data.lcr_Q_p;

    calc_data.lcr_X_s       = X_s;
    calc_data.lcr_G_p       = G_p;
    calc_data.lcr_B_p       = B_p;
    calc_data.lcr_Y_abs     = sqrtf( powf( creal(Y), 2 ) + powf(cimag(Y), 2 ) );
    calc_data.lcr_Phase_Y   = -phase_out;

    //Close files, if calibration
    if(calibration) {
        fclose(f_short);
        fclose(f_open);
    }

    return RP_OK;
}


data_t RMS(double *data, int size){
    data_t max = data[0];
    data_t min = data[0];
    for(int i = 0; i < size; i++){
        if (data[i] > max) {
            max = data[i];
        }
        if (data[i] < min) {
            min = data[i];
        }
    }
    data_t mean = (max + min) / 2.0;
    data_t result = 0;
    for(int i = 0; i < size; i++){
        data_t z = data[i] - mean;
        result +=  z * z;
    }
    result =  sqrt(result / size);
    return result;
}


data_t InterpolateLagrangePolynomial (data_t x, data_t* x_values, data_t* y_values, int size)
{
	data_t lagrange_pol = 0;
	data_t basics_pol;

	for (int i = 0; i < size; i++)
	{
		basics_pol = 1;
		for (int j = 0; j < size; j++)
		{
			if (j == i) continue;
			basics_pol *= (x - x_values[j])/(x_values[i] - x_values[j]);
		}
		lagrange_pol += basics_pol*y_values[i];
	}
	return lagrange_pol;
}

data_t calcPoint(data_t *a, data_t *b,int lenghtArray,int index){
	data_t x = 0;
	int j = 0;
	int i = index - lenghtArray > 0 ? index - lenghtArray : 0;
	for (; i < lenghtArray && i <= index; i++) {
		j = index - i;
		x +=a[i]*b[lenghtArray - j - 1];
	}
	return x;
}

data_t getMaxArg(data_t *a, data_t *b,int start,int stop, int step,int lenghtArray){
    data_t corralate[lenghtArray * 2 + 1];
    int    corralateIndex[lenghtArray * 2 + 1];

    for(int k = 0; k < lenghtArray * 2 + 1 ; k++){
        corralate[k] = 0;
     	corralateIndex[k] = -1;
    }

    for(int k = start; k <= stop ; k+=step){
        corralate[k] = calcPoint(a,b,lenghtArray,k);
        corralateIndex[k] = k;
    }

    int maxK = 0;
    int maxCor = 0;
    int init = 1;
    for(int k = start; k <= stop ; k++){
        if (corralateIndex[k] != -1)
            if (corralate[k] > maxCor || init){
                init = 0;
                maxCor = corralate[k];
                maxK = corralateIndex[k];
            }
    }
    return maxK;
}

data_t crossCorrelation(data_t *xSignalArray, data_t *ySignalArray, int lenghtArray,int sepm_Per)
{
    data_t argmax = 0;
    data_t *a = xSignalArray;
    data_t *b = ySignalArray;
    int step =  log2(lenghtArray);
    int maxK = -1;
    int start = 0;
    int stop  = lenghtArray * 2;
    int oldMaxK = -1;
    while(step>=1){
        maxK =getMaxArg(a,b,start,stop,step,lenghtArray);
        start = maxK - step * 10 < 0 ? 0 : maxK - step * 10;
        stop  = maxK + step * 10 > (lenghtArray * 2 + 1) ? (lenghtArray * 2 + 1) : maxK + step * 10;
        step /=2;
        if (maxK == oldMaxK) break;
        oldMaxK = maxK;
    }

	argmax = maxK;

	if (argmax > 0 && argmax < lenghtArray * 2){
		data_t x_axis[] = { 0 , 1 , 2};
		data_t y_axis[3];

		for(int i = argmax-1,j=0; i <= argmax+1 ;i++,j++){
			y_axis[j] = calcPoint(a,b,lenghtArray,i);
		}
		data_t eps = 0.0001;
		data_t start = 0;
		data_t stop = 2;
		data_t y_start = InterpolateLagrangePolynomial(start,x_axis,y_axis,3);
		data_t y_stop  = InterpolateLagrangePolynomial(stop,x_axis,y_axis,3);
		data_t max_inter = 0;
		while(eps  < (stop - start)){
			data_t z = (stop - start)/2.0 + start;
			data_t y_sub  = InterpolateLagrangePolynomial(z,x_axis,y_axis,3);
			if (y_sub > y_start || y_sub > y_stop){
				if (y_start > y_stop){
					stop = z;
					y_stop = y_sub;
				}else{
					start = z;
					y_start = y_sub;
				}
				max_inter = z;
			}
			else{
				max_inter = y_stop > y_start? stop : start;
				break;
			}
		}
		argmax = argmax-1 + max_inter;
	}
    return argmax;
}

data_t phaseCalculator(data_t freq_HZ, data_t samplesPerSecond, int numSamples,int sepm_Per, data_t *xSamples, data_t *ySamples)
{
    data_t timeShift, phaseShift, timeLine;
    data_t argmax = crossCorrelation(xSamples, ySamples, numSamples,sepm_Per);

    timeLine = ((numSamples - 1) / samplesPerSecond);
    timeShift = ((timeLine * argmax) / (numSamples - 1)) + (-timeLine);
    phaseShift = ((2 * M_PI) * fmod((freq_HZ * timeShift), 1.0)) - M_PI;
    if (phaseShift <= -M_PI/2)
		phaseShift += M_PI;
	else if (phaseShift >= M_PI/2)
		phaseShift -= M_PI;
	return phaseShift;
}

int static_x = 0;

int lcr_data_analysis(const float **data,
                      uint32_t size,
                      float dc_bias,
                      double r_shunt,
                      float w_out,
                      int decimation)
{
    uint32_t speed = 0;
    int ret = rp_HPGetBaseFastADCSpeedHz(&speed);
    if (ret != RP_HP_OK){
        fprintf(stderr,"[Error:lcr_ThreadAcqData] Error get ADC rate err: %d\n",ret);
        return RP_EOOR;
    }

    double T = ((double)decimation / (double)speed);
    //double c_calib = C_CALIB[getIndex(r_shunt)][getIndex(w_out/(2*M_PI*10))];
    g_isSine = isSineTester((float**)data, size, T);

    data_t u_dut[size];
    data_t i_dut[size];
	data_t buf1[size];
	data_t buf2[size];

    double r_RC = r_shunt;// (r_shunt * (1.0 / (w_out * c_calib))) / (r_shunt + (1.0 / (w_out * c_calib)));

 //   char p[200];
 //   sprintf(p,"/root/%d.buf",static_x++);

  // FILE *f = fopen(p,"w");
    for(uint32_t i = 0; i < size; i++) {
        i_dut[i] = data[1][i] / r_RC;
        u_dut[i] = data[0][i] - data[1][i];
   		buf1[i] = data[1][i];
		buf2[i] = (data[0][i]*2 - data[1][i]);
   //     fprintf(f,"%f\t%f\n",buf1[i],buf2[i]);
    }
  //  fflush(f);
  //  fclose(f);
    Fir(buf1,size);
    Fir(buf2,size);
    Normalize(buf1,size);
    Normalize(buf2,size);
    data_t sig1_rms =  RMS(i_dut,size);
	data_t sig2_rms =  RMS(u_dut,size);
    double z_ampl =  sig2_rms / sig1_rms;
    int samples_period = round((double)speed / (g_th_params.frequency * decimation));
    double phase_z_rad = phaseCalculator(g_th_params.frequency,(double)speed  / (double)decimation, size ,samples_period,buf1,buf2);
    //fprintf(stderr,"P1: %f\nP2: %f\nP3: %d\nP4:%d\n",g_th_params.frequency,ADC_SAMPLE_RATE / decimation, size ,samples_period);
	if (phase_z_rad <= -M_PI)
		phase_z_rad += 2*M_PI;
	else if (phase_z_rad >= M_PI)
		phase_z_rad -= 2*M_PI;
    accu_buf[0][accu_buf_size] = z_ampl;
    accu_buf[1][accu_buf_size] = phase_z_rad;
    accu_buf_size++;
    if (accu_buf_size >= 5){
        accu_buf_size = 0;
    }

    z_ampl = 0;
    phase_z_rad = 0;
    for(int  i = 0 ; i < 5;i++){
        z_ampl += accu_buf[0][i];
        phase_z_rad += accu_buf[1][i];
    }
    z_ampl /= 5;
    phase_z_rad /= 5;

    g_th_params.phase_out = phase_z_rad * (180.0 / M_PI);
    g_th_params.z_out = (z_ampl * cosf(phase_z_rad)) + (z_ampl * sinf(phase_z_rad) * I);
    return RP_OK;
}


/* Getters and setters */
int lcr_SetFrequency(float frequency){
    main_params.frequency = frequency;
    return RP_OK;
}

int lcr_GetFrequency(float *frequency){
    *frequency = main_params.frequency;
    return RP_OK;
}

int lcr_setRShunt(int r_shunt){
    if((r_shunt >= 0) && (r_shunt < 6)){
        main_params.r_shunt = r_shunt;
    }
    return RP_OK;
}

int lcr_getRShunt(int *r_shunt){
    *r_shunt = main_params.r_shunt;
    return RP_OK;
}

int lcr_setRShuntIsAuto(bool isAuto){
    g_isShuntAutoChange = isAuto;
    return RP_OK;
}

int lcr_SetCalibMode(calib_t calibrated){
    main_params.calibration = calibrated;
    return RP_OK;
}

int lcr_GetCalibMode(calib_t *mode){
    *mode = main_params.calibration;
    return RP_OK;
}

int lcr_SetMeasSeries(bool series){
    main_params.series = series;
    return RP_OK;
}

int lcr_GetMeasSeries(bool *series){
    *series = main_params.series;
    return RP_OK;
}

int lcr_SetMeasTolerance(int tolerance){
    main_params.tolerance = tolerance;
    return RP_OK;
}

int lcr_GetMeasTolerance(int *tolerance){
    *tolerance = main_params.tolerance;
    return RP_OK;
}

int lcr_SetMeasRangeMode(int range){
    main_params.range = range;
    return RP_OK;
}

int lcr_GetMeasRangeMode(int *range){
    *range = main_params.range;
    return RP_OK;
}

int lcr_SetRangeFormat(int format){
    main_params.range_format = format;
    return RP_OK;
}

int lcr_GetRangeFormat(int *format){
    *format = main_params.range_format;
    return RP_OK;
}

int lcr_SetRangeUnits(int units){
    main_params.range_units = units;
    return RP_OK;
}

int lcr_GetRangeUnits(int *units){
    *units = main_params.range_units;
    return RP_OK;
}

bool lcr_isSine(){
    return g_isSine;
}