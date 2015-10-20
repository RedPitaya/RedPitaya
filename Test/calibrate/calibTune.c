#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rp.h"
#include "common.h"

void printParams() {
	rp_calib_params_t calib = rp_GetCalibrationSettings();

    printf("Current calibration params:\n");
    printf("fe_ch1_fs_g_hi = 0x%08X //!< High gain front end full scale voltage, channel A\n", calib.fe_ch1_fs_g_hi);
    printf("fe_ch2_fs_g_hi = 0x%08X //!< High gain front end full scale voltage, channel B\n", calib.fe_ch2_fs_g_hi);
    printf("fe_ch1_fs_g_lo = 0x%08X //!< Low gain front end full scale voltage, channel A\n", calib.fe_ch1_fs_g_lo);
    printf("fe_ch2_fs_g_lo = 0x%08X //!< Low gain front end full scale voltage, channel B\n", calib.fe_ch2_fs_g_lo);

    printf("fe_ch1_dc_offs = 0x%08X //!< Front end DC offset, channel A\n", calib.fe_ch1_dc_offs);
    printf("fe_ch2_dc_offs = 0x%08X //!< Front end DC offset, channel B\n", calib.fe_ch2_dc_offs);

    printf("be_ch1_fs = 0x%08X //!< Back end full scale voltage, channel A\n", calib.be_ch1_fs);
    printf("be_ch2_fs = 0x%08X //!< Back end full scale voltage, channel B\n", calib.be_ch2_fs);
    printf("be_ch1_dc_offs = 0x%08X //!< Back end DC offset, on channel A\n", calib.be_ch1_dc_offs);
    printf("be_ch2_dc_offs = 0x%08X //!< Back end DC offset, on channel B\n", calib.be_ch2_dc_offs);
}

void printHelp() {
	puts("usage:  in|out(channel) lv|hv|off|fs value\n\tinfo - print current calibration params");
}

int main(int argc, char **argv) {
	ECHECK(rp_CalibInit());
	rp_calib_params_t calib = rp_GetCalibrationSettings();

	char cmd[32] = {0};
	float value = 0;

	if (argc == 4) {
		value = atof(argv[argc - 1]);
		int i;
		for (i = 1; i < argc - 1; ++i)
			strcat(cmd, argv[i]);
	} else if (argc == 2) {
		strcat(cmd, argv[1]);
	}

	if (!strcmp(cmd, "in1lv")) {
		calib.fe_ch1_fs_g_lo += value;//*1E6;
	} else if (!strcmp(cmd, "in1hv")) {
		calib.fe_ch1_fs_g_hi += value;//*1E4;
	} else if (!strcmp(cmd, "in2lv")) {
		calib.fe_ch2_fs_g_lo += value;//*1E6;
	} else if (!strcmp(cmd, "in2hv")) {
		calib.fe_ch2_fs_g_hi += value;//*1E4;
	} else if (!strcmp(cmd, "in1off")) {
		calib.fe_ch1_dc_offs += value;
	} else if (!strcmp(cmd, "in2off")) {
		calib.fe_ch2_dc_offs += value;
	} else if (!strcmp(cmd, "out1fs")) {
		calib.be_ch1_fs += value;
	} else if (!strcmp(cmd, "out2fs")) {
		calib.be_ch2_fs += value;
	} else if (!strcmp(cmd, "out1off")) {
		calib.be_ch1_dc_offs += value;
	} else if (!strcmp(cmd, "out2off")) {
		calib.be_ch2_dc_offs += value;
	} else if (!strcmp(cmd, "info")) {
		printParams();
	} else if (!strcmp(cmd, "help")) {
		printHelp();
	} else {
		printHelp();
	}

	ECHECK(rp_CalibrationWriteParams(calib));
	//ECHECK(rp_Release());

	return 0;
}

