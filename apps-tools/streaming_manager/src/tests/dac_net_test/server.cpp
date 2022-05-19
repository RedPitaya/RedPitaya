#include <iomanip>
#include <iostream>
#include <string>

#include "DACAsioNetController.h"

using namespace std;


int main(int argc, char* argv[])
{
    CDACAsioNetController *server = new CDACAsioNetController();
    server->addHandler(CDACAsioNetController::Events::CONNECTED, [](std::string host){
        std::cout << "CLIENT CONNECTED " << host << "\n" ;
    });
    server->addHandler(CDACAsioNetController::Events::DISCONNECTED, [](std::string host){
        std::cout << "CLIENT DISCONNECTED " << host << "\n" ;
    });
    server->setReceivedBufferLimit(3);
    server->startAsioNet(asionet_simple::CAsioSocketSimple::ASMode::AS_SERVER,"127.0.0.1","23121");
    std::cout << "PRESS\n";
    std::getchar();
    while(true){
        sleep(1);
//        std::getchar();
        auto b = server->getBuffer();
        if (b.empty) break;
    }
    delete server;
    std::cout << "DONE\n";

    return 0;
}


