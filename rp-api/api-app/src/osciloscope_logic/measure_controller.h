#ifndef __MEASURE_CONTROLLER_H
#define __MEASURE_CONTROLLER_H

#include <stdint.h>
#include <stdio.h>
#include <functional>
#include <mutex>
#include "constants.h"
#include "math/rp_dsp.h"
#include "rpApp.h"

class CMeasureController {

   public:
    struct AutoScaleInfo {
        std::vector<uint32_t> freqByChannels;
        std::vector<float> powerByChannels;
        std::vector<float> vppByChannels;
        uint32_t freq;
        float power;
        int8_t channel;
        auto print() -> int {
            for (auto idx = 0u; idx < freqByChannels.size(); idx++) {
                fprintf(stderr, "%s%d", (idx == 0 ? "Freq: {" : ","), freqByChannels[idx]);
            }
            fprintf(stderr, "}\n");
            for (auto idx = 0u; idx < powerByChannels.size(); idx++) {
                fprintf(stderr, "%s%f", (idx == 0 ? "Power: {" : ","), powerByChannels[idx]);
            }
            fprintf(stderr, "}\n");
            for (auto idx = 0u; idx < vppByChannels.size(); idx++) {
                fprintf(stderr, "%s%f", (idx == 0 ? "Vpp: {" : ","), vppByChannels[idx]);
            }
            fprintf(stderr, "}\n");
            fprintf(stderr, "Selected channel: %d freq: %d power: %f\n", channel + 1, freq, power);
            return 0;
        }
    };

    typedef std::function<int(rpApp_osc_source source, float volts, float* res)> func_t;

    CMeasureController();
    ~CMeasureController();

    CMeasureController(CMeasureController&) = delete;
    CMeasureController(CMeasureController&&) = delete;

    auto setUnScaleFunction(func_t _func) -> void;
    auto getUnScaleFunction() const -> func_t;

    auto setscaleFunction(func_t _func) -> void;
    auto getscaleFunction() const -> func_t;

    auto setAttenuateAmplitudeChannelFunction(func_t _func) -> void;
    auto getAttenuateAmplitudeChannelFunction() const -> func_t;

    auto scaleValue(const rpApp_osc_source _channel, float _value) -> float;
    auto unscaleValue(const rpApp_osc_source _channel, float _value) -> float;

    auto measureVpp(const rpApp_osc_source _channel, const std::vector<float>* _data, float* _Vpp) -> int;
    auto measureMax(const rpApp_osc_source _channel, const std::vector<float>* _data, float* _Max) -> int;
    auto measureMin(const rpApp_osc_source _channel, const std::vector<float>* _data, float* _Min) -> int;
    auto measureDutyCycle(const rpApp_osc_source _channel, const std::vector<float>* _data, float* _dutyCycle) -> int;
    auto measureRootMeanSquare(const rpApp_osc_source _channel, const std::vector<float>* _data, float* _rms) -> int;

    auto measureMeanVoltage(const rpApp_osc_source _channel, const std::vector<float>* _data, float* _meanVoltage) -> int;
    auto measureMaxVoltage(const rpApp_osc_source _channel, bool _inverted, const std::vector<float>* _data, float* _Vmax) -> int;
    auto measureMinVoltage(const rpApp_osc_source _channel, bool _inverted, const std::vector<float>* _data, float* _Vmin) -> int;

    auto measurePeriodCh(const float* _dataRaw, vsize_t _dataSize, float* period) -> int;
    auto measurePeriodMath(float _timeScale, float _sampPerDev, const std::vector<float>* _data, float* period) -> int;

    auto getAutoScaleStoredData(uint32_t idx) -> rp_dsp_api::data_t*;
    auto calculateAutoScaleFreq(uint32_t channels, AutoScaleInfo& info) -> void;

   private:
    auto check(const void* _data, vsize_t _sizeView) -> int;
    auto createDSPforAutoScale(uint32_t bufferSize) -> void;

    func_t m_unscaleFunc;
    func_t m_scaleFunc;
    func_t m_attAmplFunc;
    std::mutex m_settingsMutex;
    double m_sample_per;
    float m_osc_fpga_smpl_freq;
    uint8_t m_adc_bits;
    std::vector<rp_dsp_api::CDSP*> m_cdsp;
    std::vector<uint32_t> m_cdspDec;
};

#endif  // __MEASURE_CONTROLLER_H