#include "help.h"

#include <cstring>
#include <map>
#include <sstream>
#include <vector>

#include "settings_lib/stream_settings.h"

#define RPSA_STR2(x) #x
#define RPSA_STR(x) RPSA_STR2(x)

namespace {

// %P in the texts is the program name
auto put(FILE* out, const std::string& prog, const char* text) -> void {
    std::string s(text);
    for (size_t at = s.find("%P"); at != std::string::npos; at = s.find("%P", at + prog.size()))
        s.replace(at, 2, prog);
    fputs(s.c_str(), out);
}

const char* kOverview =
    "Red Pitaya streaming client: finds the boards, sets them up and streams their ADC to files,\n"
    "a file to their DAC.\n"
    "\n"
    "Usage: %P MODE [OPTIONS]\n"
    "\n"
    "Modes:\n"
    "  -d, --detect          find the boards on the network                    (--help detect)\n"
    "  -c, --config          read and write the settings of the boards         (--help config)\n"
    "  -r, --remote          start and stop the streams of the boards          (--help remote)\n"
    "  -s, --streaming       stream the ADC of the boards into files here      (--help stream)\n"
    "  -o, --out_streaming   play a file here on the DAC of the boards         (--help dac)\n"
    "  -G, --gpio            record the GPIO input here, play a file on the GPIO output (--help gpio)\n"
    "\n"
    "Common options:\n"
    "  -h, --hosts=IP[,IP]   the boards; without it the first board that answers the broadcast\n"
    "  -v, --verbose         report every step\n"
    "  -q, --quiet           only the errors and the result (no progress line)\n"
    "      --reply-timeout=T how long an answer of a board is waited for (default 10s)\n"
    "\n"
    "Times take a unit: 500ms, 5s, 2m; a bare number is in the unit the option names.\n"
    "Long options take - or _: --out-streaming, --tdms-time, --config-file. Values take any case.\n"
    "\n"
    "More help:\n"
    "  %P --help MODE        a mode: detect, config, remote, stream, dac, gpio\n"
    "  %P --help keys [TEXT] the settings of the boards (-c -i NAME=VALUE), those with TEXT only\n"
    "  %P --help formats     the file formats\n"
    "  %P --help examples    typical commands\n"
    "  %P --version          the version of the client\n"
    "\n"
    "Exit codes: 0 done, 1 wrong command line, 2 no board found or connected,\n"
    "            3 the board refused or the stream failed.\n";

const char* kDetect =
    "Detect: find the boards on the network (they announce themselves on UDP port 18902).\n"
    "\n"
    "Usage: %P -d [-t SEC]\n"
    "\n"
    "  -d, --detect          the mode\n"
    "  -t, --timeout=SEC     how long to listen, seconds or 500ms, 5s (default 5)\n"
    "\n"
    "Prints every board found: address, mode (master/slave) and model.\n";

const char* kConfig =
    "Config: read and write the settings of the boards.\n"
    "\n"
    "Usage: %P -c [-h IPs] -g V|V1|VV|F [-v]\n"
    "       %P -c [-h IPs] -s M|F [-f FILE] [-v]\n"
    "       %P -c [-h IPs] -i NAME [-w] [-v]\n"
    "       %P -c [-h IPs] -i NAME=VALUE [-w] [-v]\n"
    "\n"
    "  -c, --config          the mode\n"
    "  -h, --hosts=IP[,IP]   the boards (default: the first board found)\n"
    "  -g, --get=WHAT        read all the settings:\n"
    "                          V   JSON on the screen\n"
    "                          V1  JSON on the screen, the values only\n"
    "                          VV  a table on the screen with the meaning of the values\n"
    "                          F   into configs/config_<IP>.json here\n"
    "  -s, --set=WHERE       write all the settings of FILE to the boards:\n"
    "                          M   for now (the board forgets them on a restart)\n"
    "                          F   and save them in the config file of the board\n"
    "  -f, --config_file=FILE the settings to write (default config.json)\n"
    "  -i NAME               read one setting\n"
    "  -i NAME=VALUE         write one setting; the board refuses a wrong value\n"
    "  -w, --write           with -i NAME: also into configs/config_<IP>.json here;\n"
    "                        with -i NAME=VALUE: also into the config file of the board\n"
    "  -v, --verbose         report every step\n"
    "\n"
    "The settings and their values: %P --help keys\n";

const char* kRemote =
    "Remote: start and stop the streams of the boards; the data goes to another client or, with\n"
    "adc_pass_mode=FILE / dac_pass_mode=DAC_FILE, to and from files on the board.\n"
    "\n"
    "Usage: %P -r [-h IPs] -m COMMAND [-t MSEC] [-v]\n"
    "\n"
    "  -r, --remote          the mode\n"
    "  -h, --hosts=IP[,IP]   the boards (default: the first board found)\n"
    "  -m, --mode=COMMAND    start            start the ADC stream\n"
    "                        stop             stop the ADC stream\n"
    "                        start_stop       start the ADC stream, stop it after --timeout\n"
    "                        start_dac        start the DAC stream\n"
    "                        stop_dac         stop the DAC stream\n"
    "                        start_stop_dac   start the DAC stream, stop it after --timeout\n"
    "                        start_gpio_in    start the GPIO input  (stop_gpio_in to stop it)\n"
    "                        start_gpio_out   start the GPIO output (stop_gpio_out to stop it)\n"
    "                          with gpio_in_pass_mode / gpio_out_pass_mode=FILE the board writes\n"
    "                          or plays a file of its own (%P --help gpio)\n"
    "                        watch            follow the recordings of the boards on their SD cards:\n"
    "                          a panel (a line a board every 5 s without a terminal) until they end;\n"
    "                          leaving does not stop them. start_stop shows it while it waits\n"
    "                        status           the recordings once, key=value lines for scripts\n"
    "  -t, --timeout=MSEC    for start_stop and start_stop_dac, milliseconds or 5s, 2m (default 1000)\n"
    "  -v, --verbose         report every step\n";

const char* kStream =
    "Stream: the ADC of the boards over the network into files here, one file per board.\n"
    "The channels, decimation and resolution are the board's settings (%P -c -i ...).\n"
    "\n"
    "Usage: %P -s [-h IPs] -f tdms|wav|csv|bin [-d DIR] [-l SAMPLES] [-m raw|volt] [-t MSEC]\n"
    "          [--tdms_time] [-v]\n"
    "\n"
    "  -s, --streaming       the mode\n"
    "  -h, --hosts=IP[,IP]   the boards (default: the first board found)\n"
    "  -f, --format=FORMAT   tdms, wav, csv or bin (%P --help formats)\n"
    "  -d, --dir=DIR         where the files go (default: here)\n"
    "  -l, --limit=SAMPLES   stop after this many samples per channel, 1-18446744073709551615 (default: no limit)\n"
    "  -m, --mode=MODE       raw   the ADC codes, 8 or 16 bit (default)\n"
    "                        volt  volts as floating point (wav: limited to -1..1)\n"
    "  -t, --timeout=MSEC    stop after this time, milliseconds or 10s, 2m (default: until Ctrl+C)\n"
    "      --tdms_time       tdms: a column with the time of every sample (8 bytes each);\n"
    "                        without it the time is in the channel properties and the group Buffers\n"
    "  -v, --verbose         report every step and the statistics at the end\n"
    "\n"
    "On a terminal a progress line shows the samples, the data, the rate and the losses; at the end\n"
    "a line per board: \"Done: IP: N samples per channel, ... file\". -v prints the statistics table.\n"
    "\n"
    "The board must stream to the network (adc_pass_mode=NET): a board writing to its own SD card\n"
    "(adc_pass_mode=FILE) is stopped again and reported. A board that does not answer or connect\n"
    "within --reply-timeout ends the run with exit code 3 (no board: 2).\n"
    "Next to every file a report: <file>.log.txt, and a line per buffer in <file>.log.lost.txt.\n";

const char* kDac =
    "DAC: play a file here on the DAC of the boards over the network.\n"
    "\n"
    "Usage: %P -o [-h IPs] -f tdms|wav|bin -d FILE [-r inf|COUNT] [-v]\n"
    "\n"
    "  -o, --out_streaming   the mode\n"
    "  -h, --hosts=IP[,IP]   the boards (default: the first board found)\n"
    "  -f, --format=FORMAT   tdms, wav or bin (%P --help formats)\n"
    "  -d, --data=FILE       the file to play\n"
    "  -r, --repeat=N        how many times: 1-1000000 or inf (default 1)\n"
    "  -v, --verbose         report every step\n"
    "\n"
    "The rate is the board's dac_rate; channels 1 and 2 of the file go to OUT1 and OUT2.\n"
    "The board must take the data from the network (dac_pass_mode=DAC_NET); one that plays its own\n"
    "file (DAC_FILE) is stopped again and reported.\n"
    "The data of a channel should be a multiple of 128 bytes, else the end of the file clicks.\n";

const char* kGpio =
    "GPIO: the DIO0..7_P and DIO0..7_N lines of one board. The input goes into a file here, a file\n"
    "here goes to the output, or both at once (one connection: the board serves one client).\n"
    "\n"
    "Usage: %P -G [-h IP] --in FILE [-f bin|csv|vcd] [-l SAMPLES] [-t MSEC] [-v]\n"
    "       %P -G [-h IP] --out FILE.bin [-r inf|COUNT] [--gaps] [-v]\n"
    "       %P -G [-h IP] --in FILE --out FILE.bin ...\n"
    "\n"
    "  -G, --gpio            the mode\n"
    "  -h, --hosts=IP        the board (default: the first board found)\n"
    "      --in=FILE         record the input: bin (the BIN stream file), csv (a row where the lines\n"
    "                        change) or vcd (a value change dump for a waveform viewer)\n"
    "  -f, --format=FORMAT   the format of --in, else its extension\n"
    "  -l, --limit=SAMPLES   stop the input after this many samples (taken at a buffer)\n"
    "  -t, --timeout=MSEC    stop after this time, milliseconds or 10s, 2m (default: until Ctrl+C)\n"
    "      --out=FILE        play a BIN stream file: a GPIO recording or GPIO output words\n"
    "  -r, --repeat=N        how many times: 1-1000000 or inf (default 1)\n"
    "      --gaps            play the samples a recording lost or waited for a trigger (the lines\n"
    "                        hold their level), so that it plays in its own time\n"
    "  -v, --verbose         report every step\n"
    "\n"
    "The lines, the decimation, RLE and the triggers are the board's settings (%P --help keys gpio):\n"
    "gpio_outputs_p / gpio_outputs_n choose the lines the output drives, gpio_rle packs runs of a\n"
    "level into one word. The board must stream to the network (gpio_in_pass_mode and\n"
    "gpio_out_pass_mode = NET); with FILE it writes or plays files in streaming_files/gpio itself,\n"
    "started with %P -r -m start_gpio_in / start_gpio_out.\n"
    "After the last word of --out the board plays what it got and stops; the end line tells the\n"
    "clocks the output waited for data (the network was too slow).\n";

const char* kFormats =
    "File formats\n"
    "\n"
    "  format  ADC files  DAC plays  what it is\n"
    "  tdms    yes        yes        NI TDMS: a group per file, a channel per input, the time in the\n"
    "                                channel properties (or a time column, --tdms_time)\n"
    "  wav     yes        yes        WAV: a channel per input, 8 or 16 bit, or float with -m volt\n"
    "  csv     yes        no         text, a row per sample, a column per channel\n"
    "  bin     yes        yes        the Red Pitaya stream file: blocks as the board sent them, with\n"
    "                                the samples lost and waited for a trigger before each block;\n"
    "                                the DAC plays channels 1 and 2 of it\n"
    "\n"
    "  GPIO: --in writes bin, csv or vcd; --out plays bin (a recording of the input or output words).\n"
    "\n"
    "convert_tool turns a bin file into tdms, wav or csv (and a GPIO recording into csv or vcd).\n"
    "The board itself writes the same formats with adc_pass_mode=FILE (format_sd).\n";

const char* kExamples =
    "Examples\n"
    "\n"
    "  Find the boards:\n"
    "    %P -d\n"
    "  Two channels at decimation 64, 10 s into WAV files in ./data:\n"
    "    %P -c -h 192.168.1.10 -i channel_state_1=ON\n"
    "    %P -c -h 192.168.1.10 -i channel_state_2=ON\n"
    "    %P -c -h 192.168.1.10 -i adc_decimation=64\n"
    "    %P -s -h 192.168.1.10 -f wav -d data -t 10s\n"
    "  One million samples per channel of two boards as TDMS with the time column:\n"
    "    %P -s -h 192.168.1.10,192.168.1.11 -f tdms -l 1000000 --tdms_time\n"
    "  Save the settings of a board here, change them, write them back for good:\n"
    "    %P -c -h 192.168.1.10 -g F\n"
    "    %P -c -h 192.168.1.10 -s F -f configs/config_192.168.1.10.json\n"
    "  Play a WAV file on the DAC until Ctrl+C:\n"
    "    %P -o -h 192.168.1.10 -f wav -d signal.wav -r inf\n"
    "  Let the board write the ADC to its own SD card for 5 s:\n"
    "    %P -c -h 192.168.1.10 -i adc_pass_mode=FILE\n"
    "    %P -r -h 192.168.1.10 -m start_stop -t 5s\n"
    "  Record 10 s of the GPIO input as a VCD for a waveform viewer:\n"
    "    %P -G -h 192.168.1.10 --in lines.vcd -t 10s\n"
    "  Play a pattern on the GPIO output and record the pads at once:\n"
    "    %P -c -h 192.168.1.10 -i gpio_outputs_p=255\n"
    "    %P -G -h 192.168.1.10 --out pattern.bin --in pads.bin\n"
    "  In a script: stop on the first error\n"
    "    %P -q -s -h 192.168.1.10 -f bin -l 1000000 || echo \"failed with $?\"\n";

struct Section {
    const char* text;
    const char* usage;  // the usage lines, after an error
};

const std::map<std::string, Section>& sections() {
    static const std::map<std::string, Section> s = {
        {"detect", {kDetect, "Usage: %P -d [-t SEC]\n"}},
        {"config", {kConfig, "Usage: %P -c [-h IPs] -g V|V1|VV|F | -s M|F [-f FILE] | -i NAME[=VALUE] [-w] [-v]\n"}},
        {"remote", {kRemote, "Usage: %P -r [-h IPs] -m start|stop|start_stop|start_dac|stop_dac|start_stop_dac|watch|status [-t MSEC] [-v]\n"}},
        {"stream", {kStream, "Usage: %P -s [-h IPs] -f tdms|wav|csv|bin [-d DIR] [-l SAMPLES] [-m raw|volt] [-t MSEC] [--tdms_time] [-v]\n"}},
        {"dac", {kDac, "Usage: %P -o [-h IPs] -f tdms|wav|bin -d FILE [-r inf|COUNT] [-v]\n"}},
        {"gpio", {kGpio, "Usage: %P -G [-h IP] [--in FILE [-f bin|csv|vcd] [-l SAMPLES]] [--out FILE.bin [-r inf|COUNT] [--gaps]] [-t MSEC] [-v]\n"}},
        {"formats", {kFormats, ""}},
        {"examples", {kExamples, ""}},
    };
    return s;
}

// other names of the sections
auto canonical(const std::string& topic) -> std::string {
    static const std::map<std::string, std::string> alias = {
        {"d", "detect"},        {"-d", "detect"},           {"--detect", "detect"},     {"search", "detect"},
        {"c", "config"},        {"-c", "config"},           {"--config", "config"},     {"r", "remote"},
        {"-r", "remote"},       {"--remote", "remote"},     {"s", "stream"},            {"-s", "stream"},
        {"--streaming", "stream"}, {"streaming", "stream"}, {"adc", "stream"},          {"o", "dac"},
        {"-o", "dac"},          {"--out_streaming", "dac"}, {"-G", "gpio"}, {"--gpio", "gpio"}, {"out_streaming", "dac"}, {"--out-streaming", "dac"}, {"out-streaming", "dac"},   {"key", "keys"},
        {"settings", "keys"},   {"format", "formats"},      {"example", "examples"},
    };
    auto a = alias.find(topic);
    return a == alias.end() ? topic : a->second;
}

}  // namespace

