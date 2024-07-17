#include <math.h>
#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <threads.h>
#include <mutex>

#include "rp_algorithms.h"
#include "rp_interpolation.h"
#include "rp_dsp.h"
#include "rp_log.h"

std::mutex g_fft_mutex;
rp_dsp_api::CDSP *g_fft = NULL;
rp_dsp_api::data_t *g_fft_data = NULL;

template<typename T>
int mean(const T *data,size_t size, T *_value){
    if (size == 0) return RP_A_DATA_SIZE_ZERO;
    T result = 0;
    for(auto i = 0u; i < size; i++){
        result += (T)data[i];
    }
    result =  result / (T)size;
    *_value = result;
    return RP_A_OK;
}

int mean(const std::vector<float> &data, float *_value){
    return mean<float>(data.data(), data.size(),_value);
}

int mean(const std::vector<double> &data, double *_value){
    return mean<double>(data.data(), data.size(),_value);
}

template<typename T>
int rms(const T *data, size_t size, T *_value){
    if (size == 0) return RP_A_DATA_SIZE_ZERO;
    T result = 0;
    for(auto i = 0u; i < size; i++){
		T d = (T)data[i];
        result += d * d;
    }
    result =  sqrt(result / (T)size);
    *_value = result;
    return RP_A_OK;
}

int rms(const std::vector<float> &data, float *_value){
    return rms<float>(data.data(),data.size(),_value);
}

int rms(const std::vector<double> &data, double *_value){
    return rms<double>(data.data(),data.size(),_value);
}

template<typename T>
int p2p(const std::vector<T> &data, T *_value){
    if (data.size() == 0) return RP_A_DATA_SIZE_ZERO;
    T result = 0;
    T min = data[0];
    T max = data[0];
    for(auto i = 0u; i < data.size(); i++){
        min = min > data[i] ? data[i] : min;
        max = max < data[i] ? data[i] : max;
    }
    result =  max - min;
    *_value = result;
    return RP_A_OK;
}

int p2p(const std::vector<float> &data, float *_value){
    return p2p<float>(data,_value);
}

int p2p(const std::vector<double> &data, double *_value){
    return p2p<double>(data,_value);
}

double trapezoidalApprox(const double *data, size_t data_size){
    double result = 0;
    for(auto i = 0u; i < data_size - 1; i++){
        result += data[i] + data[i+1];
    }
    return result;
}

double trapezoidalApprox(const std::vector<double> &data){
    return trapezoidalApprox(data.data(),data.size());
}


template<typename T>
int fir(T *_in, size_t _size, fir_core_t mode){

    if (_size < mode) return RP_A_ERROR;
    constexpr int max_core = FIR_11;
    T core[max_core];
    T core_w[max_core /2 + 1];

    for(int i = 0; i < mode / 2 + 1; i++){
        core[i] = pow(2,i);
        core[mode - i - 1] = core[i];
    }
    int core_w_size = mode / 2 + 1;
    int size = _size;

    for(int i = 0; i < core_w_size; i++){
        T sum = 0;
        for(int j = 0; j < mode - i; j++){
            sum += core[j];
        }
        core_w[i] = sum;
    }

    T tmp[max_core];
    int cur_pointer = 0;

    for(int i = 0; i < size; ++i){
        T cur_core = core_w[0];
        tmp[cur_pointer] = _in[i];
        // if (i > 1 && i < _size - 1){
            _in[i] = 0;
            if (i < core_w_size - 1) {
                cur_core = core_w[core_w_size - i - 1];
            }

            if (i > (size - core_w_size)){
                cur_core = core_w[core_w_size - (_size - i)];
            }

            for( int j = -1 * mode / 2; j <= mode / 2; j++){
                int index = i + j;
                if (index < 0 || index >= size) continue;
                T x = _in[index];
                if (j <= 0){
                    x = tmp[(max_core + j + cur_pointer) % max_core];
                }
                _in[i] += x * core[j + mode / 2];
            }
            _in[i] /= cur_core;
        // }
        cur_pointer = (cur_pointer + 1) % max_core;
    }
    return RP_A_OK;
}

int fir(float *_data,size_t _size,fir_core_t _mode){
    return fir<float>(_data,_size,_mode);
}

int fir(double *_data,size_t _size,fir_core_t _mode){
    return fir<double>(_data,_size,_mode);
}

int fir(std::vector<float> &_data,fir_core_t _mode){
    return fir<float>(_data.data(),_data.size(),_mode);
}

