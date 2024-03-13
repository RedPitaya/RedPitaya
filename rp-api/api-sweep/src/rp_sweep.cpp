#include "rp_sweep.h"

#include <iostream>
#include <fstream>
#include <functional>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include "rp_hw-profiles.h"

using namespace rp_sweep_api;
using namespace std;
using namespace std::chrono;

auto getDACChannels() -> uint8_t{
    uint8_t c = 0;

    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        ERROR("Can't get fast DAC channels count")
    }
    return c;
}

uint8_t g_sweep_dac_max_channels = getDACChannels();

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

struct CSweepController::Impl {
    auto getNewFreq(Settings &_setting) -> double;
    auto reset(rp_channel_t _ch, time_point<system_clock> _tp) -> int;
    auto loop() -> void;
    thread m_Thread;
    mutex mtx;
    mutex mtx_run;
    atomic_flag m_ThreadRun = ATOMIC_FLAG_INIT;
    atomic_bool m_IsRun;
    atomic_bool m_Pause;
    Settings    m_Settings[RP_CH_4 + 1];
    time_point<system_clock> m_lastLoop;
};

CSweepController::CSweepController()
{
    m_pimpl = new Impl();
    m_pimpl->m_IsRun = false;
    m_pimpl->m_Pause = false;
}

CSweepController::~CSweepController(){
    stop();
    delete m_pimpl;
}


int convertIndex(rp_channel_t _ch){
    if (_ch < g_sweep_dac_max_channels){
        switch (_ch)
        {
            case RP_CH_1:
                return 0;
            case RP_CH_2:
                return 1;
            case RP_CH_3:
                return 2;
            case RP_CH_4:
                return 3;
            default:
                break;
        }
    }
    ERROR("Unsupported generator channel %d",_ch)
    return -1;
}

void CSweepController::run(){
    lock_guard<std::mutex> lock(m_pimpl->mtx_run);
    try {
        if (m_pimpl->m_IsRun) return;
        m_pimpl->m_IsRun = true;
        m_pimpl->m_ThreadRun.test_and_set();
        m_pimpl->m_lastLoop = std::chrono::system_clock::now();
        m_pimpl->m_Thread = std::thread(&CSweepController::Impl::loop, this->m_pimpl);
    }
    catch (const std::exception& e){
        ERROR("Thread cannot be started %s",e.what())
    }

}

void CSweepController::stop(){
    lock_guard<std::mutex> lock(m_pimpl->mtx_run);
    m_pimpl->m_ThreadRun.clear();
    if (m_pimpl->m_Thread.joinable())
        m_pimpl->m_Thread.join();
    m_pimpl->m_IsRun = false;
}

double CSweepController::Impl::getNewFreq(Settings &_setting){
    if (_setting.run == false) return -1;
    double start = min(_setting.startF,_setting.endF);
    double stop  = max(_setting.startF,_setting.endF);
    bool invert = _setting.startF > _setting.endF;

    auto timeNow = system_clock::now();
    auto curTime = time_point_cast<microseconds>(timeNow);
    auto startTime = time_point_cast<microseconds>(_setting.startTime);
    uint64_t time = curTime.time_since_epoch().count() - startTime.time_since_epoch().count();
    uint64_t cycles = time / _setting.time;
    double  x = static_cast<double>(time - cycles * _setting.time)/static_cast<double>(_setting.time);
    double freq = 0;

    if (invert) x = 1 - x;
    if (cycles % 2 != 0 && _setting.dir == RP_GEN_SWEEP_DIR_UP_DOWN) x = 1 - x;

    if (_setting.mode == RP_GEN_SWEEP_MODE_LINEAR){
        freq = ((stop - start) * x + start);
    }

    if (_setting.mode == RP_GEN_SWEEP_MODE_LOG){
        freq = start  * exp(x *log(stop/start));
    }

    return freq;
}

void CSweepController::Impl::loop()
{
    try{
        while (m_ThreadRun.test_and_set())
        {
            mtx.lock();
            if (m_Pause){
                auto curTime = time_point_cast<microseconds>(system_clock::now());
                auto lastTime = time_point_cast<microseconds>(m_lastLoop);
                auto time = curTime - lastTime;
                m_Settings[0].startTime += time;
                m_Settings[1].startTime += time;
            }else{
                auto freq1 = getNewFreq(m_Settings[0]);
                auto freq2 = getNewFreq(m_Settings[1]);
                if (freq1 > 0){
                    rp_GenFreqDirect(RP_CH_1,freq1);
                    rp_GenTriggerOnly(RP_CH_1);
                }
                if (freq2 > 0){
                    rp_GenFreqDirect(RP_CH_2,freq2);
                    rp_GenTriggerOnly(RP_CH_2);
                }
            }
            m_lastLoop = std::chrono::system_clock::now();
            mtx.unlock();
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }

    }catch (std::exception& e)
	{
		ERROR("Runtime error %s",e.what())
	}
}

