#include <DataManager.h>
#include <CustomParameters.h>
#include <rp_system.h>
#include <map>
#include <fstream>
#include <iostream>
#include <numeric>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <sstream>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <inttypes.h>
#include "rp.h"
#include "rp_hw.h"

using namespace std;
using namespace std::chrono;

CFloatParameter g_ws_cpuLoad("RP_SYSTEM_CPU_LOAD", CBaseParameter::RO, 0, 0, 0, 1000);
CFloatParameter g_ws_memoryTotal("RP_SYSTEM_TOTAL_RAM", CBaseParameter::RO, 0, 0, 0, 1e15);
CFloatParameter g_ws_memoryFree ("RP_SYSTEM_FREE_RAM", CBaseParameter::RO, 0, 0, 0, 1e15);
CFloatParameter g_ws_temperature ("RP_SYSTEM_TEMPERATURE", CBaseParameter::RO, -100, 0, -100, 1000);
CFloatParameter g_ws_slow_adc0 ("RP_SYSTEM_SLOW_ADC0", CBaseParameter::RO, 0, 0, -20, 20);
CFloatParameter g_ws_slow_adc1 ("RP_SYSTEM_SLOW_ADC1", CBaseParameter::RO, 0, 0, -20, 20);
CFloatParameter g_ws_slow_adc2 ("RP_SYSTEM_SLOW_ADC2", CBaseParameter::RO, 0, 0, -20, 20);
CFloatParameter g_ws_slow_adc3 ("RP_SYSTEM_SLOW_ADC3", CBaseParameter::RO, 0, 0, -20, 20);

CFloatParameter g_ws_vcc_pint ("RP_SYSTEM_VCC_PINT", CBaseParameter::RO, 0, 0, -20, 20);
CFloatParameter g_ws_vcc_paux ("RP_SYSTEM_VCC_PAUX", CBaseParameter::RO, 0, 0, -20, 20);
CFloatParameter g_ws_vcc_bram ("RP_SYSTEM_VCC_BRAM", CBaseParameter::RO, 0, 0, -20, 20);

CFloatParameter g_ws_vcc_int ("RP_SYSTEM_VCC_INT", CBaseParameter::RO, 0, 0, -20, 20);
CFloatParameter g_ws_vcc_aux ("RP_SYSTEM_VCC_AUX", CBaseParameter::RO, 0, 0, -20, 20);
CFloatParameter g_ws_vcc_ddr ("RP_SYSTEM_VCC_DDR", CBaseParameter::RO, 0, 0, -20, 20);

CFloatParameter g_ws_totalSD ("RP_SYSTEM_TOTAL_SD", CBaseParameter::RO, 0, 0, 0, 1e15);
CFloatParameter g_ws_freeSD ("RP_SYSTEM_FREE_SD", CBaseParameter::RO, 0, 0, 0, 1e15);

map<rp_system_mode_t,uint32_t> g_intervals;
map<rp_system_mode_t,time_point<system_clock>> g_lastUpdateTime;

uint32_t g_modes = RP_WS_NONE;
uint64_t prev_cpu_time = 0;
uint64_t prev_cpu_idle = 0;
bool     g_pause = false;

bool get_cpu_usage(unsigned long long* total_cpu_time,
                          unsigned long long* idle_cpu_time) {
    std::ifstream proc_stat("/proc/stat");
    proc_stat.ignore(5);
    std::string cpu_time_str;
    std::string first_line;
    std::getline(proc_stat, first_line);
    std::stringstream first_line_s(first_line);
    for (int i = 0; i < 10; ++i) {
        std::getline(first_line_s, cpu_time_str, ' ');
        *total_cpu_time += std::stol(cpu_time_str);
        if (i == 3) {
            *idle_cpu_time = std::stol(cpu_time_str);
        }
    }
    return true;
}

bool get_ram(float *_total, float *_freeram){
    struct sysinfo memInfo;
    int ret = sysinfo (&memInfo);
    *_total = (float)memInfo.totalram;
    *_freeram = (float)memInfo.freeram;
    return !ret;
}

