#include <unistd.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <locale>
#include <vector>

#include "common/profiler.h"
#include "common/version.h"
#include "math/rp_dsp.h"
#include "rp.h"
#include "rp_hw-profiles.h"
#include "rp_hw_calib.h"

#include "cli_parse_args.h"

#define NUM_SIGNAL_PERIODS 16

uint8_t getADCChannels() {
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK) {
        fprintf(stderr, "[Error] Can't get fast ADC channels count\n");
    }
    return c;
}

uint32_t getADCSpeed() {
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK) {
        fprintf(stderr, "[Error] Can't get fast ADC speed\n");
    }
    return c;
}

#define MAX_CHANNELS getADCChannels()
#define ADC_SAMPLE_RATE getADCSpeed()

namespace {

static rp_dsp_api::CDSP g_dsp(MAX_CHANNELS, ADC_BUFFER_SIZE, ADC_SAMPLE_RATE);
static sig_atomic_t g_quit_requested = 0;

static void quit_handler(int) {
    g_quit_requested = 1;
}

static void spectrum_worker(cli_args_t args) {

    int decimation = ADC_SAMPLE_RATE / (args.freq_max * 2 * NUM_SIGNAL_PERIODS);

    if (decimation < 16) {
        if (decimation >= 8)
            decimation = 8;
        else if (decimation >= 4)
            decimation = 4;
        else if (decimation >= 2)
            decimation = 2;
        else
            decimation = 1;
    }
    auto data = g_dsp.createData();

    rp_dsp_api::cdsp_data_ch_t max_signals;
    rp_dsp_api::cdsp_data_ch_t min_signals;
    rp_dsp_api::cdsp_data_ch_t measure_signals;
    max_signals.resize(data->m_converted.m_channels);
    min_signals.resize(data->m_converted.m_channels);
    measure_signals.resize(data->m_converted.m_channels);
    auto peak_pw_max = new rp_dsp_api::cdsp_data_t[MAX_CHANNELS];
    auto peak_pw_freq_max = new rp_dsp_api::cdsp_data_t[MAX_CHANNELS];

    auto peak_set = false;

    for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
        peak_pw_max[ch] = std::numeric_limits<rp_dsp_api::cdsp_data_t>::lowest();
        peak_pw_freq_max[ch] = std::numeric_limits<rp_dsp_api::cdsp_data_t>::lowest();
        max_signals[ch].resize(g_dsp.getOutSignalMaxLength());
        min_signals[ch].resize(g_dsp.getOutSignalMaxLength());
        measure_signals[ch].resize(g_dsp.getOutSignalMaxLength());
    }

    int count = args.count;