int fir(std::vector<double> &_data,fir_core_t _mode){
    return fir<double>(_data.data(),_data.size(),_mode);
}

template<typename T>
int analysisTrap(const T *ch1,
                 const T *ch2,
                 const size_t ch1_size,
                 const size_t ch2_size,
                 uint32_t signalFreq,
                 uint32_t decimation,
                 uint32_t adcRate,
                 T input_threshold,
                 T *phaseCh1,
                 T *phaseCh2,
                 T *gain,
                 T *phaseDiff){
    constexpr size_t ADC_BUFFER_SIZE = 1024 * 16;
    if (adcRate == 0) return RP_A_ERROR;
    if (ch1_size != ch2_size) return RP_A_ERROR;
    if (ch1_size == 0 || ch2_size == 0) return RP_A_DATA_SIZE_ZERO;
    if (ch1_size > ADC_BUFFER_SIZE) return RP_A_ERROR;

    size_t size = ch1_size;
    float w_out = (double)signalFreq * 2.0 * M_PI;

    double ang,  phase;

    double t = (double)decimation / (double)adcRate;

    double u_dut_1[ADC_BUFFER_SIZE];
    double u_dut_2[ADC_BUFFER_SIZE];
    double u_dut_1_s[2][ADC_BUFFER_SIZE];
    double u_dut_2_s[2][ADC_BUFFER_SIZE];

    double component_lock_in[2][2];
    int ret_value = RP_A_OK;

    T mean_ch1;
    T mean_ch2;
    mean<T>(ch1, ch1_size, &mean_ch1);
    mean<T>(ch2, ch2_size, &mean_ch2);

    for (size_t i = 0; i < size; i++){
        u_dut_1[i] = ch1[i]- mean_ch1;
        u_dut_2[i] = ch2[i]- mean_ch2;
    }

    if (size > 0){
        double u1_max = u_dut_1[0];
        double u1_min = u_dut_1[0];
        double u2_max = u_dut_2[0];
        double u2_min = u_dut_2[0];
        for (size_t i = 1; i < size; i++){
            if (u1_max < u_dut_1[i]) u1_max = u_dut_1[i];
            if (u2_max < u_dut_2[i]) u2_max = u_dut_2[i];
            if (u1_min > u_dut_1[i]) u1_min = u_dut_1[i];
            if (u2_min > u_dut_2[i]) u2_min = u_dut_2[i];
        }
        if ((u1_max - u1_min) < input_threshold) ret_value = RP_A_SMALL_SIGNAL;
        if ((u2_max - u2_min) < input_threshold) ret_value = RP_A_SMALL_SIGNAL;
    }


    for (size_t i = 0; i < size; i++){
        ang = ((double)i * t * w_out);

        u_dut_1_s[0][i] = u_dut_1[i] * sin(ang);
        u_dut_1_s[1][i] = u_dut_1[i] * sin(ang + (M_PI / 2));

        // printf("%f;%f;%f\n",u_dut_1[i], u_dut_1_s[0][i] , u_dut_1_s[1][i]);

        u_dut_2_s[0][i] = u_dut_2[i] * sin(ang);
        u_dut_2_s[1][i] = u_dut_2[i] * sin(ang + (M_PI / 2));
    }
    /* Trapezoidal approximation */
    component_lock_in[0][0] = trapezoidalApprox(u_dut_1_s[0],size); //U_X ch1
    component_lock_in[0][1] = trapezoidalApprox(u_dut_1_s[1],size); //U_Y ch1

    component_lock_in[1][0] = trapezoidalApprox(u_dut_2_s[0],size); //U_X ch2
    component_lock_in[1][1] = trapezoidalApprox(u_dut_2_s[1],size); //U_Y ch2


    // printf("C %.9lf , %.9lf \n",component_lock_in[0][0],component_lock_in[0][1]);

    /* Calculating voltage amplitude and phase */
    auto u_dut_1_ampl = 2.0 * (sqrtf(powf(component_lock_in[0][0], 2.0) + powf(component_lock_in[0][1], 2.0)));

    auto u_dut_1_phase = atan2f(component_lock_in[0][1], component_lock_in[0][0]);

    /* Calculating current amplitude and phase */
    auto u_dut_2_ampl = 2.0 * (sqrtf(powf(component_lock_in[1][0], 2.0) + powf(component_lock_in[1][1], 2.0)));

    auto u_dut_2_phase = atan2f(component_lock_in[1][1], component_lock_in[1][0]);

    phase = u_dut_2_phase - u_dut_1_phase;
    /* Phase has to be limited between M_PI and -M_PI. */
    if (phase <= -M_PI)
            phase += 2*M_PI;
    else if (phase >= M_PI)
            phase -= 2*M_PI;

    *phaseCh1 = u_dut_1_phase;
    *phaseCh2 = u_dut_2_phase;
    // printf("%lf %lf \n",u_dut_1_ampl * 1000,u_dut_2_ampl * 1000);
    *phaseDiff = phase * (180.0 / M_PI);
    *gain =  u_dut_2_ampl / u_dut_1_ampl;
    return ret_value;
}

