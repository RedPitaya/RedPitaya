#include <iomanip>
#include <iostream>
#include <string>

#include "File.h"
#include "Writer.h"
#include "wavReader.h"
#include "wavWriter.h"
#include "ReaderController.h"

using namespace TDMS;

#define WAV_TEST_COUNT 1000
#define TDMS_TEST_COUNT 1000
#define DATA_SIZE 1000000

//#define USE_PREDEFINE
// Test TDMS num samples: 369247 channels:1 repmode: 0 repcount: 1
#define NUMSAMP 975221
#define CHANNELS 1
#define REP_MODE 0
#define REP_COUNT 1
#define MEM_SIZE 487177

struct Buff{
    uint8_t *buffer;
    size_t size;
    Buff(){buffer = nullptr;size =0;}
    ~Buff(){ if(buffer) delete[] buffer;}
    Buff(const Buff&) = delete;
    Buff(Buff&&) = delete;
    Buff& operator=(const Buff&) = delete;
    Buff& operator=(Buff&&) = delete;
};


Buff* genBuffer(int size){
    Buff* buff = new Buff();
    auto b = new uint16_t[size];
    buff->size = size * 2;
    for(int i = 0; i < size; i++){
        b[i] = random();
    }
    buff->buffer = (uint8_t*)b;
    return buff;
}

vector<Buff*> genVector(size_t numbersCount){
    vector<Buff*> vect;
    while(numbersCount >0){
        size_t size = random() % numbersCount + 1;
        if (numbersCount < 100){
            size = numbersCount;
        }
        vect.push_back(genBuffer(size));
        numbersCount -= size;
    }
    return vect;
}


Buff* genResultBuff(vector<Buff*>* vect,int repeat){
    size_t allsize = 0;
    for(unsigned long i = 0;i < vect->size();i++){
        allsize += vect->at(i)->size;
    }

    allsize *= repeat;

    if (allsize % (32 * 1024) != 0){
        int newsize = allsize / (32 * 1024);
        allsize = (newsize + 1) * (32 * 1024);
    }

    uint8_t* b = new uint8_t[allsize];
    memset(b,0,allsize);
    int pos = 0;
    for(int z = 0 ; z < repeat;z++){
        for(unsigned long i = 0;i < vect->size();i++){
            memcpy(b+pos,vect->at(i)->buffer,vect->at(i)->size);
            pos += vect->at(i)->size;
        }
    }
    Buff* resBuf = new Buff();
    resBuf->buffer = b;
    resBuf->size = allsize;
    return resBuf;
}

Buff* genBuff(vector<Buff*>* vect){
    size_t allsize = 0;
    for(unsigned long i = 0;i < vect->size();i++){
        allsize += vect->at(i)->size;
    }

    uint8_t* b = new uint8_t[allsize];
    int pos = 0;
    for(unsigned long i = 0;i < vect->size();i++){
        memcpy(b+pos,vect->at(i)->buffer,vect->at(i)->size);
        pos += vect->at(i)->size;
    }
    Buff* resBuf = new Buff();
    resBuf->buffer = b;
    resBuf->size = allsize;
    return resBuf;
}

vector<Buff*> genWAV(size_t numCount,int channel){
    CWaveWriter writer;
    auto vect = genVector(numCount);
    auto buff = genBuff(&vect);
    Buff* ch1 = nullptr;
    Buff* ch2 = nullptr;
    if (channel == 1 || channel == 2){
        ch1 = buff;
    }
    if (channel == 2){
        ch2 = buff;
    }

    auto stream = writer.BuildWAVStream(ch1?ch1->buffer:nullptr,ch1?ch1->size:0,ch2?ch2->buffer:nullptr,ch2?ch2->size:0,16);
    ofstream myfile ("test.wav", ios::binary);
    myfile << stream->rdbuf();
    myfile.flush();
    delete stream;
    delete buff;
    return vect;
}


