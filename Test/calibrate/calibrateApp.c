
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "../src/rp.h"
#include "../src/common.h"


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

    ECHECK(rp_Init());


    puts("---Calibration application---\n");

    puts("Calibration proces started.");

    puts("Connect CH1 to ground. Press any key to continue.");
    mygetch();
    ECHECK(rp_CalibrateFrontEndOffset(RP_CH_1));

    puts("Connect CH1 to 5V and set jumpers to HV. Press any key to continue.");
    mygetch();
    ECHECK(rp_CalibrateFrontEndScaleHV(RP_CH_1, 5.0));

    puts("Connect CH1 to 1V and set jumpers to LV. Press any key to continue.");
    mygetch();
    ECHECK(rp_CalibrateFrontEndScaleLV(RP_CH_1, 1.0));



    puts("Connect CH2 to ground. Press any key to continue.\n");
    mygetch();
    ECHECK(rp_CalibrateFrontEndOffset(RP_CH_2));

    puts("Connect CH2 to 5V and set jumpers to HV. Press any key to continue.");
    mygetch();
    ECHECK(rp_CalibrateFrontEndScaleHV(RP_CH_2, 5.0));

    puts("Connect CH2 to 1V and set jumpers to LV. Press any key to continue.");
    mygetch();
    ECHECK(rp_CalibrateFrontEndScaleLV(RP_CH_2, 1.0));



    puts("Connect CH1 Outout to CH1 Input. Press any key to continue.");
    mygetch();
    ECHECK(rp_CalibrateBackEndOffset(RP_CH_1));
    ECHECK(rp_CalibrateBackEndScale(RP_CH_1));

    puts("Connect CH2 Outout to CH2 Input. Press any key to continue.");
    mygetch();
    ECHECK(rp_CalibrateBackEndOffset(RP_CH_2));
    ECHECK(rp_CalibrateBackEndScale(RP_CH_2));


    ECHECK(rp_Release());
    return 0;
}

