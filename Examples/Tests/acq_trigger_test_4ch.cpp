/* Red Pitaya C API example Acquiring a signal from a buffer
 * This application acquires a signal on a specific channel */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <list>
#include <map>
#include <cstring>
#include <sstream>
#include <iostream>

#include "rp.h"
#include "rp_hw-profiles.h"
#include "rp_hw.h"
#include "rp_hw-calib.h"
#include "scpi/scpi_client.h"
#include "common/common.h"

#define DATA_SIZE 20
#define OFFSET 10


using namespace std;

list<string> g_result;
scpi_client g_client;

struct settings{
    bool showBuffer = false;
    bool showHelp = false;
    bool enableDebugRegs = false;
    bool testTrig = false;
    bool testTrigDelay = false;
    bool testTrigSettingNow = false;
    bool testKeepArm = false;
    bool testNoise = false;
    bool testInSplitMode = false;
    bool verbose = false;
    bool stopOnFail = false;
    std::string ip = "";
};

struct testStep{
    rp_channel_t channel;
    rp_channel_trigger_t channel_t;
    rp_acq_trig_src_t t_source;
};

struct testStepSplit{
    rp_acq_trig_src_t t_source[4];
};

auto printHelp(char* prog) -> void {
    std::cout << prog << "-h SCPI_SERVER_IP [-t1] [-t2] [-t3] [-a] [-d] [-b] [-v] [-s]\n";

    auto dac = getDACChannels() >= 2;

    if (dac) {
        std::cout << "For test need connect OUT1 -> IN1 and IN2 with 50 Om load\n";
    }else{
        std::cout << "For test need connect REF Sine signal 500 kHz 1V p-p -> IN1, IN2, IN3, IN4\n";
    }

    std::string s = "\n" \
                    "\t-h : IP address of scpi server\n" \
                    "\t-t1 : Start test trigger position\n" \
                    "\t-t2 : Start test pretrigger and delay logic\n" \
                    "\t-t3 : Start test trigger setting now\n" \
                    "\t-t4 : Start test keep arm\n" \
                    "\t-t5 : Start a noise test on a channels\n" \
                    "\t-t6 : Start test trigger in split mode\n" \
                    "\t-a : Start all test\n" \
                    "\t-d : Enable debug register mode\n" \
                    "\t-b : Show captured buffer\n" \
                    "\t-s : Stop on fail\n" \
                    "\t-v : Verbose output\n";

    std::cout << s.c_str();
}

auto parseOptions(int argc, char **argv) -> settings {
    settings s;
    if (argc == 1){
        s.showHelp = true;
        return s;
    }

    for(int i = 0; i < argc; i++){
        if (strcmp(argv[i],"-h")==0){
            if (i+1 >= argc) {
                s.showHelp = true;
                return s;
            }
            s.ip = argv[i+1];
            i++;
            continue;
        }

        if (strcmp(argv[i],"-d")==0){
            s.enableDebugRegs = true;
        }

        if (strcmp(argv[i],"-b")==0){
            s.showBuffer = true;
        }

        if (strcmp(argv[i],"-t1")==0){
            s.testTrig = true;
        }

        if (strcmp(argv[i],"-t2")==0){
            s.testTrigDelay = true;
        }

        if (strcmp(argv[i],"-t3")==0){
            s.testTrigSettingNow = true;
        }

        if (strcmp(argv[i],"-t4")==0){
            s.testKeepArm = true;
        }

        if (strcmp(argv[i],"-t5")==0){
            s.testNoise = true;
        }

        if (strcmp(argv[i],"-t6")==0){
            s.testInSplitMode = true;
        }

        if (strcmp(argv[i],"-a")==0){
            s.testTrigDelay = true;
            s.testTrig = true;
            s.testTrigSettingNow = true;
            s.testKeepArm = true;
            s.testNoise = true;
            s.testInSplitMode = true;
        }

        if (strcmp(argv[i],"-v")==0){
            s.verbose = true;
        }

        if (strcmp(argv[i],"-s")==0){
            s.stopOnFail = true;
        }

    }
    return s;
}

auto startGenerator(std::string ip, rp_channel_t _channel, rp_waveform_t _wave, uint32_t _rate, float _volt, float _offset,bool _verbose) -> int {
    Color::Modifier red(Color::FG_RED);
    Color::Modifier green(Color::FG_GREEN);

    string wave = "";
    string scpi_wave = "";
    switch (_wave){
        case RP_WAVEFORM_SINE: wave = "sine"; scpi_wave = "SINE"; break;
        case RP_WAVEFORM_SQUARE: wave = "square"; scpi_wave = "SQUARE"; break;
        case RP_WAVEFORM_TRIANGLE: wave = "triangle"; scpi_wave = "TRIANGLE"; break;
        case RP_WAVEFORM_RAMP_UP: wave = "sawtooth"; scpi_wave = "SAWU"; break;
        case RP_WAVEFORM_RAMP_DOWN: wave = "reversed sawtooth"; scpi_wave = "SAWD"; break;
        case RP_WAVEFORM_DC: wave = "dc"; scpi_wave = "DC";  break;
        case RP_WAVEFORM_PWM: wave = "pwm"; scpi_wave = "PWM"; break;
        case RP_WAVEFORM_ARBITRARY: wave = "User defined wave form"; scpi_wave = "ARBITRARY";  break;
        case RP_WAVEFORM_DC_NEG: wave = "negative dc"; scpi_wave = "DC_NEG"; break;
        default:
            printf("Undefined wave type\n");
            exit(-1);
    }

    int ret = 0;
    g_client.write("RP:LOG CONSOLE");
    usleep(10000);
    g_client.write("GEN:RST");
    usleep(10000);
    g_client.write("SOUR1:VOLT:OFFS " + to_string(_offset));
    usleep(10000);
    g_client.write("SOUR1:VOLT " + to_string(_volt));
    usleep(10000);
    g_client.write("SOUR1:FREQ:FIX " + to_string(_rate));
    usleep(10000);
    g_client.write("SOUR1:FUNC " + scpi_wave);
    usleep(10000);
    g_client.write("OUTPUT1:STATE ON");
    usleep(10000);
    g_client.write("SOUR1:TRIG:INT");
    usleep(10000);
    g_client.write("*STB?");
    usleep(10000);
    string read_buff;
    if (g_client.read(read_buff,2000) == scpi_client::OK){
    }else{
        ret = -1;
    }

    if (_verbose){

        string s = "* Start signal generator. Ampl: " + to_string(_volt)  + " Vpp. Offset: " + to_string(_offset) + " V. Freq:  " + to_string(_rate) + " Hz. Wave: " + wave + " " + (ret == 0 ? green.color("[OK]") : red.color("[FAIL]"));
        std::cout << s << "\n";
    }

    return RP_OK;
}