int analysisTrap(const std::vector<float> &ch1, const std::vector<float> &ch2,
                 uint32_t signalFreq, uint32_t decimation, uint32_t adcRate, float input_threshold,
                 float *phaseCh1, float *phaseCh2, float *gain, float *phaseDiff){
    return analysisTrap<float>(ch1.data(),ch2.data(),ch1.size(),ch2.size(),signalFreq,decimation,adcRate,input_threshold,phaseCh1,phaseCh2,gain,phaseDiff);
}

int analysisTrap(const std::vector<double> &ch1, const std::vector<double> &ch2,
                 uint32_t signalFreq, uint32_t decimation, uint32_t adcRate, float input_threshold,
                 double *phaseCh1, double *phaseCh2, double *gain, double *phaseDiff){
    return analysisTrap<double>(ch1.data(),ch2.data(),ch1.size(),ch2.size(),signalFreq,decimation,adcRate,input_threshold,phaseCh1,phaseCh2,gain,phaseDiff);
}

int analysisTrap(const float *ch1, const float *ch2,
                size_t ch1_size, size_t ch2_size,
                uint32_t signalFreq, uint32_t decimation, uint32_t adcRate, float input_threshold,
                float *phaseCh1, float *phaseCh2, float *gain, float *phaseDiff){
    return analysisTrap<float>(ch1,ch2,ch1_size,ch2_size,signalFreq,decimation,adcRate,input_threshold,phaseCh1,phaseCh2,gain,phaseDiff);
}

int analysisTrap(const double *ch1, const double *ch2,
                size_t ch1_size, size_t ch2_size,
                uint32_t signalFreq, uint32_t decimation, uint32_t adcRate, float input_threshold,
                double *phaseCh1, double *phaseCh2, double *gain, double *phaseDiff){
    return analysisTrap<double>(ch1,ch2,ch1_size,ch2_size,signalFreq,decimation,adcRate,input_threshold,phaseCh1,phaseCh2,gain,phaseDiff);
}

auto oversampling(const float *_inData,size_t _inSize,float *_outData,size_t _outSize, oversampling_mode_t _mode) -> bool{

    // auto screenToBufferRepeated = [=](int i, int size, float dec, float *t) -> int {
    //     float z = (float)i * dec - 1;
    //     int x = floor(z);
    //     *t = z - x;
    //     if (x >= 0){
    //         return x % size;
    //     }else{
    //         while(x < 0){
    //             x += size;
    //         }
    //     }
    //     return  x % size;
    // };

    auto screenToBuffer = [=](int i, int size, float dec, float *t) -> int {
        float z = (float)i * dec;
        int x = floor(z);
        *t = z - x;
        if (x > size) return INT32_MAX;
        if (x < -size) return INT32_MAX;
        if (x >= 0){
            return x;
        }else{
                x += size;
        }
        return  x;
    };

    if (_inSize >= _outSize) return false;

    auto viewSize = _outSize;
    float decimationFactor = (float)_inSize / (float)_outSize;

    int startView = 0;
    int stopView = viewSize;

    float t = 0;
    uint16_t iView = 0;

    for(int idx = startView ; idx < stopView; idx++, iView++ ){
        int dataIndex1 = screenToBuffer(idx,_inSize, decimationFactor, &t);
        float y = 0;
        if (dataIndex1 != INT32_MAX){
            int dataIndex2 = (dataIndex1 + 1) % _inSize;
            switch (_mode)
            {
                case OM_DISABLED:{
                    y = linear<float>(0, _inData[dataIndex1], _inData[dataIndex2], 0 , t);
                    break;
                }

                case OM_BSPLINE:{
                    int dataIndex0 = (dataIndex1 - 1) < 0 ? _inSize - 1 : (dataIndex1 - 1) % _inSize;
                    int dataIndex3 = (dataIndex1 + 2) % _inSize;
                    y = bSpline<float>(_inData[dataIndex0], _inData[dataIndex1], _inData[dataIndex2], _inData[dataIndex3] , t);
                    break;
                }

                case OM_CATMULLROM:{
                    int dataIndex0 = (dataIndex1 - 1) < 0 ? _inSize - 1 : (dataIndex1 - 1) % _inSize;
                    int dataIndex3 = (dataIndex1 + 2) % _inSize;
                    y = catmullRom<float>(_inData[dataIndex0], _inData[dataIndex1], _inData[dataIndex2], _inData[dataIndex3] , t);
                    break;
                }
                case OM_LANCZOS:{
                    int dataIndex_1 = (dataIndex1 - 2) < 0 ? _inSize - 2 : (dataIndex1 - 2) % _inSize;
                    int dataIndex0 = (dataIndex1 - 1) < 0 ? _inSize - 1 : (dataIndex1 - 1) % _inSize;
                    int dataIndex3 = (dataIndex1 + 2) % _inSize;
                    y = lanczos<float>(_inData[dataIndex_1],_inData[dataIndex0], _inData[dataIndex1], _inData[dataIndex2], _inData[dataIndex3] , t);
                    break;
                }
                default:
                    FATAL("No function for interpolation mode")
                    break;
            }
        }
        _outData[iView] = y;
    }

    return true;
}



