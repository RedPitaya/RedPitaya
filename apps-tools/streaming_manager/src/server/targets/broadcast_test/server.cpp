#include <iomanip>
#include <iostream>
#include <string>

#include "AsioBroadcastSocket.h"
#include "NetConfigManager.h"

using namespace asionet_broadcast;
int main(int argc, char* argv[])
{

    CAsioBroadcastSocket server("127.0.0.1","9876");
    CAsioBroadcastSocket client("127.0.0.1","9876");

    server.addHandler(asionet_broadcast::ERROR,[](error_code er){
        cout << "Server ERROR: " << er.value() << " mes = " << er.message() << "\n";
    });

    client.addHandler(asionet_broadcast::ERROR,[](error_code er){
        cout << "Client ERROR: " << er.value() << " mes = " << er.message() << "\n";
    });

    server.addHandler(asionet_broadcast::SEND_DATA,[](error_code er,size_t size){
        cout << "Server sent (" << size << ") erron = " << er.value() << "\n";
    });

    client.addHandler(asionet_broadcast::RECIVED_DATA,[](error_code er,uint8_t* buf,size_t size){
        cout << "Client get " << std::string((char*)buf,size).c_str() << " erron = " << er.value() << "\n";
    });

    server.InitServer(SERVER_MASTER,1000);
    client.InitClient();
    cout << "PRESS\n";
    getchar();
    cout << "DONE\n";

    return 0;
}


