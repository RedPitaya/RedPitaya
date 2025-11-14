#include "generator.h"
#include "common.h"
#include "common/rp_sweep.h"
#include "math/rp_dsp.h"
#include "rp.h"
#include "rp_math.h"
#include "settings.h"
#include "web/rp_client.h"

static uint8_t g_dac_count = getDACChannels();
const bool is_z_present = isZModePresent();
const bool isX5Gain = rp_HPGetIsGainDACx5OrDefault();

// Generator
CBooleanParameter outState[MAX_DAC_CHANNELS] = INIT2("OUTPUT", "_STATE", CBaseParameter::RW, false, 0, CONFIG_VAR);
CFloatParameter outAmplitude[MAX_DAC_CHANNELS] = INIT2("SOUR", "_VOLT", CBaseParameter::RW, LEVEL_AMPS_DEF, 0, 0, LEVEL_AMPS_MAX, CONFIG_VAR);
CFloatParameter outOffset[MAX_DAC_CHANNELS] = INIT2("SOUR", "_VOLT_OFFS", CBaseParameter::RW, 0, 0, -LEVEL_AMPS_MAX, LEVEL_AMPS_MAX, CONFIG_VAR);
CFloatParameter outFrequancy[MAX_DAC_CHANNELS] = INIT2("SOUR", "_FREQ_FIX", CBaseParameter::RW, 1000, 0, (float)outFreqMin(), (float)outFreqMax(), CONFIG_VAR);
CFloatParameter outPhase[MAX_DAC_CHANNELS] = INIT2("SOUR", "_PHAS", CBaseParameter::RW, 0, 0, -360, 360, CONFIG_VAR);
CFloatParameter outDCYC[MAX_DAC_CHANNELS] = INIT2("SOUR", "_DCYC", CBaseParameter::RW, 50, 0, 0, 100, CONFIG_VAR);
CFloatParameter outRiseTime[MAX_DAC_CHANNELS] = INIT2("SOUR", "_RISE", CBaseParameter::RW, 1, 0, 0.1, 1000, CONFIG_VAR);
CFloatParameter outFallTime[MAX_DAC_CHANNELS] = INIT2("SOUR", "_FALL", CBaseParameter::RW, 1, 0, 0.1, 1000, CONFIG_VAR);
CStringParameter outWaveform[MAX_DAC_CHANNELS] = INIT2("SOUR", "_FUNC", CBaseParameter::RW, "0", 0, CONFIG_VAR);

CStringParameter outARBList = CStringParameter("ARB_LIST", CBaseParameter::RW, loadARBList(), 0);

