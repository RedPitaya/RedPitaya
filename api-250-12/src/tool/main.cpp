#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <signal.h>
#include <iostream>
#include <fstream> 
#include <string>
#include <system_error>
#include <functional>
#include <algorithm>

#include "rp-i2c.h"

char* getCmdOption(char ** begin, char ** end, const std::string & option,int index = 0)
{
    //    Example
    //    char * filename = getCmdOption(argv, argv + argc, "-f");

    char ** itr = std::find(begin, end, option);
    while(itr != end && ++itr != end){
        if (index <= 0)
            return *itr;
        index--;
    };    
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    //    Example
    //    if(cmdOptionExists(argv, argv+argc, "-h"))
    //    {  // Do stuff    }
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

bool is_file_exist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

void UsingArgs(char const* progName){
    printf("Usage with file: %s -f FILE_NAME [-L][-P][-F][-C][-V]\n ",progName);
    printf("\t-L (default enable) Load configuration from xml file.\n");
    printf("\t-P Print i2c register values based on configuration file.\n");
    printf("\t-С Compare register values with configuration file.\n\t   If the values match, then the return result is 0.\n");
    printf("\nUsage direct read: %s -r I2C_DEVICE ADDRESS REGISTER [-V][-F]\n ",progName);
    printf("\tI2C_DEVICE Path to device like '/dev/i2c-0'.\n");
    printf("\tADDRESS address of device on i2c bus.\n");
    printf("\tREGISTER register for read.\n");
    printf("\nUsage direct write: %s -w I2C_DEVICE ADDRESS REGISTER VALUE [-V][-F]\n ",progName);
    printf("\tI2C_DEVICE Path to device like '/dev/i2c-0'.\n");
    printf("\tADDRESS address of device on i2c bus.\n");
    printf("\tREGISTER register for write.\n");
    printf("\tVALUE register for write.\n");
    printf("\n\tADDRESS, REGISTER, VALUE used in HEX format like 0xff.\n");
    printf("\t-V Enables the mode of outputting the result of work to the console.\n");
    printf("\t-F Force mode. If device alredy used.\n");
    exit(-1);
}

int main(int argc, char* argv[])
{
    bool   use_file     = cmdOptionExists(argv, argv + argc, "-f");
    bool   read_value   = cmdOptionExists(argv, argv + argc, "-r");
    bool   write_value  = cmdOptionExists(argv, argv + argc, "-w");
    bool   verbouse     = cmdOptionExists(argv, argv + argc, "-V"); 
    bool   force_mode   = cmdOptionExists(argv, argv + argc, "-F");   

    if (!(use_file ^ read_value ^ write_value)) {
            UsingArgs(argv[0]);
        }
    
    if (verbouse) rp_i2c::rp_i2c_enable_verbous();

    if (use_file) {
        char *file_name = getCmdOption(argv, argv + argc, "-f");
        
        if (is_file_exist(file_name)== false) {
            printf("Error: file %s not exist.\n",file_name);
            UsingArgs(argv[0]);
        }
       

        int mode = 0;
        if (cmdOptionExists(argv, argv + argc, "-P")) mode = 1;
        if (cmdOptionExists(argv, argv + argc, "-C")) mode = 2;

        switch (mode)
        {
            case 1: 
                return rp_i2c::rp_i2c_print(file_name,force_mode);
            case 2:
                return  rp_i2c::rp_i2c_compare(file_name,force_mode);
            default: 
                return  rp_i2c::rp_i2c_load(file_name,force_mode);
        }
    }

    if (read_value){
        char *I2C_DEVICE = getCmdOption(argv, argv + argc, "-r",0);
        char *ADDRESS    = getCmdOption(argv, argv + argc, "-r",1);
        char *REGISTER   = getCmdOption(argv, argv + argc, "-r",2);
        if (CheckMissing(I2C_DEVICE,"I2C_DEVICE") || 
            CheckMissing(ADDRESS,"ADDRESS") || 
            CheckMissing(REGISTER,"REGISTER")){
                 UsingArgs(argv[0]);
            }
        int addr = 0;
        int reg  = 0;

        if (sscanf(ADDRESS, "%x", &addr) == 0){
            printf("Error parse address.\n");
            UsingArgs(argv[0]);
        }

        
        if (sscanf(REGISTER, "%x", &reg) == 0){
            printf("Error parse register.\n");
            UsingArgs(argv[0]);
        }
        char read_val = 0;
        if (rp_i2c::rp_read_from_i2c(I2C_DEVICE,addr,reg,read_val,force_mode)){
            printf("ERROR read i2c from %s addr: 0x%.2x\treg: 0x%.2x\n",I2C_DEVICE,addr,reg);
            return -1;
        }
        if (verbouse){
             printf("Read from %s addr: 0x%.2x\treg: 0x%.2x\tvalue: 0x%.2x\n",I2C_DEVICE,addr,reg,read_val);
        }
        return read_val;
    }

    if (write_value){
        char *I2C_DEVICE = getCmdOption(argv, argv + argc, "-w",0);
        char *ADDRESS    = getCmdOption(argv, argv + argc, "-w",1);
        char *REGISTER   = getCmdOption(argv, argv + argc, "-w",2);
        char *VALUE      = getCmdOption(argv, argv + argc, "-w",3);
        if (CheckMissing(I2C_DEVICE,"I2C_DEVICE") || 
            CheckMissing(ADDRESS,"ADDRESS") || 
            CheckMissing(VALUE,"VALUE") ||
            CheckMissing(REGISTER,"REGISTER")){
                 UsingArgs(argv[0]);
            }
        int addr = 0;
        int reg  = 0;
        int val  = 0;

        if (sscanf(ADDRESS, "%x", &addr) == 0){
            printf("Error parse address.\n");
            UsingArgs(argv[0]);
        }

        
        if (sscanf(REGISTER, "%x", &reg) == 0){
            printf("Error parse register.\n");
            UsingArgs(argv[0]);
        }

        if (sscanf(VALUE, "%x", &val) == 0){
            printf("Error parse value.\n");
            UsingArgs(argv[0]);
        }

        if (rp_i2c::rp_write_to_i2c(I2C_DEVICE,addr,reg,val,force_mode)){
            printf("ERROR write i2c from %s addr: 0x%.2x\treg: 0x%.2x\n",I2C_DEVICE,addr,reg);
            return -1;
        }
        if (verbouse){
             printf("Write to %s addr: 0x%.2x\treg: 0x%.2x\tvalue: 0x%.2x\n",I2C_DEVICE,addr,reg,val);
        }
        return 0;
    }
    return 0;
}