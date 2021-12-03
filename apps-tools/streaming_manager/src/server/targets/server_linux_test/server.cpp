#include <iomanip>
#include <iostream>

#include "UioParser.h"
#include "Oscilloscope.h"
#include "Generator.h"
#include "StreamingApplication.h"
#include "StreamingManager.h"

int stop = 0;

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

    std::cerr << std::hex << std::noshowbase << std::setfill('0');

    for (const UioT &uio : uioList)
    {
        std::cerr << "name:\t" << uio.name << "\n"
                  << "node:\t" << uio.nodeName << "\n"
                  << "maps:" << std::endl;

        for (const UioMapT &uioMap : uio.mapList)
        {
            std::cerr << "\tname: " << uioMap.name
                      << ", addr: 0x" << std::setw(8) << uioMap.addr
                      << ", offset: 0x" << std::setw(8) << uioMap.offset
                      << ", size: 0x" << std::setw(8) << uioMap.size
                      << std::endl;
        }
    }

    std::cout.copyfmt(oldState);

    // Search oscilloscope
    COscilloscope::Ptr osc = nullptr;
    CGenerator::Ptr gen = nullptr;

    int Decimation = 1;
    if (argc > 1) {
        Decimation = atoi(argv[1]);
    }

    for (const UioT &uio : uioList)
    {
        if (uio.nodeName == "rp_oscilloscope")
        {
            osc = COscilloscope::Create(uio, true , true , Decimation,true);
        }

        if (uio.nodeName == "rp_dac")
        {
            gen = CGenerator::Create(uio, true , true, 125e6);
        }
    }

    // if (!osc0)
    // {
    //     std::cerr << "Error: create osc0" << std::endl;
    //     return 1;
    // }

    // if (!osc1)
    // {
    //     std::cerr << "Error: create osc1" << std::endl;
    //     return 1;
    // }

    size_t size = 1024 * 16;
    int16_t buf1[size];
    int16_t buf2[size];
    int x = 0;
    for(int i = 0 ; i< size ; i++){
      //  x = (i / 1024)+1;
        // (i / 1024) % 2 ? 8000 : 0;
        buf1[i]= (i / 1024) % 2 ? 8000 : 0;//(i / 16) %2 ? 4096 : 0; //i < 512 || i > size - 512 ? 4096 : 0;// i % ((1 << 14) - 500) + 500;
        buf2[i]= -2048;//i % 4096; //i % ((1 << 14) - 500) + 500;
    }
    if (gen){
        std::cerr << "\n";
        gen->printReg();
        std::cerr << "\n";
        gen->prepare();
        std::cerr << "\n";
        gen->printReg();
        bool firstBuf = true;
        std::cerr << "\nInit first and second buffers\n";
        gen->initFirst((uint8_t*)buf1,(uint8_t*)buf1,size * 2, size * 2);
        gen->initSecond((uint8_t*)buf2,(uint8_t*)buf2,size * 2, size * 2);
        std::cerr << "\n";
        gen->printReg();
        gen->start();
        std::cerr << "\n";
        //sleep(2);
        std::cerr << "Reg after start\n";        
        gen->printReg();
        while(stop == 0){
//            sleep(1);
            if (gen->write(firstBuf ? (uint8_t*)buf1 : (uint8_t*)buf2,nullptr, size * 2, size * 2)){
            //    gen->printReg();
            //    std::cerr << "\n";
//                std::cerr << firstBuf << "\n";
                firstBuf != firstBuf;
      //          return 0;
            }

        }
    }
  gen->printReg();
  std::cerr << "\n";
  //  CStreamingManager::Ptr s_manger = CStreamingManager::Create("127.0.0.1","8900",asionet::Protocol::TCP);

    // Run application
  //  CStreamingApplication app(s_manger,osc0, 16 , Decimation, 3, 0 , 16);
  //  app.run("");
    std::cerr << "End of programm\n";
    return 0;
}