int CSweepController::Impl::reset(rp_channel_t _ch, time_point<system_clock> _tp){
    int index =  convertIndex(_ch);
    if (index >= 0){
        m_Settings[index].currentStep = 0;
        m_Settings[index].startTime = _tp;
        return RP_OK;
    }
    return RP_EOOR;
}

int CSweepController::genSweep(rp_channel_t _ch,bool _enable){
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index < 0) return RP_EOOR;
    if (m_pimpl->m_Settings[index].run != _enable){
        m_pimpl->m_Settings[index].run = _enable;
        return m_pimpl->reset(_ch,system_clock::now());
    }
    return RP_OK;
}

auto CSweepController::isGen(rp_channel_t _ch,bool *state) -> int{
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index >= 0){
        *state = m_pimpl->m_Settings[index].run;
        return RP_OK;
    }
    return RP_EOOR;
}


int CSweepController::setStartFreq(rp_channel_t _ch,float _freq){
    if (_freq == 0) {
        ERROR("Frequency cannot be zero")
        return RP_EOOR;
    }
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index < 0) return RP_EOOR;
    if (m_pimpl->m_Settings[index].startF != _freq){
        m_pimpl->m_Settings[index].startF = _freq;
        return m_pimpl->reset(_ch,system_clock::now());
    }
    return RP_OK;
}

int CSweepController::setStopFreq(rp_channel_t _ch,float _freq){
    if (_freq == 0) {
        ERROR("Frequency cannot be zero")
        return RP_EOOR;
    }
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index < 0) return RP_EOOR;
    if (m_pimpl->m_Settings[index].endF != _freq){
        m_pimpl->m_Settings[index].endF = _freq;
        return m_pimpl->reset(_ch,system_clock::now());
    }
    return RP_OK;
}

auto CSweepController::getStartFreq(rp_channel_t _ch,float *_freq) -> int{
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index >= 0){
        *_freq = m_pimpl->m_Settings[index].startF;
        return RP_OK;
    }
    return RP_EOOR;
}

auto CSweepController::getStopFreq(rp_channel_t _ch,float *_freq) -> int{
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index >= 0){
        *_freq = m_pimpl->m_Settings[index].endF;
        return RP_OK;
    }
    return RP_EOOR;
}

int CSweepController::setTime(rp_channel_t _ch,int _time){
    if (_time == 0) {
        ERROR("Time cannot be zero")
        return RP_EOOR;
    }
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index < 0) return RP_EOOR;
    if (m_pimpl->m_Settings[index].time != _time){
        m_pimpl->m_Settings[index].time = _time;
        return m_pimpl->reset(_ch,system_clock::now());
    }
    return RP_OK;
}

auto CSweepController::getTime(rp_channel_t _ch,int *_us) -> int{
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index >= 0){
        *_us = m_pimpl->m_Settings[index].time;
        return RP_OK;
    }
    return RP_EOOR;
}

int CSweepController::setMode(rp_channel_t _ch,rp_gen_sweep_mode_t _mode){
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index < 0) return RP_EOOR;
    if (m_pimpl->m_Settings[index].mode != _mode){
        m_pimpl->m_Settings[index].mode = _mode;
        return m_pimpl->reset(_ch,system_clock::now());
    }
    return RP_OK;
}

auto CSweepController::getMode(rp_channel_t _ch,rp_gen_sweep_mode_t *_mode) -> int{
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index >= 0){
        *_mode = m_pimpl->m_Settings[index].mode;
        return RP_OK;
    }
    return RP_EOOR;
}

int CSweepController::setDir(rp_channel_t _ch,rp_gen_sweep_dir_t _dir){
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index < 0) return RP_EOOR;
    if (m_pimpl->m_Settings[index].dir != _dir){
        m_pimpl->m_Settings[index].dir = _dir;
        return m_pimpl->reset(_ch,system_clock::now());
    }
    return RP_OK;
}

auto CSweepController::getDir(rp_channel_t _ch,rp_gen_sweep_dir_t *_dir) -> int{
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    int index =  convertIndex(_ch);
    if (index >= 0){
        *_dir = m_pimpl->m_Settings[index].dir;
        return RP_OK;
    }
    return RP_EOOR;
}

void CSweepController::resetAll(){
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    auto tp = system_clock::now();
    for(int i = RP_CH_1 ; i <= RP_CH_4 && i < g_sweep_dac_max_channels; i++){
        m_pimpl->reset((rp_channel_t)i,tp);
    }
}

auto CSweepController::isAllDisabled() -> bool{
    lock_guard<std::mutex> lock(m_pimpl->mtx);
    for(int i = RP_CH_1 ; i <= RP_CH_4; i++){
        if (m_pimpl->m_Settings[i].run) return false;
    }
    return true;
}

void CSweepController::pause(bool _state){
    m_pimpl->m_Pause = _state;
}

