/**
 * $Id: calib.c 1246 2014-02-22 19:05:19Z ales.bardorfer $
 *
 * @brief Red Pitaya FE & BE calibration utility.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>

#include "rp_eeprom.h"
#include "redpitaya/version.h"


/** Minimal number of command line arguments */
#define MINARGS 2

#if defined Z10 || defined Z20_125
#define CALIB_MAGIC 0xAABBCCDD
#endif

/** Program name */
const char *g_argv0 = NULL;

/** Bit flags to represent options on the command-line. */
typedef enum {
	WANT_READ     = 0x01,
	WANT_WRITE    = 0x02,
	WANT_DEFAULTS = 0x04,
	WANT_VERBOSE  = 0x08,
	WANT_Z_MODE   = 0x10
} WANT_FLAGS;


/** Print usage information */
void usage()
{
    const char *format =
        "%s version %s-%s\n"
        "\n"
	"Usage: %s [OPTION]...\n"
	"\n"
        "OPTIONS:\n"
	" -r    Read calibration values from eeprom (to stdout).\n"
	" -w    Write calibration values to eeprom (from stdin).\n"
	" -f    Use factory address space.\n"
	" -d    Reset calibration values in eeprom with factory defaults.\n"
	" -v    Produce verbose output.\n"
	" -h    Print this info.\n"
        "\n";

    fprintf(stderr, format, g_argv0, VERSION_STR, REVISION_STR, g_argv0);
}


/** Read eeprom calibration data and print them */
int ReadCalib(bool factory, bool verbose, bool z_mode)
{
    eepromWpData_t eepromData;
    int ret = 0;

    /* Read */
    ret = RpEepromCalDataRead(&eepromData, factory);
    if(ret) {
        fprintf(stderr, "Cannot access eeprom data!\n");
        return ret;
    }

    /* Print */
    if (verbose) {
        RpPrintEepromCalData(eepromData);
    } else {
        int i;
	if (!z_mode) {
            int size = eCalParEnd;
#if defined Z10 || defined Z20_125
            if (eepromData.feCalPar[eCalParMagic] == CALIB_MAGIC){
                size = eCalPar_F_LOW_AA_CH1;
            }
#endif
        	for(i = 0; i < size; i++) {
           		fprintf(stdout, "%20d", eepromData.feCalPar[i]);
        	}
        	fprintf(stdout, "\n");
	}else{
#ifdef Z20_250_12
		fprintf(stdout, "Unsupport mode\n");
#else
		fprintf(stdout, "%20d %20d %20d %20d %20d %20d %20d %20d %20d %20d %20d %20d\n",
                        eepromData.feCalPar[eCalPar_FE_CH1_DC_offs],
                        eepromData.feCalPar[eCalPar_FE_CH2_DC_offs],
                        eepromData.feCalPar[eCalPar_FE_CH1_FS_G_LO],
                        eepromData.feCalPar[eCalPar_FE_CH2_FS_G_LO],
                        eepromData.feCalPar[eCalPar_FE_CH1_DC_offs_HI],
                        eepromData.feCalPar[eCalPar_FE_CH2_DC_offs_HI],
                        eepromData.feCalPar[eCalPar_FE_CH1_FS_G_HI],
                        eepromData.feCalPar[eCalPar_FE_CH2_FS_G_HI],
                        eepromData.feCalPar[eCalPar_BE_CH1_DC_offs],
                        eepromData.feCalPar[eCalPar_BE_CH2_DC_offs],
                        eepromData.feCalPar[eCalPar_BE_CH1_FS],
                        eepromData.feCalPar[eCalPar_BE_CH2_FS]);
#endif
	}
    }

    return ret;
}


/** Write calibration data, obtained from stdin, to eeprom */
int WriteCalib(bool factory)
{
    eepromWpData_t eepromData;
    const char delimiters[] = " ,:;\t";
    char buf[1024];
    int ret = 0;

    /* Read current eeprom data */
    ret = RpEepromCalDataRead(&eepromData, factory);
    if (ret) return ret;

    /* Get new values from stdin - up to eCalParEnd */
    fgets(buf, sizeof(buf), stdin);
    const char *p = strtok( buf, delimiters );
    int i = 0;

    while ( p && i < eCalParEnd ) {
        eepromData.feCalPar[i] = strtol(p, NULL, 0);
        //TODO: Range checking
        p = strtok( 0, delimiters );
        i++;
    }

#ifdef DEBUG
    // Debug output
    for(i = 0; i < eCalParEnd; i++) {
        fprintf(stdout, "%20d", eepromData.feCalPar[i]);
    }
    fprintf(stdout, "\n");
#endif

    /* Write new/combined data to eeprom */
    ret = RpEepromCalDataWrite(&eepromData, factory);

    return ret;
}


/** Reset calibration data to factory defaults */
int CopyDefaultsCalib()
{
    eepromWpData_t factoryData;
    int ret = 0;

    /* Read factory eeprom data */
    ret = RpEepromCalDataRead(&factoryData, true);
    if (ret) return ret;

    /* Write factory eeprom data to user partition */
    ret = RpEepromCalDataWrite(&factoryData, false);
    if (ret) return ret;

    return ret;
}



int main(int argc, char **argv)
{
    g_argv0 = argv[0];
    int ret = 0;

    if ( argc < MINARGS ) {
        usage();
        exit ( EXIT_FAILURE );
    }

    /* Parse options */
    const char *optstring = "rwfdvhz";
    unsigned int want_bits = 0;
    bool factory = false;

    int ch = -1;
    while ( (ch = getopt( argc, argv, optstring )) != -1 ) {
        switch ( ch ) {

        case 'r':
            want_bits |= WANT_READ;
            break;

        case 'w':
            want_bits |= WANT_WRITE;
            break;

        case 'd':
            want_bits |= WANT_DEFAULTS;
            break;

        case 'f':
            factory = true;
            break;

        case 'v':
            want_bits |= WANT_VERBOSE;
            break;

        // Specal mode for 125/122 production script
        case 'z':
                want_bits |= WANT_Z_MODE;
            break;

        case 'h':
            usage();
            exit ( EXIT_SUCCESS );
            break;

        default:
            usage();
            exit( EXIT_FAILURE );
        }
    }


    /* Sanity check */
    if ( (want_bits & WANT_WRITE) && (want_bits & WANT_DEFAULTS) ) {
        fprintf(stderr, "Cannot do both: write and reset factory defaults.\n");
        usage();
        exit( EXIT_FAILURE );
    }

    /* Write */
    if (want_bits & WANT_WRITE) {
        ret = WriteCalib(factory);
        if (ret) {
            fprintf(stderr, "ERROR: Write failed!\n");
            return ret;
        }
    }

    /* Reset to factory defaults */
    if (want_bits & WANT_DEFAULTS) {
        ret = CopyDefaultsCalib();
        if (ret) {
            fprintf(stderr, "ERROR: Factory reset failed!\n");
            return ret;
        }
    }

    /* Read */
    if (want_bits & WANT_READ) {
        ret = ReadCalib(factory, want_bits & WANT_VERBOSE , want_bits  & WANT_Z_MODE);
        if (ret) {
            fprintf(stderr, "ERROR: Read failed!\n");
            return ret;
        }
    }

    return ret;
}
