//#include <DataManager.h>
//#include <CustomParameters.h>
#include <unistd.h>
#include <inttypes.h>
#include <fstream>
#include <limits>
#include <chrono>
#include <string>
#include "rp.h"
#include "rp_client.h"
#include "web/rp_websocket.h"
#include "timer.hpp"

using namespace std;
using namespace std::chrono;

uint32_t    g_wc_ping = 0;
std::string g_wc_client_id = "";
uint32_t    g_wc_client_request = 0;
uint32_t    g_parameterPeriod = 0;

rp_websocket::CWEBServer::Ptr g_websocket_server = nullptr;
std::unique_ptr<Timer> g_timer = nullptr;

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

auto writeClientId(const std::string_view id) -> bool  {
    std::ofstream client (file_path,std::ios::out | std::ios::trunc);
    if (client.is_open()){
        client << id;
        client.close();
        return true;
    }
    return false;
}

auto restartServer() -> void {
    g_websocket_server = std::make_shared<rp_websocket::CWEBServer>();
    g_websocket_server->receiveStr.connect([](auto key,auto value){
        if (key == "RP_CLIENT_ID"){
            if (value != g_wc_client_id){
                auto old_value = g_wc_client_id;
                if (writeClientId(value)){
                    g_wc_client_id = value;
                }
                g_websocket_server->send("RP_CLIENT_ID",g_wc_client_id);
            }
        }
    });

    g_websocket_server->receiveInt.connect([](auto key,auto value){
        if (key == "RP_CLIENT_REQUEST"){
            if (value & RP_WC_REQUEST_ID){
                g_websocket_server->sendRequest("RP_CLIENT_ID",g_wc_client_id);
            }
            if (value & RP_WC_DELETE_ID){
                if (writeClientId("")){
                    g_wc_client_id = "";
                    g_websocket_server->sendRequest("RP_CLIENT_ID",g_wc_client_id);
                }
            }
            g_websocket_server->sendRequest("RP_CLIENT_REQUEST",0);
            g_websocket_server->sendCache();
        }
    });

    g_websocket_server->startServer(WEB_CLIENT_PORT);

}

void rp_WC_Init(){
    g_websocket_server = std::make_shared<rp_websocket::CWEBServer>();
    restartServer();
    auto id = readClientId();
    g_wc_client_id = id;
    rp_WC_SetPingInterval(g_interval);
}

void rp_WC_UpdateParameters(){

    if (g_pause) return;

    auto now = system_clock::now();
    auto curTime = time_point_cast<milliseconds>(now);

    auto lastUpdate = time_point_cast<milliseconds>(g_lastUpdateTime);
    auto diff = curTime - lastUpdate;

    if (diff.count() >= g_interval){
        g_wc_ping++;
        if (g_websocket_server){
            if (!g_websocket_server->send("ping",g_wc_ping)){
                ERROR_LOG("Fail send ping")
            }
        }
        g_lastUpdateTime = curTime;
    }
}

void rp_WC_SetPingInterval(uint32_t ms){
    g_interval = ms;
    g_timer = std::make_unique<Timer>();
    g_timer->setInterval([](){
        rp_WC_UpdateParameters();
    }, g_interval);
}

void rp_WC_PauseSend(bool state){
    g_pause = state;
}