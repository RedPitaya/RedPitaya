#include <iomanip>
#include <iostream>
#include <string>

#include "DACAsioNetController.h"

using namespace std;


int main(int argc, char* argv[])
{
    CDACAsioNetController *server = new CDACAsioNetController();
    server->addHandler(CDACAsioNetController::Events::CONNECTED, [server](std::string host){
        std::cout << "CLIENT CONNECTED " << host << "\n" ;
        for(int i = 0 ; i < 200; i++){

            uint8_t *buf = new uint8_t [ 32 * 1024];
            bool state = server->sendBuffer(buf,32 * 1024, nullptr,0);
            std::cout << "SEND BUFF " << i << " state " <<state  << "\n" ;
            delete[] buf;
            if (!state) break;
        }
    });
    server->addHandler(CDACAsioNetController::Events::DISCONNECTED, [](std::string host){
        std::cout << "CLIENT DISCONNECTED " << host << "\n" ;
    });
    server->startAsioNet(asionet_simple::CAsioSocketSimple::ASMode::AS_CLIENT,"127.0.0.1","23121");
    std::cout << "PRESS\n";
    std::getchar();
    delete server;
    std::cout << "DONE\n";

    return 0;
}