    if (args.csv) {
        count = 1;
        std::cout << "Frequency (Hz)";
        for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
            std::cout << ", ch" << ch << " (dB)";
        }
        std::cout << "\n";
    } else if (args.csv_limit) {
        std::cout << "Frequency (Hz)";
        for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
            std::cout << ", ch" << ch << " min (dB)";
            std::cout << ", ch" << ch << " max (dB)";
        }
        std::cout << "\n";
    }
    rp_AcqReset();
    rp_AcqSetBypassFilter(RP_CH_1, true);
    rp_AcqSetBypassFilter(RP_CH_2, true);

    bool is_infinity = count < 0;
    uint32_t buffer_size = ADC_BUFFER_SIZE;
    while (!g_quit_requested && ((count > 0) || is_infinity)) {
        int avg_count = args.average_for_10 ? 10 : 1;
        while (avg_count) {
            rp_AcqSetDecimationFactor(decimation);
            rp_AcqSetTriggerDelay(buffer_size - ADC_BUFFER_SIZE / 2.0);
            rp_AcqStart();
            usleep(10);
            rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);

            rp_acq_trig_state_t stateTrig = RP_TRIG_STATE_WAITING;
            /* polling until data is ready */
            while (1) {
                if (rp_AcqGetTriggerState(&stateTrig))
                    exit(1);
                if (stateTrig == RP_TRIG_STATE_TRIGGERED)
                    break;
            }

            bool fillState = false;
            while (!fillState) {
                if (rp_AcqGetBufferFillState(&fillState))
                    exit(1);
            }
            rp_AcqStop();

            // Retrieve data and process it
            uint32_t trig_pos;
            rp_AcqGetWritePointerAtTrig(&trig_pos);
            buffers_t buff;
            buff.size = buffer_size;
            buff.use_calib_for_volts = true;
            for (int i = 0; i < MAX_CHANNELS; i++) {
                buff.ch_f[i] = data->m_in[i].data();
                buff.ch_i[i] = NULL;
                buff.ch_d[i] = NULL;
            }

            rp_AcqGetData(trig_pos, &buff);

            // float min = 1000;
            // float max = -1000;
            // for (int i = 0; i < buff.size; i++) {
            //     auto z = data->m_in[0].data()[i];
            //     if (min > z)
            //         min = z;
            //     if (max < z)
            //         max = z;
            // }
            // WARNING("min %f max %f", min, max)

            g_dsp.windowFilter(data);

            if (g_dsp.fft(data)) {
                fprintf(stderr, "Error in g_dsp.fft\n");
                return;
            }
            g_dsp.prepareFreqVector(data, decimation);
            g_dsp.decimate(data, g_dsp.getOutSignalMaxLength(), g_dsp.getOutSignalMaxLength());
            g_dsp.cnvToMetric(data, decimation, args.freq_min, args.freq_max);
            // Summary peak calculation
            if (peak_set) {
                if (args.csv_limit) {
                    for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
                        for (uint32_t i = 0; i < g_dsp.getOutSignalMaxLength(); ++i) {
                            min_signals[ch][i] = std::min(min_signals[ch][i], data->m_converted.m_result[rp_dsp_api::DBM][ch][i]);
                            max_signals[ch][i] = std::max(max_signals[ch][i], data->m_converted.m_result[rp_dsp_api::DBM][ch][i]);
                        }
                    }
                }
            } else {
                if (args.csv_limit) {
                    for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
                        for (uint32_t i = 0; i < g_dsp.getOutSignalMaxLength(); ++i) {
                            min_signals[ch][i] = data->m_converted.m_result[rp_dsp_api::DBM][ch][i];
                            max_signals[ch][i] = data->m_converted.m_result[rp_dsp_api::DBM][ch][i];
                        }
                    }
                }
                peak_set = true;
            }

            for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
                for (uint32_t i = 0; i < g_dsp.getOutSignalMaxLength(); ++i) {
                    measure_signals[ch][i] += data->m_converted.m_result[rp_dsp_api::DBM][ch][i];
                }
            }

            for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
                if (data->m_converted.m_peak_power[rp_dsp_api::DBM][ch] >= peak_pw_max[ch]) {
                    peak_pw_max[ch] = data->m_converted.m_peak_power[rp_dsp_api::DBM][ch];
                    peak_pw_freq_max[ch] = data->m_converted.m_peak_freq[rp_dsp_api::DBM][ch];
                }
            }
            avg_count--;
        }

        if (!args.csv && !args.csv_limit) {
            if (args.all_values) {
                std::cout << std::fixed << std::setprecision(6);
                for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
                    std::cout << "ch" << ch << " peak summary: " << data->m_converted.m_peak_freq[rp_dsp_api::DBM][ch] << " Hz, "
                              << data->m_converted.m_peak_power[rp_dsp_api::DBM][ch] << " dBm\n";
                    std::cout << "ch" << ch << " peak summary: " << data->m_converted.m_peak_freq[rp_dsp_api::VOLT][ch] << " Hz, "
                              << data->m_converted.m_peak_power[rp_dsp_api::VOLT][ch] << " V\n";
                    std::cout << "ch" << ch << " peak summary: " << data->m_converted.m_peak_freq[rp_dsp_api::DBU][ch] << " Hz, "
                              << data->m_converted.m_peak_power[rp_dsp_api::DBU][ch] << " dBu\n";
                    std::cout << "ch" << ch << " peak summary: " << data->m_converted.m_peak_freq[rp_dsp_api::DBV][ch] << " Hz, "
                              << data->m_converted.m_peak_power[rp_dsp_api::DBV][ch] << " dBV\n";
                    std::cout << "ch" << ch << " peak summary: " << data->m_converted.m_peak_freq[rp_dsp_api::DBuV][ch] << " Hz, "
                              << data->m_converted.m_peak_power[rp_dsp_api::DBuV][ch] << " dBuV\n";
                    std::cout << "ch" << ch << " peak summary: " << data->m_converted.m_peak_freq[rp_dsp_api::MW][ch] << " Hz, "
                              << data->m_converted.m_peak_power[rp_dsp_api::MW][ch] << " mW\n";
                    std::cout << "ch" << ch << " peak summary: " << data->m_converted.m_peak_freq[rp_dsp_api::DBW][ch] << " Hz, "
                              << data->m_converted.m_peak_power[rp_dsp_api::DBW][ch] << " dBW\n";
                }
                std::cout << std::fixed << std::setprecision(2);
            } else {
                std::cout << std::fixed << std::setprecision(3);
                for (uint32_t i = 0; i < MAX_CHANNELS; i++) {
                    std::cout << "ch" << i << " peak: " << data->m_converted.m_peak_freq[rp_dsp_api::DBM][i] << " Hz, " << data->m_converted.m_peak_power[rp_dsp_api::DBM][i]
                              << " dB\n";
                }
                std::cout << std::fixed << std::setprecision(2);
            }
        }

        if (!is_infinity) {
            --count;
        }

        usleep(10000);
    }

    for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
        for (uint32_t i = 0; i < g_dsp.getOutSignalMaxLength(); ++i) {
            measure_signals[ch][i] /= args.average_for_10 ? 10 : 1;
        }
    }

    if (peak_set) {

        if (args.csv) {
            for (size_t i = 0; i < g_dsp.getOutSignalMaxLength(); ++i) {
                if (data->m_converted.m_freq_vector[i] >= args.freq_min && data->m_converted.m_freq_vector[i] <= args.freq_max) {
                    std::cout << data->m_converted.m_freq_vector[i];
                    for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
                        std::cout << ", " << measure_signals[ch][i];
                    }
                    std::cout << "\n";
                }
            }
        } else if (args.csv_limit) {
            for (size_t i = 0; i < g_dsp.getOutSignalMaxLength(); ++i) {
                if (data->m_converted.m_freq_vector[i] >= args.freq_min && data->m_converted.m_freq_vector[i] <= args.freq_max) {
                    std::cout << data->m_converted.m_freq_vector[i];
                    for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
                        std::cout << ", " << min_signals[ch][i] << ", " << min_signals[ch][i];
                    }
                    std::cout << "\n";
                }
            }
        } else {
            if (!args.all_values) {
                for (uint32_t ch = 0; ch < MAX_CHANNELS; ch++) {
                    std::cout << "ch" << ch << " peak summary: " << peak_pw_freq_max[ch] << " Hz, " << peak_pw_max[ch] << " dB\n";
                }
            }
        }
    }

    std::cout << std::flush;
    delete data;
}

}  // namespace

