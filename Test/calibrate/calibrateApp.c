
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "redpitaya/rp.h"


int mygetch ( void )
{
    int ch;
    struct termios oldt, newt;

    tcgetattr ( STDIN_FILENO, &oldt );
    newt = oldt;
    newt.c_lflag &= ~( ICANON | ECHO );
    tcsetattr ( STDIN_FILENO, TCSANOW, &newt );
    ch = getchar();
    tcsetattr ( STDIN_FILENO, TCSANOW, &oldt );

    return ch;
}


int main(int argc, char **argv) {
    printf("Library version: %s\n", rp_GetVersion());

    rp_Init();

    puts("---Calibration application---\n");

    puts("Calibration proces started.");

    puts("Connect CH1 to ground. Press any key to continue.");
    mygetch();
    rp_CalibrateFrontEndOffset(RP_CH_1, NULL);

    puts("Connect CH1 to 5V and set jumpers to HV. Press any key to continue.");
    mygetch();
    rp_CalibrateFrontEndScaleHV(RP_CH_1, 5.0, NULL);

    puts("Connect CH1 to 1V and set jumpers to LV. Press any key to continue.");
    mygetch();
    rp_CalibrateFrontEndScaleLV(RP_CH_1, 1.0, NULL);



    puts("Connect CH2 to ground. Press any key to continue.\n");
    mygetch();
    rp_CalibrateFrontEndOffset(RP_CH_2, NULL);

    puts("Connect CH2 to 5V and set jumpers to HV. Press any key to continue.");
    mygetch();
    rp_CalibrateFrontEndScaleHV(RP_CH_2, 5.0, NULL);

    puts("Connect CH2 to 1V and set jumpers to LV. Press any key to continue.");
    mygetch();
    rp_CalibrateFrontEndScaleLV(RP_CH_2, 1.0, NULL);



    puts("Connect CH1 Outout to CH1 Input. Press any key to continue.");
    mygetch();
    rp_CalibrateBackEndOffset(RP_CH_1);
    rp_CalibrateBackEndScale(RP_CH_1);

    puts("Connect CH2 Outout to CH2 Input. Press any key to continue.");
    mygetch();
    rp_CalibrateBackEndOffset(RP_CH_2);
    rp_CalibrateBackEndScale(RP_CH_2);


    rp_Release();
    return 0;
}

