#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#include "redpitaya/rp.h"
#include "../../api/src/common.h"


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

void backupParams() {
    rp_calib_params_t calib = rp_GetCalibrationSettings();
    FILE* fout = fopen("calib_params.dat", "wb");
    fwrite(&calib, sizeof(rp_calib_params_t), 1, fout);
    fclose(fout);
}

void printParams(const rp_calib_params_t* calib) {
    rp_calib_params_t _calib;
    if(!calib) {
        _calib = rp_GetCalibrationSettings();
        calib = &_calib;
    }

    printf("Current calibration params:\n");
    printf("fe_ch1_fs_g_hi = 0x%08X //!< High gain front end full scale voltage, channel A\n", calib->fe_ch1_fs_g_hi);
    printf("fe_ch2_fs_g_hi = 0x%08X //!< High gain front end full scale voltage, channel B\n", calib->fe_ch2_fs_g_hi);
    printf("fe_ch1_fs_g_lo = 0x%08X //!< Low gain front end full scale voltage, channel A\n", calib->fe_ch1_fs_g_lo);
    printf("fe_ch2_fs_g_lo = 0x%08X //!< Low gain front end full scale voltage, channel B\n", calib->fe_ch2_fs_g_lo);

    printf("fe_ch1_lo_offs = 0x%08X //!< Front end LV offset, channel A\n", calib->fe_ch1_lo_offs);
    printf("fe_ch2_lo_offs = 0x%08X //!< Front end LV offset, channel B\n", calib->fe_ch2_lo_offs);
    printf("fe_ch1_hi_offs = 0x%08X //!< Front end HV offset, channel A\n", calib->fe_ch1_hi_offs);
    printf("fe_ch2_hi_offs = 0x%08X //!< Front end HV offset, channel B\n", calib->fe_ch2_hi_offs);

    printf("be_ch1_fs = 0x%08X //!< Back end full scale voltage, channel A\n", calib->be_ch1_fs);
    printf("be_ch2_fs = 0x%08X //!< Back end full scale voltage, channel B\n", calib->be_ch2_fs);
    printf("be_ch1_dc_offs = 0x%08X //!< Back end DC offset, on channel A\n", calib->be_ch1_dc_offs);
    printf("be_ch2_dc_offs = 0x%08X //!< Back end DC offset, on channel B\n", calib->be_ch2_dc_offs);
}

int restoreParams(const char* path) {
    rp_calib_params_t calib;
    FILE* fin = fopen(path, "rb");
    if(!fin)
        return -1;

    if(fread(&calib, sizeof(rp_calib_params_t), 1, fin) != 1) {
        fclose(fin);
        return -1;
    }

    fclose(fin);

    rp_CalibrationWriteParams(calib);
    printParams(&calib);
    return 0;
}

int main(int argc, char **argv) {
    float value;
    int ret;
    printf("Library version: %s\n", rp_GetVersion());

    rp_Init();

    if(argc == 2) {
        restoreParams(argv[1]);
        rp_Release();
        return 0;
    }

	printParams(NULL);
	puts("---Calibration application---\n");
	waitForUser();
    rp_CalibrationReset();

    puts("Calibration proces started.");

    puts("Connect CH1 LV to ground.");
    waitForUser();
    rp_CalibrateFrontEndOffset(RP_CH_1, RP_LOW, NULL);

    puts("Connect CH1 HV to ground.");
    waitForUser();
    rp_CalibrateFrontEndOffset(RP_CH_1, RP_HIGH, NULL);

    do {
        puts("Connect CH1 to reference voltage source and set jumpers to HV.");
        puts("Enter reference voltage: ");
        ret = scanf("%f", &value);
    } while ((ret != 1) && (value <= 0.f) && (value > 20.f));
    printf("Calibrating to %f V\n", value);
    rp_CalibrateFrontEndScaleHV(RP_CH_1, value, NULL);

    do {
        puts("Connect CH1 to reference voltage source and set jumpers to LV.");
        puts("Enter reference voltage: ");
        ret = scanf("%f", &value);
    } while ((ret != 1) && (value <= 0.f) && (value > 1.f));
    printf("Calibrating to %f V\n", value);
    rp_CalibrateFrontEndScaleLV(RP_CH_1, value, NULL);

    puts("Connect CH1 Outout to CH1 Input. Press any key to continue.");
    waitForUser();
    rp_CalibrateBackEnd(RP_CH_1, NULL);

    puts("Connect CH2 LV to ground.");
    waitForUser();
    rp_CalibrateFrontEndOffset(RP_CH_2, RP_LOW, NULL);

    puts("Connect CH2 HV to ground.");
    waitForUser();
    rp_CalibrateFrontEndOffset(RP_CH_2, RP_HIGH, NULL);

    do {
        puts("Connect CH2 to reference voltage source and set jumpers to HV.");
        puts("Enter reference voltage: ");
        ret = scanf("%f", &value);
    } while ((ret != 1) && (value <= 0.f) && (value > 20.f));
    printf("Calibrating to %f V\n", value);
    rp_CalibrateFrontEndScaleHV(RP_CH_2, value, NULL);

    do {
        puts("Connect CH2 to reference voltage source and set jumpers to LV.");
        puts("Enter reference voltage: ");
        ret = scanf("%f", &value);
    } while ((ret != 1) && (value <= 0.f) && (value > 1.f));
    printf("Calibrating to %f V\n", value);
    rp_CalibrateFrontEndScaleLV(RP_CH_2, value, NULL);

    puts("Connect CH2 Outout to CH2 Input.");
    waitForUser();
    rp_CalibrateBackEnd(RP_CH_2, NULL);

    printParams(NULL);
    backupParams();

    rp_Release();
    return 0;
}

