
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

#include <complex.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <atomic>
#include <thread>
#include <vector>

#include "lcr_meter.h"

#include "calib.h"
#include "math/rp_algorithms.h"
#include "rp.h"
#include "rp_hw_calib.h"
#include "utils.h"

CLCRHardware g_lcr_hw;
CLCRGenerator g_generator;

typedef double data_t;

typedef struct impendace_params {
    float frequency;
    float _Complex z_out;
    float phase_out;
} impendace_params_t;

/* Thread variables */
std::thread* g_lcr_thread = NULL;
std::mutex g_lcr_mutex;
std::mutex g_lcr_mutex_analize;
std::mutex g_lcr_mutex_data;
std::atomic_bool g_lcr_threadRun = false;
std::atomic_bool g_lcr_threadPause = false;
std::atomic_bool g_lcr_GenRun = false;

static auto g_adc_rate = rp_HPGetBaseFastADCSpeedHzOrDefault();

volatile impendace_params_t g_th_params;

// static bool                 g_isCuruitOpend            = false;
// static bool                 g_isFrequencyChange        = false;
bool g_isShuntAutoChange = true;

/* Init lcr params struct */
lcr_params_t main_params = {CALIB_NONE, false, RP_LCR_S_EXTENSION, 100};

/* Main lcr data params */
lcr_main_data_t calc_data;

/* Init the main API structure */
int lcr_Init() {

    if (rp_Init() != RP_OK) {
        FATAL("Unable to inicialize the RPI API structure")
        return RP_LCR_UERROR;
    }

    /* Set default values of the lcr structure */
    lcr_SetDefaultValues();

    /* Set calibration values */
    // FILE* f_calib = fopen("/opt/redpitaya/www/apps/lcr_meter/CAPACITOR_CALIB", "rb");
    // if(f_calib) {
    //     int readed = fread(&C_CALIB, sizeof(C_CALIB), 1, f_calib);
    //     if(readed != 1){
    //         fclose(f_calib);
    //         pthread_mutex_unlock(&g_mutex);
    //         return RP_RCA;
    //     }
    //     fclose(f_calib);
    // }
    // pthread_mutex_unlock(&g_mutex);
    return RP_LCR_OK;
}

/* Release resources used the main API structure */
int lcr_Release() {
    lcr_Stop();
    lcr_GenStop();
    rp_Release();
    TRACE_SHORT("Releasing Red Pitaya library resources.\n");
    return RP_LCR_OK;
}

/* Set default values of all rpi resources */
int lcr_Reset() {
    std::lock_guard lock(g_lcr_mutex);
    auto ret = rp_Reset();
    if (ret != RP_OK) {
        ERROR_LOG("Reset to default failed")
        return RP_LCR_UERROR;
    }
    /* Set default values of the lcr_params structure */
    return lcr_SetDefaultValues();
}

int lcr_SetPause(bool pause) {
    g_lcr_threadPause = pause;
    return RP_LCR_OK;
}

int lcr_SetDefaultValues() {
    if (main_params.shunt_mode == RP_LCR_S_EXTENSION) {
        ECHECK_LCR_APP(lcr_setRShunt(RP_LCR_S_10));
    } else {
        lcr_SetCustomShunt(100);
    }
    ECHECK_LCR_APP(lcr_SetFrequency(10.0));
    ECHECK_LCR_APP(lcr_SetCalibMode(CALIB_NONE));
    ECHECK_LCR_APP(lcr_SetMeasSeries(true));
    return RP_LCR_OK;
}

int lcr_GenRun() {
    std::lock_guard lock(g_lcr_mutex);
    g_lcr_GenRun = true;
    return g_generator.start();
}

int lcr_GenStop() {
    std::lock_guard lock(g_lcr_mutex);
    g_lcr_GenRun = false;
    return g_generator.stop();
}

/* Main call function */
int lcr_Run() {
    std::lock_guard lock(g_lcr_mutex);
    if (g_lcr_thread)
        return RP_LCR_UERROR;
    g_lcr_threadRun = true;
    g_lcr_thread = new std::thread(lcr_MainThread);
    return RP_LCR_OK;
}