auto getData(rp_channel_trigger_t _ch, uint32_t _dec, int32_t _triggerDelay, float _trigLevel, rp_acq_trig_src_t _ts, bool _verbose, buffers_t *_buffer) -> int {
    static double rate = getADCRate();
    Color::Modifier red(Color::FG_RED);
    Color::Modifier green(Color::FG_GREEN);

    if (_verbose){
        string s = "* Start capturing data";
        std::cout << s << "\n";

    }

    int ret = 0;
    ret |= rp_AcqReset();
    ret |= rp_AcqSetDecimationFactor(_dec);
    ret |= rp_AcqSetTriggerLevel(_ch, _trigLevel);
    ret |= rp_AcqSetTriggerHyst(0.005);
    ret |= rp_AcqSetTriggerDelayDirect(_triggerDelay);

    auto max_timeout = ADC_BUFFER_SIZE / (rate / RP_DEC_65536) * 1000.0;
    auto timeout = max_timeout + getClock();


    uint32_t preTriggerWait = ADC_BUFFER_SIZE - _triggerDelay;

    ret |= rp_AcqStart();

    uint32_t pretrigger = 0;
    while(pretrigger < preTriggerWait && pretrigger < ADC_BUFFER_SIZE){
        ret |= rp_AcqGetPreTriggerCounter(&pretrigger);
        if (timeout < getClock()) {
            printf("Fail pre trigger counter. Current counter %d need %d\n",pretrigger,preTriggerWait);
            return -1;
        }
    }
    ret |=rp_AcqSetTriggerSrc(_ts);
    timeout = max_timeout + getClock();
    rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
    while(1){
        ret |= rp_AcqGetTriggerState(&state);
        if(state == RP_TRIG_STATE_TRIGGERED){
            break;
        }
        if (timeout < getClock()) {
            printf("Fail wait trigger.\n");
            return -1;
        }
    }

    bool fillState = false;
    timeout = max_timeout + getClock();
    while(!fillState){
        ret |=  rp_AcqGetBufferFillState(&fillState);
        if (timeout < getClock()) {
            printf("Fail wait fill state\n");
            return -1;
        }
    }
    ret |= rp_AcqStop();
    uint32_t trig_pos;
    ret |= rp_AcqGetWritePointerAtTrig(&trig_pos);
    ret |= rp_AcqGetData(trig_pos,_buffer);

    if (_verbose){
        string s = "* End capturing data " + (ret == 0 ? green.color("[OK]") : red.color("[FAIL]"));
        std::cout << s << "\n";
    }

    return ret;
}


