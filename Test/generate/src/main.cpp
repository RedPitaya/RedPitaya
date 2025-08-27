#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generate.h"

// /** Maximal signal frequency [Hz] */
uint32_t g_max_frequency = getMaxSpeed() / 2;

// /** Minimal signal frequency [Hz] */
uint32_t g_min_frequency = 0;

// /** Maximal signal amplitude [Vpp] */
double g_max_amplitude = fullScale() * 2;

bool g_isGain = getIsGainX5();
bool g_isLoadMode = getIsLoad50Ohm();

/** Program name */
const char* g_argv0 = NULL;

std::vector<std::string> split(const std::string& s, char seperator) {
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;
    while ((pos = s.find(seperator, pos)) != std::string::npos) {
        std::string substring(s.substr(prev_pos, pos - prev_pos));
        output.push_back(substring);
        prev_pos = ++pos;
    }
    output.push_back(s.substr(prev_pos, pos - prev_pos));
    return output;
}

void usage() {
    auto list = getARBList();
    std::string s = "";
    for (auto& itm : list) {
        s += ", ";
        s += itm;
    }

    auto arr = split(std::string(g_argv0), '/');

    std::string name = "";
    if (arr.size() > 0)
        name = arr[arr.size() - 1];

    fprintf(stderr, "%s version %s-%s\n\n", name.c_str(), VERSION_STR, REVISION_STR);

    std::string commands = "Usage: %s channel amplitude frequency[,end_frequency]";
    if (g_isGain) {
        commands += " [gain]";
    }

    commands += " [type]";

    if (g_isLoadMode) {
        commands += " [load]";
    }
    commands += " [-c] [-d]\n\n";
    fprintf(stderr, commands.c_str(), name.c_str());
    fprintf(stderr, "\tchannel         Channel to generate signal on [1, 2].\n");
    fprintf(stderr, "\tamplitude       Peak-to-peak signal amplitude in Vpp [0.0 - %1.1f].\n", g_max_amplitude);
    fprintf(stderr, "\tfrequency       Signal frequency in Hz [%d - %d].\n", g_min_frequency, g_max_frequency);
    fprintf(stderr, "\tend_frequency   Sweep-to frequency in Hz [%d - %d].\n", g_min_frequency, g_max_frequency);
    if (g_isGain) {
        fprintf(stderr, "\tgain            Gain output value [x1, x5] (default value x1).\n");
    }
    fprintf(stderr, "\ttype            Signal type [sine, sqr, tri, sweep, noise, ramp_up, ramp_down, dc, dc_neg%s] (default value sine).\n", s.c_str());
    if (g_isLoadMode) {
        fprintf(stderr, "\tload            Load mode [HiZ, 50Ohm] (default value HiZ).\n");
    }
    fprintf(stderr, "\t-c              Disable calibration. By default calibration enabled.\n");
    fprintf(stderr, "\t-d              Debug FPGA registers.\n\n");
    fprintf(stderr, "Setting the frequency to 0 will disable the generator completely.\n\n");
}

/** Signal generator main */
int main(int argc, char* argv[]) {
    loadARBList();

    auto channels = getChannels();
    if (channels == 0) {
        fprintf(stderr, "There are no FAST DAC outputs on this board.\n");
        return 0;
    }

    g_argv0 = argv[0];
    config_t config;

    if (argc < 4) {
        usage();
        return -1;
    }

    /* Channel argument parsing */
    config.ch = atoi(argv[1]) - 1; /* Zero based internally */
    if (config.ch > 1) {
        fprintf(stderr, "Invalid channel: %s\n", argv[1]);
        usage();
        return -1;
    }

    /* Signal amplitude argument parsing */
    config.amp = strtod(argv[2], NULL);
    if ((config.amp < 0.0) || (config.amp > g_max_amplitude)) {
        fprintf(stderr, "Invalid amplitude: %s\n", argv[2]);
        usage();
        return -1;
    }

    /* Signal frequency argument parsing */

    std::string freqParam = argv[3];
    auto freqs = split(freqParam, ',');
    auto list = getARBList();
    config.freq = freqs.size() > 0 ? strtod(freqs[0].c_str(), NULL) : 0;
    config.end_freq = freqs.size() > 1 ? strtod(freqs[1].c_str(), NULL) : 0;

    for (int i = 1; i < argc; i++) {
        std::string param = argv[i];

        if (param == "-c") {
            config.calib = false;
            continue;
        }

        if (param == "-d") {
            config.regDebug = true;
            continue;
        }

        if (param == "50Ohm" && g_isLoadMode) {
            config.load = RP_GEN_50Ohm;
            continue;
        }

        if (param == "HiZ" && g_isLoadMode) {
            config.load = RP_GEN_HI_Z;
            continue;
        }

        if (param == "x1" && g_isGain) {
            config.gain = RP_GAIN_1X;
            continue;
        }

        if (param == "x5" && g_isGain) {
            config.gain = RP_GAIN_5X;
            continue;
        }

        if (param == "sine") {
            config.type = RP_WAVEFORM_SINE;
            continue;
        }

        if (param == "sqr") {
            config.type = RP_WAVEFORM_SQUARE;
            continue;
        }

        if (param == "tri") {
            config.type = RP_WAVEFORM_TRIANGLE;
            continue;
        }

        if (param == "ramp_up") {
            config.type = RP_WAVEFORM_RAMP_UP;
            continue;
        }

        if (param == "ramp_down") {
            config.type = RP_WAVEFORM_RAMP_DOWN;
            continue;
        }

        if (param == "dc") {
            config.type = RP_WAVEFORM_DC;
            continue;
        }

        if (param == "dc_neg") {
            config.type = RP_WAVEFORM_DC_NEG;
            continue;
        }

        if (param == "noise") {
            config.type = RP_WAVEFORM_NOISE;
            continue;
        }

        if (param == "sweep") {
            config.type = RP_WAVEFORM_SWEEP;
            continue;
        }

        for (auto& itm : list) {
            if (param == itm) {
                config.arb = itm;
            }
        }
    }

    /* Check frequency limits */
    if ((config.freq < g_min_frequency) || (config.freq > g_max_frequency)) {
        fprintf(stderr, "Invalid frequency: %s\n", argv[3]);
        usage();
        return -1;
    }

    if ((config.end_freq < g_min_frequency) || (config.end_freq > g_max_frequency)) {
        fprintf(stderr, "Invalid end frequency: %s\n", argv[3]);
        usage();
        return -1;
    }

    return gen(config);
}