template<typename T>
T calcPhase(T *data_origin, size_t size, std::vector<T> *data, T ampl, T freq, T periods, T phase, T step){
    if (step < 0.01) return 1000;
    static auto genSignal = [](std::vector<T> *data, T amp, T phase, T periods){
        for(uint32_t i = 0; i < data->size(); i++){
            T z = amp * sin((float)i / ((float)data->size()) * periods * 2 * M_PI + (phase));
            (*data)[i] = z;
        }
    };


    static auto compare = [](T *data, size_t size, std::vector<T> *sig2){
        T res = 0;
        for(size_t i = 0; i < size; i++){
            res += (data[i] - (*sig2)[i]) * (data[i] - (*sig2)[i]);
        }
        return (T)res;
    };

    T p0 = phase;
    T p1 = phase - step;
    T p2 = phase + step;

    genSignal(data,ampl,p0,periods);
    T res0 = compare(data_origin,size,data);
    genSignal(data,ampl,p1,periods);
    T res1 = compare(data_origin,size,data);
    genSignal(data,ampl,p2,periods);
    T res2 = compare(data_origin,size,data);

    // printf("P0 %f R0 %f ; P1 %f R1 %f ; P2 %f R2 %f\n",p0,res0,p1,res1,p2,res2);

    if (res1 <= res2){
        T new_step = (p0 - p1) / 2.0;
        T res =  calcPhase(data_origin,size,data,ampl,freq,periods,p1 + new_step,new_step);
        if (res != 1000){
            return res;
        }
    }

    if (res1 >= res2){
        T new_step = (p2 - p0) / 2.0;
        T res = calcPhase(data_origin,size,data,ampl,freq,periods,p0 + new_step,new_step);
        if (res != 1000){
            return res;
        }
    }
    T minimum = std::min({res0, res1, res2});
    if (minimum == res0) return p0;
    if (minimum == res1) return p1;
    if (minimum == res2) return p2;
    return 1000;
}


template<typename T>
int recoverySineSignal(T *data, size_t size, T freq, T periods, T *_ampl, T *_phase){
    static T sqrt_2 = sqrt((T)2.0);
    *_ampl = 0;
    *_phase = 0;
    if (size == 0) return RP_A_ERROR;

    T rms_value = 0;
    rms<T>(data,size,&rms_value);
    *_ampl = rms_value * sqrt_2;

    std::vector<T> signal;
    signal.resize(size);
    *_phase = calcPhase<T>(data,size,&signal,*_ampl,freq,periods,0,M_PI_2);
    if (*_phase == 1000){
        *_phase = 0;
        return RP_A_ERROR;
    }
    return RP_A_OK;
}

