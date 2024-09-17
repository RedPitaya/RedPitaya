#include "search.h"
#include "config.h"

auto startSearch(ClientOpt::Options &option) -> void
{
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
	for (auto &item : list) {
		std::string model = "";
		switch (item.model) {
			case broadcast_lib::EModel::RP_125_14:
				model = "STEAMlab 125-14";
				break;
			case broadcast_lib::EModel::RP_125_14_Z20:
				model = "STEAMlab 125-14/Z7020";
				break;
			case broadcast_lib::EModel::RP_122_16:
				model = "SDRlab 122-16";
				break;
			case broadcast_lib::EModel::RP_250_12:
				model = "SIGNALlab 250-12";
				break;
			case broadcast_lib::EModel::RP_125_4CH:
				model = "STEAMlab 125-14 4-channels";
				break;
			default:
				break;
		}
		std::cout << i++ << ")\t" << (item.mode == broadcast_lib::EMode::AB_SERVER_MASTER ? "MASTER" : "SLAVE") << "\t"
				  << (item.host == "" ? "Unknown" : item.host) << "\t" << model.c_str() << "\n";
	}
}

auto startSearch() -> std::string
{
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
