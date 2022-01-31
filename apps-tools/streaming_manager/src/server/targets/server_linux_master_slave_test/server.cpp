#include <iomanip>
#include <iostream>

#include "UioParser.h"
#include "Oscilloscope.h"
#include "StreamingApplication.h"
#include "StreamingManager.h"
#include "DACStreamingApplication.h"
#include "DACStreamingManager.h"

#include "Generator.h"

#ifndef SLAVE
#define MASTER true
#else
#define MASTER false
#endif

volatile int stop = 0;

void sigHandler (int sigNum){
    stop = 1;
}

void installTermSignalHandler()
{
#ifdef _WIN32
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
#else
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = sigHandler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
#endif
}


int main(int argc, char* argv[])
{
    installTermSignalHandler();
    std::vector<UioT> uioList = GetUioList();

    // Print UIOs
    std::ios oldState(nullptr);
    oldState.copyfmt(std::cout);

    std::cout << std::hex << std::noshowbase << std::setfill('0');

    for (const UioT &uio : uioList)
    {
        std::cout << "name:\t" << uio.name << "\n"
                  << "node:\t" << uio.nodeName << "\n"
                  << "maps:" << std::endl;

        for (const UioMapT &uioMap : uio.mapList)
        {
            std::cout << "\tname: " << uioMap.name
                      << ", addr: 0x" << std::setw(8) << uioMap.addr
                      << ", offset: 0x" << std::setw(8) << uioMap.offset
                      << ", size: 0x" << std::setw(8) << uioMap.size
                      << std::endl;
        }
    }

    std::cout.copyfmt(oldState);

    // Search oscilloscope
    COscilloscope::Ptr osc0 = nullptr;
    CGenerator::Ptr gen = nullptr;

    int Decimation = 16;
    if (argc > 1) {
        Decimation = atoi(argv[1]);
    }

    for (const UioT &uio : uioList)
    {
        if (uio.nodeName == "rp_oscilloscope")
        {
            // TODO start server;
            osc0 = COscilloscope::Create(uio, true , true , Decimation,MASTER);
            osc0->setCalibration(0,1,0,1);
            osc0->setFilterBypass(true);
        }

        if (uio.nodeName == "rp_dac")
        {
            gen = CGenerator::Create(uio, true , true, 125e6, 125e6);
        }
    }

    if (!osc0)
    {
        std::cerr << "Error: create osc0" << std::endl;
        return 1;
    }



    CStreamingManager::Ptr s_manger = nullptr;
    s_manger = CStreamingManager::Create(Stream_FileType::WAV_TYPE , "./data", 200000,false,true);
    s_manger->notifyStop = [](int status)
                        {
                            stop = 1;
                        };

    // Run application
    CStreamingApplication app(s_manger,osc0, 16 , Decimation, 3, 0 , 16);
    app.runNonBlock("");
    // CDACStreamingManager::Ptr dac_manager = CDACStreamingManager::Create("127.0.0.1","12345");
    // CDACStreamingApplication dac(dac_manager,gen);

    // dac.runNonBlock();

    while(stop == 0){
    }
    app.stop();
    s_manger = nullptr;
    std::cout << "STOP\n";

    // dac.stop();

    return 0;
}


