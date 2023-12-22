#include <math.h>
#include <iostream>
#include <string>
#include <cstring>

#include "rp_algorithms.h"
#include "rp_interpolation.h"

#define __SHORT_FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define FATAL(X)  {fprintf(stderr, "Error at line %d, file %s errno %d [%s] %s\n", __LINE__, __SHORT_FILENAME__, errno, strerror(errno),X); exit(1);}
#define WARNING(...) { char error_msg[512]; snprintf(error_msg,512,__VA_ARGS__);fprintf(stderr,"[W] %s:%d %s\n",__SHORT_FILENAME__,__LINE__,error_msg);}

#ifdef TRACE_ENABLE
#define TRACE(...) { char error_msg[512]; snprintf(error_msg,512,__VA_ARGS__);fprintf(stderr,"[T] %s:%d %s\n",__SHORT_FILENAME__,__LINE__,error_msg);}
#define TRACE_SHORT(...) { char error_msg[512]; snprintf(error_msg,512,__VA_ARGS__);fprintf(stderr,"[T] %s\n",error_msg);}
#else
#define TRACE(...)
#define TRACE_SHORT(...)
#endif

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
int rms(const std::vector<T> &data, T *_value){
    if (data.size() == 0) return RP_A_DATA_SIZE_ZERO;
    T result = 0;
    for(auto i = 0u; i < data.size(); i++){
		T d = (T)data[i];
        result += d * d;
    }
    result =  sqrt(result / (T)data.size());
    *_value = result;
    return RP_A_OK;
}

int rms(const std::vector<float> &data, float *_value){
    return rms<float>(data,_value);
}

int rms(const std::vector<double> &data, double *_value){
    return rms<double>(data,_value);
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

int trapezoidalApprox(const double *data, size_t data_size, double *_value){
    if (data_size < 2) return RP_A_ERROR;
    double result = 0;
    for(auto i = 0u; i < data_size - 1; i++){
        result += data[i] + data[i+1];
    }
    *_value = result;
    return RP_A_OK;
}

int trapezoidalApprox(const std::vector<double> &data, double *_value){
    return trapezoidalApprox(data.data(),data.size(),_value);
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
    double w_out = (double)signalFreq * 2.0 * M_PI;

    double ang,  phase;

    double t = (double)decimation / (double)adcRate;

    double u_dut_1[ADC_BUFFER_SIZE];
    double u_dut_2[ADC_BUFFER_SIZE];
    double u_dut_1_s[2][ADC_BUFFER_SIZE];
    double u_dut_2_s[2][ADC_BUFFER_SIZE];

    // std::vector<double> u_dut_1;
    // u_dut_1.resize(ch1_size);
    // std::vector<double> u_dut_2;
    // u_dut_2.resize(ch2_size);

    // std::vector<double> u_dut_1_s[2];
    // u_dut_1_s[0].resize(size);
    // u_dut_1_s[1].resize(size);

    // std::vector<double> u_dut_2_s[2];
    // u_dut_2_s[0].resize(size);
    // u_dut_2_s[1].resize(size);


    double component_lock_in[2][2];
    int ret_value = RP_A_OK;

    T mean_ch1;
    T mean_ch2;
    mean<T>(ch1, ch1_size, &mean_ch1);
    mean<T>(ch2, ch2_size, &mean_ch2);

    for (size_t i = 0; i < size; i++){
        u_dut_1[i] = ch1[i] - mean_ch1;
        u_dut_2[i] = ch2[i] - mean_ch2;
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
    trapezoidalApprox(u_dut_1_s[0], size , &(component_lock_in[0][0])); //U_X ch1
    trapezoidalApprox(u_dut_1_s[1], size , &(component_lock_in[0][1])); //U_Y ch1

    trapezoidalApprox(u_dut_2_s[0], size , &(component_lock_in[1][0])); //U_X ch2
    trapezoidalApprox(u_dut_2_s[1], size , &(component_lock_in[1][1])); //U_Y ch2


    // printf("C %.9lf , %.9lf \n",component_lock_in[0][0],component_lock_in[0][1]);

    /* Calculating voltage amplitude and phase */
    auto u_dut_1_ampl = 2.0 * (sqrt(pow(component_lock_in[0][0], 2.0) + pow(component_lock_in[0][1], 2.0)));

    auto u_dut_1_phase = atan2(component_lock_in[0][1], component_lock_in[0][0]);

    /* Calculating current amplitude and phase */
    auto u_dut_2_ampl = 2.0 * (sqrt(pow(component_lock_in[1][0], 2.0) + pow(component_lock_in[1][1], 2.0)));

    auto u_dut_2_phase = atan2(component_lock_in[1][1], component_lock_in[1][0]);

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
    *gain = u_dut_1_ampl / u_dut_2_ampl;
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