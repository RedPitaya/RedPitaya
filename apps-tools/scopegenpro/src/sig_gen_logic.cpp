#include <CustomParameters.h>
#include <DataManager.h>
#include <math.h>
#include <unistd.h>
#include <mutex>

#include "common.h"
#include "common/rp_arb.h"
#include "common/rp_sweep.h"
#include "main.h"
#include "rp_hw-profiles.h"
#include "sig_gen.h"
#include "sig_gen_logic.h"

const float LEVEL_AMPS_MAX = outAmpMax();
const float LEVEL_AMPS_DEF = outAmpDef();
const bool is_z_present = isZModePresent();

bool g_updateOutWaveFormCh[MAX_DAC_CHANNELS] = {true, true};
float g_tscale_last = 0;
std::mutex g_updateOutWaveFormCh_mtx;

CFloatBase64Signal outSignal[MAX_DAC_CHANNELS] = INIT2("output", "", CH_SIGNAL_SIZE_DEFAULT, 0.0f);

CBooleanParameter outShow[MAX_DAC_CHANNELS] = INIT2("OUTPUT", "_SHOW", CBaseParameter::RW, true, 0, CONFIG_VAR);
CBooleanParameter outState[MAX_DAC_CHANNELS] = INIT2("OUTPUT", "_STATE", CBaseParameter::RW, false, 0, CONFIG_VAR);
CFloatParameter outAmplitude[MAX_DAC_CHANNELS] = INIT2("SOUR", "_VOLT", CBaseParameter::RW, LEVEL_AMPS_DEF, 0, 0, LEVEL_AMPS_MAX, CONFIG_VAR);
CFloatParameter outOffset[MAX_DAC_CHANNELS] = INIT2("SOUR", "_VOLT_OFFS", CBaseParameter::RW, 0, 0, -LEVEL_AMPS_MAX, LEVEL_AMPS_MAX, CONFIG_VAR);
CIntParameter outFrequancy[MAX_DAC_CHANNELS] = INIT2("SOUR", "_FREQ_FIX", CBaseParameter::RW, 1000, 0, 1, (int)getDACRate(), CONFIG_VAR);

CFloatParameter outShowOffset[MAX_DAC_CHANNELS] = INIT2("GPOS_OFFSET_OUTPUT", "", CBaseParameter::RW, 0, 0, -5000, 5000, CONFIG_VAR);
CFloatParameter outScale[MAX_DAC_CHANNELS] = INIT2("GPOS_SCALE_OUTPUT", "", CBaseParameter::RW, 1, 0, 0.00005, 1000, CONFIG_VAR);

rp_sweep_api::CSweepController* g_sweepController = new rp_sweep_api::CSweepController();

CIntParameter outSweepStartFrequancy[MAX_DAC_CHANNELS] = INIT2("SOUR", "_SWEEP_FREQ_START", CBaseParameter::RW, 1000, 0, 1, (int)getDACRate(), CONFIG_VAR);
CIntParameter outSweepEndFrequancy[MAX_DAC_CHANNELS] = INIT2("SOUR", "_SWEEP_FREQ_END", CBaseParameter::RW, 10000, 0, 1, (int)getDACRate(), CONFIG_VAR);
CIntParameter outSweepMode[MAX_DAC_CHANNELS] =
    INIT2("SOUR", "_SWEEP_MODE", CBaseParameter::RW, RP_GEN_SWEEP_MODE_LINEAR, 0, RP_GEN_SWEEP_MODE_LINEAR, RP_GEN_SWEEP_MODE_LOG, CONFIG_VAR);
CIntParameter outSweepDir[MAX_DAC_CHANNELS] =
    INIT2("SOUR", "_SWEEP_DIR", CBaseParameter::RW, RP_GEN_SWEEP_DIR_NORMAL, 0, RP_GEN_SWEEP_DIR_NORMAL, RP_GEN_SWEEP_DIR_UP_DOWN, CONFIG_VAR);