auto getDataSplit(testStepSplit settings, uint32_t _dec, int32_t _triggerDelay, float _trigLevel, bool _verbose, buffers_t *_buffer) -> int {
    static double rate = getADCRate();
    Color::Modifier red(Color::FG_RED);
    Color::Modifier green(Color::FG_GREEN);

    if (_verbose){
        string s = "* Start capturing data";
        std::cout << s << "\n";

    }
    bool chEnable[4];
    chEnable[0] = settings.t_source[0] != RP_TRIG_SRC_DISABLED;
    chEnable[1] = settings.t_source[1] != RP_TRIG_SRC_DISABLED;
    chEnable[2] = settings.t_source[2] != RP_TRIG_SRC_DISABLED;
    chEnable[3] = settings.t_source[3] != RP_TRIG_SRC_DISABLED;

    int ret = 0;
    for(int ch = 0; ch < 4; ch++){
        if (chEnable[ch]) {
            ret |= rp_AcqResetCh((rp_channel_t)ch);
            ret |= rp_AcqSetDecimationFactorCh((rp_channel_t)ch,_dec);
            ret |= rp_AcqSetTriggerLevel((rp_channel_trigger_t)ch, _trigLevel);
            ret |= rp_AcqSetTriggerDelayDirectCh((rp_channel_t)ch, _triggerDelay);
        }
    }

    ret |= rp_AcqSetTriggerHyst(0.005);

    auto max_timeout = ADC_BUFFER_SIZE / (rate / RP_DEC_65536) * 1000.0;



    uint32_t preTriggerWait = ADC_BUFFER_SIZE - _triggerDelay;

    for(int ch = 0; ch < 4; ch++){
        if (chEnable[ch]) {
            ret |= rp_AcqStartCh((rp_channel_t)ch);
        }
    }

    for(int ch = 0; ch < 4; ch++){
        if (chEnable[ch]) {
            auto timeout = max_timeout + getClock();
            uint32_t pretrigger = 0;
            while(pretrigger < preTriggerWait && pretrigger < ADC_BUFFER_SIZE){
                ret |= rp_AcqGetPreTriggerCounterCh((rp_channel_t)ch, &pretrigger);
                if (timeout < getClock()) {
                    printf("Fail pre trigger counter on Ch %d. Current counter %d need %d\n",ch + 1, pretrigger,preTriggerWait);
                    return -1;
                }
            }
            ret |=rp_AcqSetTriggerSrcCh((rp_channel_t)ch, settings.t_source[ch]);
            timeout = max_timeout + getClock();
            rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
            while(1){
                ret |= rp_AcqGetTriggerStateCh((rp_channel_t)ch, &state);
                if(state == RP_TRIG_STATE_TRIGGERED){
                    break;
                }
                if (timeout < getClock()) {
                    printf("Fail wait trigger on Ch %d.\n",ch + 1);
                    return -1;
                }
            }
        }
    }

    for(int ch = 0; ch < 4; ch++){
        if (chEnable[ch]) {
            bool fillState = false;
            auto timeout = max_timeout + getClock();
            while(!fillState){
                ret |=  rp_AcqGetBufferFillStateCh((rp_channel_t)ch, &fillState);
                if (timeout < getClock()) {
                    printf("Fail wait fill state on Ch %d.\n", ch + 1);
                    return -1;
                }
            }
        }
    }

    for(int ch = 0; ch < 4; ch++){
        if (chEnable[ch]) {
            ret |= rp_AcqStopCh((rp_channel_t)ch);
            uint32_t trig_pos;
            ret |= rp_AcqGetWritePointerAtTrigCh((rp_channel_t)ch, &trig_pos);
            uint32_t size = ADC_BUFFER_SIZE;
            ret |= rp_AcqGetDataRaw((rp_channel_t)ch, trig_pos, &size , _buffer->ch_i[ch]);
        }
    }

    if (_verbose){
        string s = "* End capturing data " + (ret == 0 ? green.color("[OK]") : red.color("[FAIL]"));
        std::cout << s << "\n";
    }

    return ret;
}

auto testTrig(settings s) -> int {

    auto old_calib = rp_GetCalibrationSettings();
    auto def_calib = rp_GetDefaultCalibrationSettings();
    rp_CalibrationSetParams(def_calib);
    uint32_t adcRate = getADCRate();
    uint32_t minPointerPerPer = 4;
    uint32_t maxPointerPerPer = 100;
    uint32_t steps = 25;
    auto model = getModel();
    int result = 0;
    list<uint32_t> dec_list;
    for(uint32_t dec = RP_DEC_1; dec <= RP_DEC_65536; dec *= 2){
        dec_list.push_back(dec);
        if (dec >= RP_DEC_4096){
            steps = 6;
        }
        if (dec >= 16 && dec < RP_DEC_65536){
            dec_list.push_back(dec + 1);
            dec_list.push_back(dec + 2);
            dec_list.push_back(dec + 3);
            dec_list.push_back(dec + 4);
        }
    }

    map<uint32_t,vector<uint32_t>> test_list;

    for(auto i : dec_list){
        auto adcCurRate = adcRate / i;
        auto maxFreq = adcCurRate / minPointerPerPer;
        if (maxFreq > 17000000) { // DAC limitation
            maxFreq = 17000000;
        }

        auto minFreq = adcCurRate / maxPointerPerPer;
        minFreq = MAX(minFreq,1);
        maxFreq = MAX(maxFreq,1);

        auto freqSteps = MAX(1,(maxFreq - minFreq) / steps);
        test_list[i] = {};
        for(uint32_t freq = minFreq; freq < maxFreq; freq += freqSteps){
            if (model == RP_122_16){
                if (freq  < 100000)
                    continue;
            }
            test_list[i].push_back(freq);
        }
    }
    auto buffer = rp_createBuffer(getADCChannels(),ADC_BUFFER_SIZE,true,false,false);
    if (!buffer){
        printf("Can't allocate buffer\n");
        exit(-1);
    }

    vector<testStep> trig_list = {{RP_CH_1,RP_T_CH_1,RP_TRIG_SRC_CHA_PE},
                                  {RP_CH_1,RP_T_CH_1,RP_TRIG_SRC_CHA_NE},
                                  {RP_CH_2,RP_T_CH_2,RP_TRIG_SRC_CHB_PE},
                                  {RP_CH_2,RP_T_CH_2,RP_TRIG_SRC_CHB_NE},
                                  {RP_CH_3,RP_T_CH_3,RP_TRIG_SRC_CHC_PE},
                                  {RP_CH_3,RP_T_CH_3,RP_TRIG_SRC_CHC_NE},
                                  {RP_CH_4,RP_T_CH_4,RP_TRIG_SRC_CHD_PE},
                                  {RP_CH_4,RP_T_CH_4,RP_TRIG_SRC_CHD_NE}};


    for(auto &i : test_list){
        auto dec = i.first;
        auto freq_list = i.second;
        for(auto freq : freq_list){
            int testResult = 0;
            string testName = "Trigger position test. Decimate: " + to_string(dec) + ". Signal freq: " + to_string(freq);
            if (s.verbose){
                std::cout << testName << "\n";
            }
            testResult |= startGenerator(s.ip, RP_CH_1,RP_WAVEFORM_SINE,freq,  0.9,0,s.verbose);

            for(auto test_step : trig_list){
                testResult |=  getData(test_step.channel_t,dec,ADC_BUFFER_SIZE/2.0, 0 ,test_step.t_source,s.verbose,buffer);

                int ch = test_step.channel;
                float sign = getTrigNameDir(test_step.t_source);
                auto end = buffer->size - 1;
                bool bufferIsOk = true;
                if (sign >= 0){
                    if (!(buffer->ch_i[ch][0] > buffer->ch_i[ch][end] &&
                        buffer->ch_i[ch][0] >= 0 &&
                        buffer->ch_i[ch][end] < 0)){
                            bufferIsOk = false;
                        }
                }else{
                    if (!(buffer->ch_i[ch][0] < buffer->ch_i[ch][end] &&
                        buffer->ch_i[ch][0] <= 0 &&
                        buffer->ch_i[ch][end] > 0)){
                            bufferIsOk = false;
                        }
                }

                if (s.showBuffer || !bufferIsOk) {
                    if (!bufferIsOk)
                        printf("Fail in %s trigger\n",getTrigName(test_step.t_source).c_str());
                    printBuffer(buffer,-5,10);
                }
                testResult |= !bufferIsOk;
            }
            result |= testResult;
            if (s.verbose || testResult){
                printTestResult(g_result, testName,testResult == 0);
            }

            if (s.stopOnFail && result) {
                exit(-1);
            }
        }
    }
    rp_deleteBuffer(buffer);
    rp_CalibrationSetParams(old_calib);
    return result;
}


