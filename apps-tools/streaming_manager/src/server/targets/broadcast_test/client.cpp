#include <iomanip>
#include <iostream>
#include <string>

#include "ClientNetConfigManager.h"

using namespace std;


using namespace asionet_broadcast;
int main(int argc, char* argv[])
{

    ClientNetConfigManager *client = new ClientNetConfigManager("");
    client->addHandler(ClientNetConfigManager::Events::BROADCAST_NEW_CLIENT, [&client](std::string host){
       auto list = client->getBroadcastClients();
       std::cout << "\nNEW CLIENTS " << host <<" \n";
       for(auto &item:list){
           std::cout << (item.mode == asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_MASTER ? "MASTER" : "SLAVE") << "\t" << item.host << "\t" << item.ts << "\n";
       }
    });

    client->addHandler(ClientNetConfigManager::Events::SERVER_CONNECTED, [&client](std::string host){
        std::cout << "\nCONNECTED TO SERVER " << host << "\n";
        client->requestConfig(host);
    });

    client->addHandler(ClientNetConfigManager::Events::GET_NEW_SETTING,[&client](std::string host){
        CStreamSettings* s = client->getLocalSettingsOfHost(host);
        if (s != nullptr){
            std::cout << "Save config\n";
            s->writeToFile("./new_jost.conf");
        }
    });

    client->addHandlerError([](ClientNetConfigManager::Errors errors,std::string host){
        if (errors == ClientNetConfigManager::Errors::SERVER_INTERNAL)
            std::cout << "Error: " << host.c_str() << "\n";
        if (errors == ClientNetConfigManager::Errors::CONNECT_TIMEOUT)
            std::cout << "Timout: " << host.c_str() << "\n";

    });


    std::vector<std::string> list;
    list.push_back("127.0.0.2");
    list.push_back("128.0.0.2");
    list.push_back("129.0.0.2");
    list.push_back("130.0.0.2");
    list.push_back("131.0.0.2");
    //client->connectToServers(list,"9875");
    client->startBroadcast("127.0.0.1","8902");
    std::cout << "PRESS\n";
    std::getchar();
    delete client;
    std::cout << "DONE\n";

    return 0;
}