int lcr_Stop() {
    std::lock_guard lock(g_lcr_mutex);
    g_lcr_threadRun = false;
    if (g_lcr_thread) {
        if (g_lcr_thread->joinable()) {
            g_lcr_thread->join();
            delete g_lcr_thread;
            g_lcr_thread = NULL;
        }
    }
    return RP_LCR_OK;
}

int lcr_CopyParams(lcr_main_data_t* params) {
    std::lock_guard lock(g_lcr_mutex_data);
    if (!g_lcr_threadRun)
        return RP_LCR_NOT_STARTED;
    if (lcr_CalculateData(g_th_params.z_out, g_th_params.phase_out, g_th_params.frequency) != RP_LCR_OK) {
        return RP_LCR_UERROR;
    }
    memcpy(params, &calc_data, sizeof(lcr_main_data_t));
    return RP_LCR_OK;
}

/* Main Lcr meter thread */
void lcr_MainThread() {
    auto buffer = rp_createBuffer(2, ADC_BUFFER_SIZE, false, false, true);
    initFFT(ADC_BUFFER_SIZE, g_adc_rate);
    buffer->use_calib_for_raw = false;
    buffer->use_calib_for_volts = false;
    if (buffer == NULL) {
        FATAL("Unable to allocate memory for data buffer")
        return;
    }

    while (g_lcr_threadRun) {
        /* Main lcr meter algorithm */
        if (!g_lcr_threadPause) {
            std::lock_guard lock(g_lcr_mutex);
            if (g_lcr_GenRun) {
                int decimation;
                float freq;
                int ret_val = lcr_ThreadAcqData(buffer, &decimation, &freq);
                if (ret_val == RP_OK) {
                    if (main_params.calibration) {
                    } else {
                        if (main_params.shunt_mode == RP_LCR_S_EXTENSION)
                            lcr_CheckShunt(buffer->ch_f[0], buffer->ch_f[1], buffer->size);
                        lcr_getImpedance(buffer, decimation, freq);
                    }
                }
            }
        }
        usleep(50);
    }
    rp_deleteBuffer(buffer);
    releaseFFT();
}

/* Acquire functions. Callback to the API structure */
int lcr_ThreadAcqData(buffers_t* data, int* dec, float* freq) {
    rp_acq_trig_state_t state;
    uint32_t pos;
    bool fillState = false;
    *freq = g_generator.getFreq();
    lcr_getDecimationValue(*freq, dec, g_adc_rate);
    ECHECK(rp_AcqReset());
    ECHECK(rp_AcqSetDecimationFactor(*dec));
    ECHECK(rp_AcqSetTriggerLevel(RP_T_CH_1, 0));
    ECHECK(rp_AcqSetTriggerDelayDirect(ADC_BUFFER_SIZE));
    ECHECK(rp_AcqStart());
    ECHECK(rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW));

    state = RP_TRIG_STATE_TRIGGERED;
    while (true) {
        rp_AcqGetTriggerState(&state);
        if (state == RP_TRIG_STATE_TRIGGERED) {
            break;
        }
    }

    while (!fillState) {
        ECHECK(rp_AcqGetBufferFillState(&fillState));
    }

    ECHECK(rp_AcqStop());
    ECHECK(rp_AcqGetWritePointerAtTrig(&pos));
    ECHECK(rp_AcqGetData(pos, data));
    return RP_OK;
}

int lcr_getImpedance(buffers_t* data, int api_decimation, float freq) {
    auto r_shunt = g_lcr_hw.getShunt();
    return lcr_data_analysis(data, r_shunt, freq, api_decimation);
}

