#include <iomanip>
#include <iostream>
#include <string>

#include "AsioBroadcastSocket.h"
#include "NetConfigManager.h"

using namespace asionet_broadcast;
int main(int argc, char* argv[])
{

    CAsioBroadcastSocket::Ptr server = CAsioBroadcastSocket::Create("127.0.0.1","9876");
    CAsioBroadcastSocket::Ptr client = CAsioBroadcastSocket::Create("127.0.0.1","9876");

    server->addHandler(CAsioBroadcastSocket::ABEvents::AB_ERROR,[](std::error_code er){
        std::cout << "Server ERROR: " << er.value() << " mes = " << er.message() << "\n";
    });

    client->addHandler(CAsioBroadcastSocket::ABEvents::AB_ERROR,[](std::error_code er){
        std::cout << "Client ERROR: " << er.value() << " mes = " << er.message() << "\n";
    });

    server->addHandler(CAsioBroadcastSocket::ABEvents::AB_SEND_DATA,[](std::error_code er,size_t size){
        std::cout << "Server sent (" << size << ") erron = " << er.value() << "\n";
    });

    client->addHandler(CAsioBroadcastSocket::ABEvents::AB_RECIVED_DATA,[](std::error_code er,uint8_t* buf,size_t size){
        std::cout << "Client get " << std::string((char*)buf,size).c_str() << " erron = " << er.value() << "\n";
    });

    server->InitServer(CAsioBroadcastSocket::ABMode::AB_SERVER_MASTER,1000);
    client->InitClient();
    std::cout << "PRESS\n";
    std::getchar();
    std::cout << "DONE\n";

    return 0;
}


