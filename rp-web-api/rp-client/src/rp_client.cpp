#include <DataManager.h>
#include <CustomParameters.h>
#include <unistd.h>
#include <inttypes.h>
#include <fstream>
#include <limits>
#include <chrono>
#include <string>
#include "rp.h"
#include "rp_client.h"

using namespace std;
using namespace std::chrono;

CUIntParameter   g_wc_ping("RP_CLIENT_PING", CBaseParameter::RO, 0, 0, 0, std::numeric_limits<uint32_t>::max());
CStringParameter g_wc_client_id("RP_CLIENT_ID",CBaseParameter::RW, "", 0);
CUIntParameter   g_wc_client_request("RP_CLIENT_REQUEST", CBaseParameter::RW, 0, 0, 0, std::numeric_limits<uint32_t>::max());

CIntParameter    g_signalPeriod("RP_SIGNAL_PERIOD", CBaseParameter::RW, 20, 0, 1, 10000);
CIntParameter    g_parameterPeriod("RP_PARAM_PERIOD", CBaseParameter::RW, 50, 0, 1, 10000);

time_point<system_clock> g_lastUpdateTime;
uint32_t g_interval = 1000;
bool     g_pause = false;

constexpr char file_path[] = "/usr/local/etc/client_id";


auto readClientId() -> std::string {
    std::string line;
    std::ifstream client(file_path);
    if (client.is_open()){
        if (getline (client,line)){
            return line;
        }
        client.close();
    }
    return "";
}

auto writeClientId(const std::string& id) -> bool  {
    std::ofstream client (file_path,std::ios::out | std::ios::trunc);
    if (client.is_open()){
        client << id;
        client.close();
        return true;
    }
    return false;
}

void rp_WC_Init(){
    auto id = readClientId();
    g_wc_client_id.Set(id);

    CDataManager::GetInstance()->SetParamInterval(g_parameterPeriod.Value());
    CDataManager::GetInstance()->SetSignalInterval(g_signalPeriod.Value());
}

void rp_WC_SetPingInterval(uint32_t ms){
    g_interval = ms;
}

void rp_WC_PauseSend(bool state){
    g_pause = state;
}

void rp_WC_UpdateParameters(bool force){

    if (g_pause) return;

    auto now = system_clock::now();
    auto curTime = time_point_cast<milliseconds>(now);

    auto lastUpdate = time_point_cast<milliseconds>(g_lastUpdateTime);
    auto diff = curTime - lastUpdate;

    if (diff.count() >= g_interval || force){
        auto ping = g_wc_ping.Value() + 1;
        g_wc_ping.SendValue(ping);
        g_lastUpdateTime = curTime;
    }
}


void rp_WC_OnNewParam(){
    if (g_wc_client_id.NewValue() != g_wc_client_id.Value()){
        auto old_value = g_wc_client_id.Value();
        auto new_value = g_wc_client_id.NewValue();
        if (writeClientId(new_value)){
            g_wc_client_id.Update();
        }
        g_wc_client_id.NeedSend(true);
    }

    if (g_wc_client_request.NewValue()){
        if (g_wc_client_request.Value() & RP_WC_REQUEST_ID){
            g_wc_client_id.NeedSend(true);
        }
        if (g_wc_client_request.Value() & RP_WC_DELETE_ID){
            if (writeClientId("")){
                g_wc_client_id.SendValue("");
            }
        }
        g_wc_client_request.SendValue(0);
    }

    if (g_signalPeriod.NewValue() != g_signalPeriod.Value()){
        g_signalPeriod.Update();
        CDataManager::GetInstance()->SetSignalInterval(g_signalPeriod.Value());
        WARNING("Set signal period %d ms",g_signalPeriod.Value())
    }
    if (g_parameterPeriod.NewValue() != g_parameterPeriod.Value()){
        g_parameterPeriod.Update();
        CDataManager::GetInstance()->SetParamInterval(g_parameterPeriod.Value());
        WARNING("Set parameter period %d ms",g_parameterPeriod.Value())
    }
}