CIntParameter outGain[MAX_DAC_CHANNELS] = INIT2("CH", "_OUT_GAIN", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter outTemperatureRuntime[MAX_DAC_CHANNELS] = INIT2("SOUR", "_TEMP_RUNTIME", CBaseParameter::RO, 0, 0, 0, 1);
CIntParameter outTemperatureLatched[MAX_DAC_CHANNELS] = INIT2("SOUR", "_TEMP_LATCHED", CBaseParameter::RO, 0, 0, 0, 1);
CIntParameter outImp[MAX_DAC_CHANNELS] = INIT2("SOUR", "_IMPEDANCE", CBaseParameter::RW, 0, 0, 0, 1, isZModePresent() ? CONFIG_VAR : 0);

CBooleanParameter outImpZmode("SOUR_IMPEDANCE_Z_MODE", CBaseParameter::RO, is_z_present, 0);
CFloatParameter outAmplitudeMax("SOUR_VOLT_MAX", CBaseParameter::RO, LEVEL_AMPS_MAX, 0, LEVEL_AMPS_MAX, LEVEL_AMPS_MAX);

/// SWEEP VARIABLES /////////
CFloatParameter outSweepStartFrequancy[2] = INIT2("SOUR", "_SWEEP_FREQ_START", CBaseParameter::RW, 1000, 0, 1, MAX_DAC_FREQ, CONFIG_VAR);
CFloatParameter outSweepEndFrequancy[2] = INIT2("SOUR", "_SWEEP_FREQ_END", CBaseParameter::RW, 10000, 0, 1, MAX_DAC_FREQ, CONFIG_VAR);
CIntParameter outSweepMode[2] = INIT2("SOUR", "_SWEEP_MODE", CBaseParameter::RW, RP_GEN_SWEEP_MODE_LINEAR, 0, RP_GEN_SWEEP_MODE_LINEAR, RP_GEN_SWEEP_MODE_LOG, CONFIG_VAR);
CIntParameter outSweepRepetitions[2] = INIT2("SOUR", "_SWEEP_REP", CBaseParameter::RW, 1, 0, 1, 2147483647, CONFIG_VAR);
CBooleanParameter outSweepRepInf[2] = INIT2("SOUR", "_SWEEP_INF", CBaseParameter::RW, false, 0, CONFIG_VAR);
CIntParameter outSweepDir[2] = INIT2("SOUR", "_SWEEP_DIR", CBaseParameter::RW, RP_GEN_SWEEP_DIR_NORMAL, 0, RP_GEN_SWEEP_DIR_NORMAL, RP_GEN_SWEEP_DIR_UP_DOWN, CONFIG_VAR);
CIntParameter outSweepTime[2] = INIT2("SOUR", "_SWEEP_TIME", CBaseParameter::RW, 1000000, 0, 1, 2000000000, CONFIG_VAR);  // In microseconds
CBooleanParameter outSweepState[2] = INIT2("SOUR", "_SWEEP_STATE", CBaseParameter::RW, false, 0, CONFIG_VAR);

CBooleanParameter outSweepReset("SWEEP_RESET", CBaseParameter::RW, false, 0);

CBooleanParameter outX5Gain("SOUR_X5_GAIN", CBaseParameter::RO, isX5Gain, 0);

rp_sweep_api::CSweepController* g_sweepController;

void appGenInit() {
    g_sweepController = new rp_sweep_api::CSweepController();
    if (rp_HPIsFastDAC_PresentOrDefault()) {
        for (auto ch = 0u; ch < g_dac_count; ch++) {
            rp_GenAmp((rp_channel_t)ch, outAmplitude[ch].NewValue());
        }
    }
}

void appGenExit() {
    if (rp_HPGetFastDACIsTempProtectionOrDefault()) {
        for (auto ch = 0u; ch < g_dac_count; ch++) {
            rp_SetEnableTempProtection((rp_channel_t)ch, false);
        }
    }
    delete g_sweepController;
}

void setSweepRun(bool run) {
    if (rp_HPIsFastDAC_PresentOrDefault()) {
        g_sweepController->pause(!run);
    }
}

void UpdateGeneratorParameters(bool force) {
    for (auto ch = 0u; ch < g_dac_count; ch++) {
        // OUT1
        if (outState[ch].IsNewValue() || force) {
            if (outState[ch].NewValue()) {

                if (rp_HPGetFastDACIsTempProtectionOrDefault())
                    rp_SetLatchTempAlarm((rp_channel_t)ch, false);

                rp_GenOutEnable((rp_channel_t)ch);
                rp_GenResetTrigger((rp_channel_t)ch);
            } else {
                rp_GenOutDisable((rp_channel_t)ch);
            }
            outState[ch].Update();
            RESEND(outState[ch])
        }

        if (outAmplitude[ch].IsNewValue() || outOffset[ch].IsNewValue() || outImp[ch].IsNewValue() || force) {

            if (rp_HPGetIsGainDACx5OrDefault() && rp_HPGetIsDAC50OhmModeOrDefault()) {
                auto prevGain = outGain[ch].Value();
                auto prevAmp = outAmplitude[ch].Value();
                auto prevOff = outOffset[ch].Value();
                auto prevGainAPI = RP_GAIN_1X;
                auto prevAmpAPI = 0.0f;
                auto prevOffAPI = 0.0f;
                int res = 0;

                auto setAmpOff = [&]() {
                    auto newFpgaGain = RP_GAIN_1X;
                    float Coff = outImp[ch].Value() == 1 ? 2.0 : 1.0;  // 1 - 50Ohm. Coff = 2
                    if ((fabs(outAmplitude[ch].NewValue()) + fabs(outOffset[ch].NewValue())) * Coff > 1.0) {
                        newFpgaGain = RP_GAIN_5X;
                        res |= rp_GenAmp((rp_channel_t)ch, outAmplitude[ch].NewValue() / 5.0 * Coff);
                    } else {
                        res |= rp_GenAmp((rp_channel_t)ch, outAmplitude[ch].NewValue() * Coff);
                    }
                    res |= rp_GenOffset((rp_channel_t)ch, (outOffset[ch].NewValue() * Coff) / (outGain[ch].Value() == 1 ? 5.0 : 1.0));

                    auto curGenStatus = RP_GAIN_1X;
                    rp_GenGetGainOut((rp_channel_t)ch, &curGenStatus);
                    prevGainAPI = curGenStatus;
                    if (curGenStatus != newFpgaGain && rp_GenSetGainOut((rp_channel_t)ch, newFpgaGain) == RP_OK) {
                        outGain[ch].SendValue(newFpgaGain);
                    }
                };

                rp_GenGetAmp((rp_channel_t)ch, &prevAmpAPI);
                rp_GenGetOffset((rp_channel_t)ch, &prevOffAPI);

                setAmpOff();

                if (res == RP_OK) {
                    if (IS_NEW(outImp[ch]) && !force) {
                        float impCoff = outImp[ch].NewValue() == 1 ? 0.5 : 2.0;
                        outImp[ch].Update();
                        // auto resetValues = false;
                        if ((fabs(outAmplitude[ch].Value()) + fabs(outOffset[ch].Value())) <= outAmpMax() / 2.0) {
                            impCoff = 1.0;
                            setAmpOff();
                            // resetValues = true;
                        }
                        outAmplitude[ch].Value() = outAmplitude[ch].Value() * impCoff;
                        outAmplitude[ch].Update();
                        outOffset[ch].Value() = outOffset[ch].Value() * impCoff;
                        outOffset[ch].Update();
                        // if (resetValues){

                        // }
                    }
                } else {
                    rp_GenAmp((rp_channel_t)ch, prevAmpAPI);
                    rp_GenOffset((rp_channel_t)ch, prevOff);
                    rp_GenSetGainOut((rp_channel_t)ch, prevGainAPI);
                    outGain[ch].Update();
                    outOffset[ch].Update();
                    outAmplitude[ch].Update();
                    outGain[ch].SendValue(prevGain);
                    outAmplitude[ch].SendValue(prevAmp);
                    outOffset[ch].SendValue(prevOff);
                }
                outGain[ch].Update();
                outOffset[ch].Update();
                outAmplitude[ch].Update();
            } else {
                if (rp_HPGetIsDAC50OhmModeOrDefault()) {
                    auto prevAmp = outAmplitude[ch].Value();
                    auto prevOff = outOffset[ch].Value();
                    auto prevAmpAPI = 0.0f;
                    auto prevOffAPI = 0.0f;
                    int res = 0;

                    auto setAmpOff = [&]() {
                        float Coff = outImp[ch].Value() == 1 ? 2.0 : 1.0;  // 1 - 50Ohm. Coff = 2
                        res |= rp_GenAmp((rp_channel_t)ch, outAmplitude[ch].NewValue() * Coff);
                        res |= rp_GenOffset((rp_channel_t)ch, (outOffset[ch].NewValue() * Coff));
                    };

                    rp_GenGetAmp((rp_channel_t)ch, &prevAmpAPI);
                    rp_GenGetOffset((rp_channel_t)ch, &prevOffAPI);

                    setAmpOff();

                    if (res == RP_OK) {
                        if (IS_NEW(outImp[ch]) && !force) {
                            float impCoff = outImp[ch].NewValue() == 1 ? 0.5 : 2.0;
                            outImp[ch].Update();
                            if ((fabs(outAmplitude[ch].Value()) + fabs(outOffset[ch].Value())) <= outAmpMax() / 2.0) {
                                impCoff = 1.0;
                                setAmpOff();
                            }
                            outAmplitude[ch].Value() = outAmplitude[ch].Value() * impCoff;
                            outAmplitude[ch].Update();
                            outOffset[ch].Value() = outOffset[ch].Value() * impCoff;
                            outOffset[ch].Update();
                        }
                    } else {
                        rp_GenAmp((rp_channel_t)ch, prevAmpAPI);
                        rp_GenOffset((rp_channel_t)ch, prevOff);
                        outGain[ch].Update();
                        outOffset[ch].Update();
                        outAmplitude[ch].Update();
                        outAmplitude[ch].SendValue(prevAmp);
                        outOffset[ch].SendValue(prevOff);
                    }
                    outGain[ch].Update();
                    outOffset[ch].Update();
                    outAmplitude[ch].Update();
                } else {
                    if (rp_GenAmp((rp_channel_t)ch, outAmplitude[ch].NewValue()) == RP_OK) {
                        outAmplitude[ch].Update();
                    } else {
                        outAmplitude[ch].SendValue(outAmplitude[ch].Value());
                    }
                    if (rp_GenOffset((rp_channel_t)ch, outOffset[ch].NewValue()) == RP_OK) {
                        outOffset[ch].Update();
                    } else {
                        outOffset[ch].SendValue(outOffset[ch].Value());
                    }
                }
            }
            outOffset[ch].Update();
            outAmplitude[ch].Update();
            RESEND(outOffset[ch])
            RESEND(outAmplitude[ch])
        }

        if (IS_NEW(outFrequancy[ch]) || force) {
            rp_GenFreq((rp_channel_t)ch, outFrequancy[ch].NewValue());
            outFrequancy[ch].Update();
            float period = 1000000.0 / outFrequancy[ch].Value();
            outRiseTime[ch].SetMin(period * RISE_FALL_MIN_RATIO);
            outRiseTime[ch].SetMax(period * RISE_FALL_MAX_RATIO);
            outRiseTime[ch].Update();
            outFallTime[ch].SetMin(period * RISE_FALL_MIN_RATIO);
            outFallTime[ch].SetMax(period * RISE_FALL_MAX_RATIO);
            outFallTime[ch].Update();
            rp_GenTriggerOnly((rp_channel_t)ch);
            RESEND(outFrequancy[ch])
            RESEND(outRiseTime[ch])
            RESEND(outFallTime[ch])
        }

        if (outPhase[ch].IsNewValue() || force) {
            rp_GenPhase((rp_channel_t)ch, outPhase[ch].NewValue());
            outPhase[ch].Update();
            RESEND(outPhase[ch])
        }

        if (outDCYC[ch].IsNewValue() || force) {
            rp_GenDutyCycle((rp_channel_t)ch, outDCYC[ch].NewValue() / 100);
            outDCYC[ch].Update();
            RESEND(outDCYC[ch])
        }

        if (outRiseTime[ch].IsNewValue() || force) {
            rp_GenRiseTime((rp_channel_t)ch, outRiseTime[ch].NewValue());
            outRiseTime[ch].Update();
            RESEND(outRiseTime[ch])
        }

        if (outFallTime[ch].IsNewValue() || force) {
            rp_GenFallTime((rp_channel_t)ch, outFallTime[ch].NewValue());
            outFallTime[ch].Update();
            RESEND(outFallTime[ch])
        }

        if (outWaveform[ch].IsNewValue() || force) {
            auto wf = outWaveform[ch].NewValue();
            if (wf[0] == 'A') {
                auto signame = wf.erase(0, 1);
                float data[DAC_BUFFER_SIZE];
                uint32_t size;
                if (!rp_ARBGetSignalByName(signame, data, &size)) {
                    rp_GenArbWaveform((rp_channel_t)ch, data, size);
                    rp_GenWaveform((rp_channel_t)ch, RP_WAVEFORM_ARBITRARY);
                    outWaveform[ch].Update();
                } else {
                    outWaveform[ch].Update();
                    rp_GenWaveform((rp_channel_t)ch, RP_WAVEFORM_SINE);
                    outWaveform[ch].Value() = "0";
                }

            } else {
                try {
                    rp_waveform_t w = (rp_waveform_t)stoi(wf);
                    rp_GenWaveform((rp_channel_t)ch, w);
                    outWaveform[ch].Update();
                } catch (const std::exception&) {
                    rp_GenWaveform((rp_channel_t)ch, RP_WAVEFORM_SINE);
                    outWaveform[ch].Update();
                    outWaveform[ch].Value() = "0";
                }
            }
        }
    }

    if (rp_HPGetFastDACIsTempProtectionOrDefault()) {
        for (auto ch = 0u; ch < g_dac_count; ch++) {
            bool temperature_runtime = false;
            if (rp_GetRuntimeTempAlarm((rp_channel_t)ch, &temperature_runtime) == RP_OK) {
                if (outTemperatureRuntime[ch].Value() != temperature_runtime)
                    outTemperatureRuntime[ch].SendValue(temperature_runtime);
            }

            bool temperature_latched = false;
            if (rp_GetLatchTempAlarm((rp_channel_t)ch, &temperature_latched) == RP_OK) {
                if (outTemperatureLatched[ch].Value() != temperature_latched)
                    outTemperatureLatched[ch].SendValue(temperature_latched);
            }
        }
    }

    bool needUpdate = force | outSweepReset.IsNewValue();
    for (auto ch = 0u; ch < g_dac_count; ch++) {
        needUpdate |= outSweepStartFrequancy[ch].IsNewValue() | outSweepEndFrequancy[ch].IsNewValue() | outSweepMode[ch].IsNewValue() | outSweepRepetitions[ch].IsNewValue() |
                      outSweepRepInf[ch].IsNewValue() | outSweepDir[ch].IsNewValue() | outSweepTime[ch].IsNewValue() | outSweepState[ch].IsNewValue();
    }

    if (needUpdate) {
        for (auto ch = 0u; ch < g_dac_count; ch++) {
            outSweepStartFrequancy[ch].Update();
            outSweepEndFrequancy[ch].Update();
            outSweepMode[ch].Update();
            outSweepDir[ch].Update();
            outSweepTime[ch].Update();
            outSweepState[ch].Update();
            outSweepRepInf[ch].Update();
            outSweepRepetitions[ch].Update();

            g_sweepController->setStartFreq((rp_channel_t)ch, outSweepStartFrequancy[ch].Value());
            g_sweepController->setStopFreq((rp_channel_t)ch, outSweepEndFrequancy[ch].Value());
            g_sweepController->setMode((rp_channel_t)ch, (rp_gen_sweep_mode_t)outSweepMode[ch].Value());
            g_sweepController->setDir((rp_channel_t)ch, (rp_gen_sweep_dir_t)outSweepDir[ch].Value());
            g_sweepController->setTime((rp_channel_t)ch, outSweepTime[ch].Value());
            g_sweepController->genSweep((rp_channel_t)ch, outSweepState[ch].Value());
            g_sweepController->setNumberOfRepetitions((rp_channel_t)ch, outSweepRepInf[ch].Value(), outSweepRepetitions[ch].Value());
        }

        if (outSweepReset.NewValue()) {
            g_sweepController->resetAll();
            if (outState[0].Value() && outState[1].Value()) {
                rp_GenOutEnableSync(true);
                rp_GenSynchronise();
            }
        }

        if (outSweepState[0].Value() || outSweepState[1].Value()) {
            g_sweepController->run();
        } else {
            g_sweepController->stop();
        }
    }
}

void updateGen(void) {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return;

    for (auto ch = 0u; ch < g_dac_count; ch++) {
        float freq = 0;
        rp_GenGetFreq((rp_channel_t)ch, &freq);
        if ((int)freq != outFrequancy[ch].Value()) {
            outFrequancy[ch].SendValue((int)freq);
        }
    }
}