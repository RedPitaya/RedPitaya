#include "search.h"
#include "config.h"

auto getModelName(uint8_t model) -> std::string {
    // typedef enum {
    //     STEM_125_10_v1_0 = 0,
    //     STEM_125_14_v1_0 = 1,
    //     STEM_125_14_v1_1 = 2,
    //     STEM_122_16SDR_v1_0 = 3,
    //     STEM_122_16SDR_v1_1 = 4,
    //     STEM_125_14_LN_v1_1 = 5,
    //     STEM_125_14_Z7020_v1_0 = 6,
    //     STEM_125_14_Z7020_LN_v1_1 = 7,
    //     STEM_125_14_Z7020_4IN_v1_0 = 8,
    //     STEM_125_14_Z7020_4IN_v1_2 = 9,
    //     STEM_125_14_Z7020_4IN_v1_3 = 10,
    //     STEM_250_12_v1_0 = 11,
    //     STEM_250_12_v1_1 = 12,
    //     STEM_250_12_v1_2 = 13,
    //     STEM_250_12_120 = 14,
    //     STEM_250_12_v1_2a = 15,
    //     STEM_250_12_v1_2b = 16,
    //     STEM_125_14_LN_BO_v1_1 = 17,
    //     STEM_125_14_LN_CE1_v1_1 = 18,
    //     STEM_125_14_LN_CE2_v1_1 = 19,

    //     STEM_125_14_v2_0 = 20,
    //     STEM_125_14_Pro_v2_0 = 21,
    //     STEM_125_14_Z7020_Pro_v2_0 = 22,
    //     STEM_125_14_Z7020_Ind_v2_0 = 23,
    //     STEM_125_14_Z7020_Pro_v1_0 = 24,  // Prototype

    //     STEM_125_14_Z7020_LL_v1_1 = 25,
    //     STEM_65_16_Z7020_LL_v1_1 = 26,
    //     STEM_125_14_Z7020_LL_v1_2 = 27,
    //     STEM_125_14_Z7020_TI_v1_3 = 28,
    //     STEM_65_16_Z7020_TI_v1_3 = 29,

    //     STEM_125_14_Z7020_4IN_BO_v1_3 = 30,
    //     STEM_125_14_BO_v2_0 = 31,
    //     STEM_125_14_Pro_BO_v2_0 = 32,
    //     STEM_125_14_Z7020_Pro_BO_v2_0 = 33,
    // } rp_HPeModels_t;
    switch (model) {
        case 0:
            return "STEMlab 125-10 v1.0";
        case 1:
            return "STEMlab 125-14 v1.0";
        case 2:
            return "STEMlab 125-14 v1.1";
        case 3:
            return "SDRlab 122-16 v1.0";
        case 4:
            return "SDRlab 122-16 v1.1";
        case 5:
            return "STEMlab 125-14 LN v1.1";
        case 6:
            return "STEMlab 125-14 Z7020 v1.0";
        case 7:
            return "STEMlab 125-14 LN-Z7020 v1.1";
        case 8:
            return "STEMlab 125-14 4-Input v1.0";
        case 9:
            return "STEMlab 125-14 4-Input v1.2";
        case 10:
            return "STEMlab 125-14 4-Input v1.3";

        case 11:
            return "SIGNALlab 250-12 v1.0";
        case 12:
            return "SIGNALlab 250-12 v1.1";
        case 13:
            return "SIGNALlab 250-12 v1.2";
        case 14:
            return "SIGNALlab 250-12/120";
        case 15:
            return "SIGNALlab 250-12 v1.2a";
        case 16:
            return "SIGNALlab 250-12 v1.2b";

        case 17:
            return "STEMlab 125-14 LN BO v1.1";
        case 18:
            return "STEMlab 125-14 LN CE1 v1.1";
        case 19:
            return "STEMlab 125-14 LN CE2 v1.1";

        case 20:
            return "STEMlab 125-14 v2.0";
        case 21:
            return "STEMlab 125-14 Pro v2.0";
        case 22:
            return "STEMlab 125-14-Z7020 Pro v2.0";
        case 23:
            return "STEMlab 125-14 Ind v2.0";
        case 24:
            return "STEMlab 125-14-Z7020 Pro v1.0";

        case 25:
            return "STEMlab 125-14 LL v1.1";
        case 26:
            return "STEMlab 65-16 LL v1.1";
        case 27:
            return "STEMlab 125-14 LL v1.2";
        case 28:
            return "STEMlab 125-14 TI v1.3";
        case 29:
            return "STEMlab 65-16 TI v1.3";
        case 30:
            return "STEMlab 125-14 4-Input BO v1.3";
        case 31:
            return "STEMlab 125-14 BO v2.0";
        case 32:
            return "STEMlab 125-14 Pro BO v2.0";
        case 33:
            return "STEMlab 125-14-Z7020 Pro BO v2.0";
        default:
            break;
    }
    return "ERROR";
}

auto startSearch(ClientOpt::Options& option) -> void {
    ClientNetConfigManager client("", false);
    client.startBroadcast("127.0.0.1", NET_BROADCAST_PORT);
    int timout = option.timeout;

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
    auto value = curTime.time_since_epoch();
    auto timeBegin = value.count();
    int point = 1;
    while ((value.count() - timeBegin) < timout * 1000) {
        std::cout << "\rSearch ";
        for (int i = 0; i < 5; i++) {
            if (i < point)
                std::cout << ".";
            else
                std::cout << " ";
        }
        std::cout << std::flush;
        point++;
        if (point > 4)
            point = 0;
        sleepMs(100);
        value = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch();
    }
    std::cout << "\rSearch: DONE";
    auto list = client.getBroadcastClients();
    std::cout << "\nFound boards:\n";

    int i = 1;
    for (auto& item : list) {
        std::string model = getModelName(item.model);
        std::cout << i++ << ")\t" << (item.mode == broadcast_lib::EMode::AB_SERVER_MASTER ? "MASTER" : "SLAVE") << "\t" << (item.host == "" ? "Unknown" : item.host) << "\t"
                  << model.c_str() << "\n";
    }
}

auto startSearch() -> std::string {
    ClientNetConfigManager client("", false);
    client.startBroadcast("127.0.0.1", NET_BROADCAST_PORT);
    int timout = 1;

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
    auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(timeNow);
    auto value = curTime.time_since_epoch();
    auto timeBegin = value.count();
    while ((value.count() - timeBegin) < timout * 1000) {
        sleepMs(100);
        value = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch();
        auto list = client.getBroadcastClients();
        if (list.size() > 0)
            return list.front().host;
    }
    return "";
}