auto testTrigSettingNow(settings s) -> int {
    static double rate = getADCRate();
    Color::Modifier red(Color::FG_RED);
    Color::Modifier green(Color::FG_GREEN);
    auto max_timeout = ADC_BUFFER_SIZE / (rate / RP_DEC_65536) * 1000.0;

    int result = 0;
    list<uint32_t> dec_list;
    for(uint32_t dec = RP_DEC_1; dec <= RP_DEC_65536; dec *= 2){
        dec_list.push_back(dec);
        if (dec >= 16 && dec < RP_DEC_65536){
            dec_list.push_back(dec + 1);
            dec_list.push_back(dec + 2);
            dec_list.push_back(dec + 3);
            dec_list.push_back(dec + 4);
        }
    }
    result |= startGenerator(s.ip,RP_CH_1,RP_WAVEFORM_SINE,1000,  0.9 ,0,s.verbose);  // low frequency for very high decimations
    result |= rp_AcqReset();
    result |= rp_AcqSetTriggerLevel(RP_T_CH_1, 0);
    result |= rp_AcqSetTriggerHyst(0.005);
    result |= rp_AcqSetTriggerDelayDirect(ADC_BUFFER_SIZE/2.0);

    uint32_t trig_pos_prev;
    uint32_t write_pos_prev;

    for(auto dec : dec_list){
        string testName = "Trigger Reset Lock Testing. Decimate: " + to_string(dec);
        if (s.verbose){
            std::cout << testName << "\n";
        }

        int ret = 0;
        auto timeLoop = getClock();
        ret |= rp_AcqStart();
        ret |= rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
        auto timeout = max_timeout + getClock();
        rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
        while(1){
            ret |= rp_AcqGetTriggerState(&state);
            if(state == RP_TRIG_STATE_TRIGGERED){
                break;
            }
            if (timeout < getClock()) {
                printf("Fail wait trigger.\n");
                ret |= true;
                break;
            }
        }

        bool fillState = false;
        timeout = max_timeout + getClock();
        while(!fillState){
            ret |=  rp_AcqGetBufferFillState(&fillState);
            if (timeout < getClock()) {
                printf("Fail wait fill state\n");
                ret |= true;
                break;
            }
        }
        uint32_t trig_pos;
        uint32_t write_pos;

        ret |= rp_AcqGetWritePointerAtTrig(&trig_pos);
        ret |= rp_AcqGetWritePointer(&write_pos);


        ret |= rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);

        if (s.verbose)
            printf("Time %f\n",getClock() - timeLoop);
        usleep(500000);
        uint32_t new_trig_pos;
        uint32_t new_write_pos;

        ret |= rp_AcqGetWritePointerAtTrig(&new_trig_pos);
        ret |= rp_AcqGetWritePointer(&new_write_pos);

        bool testResult = new_trig_pos != trig_pos || new_write_pos != write_pos;
        bool posNotChanged = trig_pos == trig_pos_prev || write_pos == write_pos_prev;

        if (posNotChanged){
            std::cout << "Position was not changed\n";
            std::cout << "Trigger position " << trig_pos << " position in old test " << trig_pos_prev << "\n";
            std::cout << "Write position " << write_pos << " position in old test " << write_pos_prev << "\n";
        }

        trig_pos_prev = trig_pos;
        write_pos_prev = write_pos;

        if (testResult){
            std::cout << "Required trigger position " << trig_pos << " after set now " << new_trig_pos << "\n";
            std::cout << "Required write position " << write_pos << " after set now " << new_write_pos << "\n";
        }

        ret |= testResult;
        ret |= posNotChanged;
        result |= ret;
        if (s.verbose || ret){
            std::cout << "Trigger position " << trig_pos << "\n";
            std::cout << "Write position " << write_pos << "\n";
            printTestResult(g_result, testName,ret == 0);
        }

        if (s.stopOnFail && result) {
            exit(-1);
        }
    }
    result |= rp_AcqStop();


    return result;
}


