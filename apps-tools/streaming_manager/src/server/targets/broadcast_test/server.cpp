#include <iomanip>
#include <iostream>
#include <string>

#include "ServerNetConfigManager.h"

using namespace std;

std::string gen_random(const int len) {

    std::string tmp_s;
    static const char alphanum[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    srand( (unsigned) time(NULL) * getpid());

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];


    return tmp_s;

}

using namespace asionet_broadcast;
int main(int argc, char* argv[])
{
    auto s = gen_random(4);
    ServerNetConfigManager *server = new ServerNetConfigManager("json_server.conf",asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_MASTER, "127.0.0.1","9875");
    server->addHandler(ServerNetConfigManager::Events::CLIENT_CONNECTED, [](){
        std::cout << "CLIENT CONNECTED\n";
    });
    server->addHandler(ServerNetConfigManager::Events::CLIENT_DISCONNECTED, [](){
        std::cout << "CLIENT DISCONNECTED\n";
    });
    server->startBroadcast("127.0.0.1_" + s,"8902");
    std::cout << "PRESS\n";
    std::getchar();
    delete server;
    std::cout << "DONE\n";

    return 0;
}


