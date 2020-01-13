#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <signal.h>
#include <fstream> 
#include <functional>
#include <algorithm>
#include <numeric>

char* getCmdOption(char ** begin, char ** end, const std::string & option,int index = 0){
    char ** itr = std::find(begin, end, option);
    while(itr != end && ++itr != end){
        if (index <= 0)
            return *itr;
        index--;
    };    
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option){
    return std::find(begin, end, option) != end;
}

bool CheckMissing(const char* val,const char* Message){
    if (val == NULL) {
        std::cout << "Missing parameters: " << Message << std::endl;
        return true;
    }
    return false;
}

bool is_file_exist(const char *fileName){
    std::ifstream infile(fileName);
    return infile.good();
}

float sign(float x){
        if (x > 0) return 1;
        return -1;
}

float findMax(std::vector<float> &data,int startIndex,int endIndex){
        if (startIndex == endIndex) return data[startIndex];
        float subitem = findMax(data,startIndex + 1,  endIndex);
        return data[startIndex] < subitem ? subitem : data[startIndex];
}

float findMin(std::vector<float> &data,int startIndex,int endIndex){
        if (startIndex == endIndex) return data[startIndex];
        float subitem = findMin(data,startIndex + 1,  endIndex);
        return data[startIndex] > subitem ? subitem : data[startIndex];
}

float avg(std::vector<float> &data){
    int size = data.size();
    float sum = std::accumulate(data.begin(), data.end(), 0);
    if (size != 0)
        return sum/size;
    return 0;
}

float findAvg(bool isMax, std::vector<float> &data,std::vector<float> &dataAvg,int startIndex,int level){

        int size = data.size();
        int endIndex = startIndex;
        while(endIndex < size-1){
                if (sign(data[endIndex]) < sign(data[endIndex+1])){
                    if (isMax){
                            if (level > 0 ){
                                float val = findMax(data ,startIndex,endIndex);
                                dataAvg.push_back(val);
                            }
                            return findAvg(isMax,data,dataAvg,endIndex+1,level+1);
                      }else{
                            if (level > 0 ){
                                float val = findMin(data ,startIndex,endIndex);
                                dataAvg.push_back(val);
                            }
                            return findAvg(isMax,data,dataAvg,endIndex+1,level+1);    
                      }
                    
                }else{
                    endIndex++;
                }
        }
        return 0;
}

void UsingArgs(char const* progName){
    printf("Usage with file: %s -f FILE_NAME\n",progName);
    exit(-1);
}

int main(int argc, char* argv[] ) {

        bool   use_file     = cmdOptionExists(argv, argv + argc, "-f");
        if (!(use_file)) {
            UsingArgs(argv[0]);
        }

        char *file_name = getCmdOption(argv, argv + argc, "-f");

        float f1,f2;
        std::vector<float>  ch1;
        std::vector<float>  ch2;
        std::vector<float>  chAvg;
        
        FILE * pFile;

        pFile = fopen (file_name,"r");
        if (pFile == NULL)
        {
                printf("We can't open the file.");
                fclose(pFile);
                return 1;
        }
        
        while(fscanf (pFile, "%f %f", &f1, &f2)==2)
        {
            ch1.push_back(f1);
            ch2.push_back(f2);
        }

        findAvg(true,ch1,chAvg,0,0);
        float ch1Max = avg(chAvg);
        chAvg.clear();
        findAvg(true,ch2,chAvg,0,0);
        float ch2Max = avg(chAvg);
        chAvg.clear();
        findAvg(false,ch1,chAvg,0,0);
        float ch1Min = avg(chAvg);
        chAvg.clear();
        findAvg(false,ch2,chAvg,0,0);
        float ch2Min = avg(chAvg);
        chAvg.clear();
        
        printf("CH1Max=%f CH2Max=%f CH1Min=%f CH2Min=%f\n", ch1Max , ch2Max , ch1Min , ch2Min);
        fclose (pFile);

        return 0;
}