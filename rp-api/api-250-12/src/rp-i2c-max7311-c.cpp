#include "rp-i2c-max7311-c.h"
#include "rp-i2c-max7311.h"
#include <iostream>
#include <fstream>
#include <unistd.h>

int  rp_initController_C(){
    return rp_max7311::rp_initController();
}


int  rp_setAC_DC_C(char port,char mode){
    return rp_max7311::rp_setAC_DC(port,mode);
}


int  rp_setAttenuator_C(char port,char mode){
    return rp_max7311::rp_setAttenuator(port,mode);
}


int  rp_setGainOut_C(char port,char mode){
    return rp_max7311::rp_setGainOut(port,mode);
}

void rp_setSleepTime_C(unsigned long time){
    rp_max7311::rp_setSleepTime(time);
}