auto availableSpace(std::string dst,  uint64_t* availableSize, uint64_t* fullSize) -> int {
    int result = -1;
    try {
        struct statvfs devData;
        memset(&devData, 0, sizeof (struct statvfs));
        if ((statvfs(dst.c_str(), &devData)) >= 0) {
            //I don't know if it's right, but I'll set availableSize only if the available size doesn't pass the ulong limit.
            if (devData.f_bavail  > (std::numeric_limits<uint64_t>::max() / devData.f_bsize)) {
                *availableSize = std::numeric_limits<uint64_t>::max();
            } else {
                *availableSize = (uint64_t)devData.f_bavail * (uint64_t)devData.f_bsize;
            }

            if (devData.f_blocks  > (std::numeric_limits<uint64_t>::max() / devData.f_bsize)) {
                *fullSize = std::numeric_limits<uint64_t>::max();
            } else {
                *fullSize = (uint64_t)devData.f_blocks * (uint64_t)devData.f_bsize;
            }
            result = 0;
        }
    }
    catch (std::exception& e)
    {
        WARNING("Error in AvailableSpace(): %s\n",e.what());
    }
    return result;
}

void rp_WS_Init(){
    g_modes = RP_WS_ALL;
    auto now = system_clock::now();
    auto defInterval = 1000;
    g_intervals[RP_WS_CPU] = defInterval;
    g_intervals[RP_WS_RAM] = defInterval;
    g_intervals[RP_WS_TEMPERATURE] = defInterval;
    g_intervals[RP_WS_SLOW_DAC] = defInterval;
    g_intervals[RP_WS_SENSOR_VOLT] = defInterval;
    g_intervals[RP_WS_DISK_SIZE] = defInterval;

    g_lastUpdateTime[RP_WS_CPU] = now;
    g_lastUpdateTime[RP_WS_RAM] = now;
    g_lastUpdateTime[RP_WS_TEMPERATURE] = now;
    g_lastUpdateTime[RP_WS_SLOW_DAC] = now;
    g_lastUpdateTime[RP_WS_SENSOR_VOLT] = now;
    g_lastUpdateTime[RP_WS_DISK_SIZE] = now;

}

void rp_WS_PauseSend(bool state){
    g_pause = state;
}

void rp_WS_SetMode(rp_system_mode_t mode){
    g_modes = mode;
}

void rp_WS_SetInterval(rp_system_mode_t mode,uint32_t ms){
    g_intervals[mode] = ms;
}

