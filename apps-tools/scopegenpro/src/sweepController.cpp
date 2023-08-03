#include "sweepController.h"

#include <iostream>
#include <fstream>
#include <functional>
#include <cstdlib>
#include <cmath>


CSweepController::CSweepController() :
    m_Thread(),
    mtx(),
    m_IsRun(false),
    m_Pause(false)
{
}

CSweepController::~CSweepController()
{
    stop();
}

void CSweepController::run()
{
    try {
        if (m_IsRun) return;
        m_IsRun = true;
        m_ThreadRun.test_and_set();
        m_lastLoop = std::chrono::system_clock::now();
        m_Thread = std::thread(&CSweepController::loop, this);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: CSweepController::run(), " << e.what() << std::endl;
    }
}

void CSweepController::stop(){
    
    m_ThreadRun.clear();
    if (m_Thread.joinable())
        m_Thread.join();
    m_IsRun = false;
}

double CSweepController::getNewFreq(Settings &_setting){
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

void CSweepController::loop()
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
                rp_GenTriggerOnly(RP_CH_1);
            }
        }
        m_lastLoop = std::chrono::system_clock::now();
        mtx.unlock();
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
    
}catch (std::exception& e)
	{
		std::cerr << "Error: CSweepController::loop() " << e.what() << std::endl ;
	}
}

void CSweepController::reset(rp_channel_t _ch){
    int index = _ch == RP_CH_1 ? 0 : 1;
    m_Settings[index].currentStep = 0;
    m_Settings[index].startTime = system_clock::now();
}

void CSweepController::genSweep(rp_channel_t _ch,bool _enable){
    lock_guard<std::mutex> lock(mtx);
    int index = _ch == RP_CH_1 ? 0 : 1;
    if (m_Settings[index].run != _enable){
        m_Settings[index].run = _enable;
        reset(_ch);
    }
}

void CSweepController::setStartFreq(rp_channel_t _ch,float _freq){
    if (_freq == 0) return;
    lock_guard<std::mutex> lock(mtx);
    int index = _ch == RP_CH_1 ? 0 : 1;
    if (m_Settings[index].startF != _freq){
        m_Settings[index].startF = _freq;
        reset(_ch);
    }
}

void CSweepController::setStopFreq(rp_channel_t _ch,float _freq){
    if (_freq == 0) return;
    lock_guard<std::mutex> lock(mtx);
    int index = _ch == RP_CH_1 ? 0 : 1;
    if (m_Settings[index].endF != _freq){
        m_Settings[index].endF = _freq;
        reset(_ch);
    }
}

void CSweepController::setTime(rp_channel_t _ch,int _time){
    if (_time == 0) return;
    lock_guard<std::mutex> lock(mtx);
    int index = _ch == RP_CH_1 ? 0 : 1;
    if (m_Settings[index].time != _time){
        m_Settings[index].time = _time;
        reset(_ch);
    }
}

void CSweepController::setMode(rp_channel_t _ch,rp_gen_sweep_mode_t _mode){
    lock_guard<std::mutex> lock(mtx);
    int index = _ch == RP_CH_1 ? 0 : 1;
    if (m_Settings[index].mode != _mode){
        m_Settings[index].mode = _mode;
        reset(_ch);
    }
}

void CSweepController::setDir(rp_channel_t _ch,rp_gen_sweep_dir_t _dir){
    lock_guard<std::mutex> lock(mtx);
    int index = _ch == RP_CH_1 ? 0 : 1;
    if (m_Settings[index].dir != _dir){
        m_Settings[index].dir = _dir;
        reset(_ch);
    }
}

void CSweepController::resetAll(){
    lock_guard<std::mutex> lock(mtx);
    reset(RP_CH_1);
    reset(RP_CH_2);
}

void CSweepController::pause(bool _state){
    m_Pause = _state;
} 