void lcr_CheckShunt(const float* ch1, const float* ch2, const uint32_t _size) {
    // Is AUTO or MANUAL mode is selected
    if (!g_isShuntAutoChange) {
        return;
    }

    // Calculate pick to pick voltage
    float u0_max = ch1[0];
    float u0_min = ch1[0];
    float u1_max = ch2[0];
    float u1_min = ch2[0];
    for (uint32_t i = 0; i < _size; i++) {
        u0_max = u0_max < ch1[i] ? ch1[i] : u0_max;
        u0_min = u0_min > ch1[i] ? ch1[i] : u0_min;
        u1_max = u1_max < ch2[i] ? ch2[i] : u1_max;
        u1_min = u1_min > ch2[i] ? ch2[i] : u1_min;
    }
    float p2p0 = (u0_max - u0_min);
    float p2p1 = (u1_max - u1_min);
    float maxp2p = p2p0 > p2p1 ? p2p0 : p2p1;
    float measurep2p = p2p0 < p2p1 ? p2p0 : p2p1;

    if (maxp2p * 0.15 > measurep2p) {
        auto shunt = g_lcr_hw.getShunt();
        if (shunt < RP_LCR_S_1M)
            shunt = (lcr_shunt_t)((int)shunt + 1);
        g_lcr_hw.setI2CShunt(shunt);
    }

    if (maxp2p * 0.85 < measurep2p) {
        auto shunt = g_lcr_hw.getShunt();
        if (shunt > RP_LCR_S_10)
            shunt = (lcr_shunt_t)((int)shunt - 1);
        g_lcr_hw.setI2CShunt(shunt);
    }
}

