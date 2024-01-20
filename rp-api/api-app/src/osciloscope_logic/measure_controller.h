#ifndef __MEASURE_CONTROLLER_H
#define __MEASURE_CONTROLLER_H

#include <stdint.h>
#include <mutex>
#include <functional>
#include "constants.h"
#include "rpApp.h"

class CMeasureController{

    public:
        typedef std::function<int(rpApp_osc_source source, float volts, float *res)> func_t;

    CMeasureController();
    ~CMeasureController();

    CMeasureController(CMeasureController &) = delete;
    CMeasureController(CMeasureController &&) = delete;

    auto setUnScaleFunction(func_t _func) -> void;
    auto getUnScaleFunction() const -> func_t;

    auto setscaleFunction(func_t _func) -> void;
    auto getscaleFunction() const -> func_t;

    auto setAttenuateAmplitudeChannelFunction(func_t _func) -> void;
    auto getAttenuateAmplitudeChannelFunction() const -> func_t;

    auto measureVpp(const rpApp_osc_source _channel, const std::vector<float> *_data, float *_Vpp) -> int;
    auto measureMax(const rpApp_osc_source _channel, const std::vector<float> *_data, float *_Max) -> int;
    auto measureMin(const rpApp_osc_source _channel, const std::vector<float> *_data, float *_Min) -> int;
    auto measureDutyCycle(const rpApp_osc_source _channel, const std::vector<float> *_data, float *_dutyCycle) -> int;
    auto measureRootMeanSquare(const rpApp_osc_source _channel, const std::vector<float> *_data, float *_rms) -> int;

    auto measureMeanVoltage(const rpApp_osc_source _channel, const std::vector<float> *_data, float *_meanVoltage) -> int;
    auto measureMaxVoltage(const rpApp_osc_source _channel,bool _inverted, const std::vector<float> *_data, float *_Vmax) -> int;
    auto measureMinVoltage(const rpApp_osc_source _channel,bool _inverted, const std::vector<float> *_data, float *_Vmin) -> int;

    auto measurePeriodCh(const float *_dataRaw, vsize_t _dataSize, float *period) -> int;
    auto measurePeriodMath(float _timeScale, float _sampPerDev, const std::vector<float> *_data, float *period) -> int;

private:

    auto check(const void *_data, vsize_t _sizeView) -> int;

    func_t m_unscaleFunc;
    func_t m_scaleFunc;
    func_t m_attAmplFunc;
    std::mutex m_settingsMutex;
    double m_sample_per;
    float  m_osc_fpga_smpl_freq;
    uint8_t m_adc_bits;
};

#endif // __MEASURE_CONTROLLER_H