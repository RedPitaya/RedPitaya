#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <system_error>
#include <functional>

#include <asio.hpp>
#include <chrono>
#include "rpsa/server/core/AsioNet.h"
#include "rpsa/server/core/StreamingManager.h"


using namespace std;

CStreamingManager::Ptr g_manger = nullptr;


char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

bool CheckMissing(const char* val,const char* Message)
{
    if (val == NULL) {
        std::cout << "Missing parameters: " << Message << std::endl;
        return true;
    }
    return false;
}

void UsingArgs(char const* progName){
    std::cout << "Usage: " << progName << " file_name [-i][-s start][-e end]\n";
    std::cout << "\t-i get info about file\n";
    std::cout << "\t-s Segment from which the conversion starts\n";
    std::cout << "\t-e Segment where the conversion will end\n";

}

void sigHandlerStopCSV (int sigNum){
    if (g_manger != nullptr)
        g_manger->stopWriteToCSV();
}

int ParseInt(string value) noexcept{
    try {
        int x = std::stoi (value);
        if ( x <= 0){
            std::cout << "Error read parameter\n";
            return -1;
        }
        return x;
    }
    catch (std::exception& e)
    {
        std::cout << "Error read parameter\n";
        return -1;
    }
}

int main(int argc, char* argv[])
{
    signal(SIGINT, sigHandlerStopCSV);
    if (argc < 2) {
        UsingArgs(argv[0]);
        return -1;
    }

    bool check_info = cmdOptionExists(argv, argv + argc, "-i");
    int32_t s = -2;
    int32_t e = -2;
    if (cmdOptionExists(argv, argv + argc, "-s")){
        char *start_char  = getCmdOption(argv, argv + argc, "-s");
        if (CheckMissing(start_char,"start segment")){
            UsingArgs(argv[0]);
            return -1;
        }
        s = ParseInt(start_char);
        if (s == -1) {
            UsingArgs(argv[0]);
            return -1;
        }
    }

    if (cmdOptionExists(argv, argv + argc, "-e")){
        char *end_char  = getCmdOption(argv, argv + argc, "-e");
        if (CheckMissing(end_char,"end segment")){
            UsingArgs(argv[0]);
            return -1;
        }
        e = ParseInt(end_char);
        if (e == -1) {
            UsingArgs(argv[0]);
            return -1;
        }
    }

    if (s >0 && e >0 && s > e) {
        std::cout << "The start segment must be less than or equal to the end.\n";
        return -1;
    }
    
    std::string file_name = argv[1];
    if (!check_info){
        g_manger = CStreamingManager::Create(Stream_FileType::CSV_TYPE , file_name, -1 , false);
        g_manger->convertToCSV(file_name, s, e);
    }else{
        std::fstream fs;
        fs.open(file_name, std::ios::binary | std::ofstream::in | std::ofstream::out);
        if (fs.fail()) {
            std::cout <<" Error open file: " << file_name << "\n";
        }else{
            auto bi = FileQueueManager::ReadBinInfo(&fs);
            string dft = "Unknown";
            if (bi.dataFormatSize == 1) dft = "Int8";
            if (bi.dataFormatSize == 2) dft = "Int16";
            if (bi.dataFormatSize == 4) dft = "Float";
            printf("Data format type: %s\n",dft.c_str());
            printf("Samples count CH1(%d) CH2(%d)\n",bi.size_ch1,bi.size_ch2);
            printf("Lost samples count: %lld\n",bi.lostCount);
            printf("Segments count: %d\n",bi.segCount);
            printf("Samples per segment: %d\n",bi.segSamplesCount);
            printf("Samples in last segment: %d\n",bi.segLastSamplesCount);
            printf("Status of last segment: %s\n",bi.lastSegState ? "OK": "BROKEN");
        }
    }

    return 0;
}
