#pragma once
#include "rp.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

using namespace std;
using namespace std::chrono;

class CSweepController
{
    struct Settings{
        time_point<system_clock> startTime;
        bool run = false;
        int  currentStep = 0;
        int  startF = 0;
        int  endF = 0;
        int  time = 1;
        rp_gen_sweep_mode_t mode = RP_GEN_SWEEP_MODE_LINEAR;
        rp_gen_sweep_dir_t dir = RP_GEN_SWEEP_DIR_NORMAL;
    };

public:
    CSweepController();
    ~CSweepController();
    auto run() -> void;
    auto stop() -> void;
    auto pause(bool _state) -> void;
    auto genSweep(rp_channel_t _ch,bool _enable) -> void;
    auto setStartFreq(rp_channel_t _ch,float _freq) -> void;
    auto setStopFreq(rp_channel_t _ch,float _freq) -> void;
    auto setTime(rp_channel_t _ch,int _time) -> void;
    auto setMode(rp_channel_t _ch,rp_gen_sweep_mode_t _mode) -> void;
    auto setDir(rp_channel_t _ch,rp_gen_sweep_dir_t _dir) -> void;
    auto resetAll() -> void;
private:
    auto getNewFreq(Settings &_setting) -> double;
    auto reset(rp_channel_t _ch) -> void;
    auto loop() -> void;
    thread m_Thread;
    mutex mtx;
    atomic_flag m_ThreadRun = ATOMIC_FLAG_INIT;
    atomic_bool m_IsRun;
    atomic_bool m_Pause;
    Settings    m_Settings[2];
    time_point<system_clock> m_lastLoop; 
};