auto testKeepArm(settings s) -> int {
    static double rate = getADCRate();
    Color::Modifier red(Color::FG_RED);
    Color::Modifier green(Color::FG_GREEN);
    auto max_timeout = ADC_BUFFER_SIZE / (rate / RP_DEC_65536) * 1000.0;
    auto genFreq = 100;
    auto model = getModel();
    auto maxDec = RP_DEC_8192;
    auto minDec = RP_DEC_65536;

    if (model == RP_122_16){
        genFreq = 100000;
        maxDec = RP_DEC_16;
        minDec = RP_DEC_4096;
    }
    int result = 0;
    list<uint32_t> dec_list;
    for(uint32_t dec = maxDec; dec <= minDec; dec *= 2){
        dec_list.push_back(dec);
        if (dec >= 16 && dec < RP_DEC_65536){
            dec_list.push_back(dec + 1);
            dec_list.push_back(dec + 2);
            dec_list.push_back(dec + 3);
            dec_list.push_back(dec + 4);
        }
    }
    result |= startGenerator(s.ip,RP_CH_1,RP_WAVEFORM_SINE,genFreq,  0.9, 0, s.verbose);  // low frequency for very high decimations
    result |= rp_AcqReset();
    result |= rp_AcqSetTriggerLevel(RP_T_CH_1, 0);
    result |= rp_AcqSetTriggerHyst(0.005);
    result |= rp_AcqSetTriggerDelayDirect(ADC_BUFFER_SIZE/2.0);
    result |= rp_AcqSetArmKeep(true);

    uint32_t prev_test_trig = 0xFFFFFFFF;

    for(auto dec : dec_list){
        string testName = "Keep arm test. Decimate: " + to_string(dec);
        if (s.verbose){
            std::cout << testName << "\n";
        }

        int ret = 0;

        ret |= rp_AcqSetDecimationFactor(dec);
        ret |= rp_AcqSetTriggerSrc(RP_TRIG_SRC_DISABLED);
        ret |= rp_AcqStart(); //start at each loop

        ret |= rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);

        auto timeout = max_timeout + getClock();
        rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
        while(1){
            ret |= rp_AcqGetTriggerState(&state);
            if(state == RP_TRIG_STATE_TRIGGERED){
                break;
            }
            if (timeout < getClock()) {
                printf("Fail wait trigger.\n");
                ret |= true;
                break;
            }
        }

        bool fillState = false;
        timeout = max_timeout + getClock();
        while(!fillState){
            ret |=  rp_AcqGetBufferFillState(&fillState);
            if (timeout < getClock()) {
                printf("Fail wait fill state\n");
                ret |= true;
                break;
            }
        }
        // Testing write pointer after
        uint16_t readCount = 10;

        uint32_t trig_pos_prev = 0xFFFFFFFF;
        uint32_t write_pos_prev = 0xFFFFFFFF;


        uint32_t trig_pos = 0xFFFFFFFF;
        uint32_t write_pos = 0xFFFFFFFF;

        ret |= rp_AcqGetWritePointerAtTrig(&trig_pos);
        ret |= rp_AcqGetWritePointer(&write_pos);

        trig_pos_prev = trig_pos;
        write_pos_prev = write_pos;
        usleep(1000);

        if (trig_pos == prev_test_trig){
            std::cout << "Trigger Position was not changed after prev test\n";
            std::cout << "Trigger position " << write_pos << " prev test position " << write_pos_prev << "\n";
            ret |= true;
        }

        while(readCount){
            ret |= rp_AcqGetWritePointerAtTrig(&trig_pos);
            ret |= rp_AcqGetWritePointer(&write_pos);

            if (write_pos == write_pos_prev){
                std::cout << "Write Position was not changed\n";
                std::cout << "Write position " << write_pos << " prev position " << write_pos_prev << "\n";
                ret |= true;
            }

            if (trig_pos != trig_pos_prev){
                std::cout << "Trigger Position was changed\n";
                std::cout << "Trigger position " << write_pos << " prev position " << write_pos_prev << "\n";
                ret |= true;
            }

            trig_pos_prev = trig_pos;
            write_pos_prev = write_pos;
            readCount--;
            usleep(1000);
        }

        prev_test_trig = trig_pos;

        result |= ret;

        if (s.verbose || ret){
            printTestResult(g_result, testName,ret == 0);
        }

        if (s.stopOnFail && result) {
            exit(-1);
        }
    }
    result |= rp_AcqStop();

    return result;
}


