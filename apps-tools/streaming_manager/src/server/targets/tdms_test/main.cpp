#include <iomanip>
#include <iostream>
#include <string>

#include "File.h"
#include "Writer.h"
#include "wavReader.h"

using namespace TDMS;

int main(int argc, char* argv[])
{
    CWaveReader wr;
    wr.openFile("3.wav");
    auto h1 = wr.getHeader();
    uint8_t  *ch1 = nullptr;
    uint8_t  *ch2 = nullptr;
    size_t    size_ch1 = 0,size_ch2 = 0;

    while (wr.getBuffers(&ch1,&size_ch1,&ch2,&size_ch2)){
        std::cout << "Get wav buff " << size_ch1 << " - " << size_ch2 << "\n";
        if (size_ch1 == 0 && size_ch2 == 0) break;
        delete[] ch1;
        delete[] ch2;
    }

    File outFile;
    //auto m = outFile.ReadFile("type_01_int8_one_segment.tdms");
    //auto m = outFile.ReadFile("2015-11-03-1900 zero.tdms");
    auto m = outFile.ReadFile("dd.tdms");
    outFile.Print(m,true,20);

    auto m2 = outFile.ReadFileWithoutClose("dd.tdms");
    for(auto &seg : m2){
        auto meta = outFile.GetMetadata(seg);
        outFile.Print(meta,true,100);
        getchar();
    }
    outFile.Close();

    WriterSegment seg;
    seg.LoadMetadata(m);
    cout << "Root present:" << (seg.IsRootNodePresent() ? "true" : "false") << endl;
    //   outFile.WriteFile("dd.tdms",seg, true);

    vector<shared_ptr<Metadata>> data;

    auto root = seg.GenerateRoot();
    root->TableOfContents.HasMetaData = true;
    root->TableOfContents.HasRawData  = true;
    data.push_back(root);
    auto group =seg.GenerateGroup("Group1");
    data.push_back(group);

    auto channel =seg.GenerateChannel("Group1","ch1");
    data.push_back(channel);
    auto channel2 =seg.GenerateChannel("Group1","ch2");
    data.push_back(channel2);
    auto channel3 =seg.GenerateChannel("Group2","time channel");
    data.push_back(channel3);

    DataType dataprop;
    dataprop.InitDataType(TDMSType::UnsignedInteger8,DataType::MakeData<uint8_t>(10));
    seg.AddProperties(channel2,"key-param",dataprop);

    char* buff = new char[5];
    memcpy(buff,"Hello",5);
    dataprop.InitStringType(5,buff);
    seg.AddProperties(channel2,"key-param-string",dataprop);

    std::time_t tim_sec = std::time(0);
    std::time_t local = std::mktime(std::localtime(&tim_sec));
    std::time_t gmt = std::mktime(std::gmtime(&tim_sec));
    timezone = static_cast<long> (local - gmt);

    auto *time = DataType::GetRawTimeValue(tim_sec + timezone);

    seg.AddRaw(channel3,TDMSType::TimeStamp,1,time);


    uint8_t *raw_1 = new uint8_t[5];
    raw_1[0] = 1;
    raw_1[1] = 2;
    raw_1[2] = 3;
    raw_1[3] = 4;
    raw_1[4] = 5;
    seg.AddRaw(channel2,TDMSType::UnsignedInteger8,5,raw_1);

    double *raw_2 = new double[6];
    raw_2[0] = 1;
    raw_2[1] = 2;
    raw_2[2] = 3;
    raw_2[3] = 4;
    raw_2[4] = 5;
    raw_2[5] = 5;
    seg.AddRaw(channel,TDMSType::DoubleFloat,6,raw_2);


    seg.LoadMetadata(data);

    outFile.WriteFile("dd2.tdms",seg, true);

    std::cout << "All done";

    return 0;
}