int analysis(std::vector<float> &ch1, std::vector<float> &ch2,
                uint32_t signalFreq, uint32_t periods,
                uint32_t decimation, uint32_t adcRate,
                float *amplutudeCh1, float *amplutudeCh2,
                float *phaseCh1, float *phaseCh2, float *gain, float *phaseDiff){
    float ampl[2];
    float phase[2];
    auto ret1 = recoverySineSignal<float>(ch1.data(),ch1.size(), signalFreq, periods, &ampl[0],&phase[0]);
    auto ret2 = recoverySineSignal<float>(ch2.data(),ch2.size(), signalFreq, periods, &ampl[1],&phase[1]);

    if (ret1 == RP_A_OK && ret2 == RP_A_OK){
        if (ampl[1] == 0) return RP_A_ERROR;
        *amplutudeCh1 = ampl[0];
        *amplutudeCh2 = ampl[1];
        *phaseCh1 = phase[0];
        *phaseCh2 = phase[1];
        *gain = ampl[1] / ampl[0];
        *phaseDiff = phase[1] - phase[0];
        if (*phaseDiff <= -M_PI)
            *phaseDiff += 2 * M_PI;
        else if (*phaseDiff >= M_PI)
            *phaseDiff -= 2 * M_PI;
        *phaseDiff *= 180/M_PI;
        return RP_A_OK;
    }
    return RP_A_ERROR;
}

int analysis(std::vector<double> &ch1, std::vector<double> &ch2,
                uint32_t signalFreq, uint32_t periods,
                uint32_t decimation, uint32_t adcRate,
                double *amplutudeCh1, double *amplutudeCh2,
                double *phaseCh1, double *phaseCh2, double *gain, double *phaseDiff){
    double ampl[2];
    double phase[2];
    auto ret1 = recoverySineSignal<double>(ch1.data(),ch1.size(), signalFreq, periods, &ampl[0],&phase[0]);
    auto ret2 = recoverySineSignal<double>(ch2.data(),ch2.size(), signalFreq, periods, &ampl[1],&phase[1]);

    if (ret1 == RP_A_OK && ret2 == RP_A_OK){
        if (ampl[1] == 0) return RP_A_ERROR;
        *amplutudeCh1 = ampl[0];
        *amplutudeCh2 = ampl[1];
        *phaseCh1 = phase[0];
        *phaseCh2 = phase[1];
        *gain = ampl[0] / ampl[1];
        *phaseDiff = phase[1] - phase[0];
        return RP_A_OK;
    }
    return RP_A_ERROR;
}

int analysis(float *ch1, float *ch2,
                size_t ch1_size, size_t ch2_size,
                uint32_t signalFreq, uint32_t periods,
                uint32_t decimation, uint32_t adcRate,
                float *amplutudeCh1, float *amplutudeCh2,
                float *phaseCh1, float *phaseCh2, float *gain, float *phaseDiff){
    float ampl[2];
    float phase[2];
    auto ret1 = recoverySineSignal<float>(ch1, ch1_size, signalFreq, periods, &ampl[0],&phase[0]);
    auto ret2 = recoverySineSignal<float>(ch2, ch2_size, signalFreq, periods, &ampl[1],&phase[1]);

    if (ret1 == RP_A_OK && ret2 == RP_A_OK){
        if (ampl[1] == 0) return RP_A_ERROR;
        *amplutudeCh1 = ampl[0];
        *amplutudeCh2 = ampl[1];
        *phaseCh1 = phase[0];
        *phaseCh2 = phase[1];
        *gain = ampl[0] / ampl[1];
        *phaseDiff = phase[1] - phase[0];
        return RP_A_OK;
    }
    return RP_A_ERROR;
}

int analysis(double *ch1, double *ch2,
                size_t ch1_size, size_t ch2_size,
                uint32_t signalFreq, uint32_t periods,
                uint32_t decimation, uint32_t adcRate,
                double *amplutudeCh1, double *amplutudeCh2,
                double *phaseCh1, double *phaseCh2, double *gain, double *phaseDiff){
    double ampl[2];
    double phase[2];
    auto ret1 = recoverySineSignal<double>(ch1, ch1_size, signalFreq, periods, &ampl[0],&phase[0]);
    auto ret2 = recoverySineSignal<double>(ch2, ch2_size, signalFreq, periods, &ampl[1],&phase[1]);

    if (ret1 == RP_A_OK && ret2 == RP_A_OK){
        if (ampl[1] == 0) return RP_A_ERROR;
        *amplutudeCh1 = ampl[0];
        *amplutudeCh2 = ampl[1];
        *phaseCh1 = phase[0];
        *phaseCh2 = phase[1];
        *gain = ampl[0] / ampl[1];
        *phaseDiff = phase[1] - phase[0];
        return RP_A_OK;
    }
    return RP_A_ERROR;
}

