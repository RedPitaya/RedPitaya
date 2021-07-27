#include "search.h"

auto startSearch(ClientOpt::Optons &option) -> void{
    ClientNetConfigManager client("",false);
    client.startBroadcast("127.0.0.1",option.port != "" ? option.port : "8902");
    int timout = option.timeout;

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds >(timeNow);
    auto value = curTime.time_since_epoch();
    auto timeBegin = value.count();
    int point = 1;
    while ((value.count() - timeBegin) < timout * 1000) {
        std::cout << "\rSearch ";
        for(int i = 0 ; i < 5 ; i++) {
            if (i < point)
                std::cout << ".";
            else
                std::cout << " ";
        }
        std::cout << std::flush;
        point++;
        if (point > 4) point = 0;
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif // _WIN32
        value = std::chrono::time_point_cast<std::chrono::milliseconds >(std::chrono::system_clock::now()).time_since_epoch();
    }
    std::cout << "\rSearch: DONE";
    auto list = client.getBroadcastClients();
    std::cout << "\nFound boards:\n";
    int i = 1;
    for(auto &item:list){
        std::string model = "";
        switch (item.model) {
            case asionet_broadcast::CAsioBroadcastSocket::Model::RP_125_14:
                model = "STEAMlab 125-14";
                break;
            case asionet_broadcast::CAsioBroadcastSocket::Model::RP_125_14_Z20:
                model = "STEAMlab 125-14/Z7020";
                break;
            case asionet_broadcast::CAsioBroadcastSocket::Model::RP_122_16:
                model = "SDRlab 122-16";
                break;
            case asionet_broadcast::CAsioBroadcastSocket::Model::RP_250_12:
                model = "SIGNALlab 250-12";
                break;
            default:
                break;
        }
        std::cout << i++ << ")\t"  << (item.mode == asionet_broadcast::CAsioBroadcastSocket::ABMode::AB_SERVER_MASTER ? "MASTER" : "SLAVE") << "\t" << item.host << "\t" << model.c_str() << "\n";
    }
}
