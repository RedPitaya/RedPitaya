
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "rp.h"
#include "common.h"


void waitForUser ( void )
{
    int ch;
    struct termios oldt, newt;

    puts("Press 'c' to continue or 'q' to exit.");
    
    tcgetattr ( STDIN_FILENO, &oldt );
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr ( STDIN_FILENO, TCSANOW, &newt );

    do {
        ch = getc(stdin);
    } while((ch != 'C') && (ch != 'c') && (ch != 'Q') && (ch != 'q'));
    
    tcsetattr ( STDIN_FILENO, TCSANOW, &oldt );
    
    if((ch == 'Q') || (ch == 'q'))
        exit(1);
}


int main(int argc, char **argv) {
    float value;
    int ret;
    printf("Library version: %s\n", rp_GetVersion());

    ECHECK(rp_Init());
    ECHECK(rp_CalibrationReset());

    puts("---Calibration application---\n");

    puts("Calibration proces started.");

    puts("Connect CH1 to ground.");
    waitForUser();
    ECHECK(rp_CalibrateFrontEndOffset(RP_CH_1));

    do {
        puts("Connect CH1 to reference voltage source and set jumpers to HV.");
        puts("Enter reference voltage: ");
        ret = scanf("%f", &value);
    } while ((ret != 1) && (value <= 0.f) && (value > 20.f));
    printf("Calibrating to %f V\n", value);
    ECHECK(rp_CalibrateFrontEndScaleHV(RP_CH_1, value));

    do {
        puts("Connect CH1 to reference voltage source and set jumpers to LV.");
        puts("Enter reference voltage: ");
        ret = scanf("%f", &value);
    } while ((ret != 1) && (value <= 0.f) && (value > 1.f));
    printf("Calibrating to %f V\n", value);
    ECHECK(rp_CalibrateFrontEndScaleLV(RP_CH_1, value));

    puts("Connect CH1 Outout to CH1 Input. Press any key to continue.");
    waitForUser();
    ECHECK(rp_CalibrateBackEnd(RP_CH_1));

    puts("Connect CH2 to ground.");
    waitForUser();
    ECHECK(rp_CalibrateFrontEndOffset(RP_CH_2));

    do {
        puts("Connect CH2 to reference voltage source and set jumpers to HV.");
        puts("Enter reference voltage: ");
        ret = scanf("%f", &value);
    } while ((ret != 1) && (value <= 0.f) && (value > 20.f));
    printf("Calibrating to %f V\n", value);
    ECHECK(rp_CalibrateFrontEndScaleHV(RP_CH_2, value));

    do {
        puts("Connect CH2 to reference voltage source and set jumpers to LV.");
        puts("Enter reference voltage: ");
        ret = scanf("%f", &value);
    } while ((ret != 1) && (value <= 0.f) && (value > 1.f));
    printf("Calibrating to %f V\n", value);
    ECHECK(rp_CalibrateFrontEndScaleLV(RP_CH_2, value));

    puts("Connect CH2 Outout to CH2 Input.");
    waitForUser();
    ECHECK(rp_CalibrateBackEnd(RP_CH_2));

    
    ECHECK(rp_Release());
    return 0;
}