auto ClientHelp::topicOfKey(const std::string& key) -> std::string {
    if (key.empty() || key[0] != '-')
        return "";  // a mode is a key (-s, --streaming), never a value
    auto t = canonical(key);
    return sections().count(t) && t != "formats" && t != "examples" ? t : "";
}

auto ClientHelp::print(FILE* out, const std::string& prog, const std::string& topic, const std::string& filter) -> bool {
    const auto t = canonical(topic);
    if (t.empty()) {
        put(out, prog, kOverview);
        return true;
    }
    if (t == "keys") {
        // the settings of the boards, one per line; with a filter those that name it
        std::istringstream all(CStreamSettings::getHelp());
        std::string line;
        int shown = 0;
        fprintf(out, "Settings of the boards (%s -c -i NAME=VALUE):\n\n", prog.c_str());
        while (std::getline(all, line)) {
            if (!filter.empty() && line.find(filter) == std::string::npos)
                continue;
            fprintf(out, "  %s\n", line.c_str());
            shown++;
        }
        if (!shown)
            fprintf(out, "  none with \"%s\"\n", filter.c_str());
        return true;
    }
    auto s = sections().find(t);
    if (s == sections().end()) {
        fprintf(stderr, "[ERROR] No help on \"%s\". Sections: detect, config, remote, stream, dac, gpio, keys, formats, examples\n", topic.c_str());
        return false;
    }
    put(out, prog, s->second.text);
    return true;
}

auto ClientHelp::synopsis(FILE* out, const std::string& prog, const std::string& topic) -> void {
    auto s = sections().find(canonical(topic));
    if (s != sections().end() && *s->second.usage) {
        put(out, prog, s->second.usage);
        fprintf(out, "More: %s --help %s\n", prog.c_str(), s->first.c_str());
    } else {
        fprintf(out, "Usage: %s MODE [OPTIONS]\nMore: %s --help\n", prog.c_str(), prog.c_str());
    }
}

auto ClientHelp::version() -> std::string {
    return std::string(RPSA_STR(VERSION)) + "-" + RPSA_STR(REVISION);
}

auto ClientHelp::programName(const char* argv0) -> std::string {
    std::string p = argv0 ? argv0 : "rpsa_client";
    auto at = p.find_last_of("/\\");
    return at == std::string::npos ? p : p.substr(at + 1);
}
