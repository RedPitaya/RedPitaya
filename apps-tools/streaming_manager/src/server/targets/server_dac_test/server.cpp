#include <iomanip>
#include <iostream>

#include "UioParser.h"
#include "Oscilloscope.h"
#include "StreamingApplication.h"
#include "StreamingManager.h"
#include "DACStreamingApplication.h"
#include "DACStreamingManager.h"

#include "Generator.h"

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

    int dacSpeed = 125e6 / 4;
    if (argc > 1) {
        dacSpeed = atoi(argv[1]);
    }

    for (const UioT &uio : uioList)
    {

        if (uio.nodeName == "rp_dac")
        {
            gen = CGenerator::Create(uio, true , true, dacSpeed,125e6);
            gen->setDacHz(dacSpeed);
        }
    }

    if (!gen)
    {
        std::cerr << "Error: create gen" << std::endl;
        return 1;
    }



    CDACStreamingManager::Ptr dac_manager = CDACStreamingManager::Create(CDACStreamingManager::WAV_TYPE,"test.wav",CStreamSettings::DAC_REP_INF,1,10000000);
    CDACStreamingApplication dac(dac_manager,gen);

    dac.runNonBlock();
    while(stop == 0){
//        sleep(5);
//        gen->printReg();

    }
    std::cout << "STOP\n";
    dac.stop();

    return 0;
}