int main(int argc, char* argv[]) {
    std::cout.imbue(std::locale::classic());

    // Use fixed float values
    std::cout << std::fixed << std::setprecision(2);
    cli_args_t args;
    if (!cli_parse_args(argc, argv, args)) {
        fprintf(stderr, "%s Version: %s-%s\n", argv[0], VERSION_STR, REVISION_STR);
        fprintf(stderr, cli_help_string().c_str(), F_MAX);
        return 1;
    }

    if (args.help) {
        // std::cout has used only for the measurement results
        fprintf(stderr, "%s Version: %s-%s\n", argv[0], VERSION_STR, REVISION_STR);
        fprintf(stderr, cli_help_string().c_str(), F_MAX);
        return 0;
    }

    // Init
    int error_code = RP_OK;

    error_code = rp_InitReset(!args.test);

    if (error_code != RP_OK) {
        std::cerr << "Error: rp_Init, code: " << error_code << std::endl;
        return 1;
    }

    error_code = g_dsp.window_init(args.wm);

    if (error_code != 0) {
        std::cerr << "Error: rp_spectr_init, code: " << error_code << std::endl;
        rp_Release();
        return 1;
    }

    error_code = g_dsp.fftInit();

    if (error_code != 0) {
        std::cerr << "Error: rp_spectr_fft_init, code: " << error_code << std::endl;
        rp_Release();
        return 1;
    }

    // Handle interrupt signals
    signal(SIGINT, quit_handler);
    signal(SIGTERM, quit_handler);

    spectrum_worker(args);

    rp_Release();

    return 0;
}