auto testTrigDelay(settings s) -> int {
    static double rate = getADCRate();
    Color::Modifier red(Color::FG_RED);
    Color::Modifier green(Color::FG_GREEN);
    auto max_timeout = ADC_BUFFER_SIZE / (rate / RP_DEC_65536) * 1000.0;
    auto model = getModel();
    if (model == RP_122_16){
        printf("Test not available on 122-16\n");
        return 0;
    }

    auto old_calib = rp_GetCalibrationSettings();
    auto def_calib = rp_GetDefaultCalibrationSettings();
    rp_CalibrationSetParams(def_calib);

    uint8_t bits;
    rp_HPGetFastADCBits(&bits);

    int result = 0;
    list<uint32_t> dec_list;
    for(uint32_t dec = RP_DEC_1; dec <= RP_DEC_65536; dec *= 2){
        dec_list.push_back(dec);
        if (dec >= 16 && dec < RP_DEC_65536){
            dec_list.push_back(dec + 1);
            dec_list.push_back(dec + 2);
            dec_list.push_back(dec + 3);
            dec_list.push_back(dec + 4);
        }
    }

    auto buffer = rp_createBuffer(getADCChannels(),ADC_BUFFER_SIZE,true,false,true);
    if (!buffer){
        printf("Can't allocate buffer\n");
        exit(-1);
    }

    for(auto dec : dec_list){
        for(uint32_t i = 1; i <= 1024; i *= 2){
            string testName = "Testing trigger delay. Delay: " + to_string(i) +  " Decimate: " + to_string(dec);
            if (s.verbose){
                std::cout << testName << "\n";
            }

            int ret = 0;
            ret |= startGenerator(s.ip,RP_CH_1,RP_WAVEFORM_DC,1000,0,0,s.verbose);
            // Clean buffer in FPGA
            usleep(1000);
            ret |= getData(RP_T_CH_1,1,ADC_BUFFER_SIZE,0,RP_TRIG_SRC_NOW,s.verbose,buffer);
            ret |= getData(RP_T_CH_1,1,ADC_BUFFER_SIZE,0,RP_TRIG_SRC_NOW,s.verbose,buffer);


            ret |= rp_AcqReset();
            ret |= rp_AcqSetDecimationFactor(dec);
            ret |= rp_AcqSetTriggerLevel(RP_T_CH_1, 0.7); // need low threshold so the very first sample can be high
            ret |= rp_AcqSetTriggerHyst(0.005);
            ret |= rp_AcqSetTriggerDelayDirect(i);
            ret |= rp_AcqStart();

            uint32_t oldPreTrigger = 0, newPretrigger = 0;
            for(int i = 0; i < 10; i++){
                rp_AcqGetPreTriggerCounter(&newPretrigger);
                usleep(1000);
                if (newPretrigger == oldPreTrigger && oldPreTrigger != 0){
                    printf("Fail check pre trigger counter: %d.\n",newPretrigger);
                    ret |= true;
                }
                oldPreTrigger = newPretrigger;
            }

            ret |= rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE);
            ret |= startGenerator(s.ip,RP_CH_1,RP_WAVEFORM_DC,1000,1,0,s.verbose);


            auto timeout = max_timeout + getClock();
            rp_acq_trig_state_t state = RP_TRIG_STATE_TRIGGERED;
            while(1){
                ret |= rp_AcqGetTriggerState(&state);
                if(state == RP_TRIG_STATE_TRIGGERED){
                    break;
                }
                if (timeout < getClock()) {
                    printf("Fail wait trigger.\n");
                    ret |= true;
                    break;
                }
            }

            bool fillState = false;
            timeout = max_timeout + getClock();
            while(!fillState){
                ret |=  rp_AcqGetBufferFillState(&fillState);
                if (timeout < getClock()) {
                    printf("Fail wait fill state.\n");
                    ret |= true;
                    break;
                }
            }
            ret |= rp_AcqStop();
            uint32_t trig_pos;
            ret |= rp_AcqGetWritePointerAtTrig(&trig_pos);
            ret |= rp_AcqGetData(trig_pos,buffer);

            uint32_t sampWithData = 0;
            for(uint32_t z = 0; z < buffer->size; z++){
                if (buffer->ch_i[0][z] >= (1 << (bits -1)) * 0.7) { // must take into account the rise time and initial ringing of step signal
                    sampWithData++;
                }
            }

            bool testResult = sampWithData != i;
            ret |= testResult;
            if (testResult){
                std::cout << "The number of data samples is not equal to the trigger delay\n";
                std::cout << "Delay " << i << " samples " << sampWithData << "\n";
                printBuffer(buffer,-2,sampWithData + 3);
            }

            if (s.verbose || ret){
                printTestResult(g_result, testName,ret == 0);
            }

            if (s.stopOnFail && ret) {
                exit(-1);
            }
            result |= ret;
        }
    }

    rp_deleteBuffer(buffer);
    rp_CalibrationSetParams(old_calib);
    return result;
}