vector<Buff*> genTDMS(size_t numCount,int channel){
    auto vect = genVector(numCount);

    File outFile;

    bool append = false;
    for(unsigned long i = 0 ; i < vect.size();i++){
        {
            WriterSegment seg;
            vector<shared_ptr<Metadata>> data;

            auto root = seg.GenerateRoot();
            root->TableOfContents.HasMetaData = true;
            root->TableOfContents.HasRawData  = true;
            data.push_back(root);

            auto group =seg.GenerateGroup("Group");
            data.push_back(group);

            if (channel == 1 || channel == 3){
                auto channelSeg = seg.GenerateChannel("Group","ch1");
                data.push_back(channelSeg);
                uint8_t *tbuf = new uint8_t[vect[i]->size];
                memcpy(tbuf,vect[i]->buffer,vect[i]->size);
                seg.AddRaw(channelSeg,TDMSType::UnsignedInteger16,vect[i]->size/2,tbuf);
            }

            if (channel == 2 || channel == 3){
                auto channel2Seg =seg.GenerateChannel("Group","ch2");
                data.push_back(channel2Seg);
                uint8_t *tbuf = new uint8_t[vect[i]->size];
                memcpy(tbuf,vect[i]->buffer,vect[i]->size);
                seg.AddRaw(channel2Seg,TDMSType::UnsignedInteger16,vect[i]->size/2,tbuf);
            }
            seg.LoadMetadata(data);
            outFile.WriteFile("test.tdms",seg, append);
            append = true;
        }
    }
    return vect;

}



void checkWAV(){
    for(int i = 0 ;i < WAV_TEST_COUNT;i++){
        size_t numSamples = random() % DATA_SIZE + 1;
        int ch = random() % 2 + 1;
#ifdef USE_PREDEFINE
        ch = CHANNELS;
        numSamples = NUMSAMP;
#endif
        auto vec = genWAV(numSamples,ch);

        auto enableRepeat = (random() % 100 < 50) ?  CStreamSettings::DACRepeat::DAC_REP_OFF : CStreamSettings::DACRepeat::DAC_REP_ON;
        int repCount = 1;
        if (enableRepeat == CStreamSettings::DACRepeat::DAC_REP_ON){
            repCount = random() % 100 + 1;
        }
        int mem = random() % 1000000;

#ifdef USE_PREDEFINE
        mem = MEM_SIZE;
        enableRepeat = (CStreamSettings::DACRepeat)REP_MODE;
        repCount = REP_COUNT;
#endif

        auto resBuff = genResultBuff(&vec,repCount);
        uint8_t *ch1Res = new uint8_t[resBuff->size];
        uint8_t *ch2Res = new uint8_t[resBuff->size];
        size_t ch1Pos = 0;
        size_t ch2Pos = 0;

        CReaderController reader(CStreamSettings::DataFormat::WAV,"test.wav",enableRepeat,repCount,mem);
        if (reader.isOpen() == CReaderController::OR_OK){
            while(1){
                uint8_t *ch1 = nullptr;
                uint8_t *ch2 = nullptr;
                size_t size1 = 0;
                size_t size2 = 0;
                auto res = reader.getBufferPrepared(&ch1,&size1,&ch2,&size2);
                if (ch1) {
                    memcpy(ch1Res+ch1Pos,ch1,size1);
                    ch1Pos += size1;
                }
                if (ch2) {
                    memcpy(ch2Res+ch2Pos,ch2,size2);
                    ch2Pos += size2;
                }
                if (ch1) delete[](ch1);
                if (ch2) delete[](ch2);

                if (res != CReaderController::BR_OK)
                    break;
            }            
        }else{
            std::cout << "Error open wav test file.\n";
            exit(-1);
        }

        int compareRes = 0;

        if (ch == 1 || ch == 2){
            if (resBuff->size == ch1Pos){
                for(size_t i = 0 ;i < resBuff->size;i++){
                    if (resBuff->buffer[i] != ch1Res[i]){
                        compareRes = compareRes | 0x1;
                        break;
                    }
                }
            }else{
                compareRes = compareRes | 0x1;
            }
        }

        if (ch == 2){
            if (resBuff->size == ch2Pos){
                for(size_t i = 0 ;i < resBuff->size;i++){
                    if (resBuff->buffer[i] != ch2Res[i]){
                        compareRes = compareRes | 0x2;
                        break;
                    }
                }
            }else{
                compareRes = compareRes | 0x2;
            }
        }

        std::cout << i << " Test WAV num samples: " << numSamples << " channels:" << ch << " repmode: " << enableRepeat << " repcount: " << repCount << " mem: " << mem;
        if (compareRes == 0) std::cout << " [OK]\n";
        if (compareRes & 0x3) {
            std::cout << " [FAIL ch1 and ch2]\n";
        }else{
            if (compareRes & 0x1) std::cout << " [FAIL ch1]\n";
            if (compareRes & 0x2) std::cout << " [FAIL ch2]\n";
        }

        if (compareRes != 0){
           exit(-1);
        }

        for(unsigned long i = 0 ;i < vec.size();i++){
            delete vec[i];
        }
        delete resBuff;
        delete[] ch1Res;
        delete[] ch2Res;
    }

}