int lcr_CalculateData(float _Complex z_measured, float phase_measured, float freq) {
    int status;
    bool calibration = false;

    //Client depended parameters
    // double R_out, C_out, L_out, ESR_out;

    //Client independed
    // data_t ampl_out, phase_out, Q_out, D_out;

    const char* calibrations[] = {"/opt/redpitaya/www/apps/lcr_meter/CALIB_OPEN", "/opt/redpitaya/www/apps/lcr_meter/CALIB_SHORT"};

    FILE* f_open = fopen(calibrations[0], "r");
    FILE* f_short = fopen(calibrations[1], "r");

    //Calibration was made
    if ((f_open != NULL) && (f_short != NULL)) {
        calibration = true;
    }

    float _Complex z_open[CALIB_STEPS] = {0, 0, 0, 0};
    float _Complex z_short[CALIB_STEPS] = {0, 0, 0, 0};
    float _Complex z_final;

    /* Read calibration from files */
    if (calibration) {
        int line = 0;
        while (!feof(f_open)) {
            float z_open_imag, z_open_real;
            status = fscanf(f_open, "%f %fi", &z_open_real, &z_open_imag);
            if (!status) {
                fclose(f_open);
                fclose(f_short);
                return RP_RCA;
            }
            z_open[line] = z_open_real + z_open_imag * I;
            line++;
        }

        line = 0;
        while (!feof(f_short)) {
            float z_short_imag, z_short_real;
            status = fscanf(f_short, "%f %fi", &z_short_real, &z_short_imag);
            if (!status) {
                fclose(f_open);
                fclose(f_short);
                return RP_RCA;
            }
            z_short[line] = z_short_real + z_short_imag * I;
            line++;
        }
    }

    /* --------------- CALCULATING OUTPUT PARAMETERS --------------- */
    int index = log10(freq) - 2;

    //Calibration was made
    if (calibration) {
        z_final = z_open[index] - ((z_short[index] - z_measured) / (z_measured - z_open[index]));

        //No calibration was made
    } else {
        z_final = z_measured;
    }

    data_t w_out = 2 * M_PI * freq;

    // auto Z = z_final;
    auto Z_abs = cabs(z_final);
    auto Rs_ESR = creal(z_final);
    auto Xs = cimag(z_final);
    auto Y = 1.0 / z_final;
    auto Y_abs = 1.0 / Z_abs;
    auto Gp = creal(Y);
    auto Bp = cimag(Y);
    auto Cs = -1.0 / (w_out * Xs);
    auto Cp = Bp / w_out;
    auto Ls = Xs / w_out;
    auto Lp = -1.0 / (w_out * Bp);
    auto Rp = 1.0 / Gp;
    auto Qp = Bp / Gp;
    auto Qs = Xs / Rs_ESR;
    auto Dp = -1.0 / Qp;
    auto Ds = -1.0 / Qs;
    auto phase = phase_measured;
    auto phase_Y = -phase_measured;

    calc_data.lcr_freq = freq;
    calc_data.lcr_amplitude = Z_abs;
    calc_data.lcr_phase = phase;
    calc_data.lcr_D = main_params.series ? Ds : Dp;
    calc_data.lcr_Q = main_params.series ? Qs : Qp;
    calc_data.lcr_ESR = main_params.series ? Rs_ESR : 1.0;
    calc_data.lcr_L = main_params.series ? Ls : Lp;
    calc_data.lcr_C = main_params.series ? Cs : Cp;
    calc_data.lcr_R = main_params.series ? Rs_ESR : Rp;
    calc_data.lcr_L_s = Ls;
    calc_data.lcr_C_s = Cs;
    calc_data.lcr_R_s = Rs_ESR;
    calc_data.lcr_L_p = Lp;
    calc_data.lcr_C_p = Cp;
    calc_data.lcr_R_p = Rp;
    calc_data.lcr_Q_s = Qs;
    calc_data.lcr_Q_p = Qp;
    calc_data.lcr_D_s = Ds;
    calc_data.lcr_D_p = Dp;
    calc_data.lcr_X_s = Xs;
    calc_data.lcr_G_p = Gp;
    calc_data.lcr_B_p = Bp;
    calc_data.lcr_Y_abs = Y_abs;
    calc_data.lcr_Phase_Y = phase_Y;

    // auto Y = 1.0 / z_final;
    // data_t G_p = creal(Y);
    // data_t B_p = cimag(Y);
    // data_t X_s = cimag(z_final);

    // /*  mode */
    // if(main_params.series){
    //     R_out = creal(z_final);
    //     C_out = -1.0 / (w_out * X_s);
    //     L_out = X_s / w_out;
    //     ESR_out = R_out;

    // } else {
    //     /* Parallel mode */
    //     R_out = 1.0 / G_p;
    //     C_out = B_p / w_out;
    //     L_out = -1.0 * (w_out * B_p);
    //     ESR_out = 1.0; //TODO
    // }

    // Q_out = X_s / R_out; // Quality factor
    // D_out = -1 / Q_out;
    // ampl_out = cabs(z_final);
    // phase_out = phase_measured;
    // //Set output structure pointers
    // calc_data.lcr_amplitude = ampl_out; // |Z|
    // calc_data.lcr_phase     = phase_out; // Phase angle of Z

    // calc_data.lcr_D 	    = D_out;
    // calc_data.lcr_Q         = Q_out;
    // calc_data.lcr_ESR       = ESR_out;
    // calc_data.lcr_L         = L_out;
    // calc_data.lcr_C         = C_out;
    // calc_data.lcr_R         = R_out;

    // // Update data for conosle tool

    // calc_data.lcr_L_s = X_s / w_out;
    // calc_data.lcr_C_s = -1.0 / (w_out * X_s);
    // calc_data.lcr_R_s = creal(z_final);

    // calc_data.lcr_L_p = -1.0 * (w_out * B_p);
    // calc_data.lcr_C_p = B_p / w_out;
    // calc_data.lcr_R_p = 1.0 / G_p;

    // calc_data.lcr_Q_s = X_s / calc_data.lcr_R_s;
    // calc_data.lcr_D_s = -1.0 / calc_data.lcr_Q_s;
    // calc_data.lcr_Q_p = X_s / calc_data.lcr_R_p;
    // calc_data.lcr_D_p = -1.0 / calc_data.lcr_Q_p;

    // calc_data.lcr_X_s       = X_s;
    // calc_data.lcr_G_p       = G_p;
    // calc_data.lcr_B_p       = B_p;
    // calc_data.lcr_Y_abs     = sqrtf( powf( creal(Y), 2 ) + powf(cimag(Y), 2 ) );
    // calc_data.lcr_Phase_Y   = -phase_out; // Phase angle of
    // calc_data.lcr_freq = freq;

    //Close files, if calibration
    if (calibration) {
        fclose(f_short);
        fclose(f_open);
    }

    return RP_LCR_OK;
}

