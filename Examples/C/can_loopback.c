/* @brief This is a simple application for testing CAN communication on a RedPitaya
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rp_hw_can.h"


int main(int argc, char *argv[]){

    int res = rp_CanSetFPGAEnable(true); // init can in fpga for pass can controller to GPIO (N7,P7) 
    printf("Init result: %d\n",res);
    
    res = rp_CanStop(RP_CAN_0); // set can0 interface to DOWN  for configure
    printf("Stop can0: %d\n",res);
    
    res = rp_CanSetBitrate(RP_CAN_0,200000); // set can0 bitrate
    printf("Set bitrate: %d\n",res);

    res = rp_CanSetControllerMode(RP_CAN_0,RP_CAN_MODE_LOOPBACK,true); // set loopback mode
    printf("Set loopback mode ON: %d\n",res);

    res = rp_CanStart(RP_CAN_0); // set can0 interface to UP
    printf("Start can0: %d\n",res);

    res = rp_CanOpen(RP_CAN_0); // open socket for can0
    printf("Open socket: %d\n",res);

    unsigned char tx_buffer[8];
    tx_buffer[0] = 1;
    tx_buffer[1] = 2;
    tx_buffer[2] = 3;
    res = rp_CanSend(RP_CAN_0,123, tx_buffer,3,false,false,0); // write buffer to can0
    printf("Write result: %d\n",res);

    rp_can_frame_t frame;
    res = rp_CanRead(RP_CAN_0,2000, &frame); // read frame from can0
    printf("Read result: %d\n",res);   
    printf("Can ID: %d data: %d,%d,%d\n",frame.can_id,frame.data[0],frame.data[1],frame.data[2]);

    res = rp_CanClose(RP_CAN_0); // close socket for can0
    printf("Close can0 result: %d\n",res);
    return 0;
}