auto testNoise(settings s) -> int {

    rp_SetLEDEthState(false);
    rp_SetLEDHeartBeatState(false);
    rp_SetLEDMMCState(false);

    auto old_calib = rp_GetCalibrationSettings();
    auto def_calib = rp_GetDefaultCalibrationSettings();
    rp_CalibrationSetParams(def_calib);
    uint32_t adcRate = getADCRate();
    uint32_t minPointerPerPer = 8;
    uint32_t maxPointerPerPer = 100;
    uint32_t steps = 25;
    auto model = getModel();
    int result = 0;
    list<uint32_t> dec_list;
    for(uint32_t dec = RP_DEC_1; dec <= RP_DEC_65536; dec *= 2){
        dec_list.push_back(dec);
        if (dec >= RP_DEC_4096){
            steps = 6;
        }
        if (dec >= 16 && dec < RP_DEC_65536){
            dec_list.push_back(dec + 1);
            dec_list.push_back(dec + 2);
            dec_list.push_back(dec + 3);
            dec_list.push_back(dec + 4);
        }
    }

    map<uint32_t,vector<uint32_t>> test_list;

    for(auto i : dec_list){
        auto adcCurRate = adcRate / i;
        auto maxFreq = adcCurRate / minPointerPerPer;
        if (maxFreq > 17000000) { // DAC limitation
            maxFreq = 17000000;
        }

        auto minFreq = adcCurRate / maxPointerPerPer;
        minFreq = MAX(minFreq,1);
        maxFreq = MAX(maxFreq,1);

        auto freqSteps = MAX(1,(maxFreq - minFreq) / steps);
        test_list[i] = {};
        for(uint32_t freq = minFreq; freq < maxFreq; freq += freqSteps){
            if (model == RP_122_16){
                if (freq  < 100000)
                    continue;
            }
            test_list[i].push_back(freq);
        }
    }
    auto buffer = rp_createBuffer(getADCChannels(),ADC_BUFFER_SIZE,true,false,false);
    if (!buffer){
        printf("Can't allocate buffer\n");
        exit(-1);
    }

    vector<testStep> trig_list = {{RP_CH_1,RP_T_CH_1,RP_TRIG_SRC_CHA_PE},
                                  {RP_CH_2,RP_T_CH_2,RP_TRIG_SRC_CHB_PE},
                                  {RP_CH_3,RP_T_CH_3,RP_TRIG_SRC_CHC_PE},
                                  {RP_CH_4,RP_T_CH_4,RP_TRIG_SRC_CHD_PE}};


    for(auto &i : test_list){
        auto dec = i.first;
        auto freq_list = i.second;
        for(auto freq : freq_list){
            int testResult = 0;
            string testName = "Noise test. Decimate: " + to_string(dec) + ". Signal freq: " + to_string(freq);
            if (s.verbose){
                std::cout << testName << "\n";
            }
            testResult |= startGenerator(s.ip, RP_CH_1,RP_WAVEFORM_SINE,freq,  0.9,0,s.verbose);

            for(auto test_step : trig_list){
                testResult |=  getData(test_step.channel_t,dec,ADC_BUFFER_SIZE, 0 ,test_step.t_source,s.verbose,buffer);

                int ch = test_step.channel;
                auto end = buffer->size - 1;
                bool bufferIsOk = true;

                unsigned int idx = 0;
                for(; idx < end - 2; idx++){

                    if (buffer->ch_i[ch][idx] > 0 && buffer->ch_i[ch][idx + 2] > 0){
                        if (buffer->ch_i[ch][idx+1] <= 0){
                            bufferIsOk = false;
                            break;
                        }
                    }

                    if (buffer->ch_i[ch][idx] < 0 && buffer->ch_i[ch][idx + 2] < 0){
                        if (buffer->ch_i[ch][idx+1] >= 0){
                            bufferIsOk = false;
                            break;
                        }
                    }
                }

                if (s.showBuffer || !bufferIsOk) {
                    if (!bufferIsOk)
                        printf("Fail in %s trigger\n",getTrigName(test_step.t_source).c_str());
                    printBuffer(buffer,idx - 3 ,7);
                }
                testResult |= !bufferIsOk;
            }
            result |= testResult;
            if (s.verbose || testResult){
                printTestResult(g_result, testName,testResult == 0);
            }

            if (s.stopOnFail && result) {
                exit(-1);
            }
        }
    }
    rp_deleteBuffer(buffer);
    rp_CalibrationSetParams(old_calib);
    return result;
}