int initFFT(uint32_t max_buffer,uint32_t adcRate){
    std::lock_guard lock(g_fft_mutex);
    if (g_fft != NULL){
        ERROR("Can't init memory")
        return RP_A_ERROR;
    }
    g_fft = new rp_dsp_api::CDSP(2,max_buffer,adcRate);
    if (g_fft == NULL){
        ERROR("Can't init memory")
        return RP_A_ERROR;
    }
    g_fft_data = g_fft->createData();
    if (g_fft_data == NULL) return RP_A_ERROR;
    return RP_A_OK;
}

int releaseFFT(){
    std::lock_guard<std::mutex> lock(g_fft_mutex);
    if (g_fft)
        g_fft->deleteData(g_fft_data);
    delete g_fft;
    g_fft = NULL;
    return RP_A_OK;
}

template<typename T>
int analysisFFT(const T *ch1, const T *ch2,
                size_t size,
                T _freq,
                int decimation,
                T *gain,
                T *phase_out,
                float input_threshold)
{
    std::lock_guard lock(g_fft_mutex);
    // T w_out = (T)_freq * 2.0 * M_PI;
    int ret_value = RP_A_OK;
    if (size > 0){
            double u1_max = ch1[0];
            double u1_min = ch1[0];
            double u2_max = ch2[0];
            double u2_min = ch2[0];
            for (size_t i = 0; i < size; i++){
                    if (u1_max < ch1[i]) u1_max = ch1[i];
                    if (u2_max < ch2[i]) u2_max = ch2[i];
                    if (u1_min > ch1[i]) u1_min = ch1[i];
                    if (u2_min > ch2[i]) u2_min = ch2[i];
                g_fft_data->m_in[0][i] = ch1[i];
                g_fft_data->m_in[1][i] = ch2[i];
            }
            if ((u1_max - u1_min) < input_threshold) ret_value = RP_A_SMALL_SIGNAL;
            if ((u2_max - u2_min) < input_threshold) ret_value = RP_A_SMALL_SIGNAL;
    }

    if (g_fft->setSignalLengthDiv2(size)){
        ERROR("Can't set buffer size for FFT")
        return RP_A_ERROR;
    }

    if (g_fft->window_init(rp_dsp_api::FLAT_TOP)){
        ERROR("Can't init window")
        return RP_A_ERROR;
    }

    g_fft->fftInit();
    g_fft->prepareFreqVector(g_fft_data,decimation);


    g_fft->windowFilter(g_fft_data);
    g_fft->fft(g_fft_data);
    double amp[2];
    double phase[2];
    g_fft->getAmpAndPhase(g_fft_data, _freq, &amp[0],&phase[0],&amp[1],&phase[1]);

    TRACE_SHORT("A1 %f A2 %f P1 %f P2 %f",amp[0],amp[1],phase[0] * 180 / M_PI ,phase[1] * 180 / M_PI);
    auto phase2 = phase[1] - phase[0];
    if (phase2 <= -M_PI)
        phase2 += 2 * M_PI;
    else if (phase2 >= M_PI)
        phase2 -= 2 * M_PI;
    phase2 *= 180 / M_PI;
    *phase_out = phase2;
    *gain = amp[1]/amp[0];
    return ret_value;
}

int analysisFFT(const float *ch1, const float *ch2,
                size_t size,
                float freq,
                int decimation,
                float *gain,
                float *phase_out,
                float input_threshold){
    return analysisFFT<float>(ch1,ch2,size,freq,decimation,gain,phase_out,input_threshold);
}

int analysisFFT(const double *ch1, const double *ch2,
                size_t size,
                double freq,
                int decimation,
                double *gain,
                double *phase_out,
                float input_threshold){
    return analysisFFT<double>(ch1,ch2,size,freq,decimation,gain,phase_out,input_threshold);
}

int analysisFFT(const std::vector<float> &ch1, const std::vector<float> &ch2,
                float freq,
                int decimation,
                float *gain,
                float *phase_out,
                float input_threshold){
    if (ch1.size() != ch2.size()) return RP_A_ERROR;
    auto size = ch1.size();
    return analysisFFT<float>(ch1.data(),ch2.data(),size,freq,decimation,gain,phase_out,input_threshold);
}

int analysisFFT(const std::vector<double> &ch1, const std::vector<double> &ch2,
                double freq,
                int decimation,
                double *gain,
                double *phase_out,
                double input_threshold){
    if (ch1.size() != ch2.size()) return RP_A_ERROR;
    auto size = ch1.size();
    return analysisFFT<double>(ch1.data(),ch2.data(),size,freq,decimation,gain,phase_out,input_threshold);
}