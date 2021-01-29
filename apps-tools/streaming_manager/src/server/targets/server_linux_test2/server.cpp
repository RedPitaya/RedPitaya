#include <iomanip>
#include <iostream>

#include "rpsa/server/core/UioParser.h"
#include "rpsa/server/core/Oscilloscope.h"
#include "rpsa/server/core/StreamingApplication.h"
#include "rpsa/server/core/StreamingManager.h"


// void sigHandler (int sigNum){
//     g_manger->stop();
//     g_asionet->Stop();
//     g_terminate = true;
// }


int main(int argc, char* argv[])
{
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
    COscilloscope::Ptr osc1 = nullptr;
    int Decimation = 16;
    if (argc > 1) {
        Decimation = atoi(argv[1]);
    }

    for (const UioT &uio : uioList)
    {
        if (uio.nodeName == "rp_oscilloscope")
        {
            // TODO start server;
            osc0 = COscilloscope::Create(uio, true , true , Decimation);
            osc0->setCalibration(0,1,0,1);
            osc0->setFilterBypass(true);
 //           osc1 = COscilloscope::Create(uio, 1 , 1);
            break;
        }
    }

    if (!osc0)
    {
        std::cerr << "Error: create osc0" << std::endl;
        return 1;
    }


    CStreamingManager::Ptr s_manger = nullptr;
    s_manger = CStreamingManager::Create(Stream_FileType::CSV_TYPE , ".", 100000,false);


    // Run application
    CStreamingApplication app(s_manger,osc0, 16 , Decimation, 3, 0 , 16);

    app.run();

    return 0;
}