void checkTDMS(){
    for(int i = 0 ;i < TDMS_TEST_COUNT;i++){
        size_t numSamples = random() % DATA_SIZE + 1;
        int ch = random() % 3 + 1;

#ifdef USE_PREDEFINE
        ch = CHANNELS;
        numSamples = NUMSAMP;
#endif

        auto vec = genTDMS(numSamples,ch);

        auto enableRepeat = (random() % 100 < 50) ?  CStreamSettings::DACRepeat::DAC_REP_OFF : CStreamSettings::DACRepeat::DAC_REP_ON;
        int repCount = 1;
        if (enableRepeat == CStreamSettings::DACRepeat::DAC_REP_ON){
            repCount = random() % 100 + 1;
        }
        int mem = random() % 1000000;

#ifdef USE_PREDEFINE
        mem = MEM_SIZE;
        enableRepeat = (CStreamSettings::DACRepeat)REP_MODE;
        repCount = REP_COUNT;
#endif
        auto resBuff = genResultBuff(&vec,repCount);
        uint8_t *ch1Res = new uint8_t[resBuff->size];
        uint8_t *ch2Res = new uint8_t[resBuff->size];
        size_t ch1Pos = 0;
        size_t ch2Pos = 0;

        CReaderController reader(CStreamSettings::DataFormat::TDMS,"test.tdms",enableRepeat,repCount,mem);
        if (reader.isOpen() == CReaderController::OR_OK){
            while(1){
                uint8_t *ch1 = nullptr;
                uint8_t *ch2 = nullptr;
                size_t size1 = 0;
                size_t size2 = 0;
                auto res = reader.getBufferPrepared(&ch1,&size1,&ch2,&size2);
                if (ch1) {
                    memcpy(ch1Res+ch1Pos,ch1,size1);
                    ch1Pos += size1;
                }
                if (ch2) {
                    memcpy(ch2Res+ch2Pos,ch2,size2);
                    ch2Pos += size2;
                }
                if (ch1) delete[](ch1);
                if (ch2) delete[](ch2);
                if (res != CReaderController::BR_OK)
                    break;
            }
        }else{
            std::cout << "Error open wav test file.\n";
            exit(-1);
        }

        int compareRes = 0;

        if (ch == 1 || ch == 3){
            if (resBuff->size == ch1Pos){
                for(size_t i = 0 ;i < resBuff->size;i++){
                    if (resBuff->buffer[i] != ch1Res[i]){
                        std::cout <<  "Index  = " << i << " Value 1 = " << (uint8_t)resBuff->buffer[i] << " Value 2 = " << (uint8_t)ch1Res[i] << "\n";
                        compareRes = compareRes | 0x1;
                        break;
                    }
                }
            }else{
                compareRes = compareRes | 0x1;
            }
        }

        if (ch == 2 || ch == 3){
            if (resBuff->size == ch2Pos){
                for(size_t i = 0 ;i < resBuff->size;i++){
                    if (resBuff->buffer[i] != ch2Res[i]){
                        std::cout <<  "Index  = " << i << " Value 1 = " << (uint8_t)resBuff->buffer[i] << " Value 2 = " << (uint8_t)ch2Res[i] << "\n";
                        compareRes = compareRes | 0x2;
                        break;
                    }
                }
            }else{
                compareRes = compareRes | 0x2;
            }
        }

        std::cout << i <<  " Test TDMS num samples: " << numSamples << " channels:" << ch << " repmode: " << enableRepeat << " repcount: " << repCount << " mem: " << mem;
        if (compareRes == 0) std::cout << " [OK]\n";
        if (compareRes & 0x3) {
            std::cout << " [FAIL ch1 and ch2]\n";
        }else{
            if (compareRes & 0x1) std::cout << " [FAIL ch1]\n";
            if (compareRes & 0x2) std::cout << " [FAIL ch2]\n";
        }

        if (compareRes != 0){
           exit(-1);
        }

        for(unsigned long i = 0 ;i < vec.size();i++){
            delete vec[i];
        }
        delete resBuff;
        delete[] ch1Res;
        delete[] ch2Res;
    }

}

int main(int argc, char* argv[])
{
    checkWAV();
    checkTDMS();

    std::cout << "All done";

    return 0;
}


