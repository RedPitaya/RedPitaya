#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <system_error>
#include <functional>
#include <inttypes.h>
#include <chrono>
#include "converter_lib/converter.h"
#include "writer_lib/file_helper.h"
#include "data_lib/thread_cout.h"
#include "rp-formatter/rp_formatter.h"

#define MAX(X,Y) ((X > Y) ? X: Y)

using namespace std;
using namespace rp_formatter_api;

bool g_stopWrite = false;
converter_lib::CConverter::Ptr g_converter = nullptr;

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

void sigHandlerStopCSV (int){
    g_converter->stopWriteToCSV();
    g_stopWrite = true;
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


bool convertToCSV(std::string _file_name,int32_t start_seg, int32_t end_seg,std::string _prefix){
    bool ret = true;
    g_stopWrite = false;
    CFormatter *formatter = nullptr;
    try{
        if (g_stopWrite) return false;
        if (_prefix != "") {
            _prefix = "["+_prefix + "] ";
        }
        aprintf(stdout,"%s Started converting to CSV\n",_prefix.c_str());
        std::string csv_file = _file_name.substr(0, _file_name.size()-3) + "csv";

        aprintf(stdout,"%s %s\n",_prefix.c_str(),csv_file.c_str());
        std::fstream fs;
        std::fstream fs_out;
        fs.open(_file_name, std::ios::binary | std::ofstream::in | std::ofstream::out);
        fs_out.open(csv_file, std::ofstream::in |  std::ofstream::trunc | std::ofstream::out);
        if (fs.fail() || fs_out.fail()) {
            aprintf(stderr,"Error open files\n");
            ret = false;
        }else{
            fs.seekg(0, std::ios::end);
            int64_t Length = fs.tellg();
            int64_t position = 0;
            int32_t curSegment = 0;
            uint64_t samplePos = 0;
            int     channels = 0;
            start_seg = MAX(start_seg,1);
            while(position >= 0){
                auto freeSize = getFreeSpaceDisk(csv_file);
                if (freeSize <= USING_FREE_SPACE){
                    aprintf(stdout,"%s Disk is full\n",_prefix.c_str());
                    ret = false;
                    break;
                }
                if (g_stopWrite){
                    aprintf(stdout,"%s Abort writing to CSV file\n",_prefix.c_str());
                    ret = false;
                    break;
                }
                curSegment++;
                bool notSkip = (start_seg <= curSegment) && ((end_seg != -2 && end_seg >= curSegment) || end_seg == -2);
                auto data_seg = readCSV(&fs,&position, &channels,&samplePos, !notSkip);

// TODO
//                auto bin_seg = readBinData(&fs,&position);

                if (end_seg == -2){
                    if (position >=0) {
                        aprintf(stdout, "\r%s PROGRESS: %d %",_prefix.c_str(),(position * 100) / Length);
                    }else{
                        if (position == -2){
                            aprintf(stdout, "\r%s PROGRESS: 100 %",_prefix.c_str());
                        }
                    }
                }else{
                    if (curSegment - start_seg >= 0 && (end_seg-1) > start_seg) {
                        aprintf(stdout, "\r%s PROGRESS: %d %",_prefix.c_str(),((curSegment - start_seg - 1) * 100) / (end_seg - start_seg));
                    }
                }

                if (notSkip && data_seg){

// TODO Convert to a formatter, taking into account the presence of capture losses.
                    // if (formatter == nullptr){
                    //     formatter = new CFormatter(rp_formatter_api::RP_F_CSV,bin_seg->adcRate);
                    //     formatter->openFile(csv_file);
                    // }

                    // if (formatter->isOpenFile()){
                    //     formatter->clearBuffer();
                    //     for(int i = 0; i < 4; i++){
                    //         if (bin_seg->ch_size)
                    //             formatter->setChannel((rp_channel_t)i,)
                    //     }
                    //     formatter.writeToFile();
                    // }
                }
                delete data_seg;

                if (end_seg != -2 && end_seg < curSegment){
                    break;
                }

                if (fs.fail() || fs_out.fail()) {
                    aprintf(stdout, "\n%s Error write to CSV file\n",_prefix.c_str());
                    if (fs.fail())  aprintf(stdout, "\n%s FS is fail\n",_prefix.c_str());
                    if (fs_out.fail())  aprintf(stdout, "\n%s FS out is fail\n",_prefix.c_str());

                    ret = false;
                    break;
                }

            }
        }
        aprintf(stdout, "\n%s Ended converting\n",_prefix.c_str());
    }catch (std::exception& e)
	{
        aprintf(stderr,"%s Error: convertToCSV() : %s\n",_prefix.c_str(),e.what());
        ret = false;
	}
    delete formatter;
    return ret;
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
        g_converter = converter_lib::CConverter::create();
        g_converter->convertToCSV(file_name,s,e,"");
        // TODO
        // convertToCSV(file_name,s,e,"");
    }else{
        std::fstream fs;
        fs.open(file_name, std::ios::binary | std::ofstream::in | std::ofstream::out);
        if (fs.fail()) {
            std::cout <<" Error open file: " << file_name << "\n";
        }else{
            auto bi = readBinInfo(&fs);

            aprintf(stdout,"Segments count: %llu\n",bi.segCount);
            aprintf(stdout,"Samples per segment: %llu\n",bi.segSamplesCount);
            aprintf(stdout,"Samples in last segment: %llu\n",bi.segLastSamplesCount);
            aprintf(stdout,"Status of last segment: %s\n",bi.lastSegState ? "OK": "BROKEN");

            for(int i = 0; i < 4 ; i++){
                aprintf(stdout,"\nChannel %d:\n",i+1);
                string dft = "Unknown";
                if (bi.dataFormatSize[i] == 1) dft = "Int8";
                if (bi.dataFormatSize[i] == 2) dft = "Int16";
                if (bi.dataFormatSize[i] == 4) dft = "Float";
                aprintf(stdout,"\tData format type:\t%s\n",dft.c_str());
                aprintf(stdout,"\tSamples count:\t%llu\n",bi.samples_ch[i]);
                aprintf(stdout,"\tLost samples count: %llu\n",bi.lostCount[i]);
            }


        }
    }

    return 0;
}
