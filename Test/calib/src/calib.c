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
#include "version.h"
#include "rp.h"
#include "rp_hw-profiles.h"


/** Minimal number of command line arguments */
#define MINARGS 2

/** Program name */
const char *g_argv0 = NULL;

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
        " -d    Reset calibration values in eeprom from factory zone.\n"
        " -i    Reset calibration values in eeprom by default\n"
        " -v    Produce verbose output.\n"
        " -h    Print this info.\n"
        " -x    Print in hex.\n"
        " -u    Print stored calibration in unified format.\n"
        "\n";

    fprintf(stderr, format, g_argv0, VERSION_STR, REVISION_STR, g_argv0);
}

/** Write calibration data, obtained from stdin, to eeprom */
int WriteCalib(rp_HPeModels_t model, bool factory)
{
    const char delimiters[] = " ,:;\t";
    char buf[2048];

    rp_eepromWpData_t eeprom;
    int ret = rp_CalibGetEEPROM(&eeprom,factory);

    if (ret) {
        fprintf(stderr, "ERROR: Read failed!\n");
        return ret;
    }

    /* Get new values from stdin - up to eCalParEnd */
    if (fgets(buf, sizeof(buf), stdin) == NULL){
        return ret;
    }

    const char *p = strtok( buf, delimiters );
    int i = 0;
    int calibSize = getCalibSize(model);
    if (calibSize < 0) return -1;

    while ( p && i < calibSize ) {
        eeprom.feCalPar[i] = strtol(p, NULL, 0);
        //TODO: Range checking
        p = strtok( 0, delimiters );
        i++;
    }

#ifdef DEBUG
    // Debug output
    for(i = 0; i < calibSize; i++) {
        fprintf(stdout, "%20d", eeprom.feCalPar[i]);
    }
    fprintf(stdout, "\n");
#endif

    rp_calib_params_t calib;
    ret =  rp_CalibConvertEEPROM(&eeprom,&calib);
    if (ret) {
        fprintf(stderr, "ERROR: Convert data failed!\n");
        return ret;
    }

    return rp_CalibrationWriteParams(calib,factory);
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
    const char *optstring = "rwfdvhzxiu";
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

        case 'i':
            want_bits |= WANT_INIT;
            break;

        case 'f':
            factory = true;
            break;

        case 'v':
            want_bits |= WANT_VERBOSE;
            break;

        case 'u':
            want_bits |= WANT_PRINT;
            break;

        case 'x':
            want_bits |= WANT_HEX;
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
    rp_HPeModels_t model;
    if (rp_HPGetModel(&model) != RP_HP_OK){
        fprintf(stderr,"[Error] Unknown model: %d.\n",model);
        exit( EXIT_FAILURE);
    }

    /* Write */
    if (want_bits & WANT_WRITE) {
        ret = WriteCalib(model,factory);
        if (ret) {
            fprintf(stderr, "ERROR: Write failed!\n");
            return ret;
        }
    }

    /* Reset to factory defaults */
    if (want_bits & WANT_DEFAULTS) {
        ret = rp_CalibrationFactoryReset();
        if (ret) {
            fprintf(stderr, "ERROR: Factory reset failed!\n");
            return ret;
        }
    }

    if (want_bits & WANT_PRINT) {
        rp_eepromWpData_t eeprom;
        int ret = rp_CalibGetEEPROM(&eeprom,factory);
        if (ret) {
            fprintf(stderr, "ERROR: Read failed!\n");
            return ret;
        }
        rp_calib_params_t calib;
        ret =  rp_CalibConvertEEPROM(&eeprom,&calib);
        if (ret) {
            fprintf(stderr, "ERROR: Convert data failed!\n");
            return ret;
        }
        rp_CalibPrint(&calib);
    }

    /* Reset to factory defaults */
    if (want_bits & WANT_INIT) {
        ret= rp_CalibInit();
        if (ret) {
            fprintf(stderr, "ERROR: Init failed!\n");
            return ret;
        }
        ret = rp_CalibrationReset(factory);
        if (ret) {
            fprintf(stderr, "ERROR: Reset failed!\n");
            return ret;
        }
    }

    /* Read */
    if (want_bits & WANT_READ) {
        rp_eepromWpData_t eeprom;
        int ret = rp_CalibGetEEPROM(&eeprom,factory);

        if (ret) {
            fprintf(stderr, "ERROR: Read failed!\n");
            return ret;
        }
        print_eeprom(model,&eeprom, want_bits);
    }

    return ret;
}