void rp_WS_UpdateParameters(bool force){
    if (g_pause) return;
    auto now = system_clock::now();
    auto curTime = time_point_cast<milliseconds>(now);
    if (g_modes & RP_WS_CPU){
        uint64_t idle_time=0, total_time =0;
        if (get_cpu_usage(&total_time, &idle_time)){
            auto lastUpdate = time_point_cast<milliseconds>(g_lastUpdateTime[RP_WS_CPU]);
            auto diff = curTime - lastUpdate;
            if (diff.count() >= g_intervals[RP_WS_CPU] || force){
                auto total_time2 =  total_time - prev_cpu_time;
                prev_cpu_time = total_time;
                auto idle_time2 = idle_time - prev_cpu_idle;
                prev_cpu_idle = idle_time;
                const float utilization = ((total_time2 - idle_time2) * 100.0) / total_time2;
                g_ws_cpuLoad.SendValue(utilization);
                g_lastUpdateTime[RP_WS_CPU] = curTime;
            }
        }
    }

    if (g_modes & RP_WS_RAM){
        float total = 0, freeram = 0;
        if (get_ram(&total, &freeram)){
            auto lastUpdate = time_point_cast<milliseconds>(g_lastUpdateTime[RP_WS_RAM]);
            auto diff = curTime - lastUpdate;

            if (diff.count() >= g_intervals[RP_WS_RAM] || force){
                g_ws_memoryTotal.SendValue(total);
                g_ws_memoryFree.SendValue(freeram);
                g_lastUpdateTime[RP_WS_RAM] = curTime;
            }
        }
    }

    if (g_modes & RP_WS_TEMPERATURE){
        float temp = 0;
        uint32_t raw = 0;
        temp = rp_GetCPUTemperature(&raw);
        if (temp != -1){
            auto lastUpdate = time_point_cast<milliseconds>(g_lastUpdateTime[RP_WS_TEMPERATURE]);
            auto diff = curTime - lastUpdate;

            if (diff.count() >= g_intervals[RP_WS_TEMPERATURE] || force){
                g_ws_temperature.SendValue(temp);
                g_lastUpdateTime[RP_WS_TEMPERATURE] = curTime;
            }
        }
    }

    if (g_modes & RP_WS_DISK_SIZE){
        uint64_t freeSize = 0;
        uint64_t totalSize = 0;
        int ret = availableSpace("/",&freeSize,&totalSize);
        if (ret == 0){
            auto lastUpdate = time_point_cast<milliseconds>(g_lastUpdateTime[RP_WS_DISK_SIZE]);
            auto diff = curTime - lastUpdate;

            if (diff.count() >= g_intervals[RP_WS_DISK_SIZE] || force){
                g_ws_totalSD.SendValue(totalSize);
                g_ws_freeSD.SendValue(freeSize);
                g_lastUpdateTime[RP_WS_DISK_SIZE] = curTime;
            }
        }else{
            WARNING("Error get free space")
        }
    }

    if (g_modes & RP_WS_SLOW_DAC){
        float value[4] = {0,0,0,0};
        uint32_t raw[4] = {0,0,0,0};
        auto ret = rp_ApinGetValue(RP_AIN0,&value[0],&raw[0]);
        ret |= rp_ApinGetValue(RP_AIN1,&value[1],&raw[1]);
        ret |= rp_ApinGetValue(RP_AIN2,&value[2],&raw[2]);
        ret |= rp_ApinGetValue(RP_AIN3,&value[3],&raw[3]);
        if (ret == RP_OK){
            auto lastUpdate = time_point_cast<milliseconds>(g_lastUpdateTime[RP_WS_SLOW_DAC]);
            auto diff = curTime - lastUpdate;

            if (diff.count() >= g_intervals[RP_WS_SLOW_DAC] || force){
                g_ws_slow_adc0.SendValue(value[0]);
                g_ws_slow_adc1.SendValue(value[1]);
                g_ws_slow_adc2.SendValue(value[2]);
                g_ws_slow_adc3.SendValue(value[3]);
                g_lastUpdateTime[RP_WS_SLOW_DAC] = curTime;
            }
        }
    }

     if (g_modes & RP_WS_SENSOR_VOLT){
        float value[6] = {0,0,0,0,0,0};
        uint32_t raw[6] = {0,0,0,0,0,0};
        auto ret = rp_GetPowerVCCPINT(&raw[0],&value[0]);
        ret |= rp_GetPowerVCCPAUX(&raw[1],&value[1]);
        ret |= rp_GetPowerVCCBRAM(&raw[2],&value[2]);
        ret |= rp_GetPowerVCCINT(&raw[3],&value[3]);
        ret |= rp_GetPowerVCCAUX(&raw[4],&value[4]);
        ret |= rp_GetPowerVCCDDR(&raw[5],&value[5]);

        if (ret == RP_OK){
            auto lastUpdate = time_point_cast<milliseconds>(g_lastUpdateTime[RP_WS_SENSOR_VOLT]);
            auto diff = curTime - lastUpdate;

            if (diff.count() >= g_intervals[RP_WS_SENSOR_VOLT] || force){
                g_ws_vcc_pint.SendValue(value[0]);
                g_ws_vcc_paux.SendValue(value[1]);
                g_ws_vcc_bram.SendValue(value[2]);
                g_ws_vcc_int.SendValue(value[3]);
                g_ws_vcc_aux.SendValue(value[4]);
                g_ws_vcc_ddr.SendValue(value[5]);
                g_lastUpdateTime[RP_WS_SENSOR_VOLT] = curTime;
            }
        }
    }

}