int lcr_data_analysis(buffers_t* data, lcr_shunt_t r_shunt, float sigFreq, int decimation) {
    std::lock_guard<std::mutex> lock(g_lcr_mutex_analize);
    static std::vector<data_t> u_dut;
    static std::vector<data_t> i_dut;
    u_dut.resize(data->size);
    i_dut.resize(data->size);

    auto r_RC = 1;
    if (main_params.shunt_mode == RP_LCR_S_EXTENSION)
        r_RC = g_lcr_hw.calibShunt(r_shunt, sigFreq);

    if (main_params.shunt_mode == RP_LCR_S_CUSTOM)
        r_RC = main_params.shunt;

    // WARNING("Shunt %d freq %f decimation %d",r_RC,sigFreq,decimation)

    for (uint32_t i = 0; i < data->size; i++) {
        u_dut[i] = data->ch_f[0][i] - data->ch_f[1][i];
        i_dut[i] = data->ch_f[1][i] / r_RC;
    }

    double z_ampl;
    double phase_z_deg;
    analysisFFT(i_dut, u_dut, sigFreq, decimation, &z_ampl, &phase_z_deg, 0);
    // double p1,p2;
    // analysisTrap(i_dut,u_dut,sigFreq,decimation,g_adc_rate,0,&p1,&p2,&z_ampl,&phase_z_deg);

    // WARNING("sigFreq %f\n",sigFreq);
    // WARNING("Amp %f\n",z_ampl);
    // WARNING("phase_z_deg %f\n",phase_z_deg);

    std::lock_guard lockData(g_lcr_mutex_data);
    g_th_params.phase_out = phase_z_deg;
    auto phase_z_rad = phase_z_deg * M_PI / 180.0;
    g_th_params.z_out = (z_ampl * cosf(phase_z_rad)) + (z_ampl * sinf(phase_z_rad) * I);
    g_th_params.frequency = sigFreq;
    return RP_LCR_OK;
}

int lcr_SetFrequency(float frequency) {
    g_generator.setFreq(frequency);
    return RP_LCR_OK;
}

int lcr_GetFrequency(float* frequency) {
    *frequency = g_generator.getFreq();
    return RP_LCR_OK;
}

int lcr_SetAmplitude(float volt) {
    g_generator.setAmplitude(volt);
    return RP_LCR_OK;
}

int lcr_GetAmplitude(float* volt) {
    *volt = g_generator.getAmplitude();
    return RP_LCR_OK;
}

int lcr_SetOffset(float offset) {
    g_generator.setOffset(offset);
    return RP_LCR_OK;
}

int lcr_GetOffset(float* offset) {
    *offset = g_generator.getOffset();
    return RP_LCR_OK;
}

int lcr_setRShunt(lcr_shunt_t r_shunt) {
    return g_lcr_hw.setI2CShunt(r_shunt);
}

int lcr_getRShunt(lcr_shunt_t* r_shunt) {
    *r_shunt = g_lcr_hw.getShunt();
    return RP_LCR_OK;
}

int lcr_setRShuntIsAuto(bool isAuto) {
    g_isShuntAutoChange = isAuto;
    return RP_LCR_OK;
}

int lcr_SetCalibMode(calib_t calibrated) {
    main_params.calibration = calibrated;
    return RP_LCR_OK;
}

int lcr_GetCalibMode(calib_t* mode) {
    *mode = main_params.calibration;
    return RP_LCR_OK;
}

int lcr_SetMeasSeries(bool series) {
    main_params.series = series;
    return RP_LCR_OK;
}

int lcr_GetMeasSeries(bool* series) {
    *series = main_params.series;
    return RP_LCR_OK;
}

int lcr_CheckModuleConnection(bool _muteWarnings) {
    if (g_lcr_hw.checkExtensionModuleConnection(_muteWarnings) != 0)
        return RP_EMNC;
    return RP_LCR_OK;
}

int lcr_IsModuleConnected(bool* state) {
    *state = g_lcr_hw.isExtensionConnected();
    return RP_LCR_OK;
}

int lcr_SetCustomShunt(int shunt) {
    if (shunt < 1 || shunt > 10e6) {
        return RP_LCR_ERROR_INVALID_VALUE;
    }
    main_params.shunt = shunt;
    return RP_LCR_OK;
}

int lcr_GetCustomShunt(int* shunt) {
    *shunt = main_params.shunt;
    return RP_LCR_OK;
}

int lcr_SetShuntMode(lcr_shunt_mode_t shunt_mode) {
    main_params.shunt_mode = shunt_mode;
    return RP_LCR_OK;
}

int lcr_GetShuntMode(lcr_shunt_mode_t* shunt_mode) {
    *shunt_mode = main_params.shunt_mode;
    return RP_LCR_OK;
}