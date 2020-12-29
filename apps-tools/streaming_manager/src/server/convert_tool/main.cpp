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

CStreamingManager::Ptr g_manger;


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
    std::cout << "Usage: " << progName << " file_name\n";
}

void sigHandlerStopCSV (int sigNum){
    g_manger->stopWriteToCSV();
}


int main(int argc, char* argv[])
{
    signal(SIGINT, sigHandlerStopCSV);
    if (argc < 2) {
        UsingArgs(argv[0]);
        return -1;
    }
    std::string file_name = argv[1];
    g_manger = CStreamingManager::Create(Stream_FileType::CSV_TYPE , file_name, -1 , false);
    g_manger->convertToCSV(file_name);
    return 0;
}