auto testSplitMode(settings s) -> int {

    if (rp_AcqSetSplitTrigger(true) != RP_OK){
        printf("Split mode not supported\n");
        return RP_NOTS;
    }

    auto old_calib = rp_GetCalibrationSettings();
    auto def_calib = rp_GetDefaultCalibrationSettings();
    rp_CalibrationSetParams(def_calib);
    uint32_t adcRate = getADCRate();
    uint32_t minPointerPerPer = 4;
    uint32_t maxPointerPerPer = 100;
    uint32_t steps = 25;
    auto model = getModel();
    int result = 0;
    list<uint32_t> dec_list;
    for(uint32_t dec = RP_DEC_1; dec <= RP_DEC_65536; dec *= 2){
        dec_list.push_back(dec);
        if (dec >= RP_DEC_4096){
            steps = 6;
        }
        if (dec >= 16 && dec < RP_DEC_65536){
            dec_list.push_back(dec + 1);
            dec_list.push_back(dec + 2);
            dec_list.push_back(dec + 3);
            dec_list.push_back(dec + 4);
        }
    }

    map<uint32_t,vector<uint32_t>> test_list;

    for(auto i : dec_list){
        auto adcCurRate = adcRate / i;
        auto maxFreq = adcCurRate / minPointerPerPer;
        if (maxFreq > 17000000) { // DAC limitation
            maxFreq = 17000000;
        }

        auto minFreq = adcCurRate / maxPointerPerPer;
        minFreq = MAX(minFreq,1);
        maxFreq = MAX(maxFreq,1);

        auto freqSteps = MAX(1,(maxFreq - minFreq) / steps);
        test_list[i] = {};
        for(uint32_t freq = minFreq; freq < maxFreq; freq += freqSteps){
            if (model == RP_122_16){
                if (freq  < 100000)
                    continue;
            }
            test_list[i].push_back(freq);
        }
    }
    auto buffer = rp_createBuffer(getADCChannels(),ADC_BUFFER_SIZE,true,false,false);
    if (!buffer){
        printf("Can't allocate buffer\n");
        exit(-1);
    }

    vector<testStepSplit> trig_list = {{.t_source = {RP_TRIG_SRC_CHA_PE, RP_TRIG_SRC_CHB_PE, RP_TRIG_SRC_CHC_PE, RP_TRIG_SRC_CHD_PE}},
                                       {.t_source = {RP_TRIG_SRC_CHA_PE, RP_TRIG_SRC_CHB_NE, RP_TRIG_SRC_CHC_PE, RP_TRIG_SRC_CHD_NE}},
                                       {.t_source = {RP_TRIG_SRC_CHA_NE, RP_TRIG_SRC_CHB_NE, RP_TRIG_SRC_CHC_NE, RP_TRIG_SRC_CHD_NE}},
                                       {.t_source = {RP_TRIG_SRC_CHA_NE, RP_TRIG_SRC_CHB_PE, RP_TRIG_SRC_CHC_NE, RP_TRIG_SRC_CHD_PE}},
                                       {.t_source = {RP_TRIG_SRC_CHA_NE, RP_TRIG_SRC_DISABLED, RP_TRIG_SRC_CHC_NE, RP_TRIG_SRC_DISABLED}},
                                       {.t_source = {RP_TRIG_SRC_DISABLED, RP_TRIG_SRC_CHB_PE, RP_TRIG_SRC_DISABLED, RP_TRIG_SRC_CHD_PE}},
                                       {.t_source = {RP_TRIG_SRC_CHA_NE, RP_TRIG_SRC_DISABLED, RP_TRIG_SRC_CHC_PE, RP_TRIG_SRC_DISABLED}}
                                        //,
                                    //    {.t_source = {RP_TRIG_SRC_CHA_PE, RP_TRIG_SRC_CHA_PE, RP_TRIG_SRC_CHA_PE, RP_TRIG_SRC_CHA_PE}},
                                    //    {.t_source = {RP_TRIG_SRC_CHD_PE, RP_TRIG_SRC_CHD_PE, RP_TRIG_SRC_CHD_PE, RP_TRIG_SRC_CHD_PE}}
                                       };


    for(auto &i : test_list){
        auto dec = i.first;
        auto freq_list = i.second;
        for(auto freq : freq_list){
            int testResult = 0;
            string testName = "Trigger position test. Decimate: " + to_string(dec) + ". Signal freq: " + to_string(freq);
            if (s.verbose){
                std::cout << testName << "\n";
            }
            testResult |= startGenerator(s.ip, RP_CH_1,RP_WAVEFORM_SINE,freq,  0.9,0,s.verbose);

            for(auto test_step : trig_list){
                testResult |=  getDataSplit(test_step,dec,ADC_BUFFER_SIZE/2.0, 0, s.verbose, buffer);
                printf("\t - Test ");
                for(int ch = 0 ; ch < 4; ch++){
                    if (test_step.t_source[ch] != RP_TRIG_SRC_DISABLED){
                        printf("Ch %d: %s ", ch + 1,getTrigName(test_step.t_source[ch]).c_str());
                    }
                }
                printf("\n");
                for(int ch = 0 ; ch < 4; ch++){
                    if (test_step.t_source[ch] != RP_TRIG_SRC_DISABLED){
                        float sign = getTrigNameDir(test_step.t_source[ch]);
                        auto end = buffer->size - 1;
                        bool bufferIsOk = true;
                        if (sign >= 0){
                        if (!(buffer->ch_i[ch][0] > buffer->ch_i[ch][end] &&
                            buffer->ch_i[ch][0] >= 0 &&
                            buffer->ch_i[ch][end] < 0)){
                                bufferIsOk = false;
                            }
                        }else{
                            if (!(buffer->ch_i[ch][0] < buffer->ch_i[ch][end] &&
                                buffer->ch_i[ch][0] <= 0 &&
                                buffer->ch_i[ch][end] > 0)){
                                    bufferIsOk = false;
                                }
                        }

                        if (s.showBuffer || !bufferIsOk) {
                            if (!bufferIsOk)
                                printf("Fail in %s trigger\n",getTrigName(test_step.t_source[ch]).c_str());
                            printBuffer(buffer,-5,10);
                        }
                        testResult |= !bufferIsOk;
                    }
                }


            }
            result |= testResult;
            if (s.verbose || testResult){
                printTestResult(g_result, testName,testResult == 0);
            }

            if (s.stopOnFail && result) {
                exit(-1);
            }
        }
    }
    rp_deleteBuffer(buffer);
    rp_CalibrationSetParams(old_calib);
    return result;
}


int main(int argc, char **argv){

    fprintf(stderr,"\nTo test 4-channel mode, a second Red Pitaya with a generator in SCPI mode is required. The connection should be RP (Out1) -> RP 4ch (In1, In2, In3, In4)\n\n");

    int result = 0;
    settings s = parseOptions(argc,argv);
    if (s.showHelp){
        printHelp(argv[0]);
        return 0;
    }

    if (s.enableDebugRegs){
        rp_EnableDebugReg();
    }

    g_client.open(s.ip,5000);
    if (!g_client.is_opened()){
        fprintf(stderr,"Error open scpi connection to %s\n",s.ip.c_str());
        return 1;
    }

    if(rp_InitReset(false) != RP_OK){
        fprintf(stderr, "Rp api init failed!\n");
        return 1;
    }

    rp_AcqSetSplitTrigger(false);

    g_result.clear();

    if (s.testTrig){
        result |=  testTrig(s);
    }

    if (s.testTrigDelay){
        result |=  testTrigDelay(s);
    }

    if (s.testTrigSettingNow){
        result |=  testTrigSettingNow(s);
    }

    if (s.testKeepArm){
        result |=  testKeepArm(s);
    }

    if (s.testNoise){
        result |=  testNoise(s);
    }

    if (s.testInSplitMode){
        result |=  testSplitMode(s);
    }

    printAllResult(g_result);

    rp_Release();
    return result;
}