CIntParameter outSweepTime[MAX_DAC_CHANNELS] = INIT2("SOUR", "_SWEEP_TIME", CBaseParameter::RW, 1000000, 0, 1, 2000000000, CONFIG_VAR);  // In microseconds
CBooleanParameter outSweepState[MAX_DAC_CHANNELS] = INIT2("SOUR", "_SWEEP_STATE", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter outSweepReset("SWEEP_RESET", CBaseParameter::RW, false, 0);

CIntParameter outPhase[MAX_DAC_CHANNELS] = INIT2("SOUR", "_PHAS", CBaseParameter::RW, 0, 0, -360, 360, CONFIG_VAR);
CFloatParameter outDCYC[MAX_DAC_CHANNELS] = INIT2("SOUR", "_DCYC", CBaseParameter::RW, 50, 0, 0, 100, CONFIG_VAR);
CFloatParameter outRiseTime[MAX_DAC_CHANNELS] = INIT2("SOUR", "_RISE", CBaseParameter::RW, 1, 0, 0.1, 1000, CONFIG_VAR);
CFloatParameter outFallTime[MAX_DAC_CHANNELS] = INIT2("SOUR", "_FALL", CBaseParameter::RW, 1, 0, 0.1, 1000, CONFIG_VAR);

CStringParameter outWaveform[MAX_DAC_CHANNELS] = INIT2("SOUR", "_FUNC", CBaseParameter::RW, "0", 0, CONFIG_VAR);
CStringParameter outName[MAX_DAC_CHANNELS] = INIT2("OUT", "_CHANNEL_NAME_INPUT", CBaseParameter::RW, "", 0, CONFIG_VAR);
CIntParameter outTriggerSource[MAX_DAC_CHANNELS] =
    INIT2("SOUR", "_TRIG_SOUR", CBaseParameter::RW, RP_GEN_TRIG_SRC_INTERNAL, 0, RP_GEN_TRIG_SRC_INTERNAL, RP_GEN_TRIG_SRC_EXT_NE, CONFIG_VAR);

CIntParameter outTemperatureRuntime[MAX_DAC_CHANNELS] = INIT2("SOUR", "_TEMP_RUNTIME", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter outTemperatureLatched[MAX_DAC_CHANNELS] = INIT2("SOUR", "_TEMP_LATCHED", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter outImp[MAX_DAC_CHANNELS] = INIT2("SOUR", "_IMPEDANCE", CBaseParameter::RW, 0, 0, 0, 1, is_z_present ? CONFIG_VAR : 0);

CBooleanParameter outBurstState[MAX_DAC_CHANNELS] = INIT2("SOUR", "_BURST_STATE", CBaseParameter::RW, false, 0, CONFIG_VAR);
CIntParameter outBurstCount[MAX_DAC_CHANNELS] = INIT2("SOUR", "_BURST_COUNT", CBaseParameter::RW, 1, 0, 1, 50000, CONFIG_VAR);
CIntParameter outBurstRepetitions[MAX_DAC_CHANNELS] = INIT2("SOUR", "_BURST_REP", CBaseParameter::RW, 1, 0, 1, 50000, CONFIG_VAR);
CBooleanParameter outBurstRepInf[MAX_DAC_CHANNELS] = INIT2("SOUR", "_BURST_INF", CBaseParameter::RW, false, 0, CONFIG_VAR);
CIntParameter outBurstDelay[MAX_DAC_CHANNELS] = INIT2("SOUR", "_BURST_DELAY", CBaseParameter::RW, 1, 0, 1, 2000000000, CONFIG_VAR);
CBooleanParameter outGenSyncReset("SYNC_GEN", CBaseParameter::RW, false, 0);

CFloatParameter outExtTrigDeb("SOUR_DEB", CBaseParameter::RW, 500, 0, 0.008, 8338, CONFIG_VAR);

CBooleanParameter outImpZmode("SOUR_IMPEDANCE_Z_MODE", CBaseParameter::RO, is_z_present, 0);
CFloatParameter outAmplitudeMax("SOUR_VOLT_MAX", CBaseParameter::RO, LEVEL_AMPS_MAX, 0, LEVEL_AMPS_MAX, LEVEL_AMPS_MAX);

CIntParameter outGain[MAX_DAC_CHANNELS] = INIT2("OSC_CH", "_OUT_GAIN", CBaseParameter::RW, RP_GAIN_1X, 0, 0, 1, CONFIG_VAR);
CStringParameter outARBList = CStringParameter("ARB_LIST", CBaseParameter::RW, loadARBList(), 0);

static const uint8_t g_dac_channels = getDACChannels();

auto resumeSweepController(bool pause) -> void {
    if (rp_HPIsFastDAC_PresentOrDefault()) {
        g_sweepController->pause(pause);
    }
}

auto deleteSweepController() -> void {
    delete g_sweepController;
    g_sweepController = nullptr;
}

auto generateOutSignalForWeb(float tscale) -> void {
    /* ------ UPDATE OUT SIGNALS ------*/
    if (rp_HPIsFastDAC_PresentOrDefault()) {
        auto need_upate = g_tscale_last != tscale;
        for (int i = 0; i < g_dac_channels; i++) {
            if (outShow[i].Value() && outState[i].Value()) {
                if (outSignal[i].GetSize() != CH_SIGNAL_SIZE_DEFAULT) {
                    outSignal[i].Resize(CH_SIGNAL_SIZE_DEFAULT);
                }
                std::lock_guard<std::mutex> lock(g_updateOutWaveFormCh_mtx);
                if (g_updateOutWaveFormCh[i] || need_upate) {
                    generate((rp_channel_t)i, tscale);
                    g_updateOutWaveFormCh[i] = false;
                }
                outSignal[i].ForceSend();
            } else {
                outSignal[i].Resize(0);
            }
        }
    }
}

auto loadARBList() -> std::string {
    uint32_t c = 0;
    rp_ARBInit();
    std::string list;
    if (!rp_ARBGetCount(&c)) {
        for (uint32_t i = 0; i < c; i++) {
            std::string name;
            if (!rp_ARBGetName(i, &name)) {
                bool is_valid;
                if (!rp_ARBIsValid(name, &is_valid)) {
                    if (is_valid) {
                        uint32_t color;
                        rp_ARBGetColor(i, &color);
                        list += "A" + name + "\t" + std::to_string(color) + "\n";
                    }
                }
            }
        }
    }
    return list;
}

auto generate(rp_channel_t channel, float tscale) -> void {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return;

    CFloatBase64Signal* signal;
    std::string waveform;
    rp_waveform_t waveform_api = RP_WAVEFORM_SINE;
    rp_gen_sweep_mode_t sweep_mode;
    rp_gen_sweep_dir_t sweep_dir;
    rp_gen_mode_t gen_mode;
    float frequency, phase, amplitude, offset, showOff, duty_cycle, freqSweepStart, freqSweepEnd, riseTime, fallTime;
    int burstCount, burstPeriod, burstReps;
    // std::vector<float> data;

    signal = &outSignal[channel];
    waveform = outWaveform[channel].Value();
    frequency = outFrequancy[channel].Value();
    phase = (float)(outPhase[channel].Value() / 180.0f * M_PI);
    amplitude = outAmplitude[channel].Value() / outScale[channel].Value();
    offset = outOffset[channel].Value() / outScale[channel].Value();
    showOff = outShowOffset[channel].Value() / outScale[channel].Value();
    duty_cycle = outDCYC[channel].Value() / 100;
    freqSweepStart = outSweepStartFrequancy[channel].Value();
    freqSweepEnd = outSweepEndFrequancy[channel].Value();
    sweep_mode = (rp_gen_sweep_mode_t)outSweepMode[channel].Value();
    sweep_dir = (rp_gen_sweep_dir_t)outSweepDir[channel].Value();
    // data = getSignalFromFile("/tmp/gen_ch1.csv");
    gen_mode = outBurstState[channel].Value() ? RP_GEN_MODE_BURST : RP_GEN_MODE_CONTINUOUS;
    burstCount = outBurstCount[channel].Value();
    burstPeriod = outBurstDelay[channel].Value();
    burstReps = outBurstRepetitions[channel].Value();
    riseTime = outRiseTime[channel].Value();
    fallTime = outFallTime[channel].Value();

    // float tscale = atof(inTimeScale.Value().c_str());
    if (tscale == 0)
        return;

    if (waveform[0] == 'A') {
        auto sigSize = (*signal).GetSize();
        if (sigSize) {
            auto signame = waveform.erase(0, 1);
            float data[DAC_BUFFER_SIZE];
            uint32_t size;
            if (!rp_ARBGetSignalByName(signame, data, &size)) {
                synthesis_arb(signal, data, size, frequency, amplitude, offset, showOff, tscale);
            }
        }
    } else {
        try {
            waveform_api = (rp_waveform_t)std::stoi(waveform);
        } catch (const std::exception&) {
            waveform_api = RP_WAVEFORM_SINE;
        }
        switch (waveform_api) {
            case RP_WAVEFORM_SINE:
                if (gen_mode == RP_GEN_MODE_CONTINUOUS)
                    synthesis_sin(signal, frequency, phase, amplitude, offset, showOff, tscale);
                else
                    synthesis_sin_burst(signal, frequency, phase, amplitude, offset, showOff, burstCount, burstPeriod, burstReps, tscale);
                break;
            case RP_WAVEFORM_TRIANGLE:
                if (gen_mode == RP_GEN_MODE_CONTINUOUS)
                    synthesis_triangle(signal, frequency, phase, amplitude, offset, showOff, tscale);
                else
                    synthesis_triangle_burst(signal, frequency, phase, amplitude, offset, showOff, burstCount, burstPeriod, burstReps, tscale);
                break;
            case RP_WAVEFORM_SQUARE:
                if (gen_mode == RP_GEN_MODE_CONTINUOUS)
                    synthesis_square(signal, frequency, phase, amplitude, offset, showOff, tscale, riseTime, fallTime);
                else
                    synthesis_square_burst(signal, frequency, phase, amplitude, offset, showOff, burstCount, burstPeriod, burstReps, tscale, riseTime,
                                           fallTime);
                break;
            case RP_WAVEFORM_RAMP_UP:
                if (gen_mode == RP_GEN_MODE_CONTINUOUS)
                    synthesis_rampUp(signal, frequency, phase, amplitude, offset, showOff, tscale);
                else
                    synthesis_rampUp_burst(signal, frequency, phase, amplitude, offset, showOff, burstCount, burstPeriod, burstReps, tscale);
                break;
            case RP_WAVEFORM_RAMP_DOWN:
                if (gen_mode == RP_GEN_MODE_CONTINUOUS)
                    synthesis_rampDown(signal, frequency, phase, amplitude, offset, showOff, tscale);
                else
                    synthesis_rampDown_burst(signal, frequency, phase, amplitude, offset, showOff, burstCount, burstPeriod, burstReps, tscale);
                break;
            case RP_WAVEFORM_DC:
                if (gen_mode == RP_GEN_MODE_CONTINUOUS)
                    synthesis_DC(signal, frequency, phase, amplitude, offset, showOff);
                else
                    synthesis_DC_burst(signal, frequency, phase, amplitude, offset, showOff, burstCount, burstPeriod, burstReps, tscale);
                break;
            case RP_WAVEFORM_DC_NEG:
                if (gen_mode == RP_GEN_MODE_CONTINUOUS)
                    synthesis_DC_NEG(signal, frequency, phase, amplitude, offset, showOff);
                else
                    synthesis_DC_NEG_burst(signal, frequency, phase, amplitude, offset, showOff, burstCount, burstPeriod, burstReps, tscale);
                break;
            case RP_WAVEFORM_PWM:
                if (gen_mode == RP_GEN_MODE_CONTINUOUS)
                    synthesis_PWM(signal, frequency, phase, amplitude, offset, showOff, duty_cycle, tscale);
                else
                    synthesis_PWM_burst(signal, frequency, phase, amplitude, offset, showOff, duty_cycle, burstCount, burstPeriod, burstReps, tscale);
                break;
            case RP_WAVEFORM_SWEEP:
                synthesis_sweep(signal, frequency, freqSweepStart, freqSweepEnd, sweep_mode, sweep_dir, phase, amplitude, offset, showOff, tscale);
                break;
            case RP_WAVEFORM_NOISE:
                synthesis_noise(signal, amplitude, offset, showOff);
                break;
            default:
                break;
        }
    }
}

auto checkBurstDelayChanged(rp_channel_t ch) -> void {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return;

    uint32_t value = 0;
    rp_GenGetBurstPeriod(ch, &value);
    if (value != outBurstDelay[ch].Value()) {
        outBurstDelay[ch].SendValue(value);
    }
}

auto setBurstParameters(bool force) -> void {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return;

    for (int i = 0; i < g_dac_channels; i++) {
        auto ch = (rp_channel_t)i;

        // The order of initialization must be followed exactly.

        if (IS_NEW(outBurstRepetitions[ch]) || force) {
            if (!outBurstRepInf[ch].Value()) {
                if (rp_GenBurstRepetitions(ch, outBurstRepetitions[ch].NewValue()) == RP_OK) {
                    outBurstRepetitions[ch].Update();
                }
            } else {
                outBurstRepetitions[ch].Update();
            }
        }

        if (IS_NEW(outBurstRepInf[ch]) || force) {
            outBurstRepInf[ch].Update();
            if (outBurstRepInf[ch].Value()) {
                rp_GenBurstRepetitions(ch, 0x10000);
            } else {
                rp_GenBurstRepetitions(ch, outBurstRepetitions[ch].Value());
            }
        }

        if (IS_NEW(outBurstCount[ch]) || force) {
            if (rp_GenBurstCount(ch, outBurstCount[ch].NewValue()) == RP_OK) {
                outBurstCount[ch].Update();
                if (!force)
                    checkBurstDelayChanged(ch);
            }
        }

        if (IS_NEW(outBurstDelay[ch]) || force) {
            if (rp_GenBurstPeriod(ch, outBurstDelay[ch].NewValue()) == RP_OK) {
                outBurstDelay[ch].Update();
                if (!force)
                    checkBurstDelayChanged(ch);
            }
        }

        if (IS_NEW(outBurstState[ch]) || force) {
            rp_GenMode(ch, outBurstState[ch].NewValue() ? RP_GEN_MODE_BURST : RP_GEN_MODE_CONTINUOUS);
            outBurstState[ch].Update();
            if (!force) {
                if (outBurstState[ch].Value()) {
                    outSweepState[ch].SendValue(false);
                    g_sweepController->genSweep(ch, outSweepState[ch].Value());
                }
                checkBurstDelayChanged(ch);
            }
        }
    }

    if (IS_NEW(outGenSyncReset)) {
        if (outGenSyncReset.NewValue()) {
            rp_GenSynchronise();
        }
    }
}

auto setSweepParameters(bool force) -> void {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return;

    for (int i = 0; i < g_dac_channels; i++) {
        auto ch = (rp_channel_t)i;
        if ((IS_NEW(outSweepStartFrequancy[i])) || (IS_NEW(outSweepEndFrequancy[i])) || (IS_NEW(outSweepMode[i])) || (IS_NEW(outSweepDir[i])) ||
            (IS_NEW(outSweepTime[i])) || (IS_NEW(outSweepState[i])) || force) {

            outSweepStartFrequancy[i].Update();
            outSweepEndFrequancy[i].Update();
            outSweepMode[i].Update();
            outSweepDir[i].Update();
            outSweepTime[i].Update();
            outSweepState[i].Update();
            g_sweepController->setStartFreq(ch, outSweepStartFrequancy[ch].Value());
            g_sweepController->setStopFreq(ch, outSweepEndFrequancy[ch].Value());
            g_sweepController->setMode(ch, (rp_gen_sweep_mode_t)outSweepMode[ch].Value());
            g_sweepController->setDir(ch, (rp_gen_sweep_dir_t)outSweepDir[ch].Value());
            g_sweepController->setTime(ch, outSweepTime[ch].Value());
            g_sweepController->genSweep(ch, outSweepState[ch].Value());

            if (outSweepState[ch].Value()) {
                rp_GenMode(ch, RP_GEN_MODE_CONTINUOUS);
                outBurstState[ch].SendValue(false);
            }
        }
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

auto initGenAfterLoad() -> void {
    if (!rp_HPIsFastDAC_PresentOrDefault())
        return;

    for (int i = 0; i < g_dac_channels; i++) {
        auto ch = (rp_channel_t)i;

        if (rp_HPGetFastDACIsTempProtectionOrDefault()) {
            bool state = false;
            if (rp_GetEnableTempProtection(ch, &state) == RP_OK) {
                if (!state) {
                    rp_SetEnableTempProtection(ch, true);
                }
            }
        }
    }

    for (int i = 0; i < g_dac_channels; i++) {
        auto ch = (rp_channel_t)i;
        if (outName[ch].Value() == "") {
            outName[ch].Value() = std::string("OUT") + std::to_string(i + 1);
        }
    }
}

auto setNeedUpdateGenSignal() -> void {
    std::lock_guard<std::mutex> lock(g_updateOutWaveFormCh_mtx);
    for (int i = 0; i < g_dac_channels; i++) {
        g_updateOutWaveFormCh[i] = true;
    }
}

auto initGenBeforeLoadConfig() -> void {
    for (int i = 0; i < g_dac_channels; i++) {
        auto ch = (rp_channel_t)i;
        outName[ch].Value() = std::string("OUT") + std::to_string(i + 1);
    }

    if (getModelName() == "Z20") {
        for (int i = 0; i < g_dac_channels; i++) {
            auto ch = (rp_channel_t)i;
            outFrequancy[ch].SetMin(300000);
        }
    }
}

auto updateGeneratorParameters(bool force) -> void {
    auto requestSendScale = false;

    if (!rp_HPIsFastDAC_PresentOrDefault())
        return;

    for (int i = 0; i < g_dac_channels; i++) {
        auto ch = (rp_channel_t)i;

        if (IS_NEW(outState[i]) || IS_NEW(outAmplitude[i]) || IS_NEW(outOffset[i]) || IS_NEW(outPhase[i]) || IS_NEW(outDCYC[i]) || IS_NEW(outWaveform[i]) ||
            IS_NEW(outTriggerSource[i]) || IS_NEW(outRiseTime[i]) || IS_NEW(outFallTime[i]) || IS_NEW(outImp[i]) || force) {
            // updateOutCh1 = true;

            if (IS_NEW(outState[i]) || force) {
                if (outState[i].NewValue()) {

                    if (rp_HPGetFastDACIsTempProtectionOrDefault()) {
                        rp_SetLatchTempAlarm(ch, false);
                    }
                    if (!force) {
                        rp_GenResetChannelSM(ch);
                        usleep(1000);
                    }
                    rp_GenOutEnable(ch);
                    if (!force)
                        rp_GenTriggerOnly(ch);
                } else {
                    rp_GenOutDisable(ch);
                }
            }
            outState[i].Update();

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
                        rp_GenGetGainOut(ch, &curGenStatus);
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
                            outImp[i].Update();
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
                                outImp[i].Update();
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
                requestSendScale = true;
                g_updateOutWaveFormCh[ch] = true;
            }

            if (IS_NEW(outPhase[i]) || force) {
                rp_GenPhase(ch, outPhase[i].NewValue());
                outPhase[i].Update();
            }

            if (IS_NEW(outDCYC[i]) || force) {
                rp_GenDutyCycle(ch, outDCYC[i].NewValue() / 100);
                outDCYC[i].Update();
            }

            if (IS_NEW(outRiseTime[i]) || force) {
                rp_GenRiseTime(ch, outRiseTime[i].NewValue());
                outRiseTime[i].Update();
            }

            if (IS_NEW(outFallTime[i]) || force) {
                rp_GenFallTime(ch, outFallTime[i].NewValue());
                outFallTime[i].Update();
            }

            if (IS_NEW(outWaveform[i]) || force) {
                auto wf = outWaveform[i].NewValue();
                if (wf[0] == 'A') {
                    auto signame = wf.erase(0, 1);
                    float data[DAC_BUFFER_SIZE];
                    uint32_t size;
                    if (!rp_ARBGetSignalByName(signame, data, &size)) {
                        rp_GenArbWaveform(ch, data, size);
                        rp_GenWaveform(ch, RP_WAVEFORM_ARBITRARY);
                        outWaveform[i].Update();
                    } else {
                        outWaveform[i].Update();
                        rp_GenWaveform(ch, RP_WAVEFORM_SINE);
                        outWaveform[i].Value() = "0";
                    }

                } else {
                    try {
                        rp_waveform_t w = (rp_waveform_t)stoi(wf);
                        rp_GenWaveform(ch, w);
                        outWaveform[i].Update();
                    } catch (const std::exception&) {
                        rp_GenWaveform(ch, RP_WAVEFORM_SINE);
                        outWaveform[i].Update();
                        outWaveform[i].Value() = "0";
                    }
                }
            }

            if (IS_NEW(outTriggerSource[i]) || force) {
                rp_GenTriggerSource(ch, (rp_trig_src_t)outTriggerSource[i].NewValue());
                outTriggerSource[i].Update();
                if (!force)
                    rp_GenResetTrigger(ch);
            }
        }

        if (IS_NEW(outScale[i]) || force) {
            outScale[i].Update();
            std::lock_guard lock(g_updateOutWaveFormCh_mtx);
            g_updateOutWaveFormCh[i] = true;
        }

        if (IS_NEW(outFrequancy[i]) || force) {
            float period = 1000000.0 / outFrequancy[ch].NewValue();
            outRiseTime[ch].SetMin(period * RISE_FALL_MIN_RATIO);
            outRiseTime[ch].SetMax(period * RISE_FALL_MAX_RATIO);
            outRiseTime[ch].Update();
            outFallTime[ch].SetMin(period * RISE_FALL_MIN_RATIO);
            outFallTime[ch].SetMax(period * RISE_FALL_MAX_RATIO);
            outFallTime[ch].Update();
            rp_GenFreq(ch, outFrequancy[i].NewValue());
            outFrequancy[i].Update();
            if (!force)
                checkBurstDelayChanged(ch);
            rp_GenTriggerOnly(ch);
        }

        if (IS_NEW(outName[i]) || force) {
            if (outName[i].NewValue() == "") {
                auto str = outName[i].Value();
                outName[i].Update();
                outName[i].SendValue(str);
            } else {
                outName[i].Update();
            }
        }

        outShow[i].Update();
        outShowOffset[i].Update();

        if (requestSendScale) {
            outScale[i].SendValue(outScale[i].Value());
        }
    }

    if (IS_NEW(outExtTrigDeb) || force) {
        rp_GenSetExtTriggerDebouncerUs(outExtTrigDeb.NewValue());
        outExtTrigDeb.Update();
    }

    setBurstParameters(force);
    setSweepParameters(force);
}

auto updateGenTempProtection() -> void {
    if (rp_HPGetFastDACIsTempProtectionOrDefault()) {
        for (int i = 0; i < g_dac_channels; i++) {
            auto ch = (rp_channel_t)i;
            bool temperature_runtime = false;
            if (rp_GetRuntimeTempAlarm(ch, &temperature_runtime) == RP_OK) {
                if (outTemperatureRuntime[ch].Value() != temperature_runtime) {
                    outTemperatureRuntime[ch].SendValue(temperature_runtime);
                }
            }
            bool temperature_latched = false;
            if (rp_GetLatchTempAlarm(ch, &temperature_latched) == RP_OK) {
                if (outTemperatureLatched[ch].Value() != temperature_latched) {
                    outTemperatureLatched[ch].SendValue(temperature_latched);
                }
            }
        }
    }
}

auto sendFreqInSweepMode() -> void {
    if (rp_HPIsFastDAC_PresentOrDefault()) {
        for (int i = 0; i < g_dac_channels; i++) {
            float freq = 0;
            rp_GenGetFreq((rp_channel_t)i, &freq);
            if ((int)freq != outFrequancy[i].Value()) {
                outFrequancy[i].SendValue((int)freq);
                std::lock_guard lock(g_updateOutWaveFormCh_mtx);
                g_updateOutWaveFormCh[i] = true;
            }
        }
    }
}
