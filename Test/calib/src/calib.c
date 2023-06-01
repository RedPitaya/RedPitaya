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
// #define DEBUG

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
        "       The -n flag has no effect. The system automatically determines the type of stored data.\n"
        "\n"
        " -w    Write calibration values to eeprom (from stdin).\n"
        "       Possible combination of flags: -wn, -wf, -wfn, -wmn, -wfmn\n"
        "\n"
        " -f    Use factory address space.\n"
        " -d    Reset calibration values in eeprom from factory zone.\n"
        "       Possible combination of flags: -dn. With this combination, the program will convert from the old format to the universal one when copying from the factory zone.\n"
        "\n"
        " -i    Reset calibration values in eeprom by default\n"
        "       Possible combination of flags: -in , -inf.\n"
        "\n"
        " -v    Produce verbose output.\n"
        " -h    Print this info.\n"
        " -x    Print in hex.\n"
        " -u    Print stored calibration in unified format.\n"
        " -m    Modify specific parameter in universal calibration\n"
        " -n    Flag for working with the new calibration storage format.\n"
        "\n";

    fprintf(stderr, format, g_argv0, VERSION_STR, REVISION_STR, g_argv0);
}

/** Write calibration data, obtained from stdin, to eeprom */
int WriteCalib(rp_HPeModels_t model, bool factory,bool is_new,bool is_modify)
{
    const char delimiters[] = " ,:;\t";
    char buf[2048];

    uint8_t *buff = NULL;
    uint16_t size = 0;
    int ret = rp_CalibGetEEPROM(&buff,&size,factory);

    if (ret || !buff || !size) {
        free(buff);
        fprintf(stderr, "ERROR: Read failed!\n");
        return ret;
    }
    uint8_t dataStruct = buff[0];
    uint8_t wp = buff[1];


    rp_eepromWpData_t eeprom;
    rp_eepromUniData_t new_eeprom;

    /* Get new values from stdin - up to eCalParEnd */
    if (fgets(buf, sizeof(buf), stdin) == NULL){
        return -1;
    }

    if (is_modify && !is_new){
        free(buff);
        fprintf(stderr, "ERROR: Invalid flag combination!\n");
        return -1;
    }
    if (!is_modify){
        const char *p = strtok( buf, delimiters );
        int i = 0;
        int calibSize = is_new ? MAX_UNIVERSAL_ITEMS_COUNT : getCalibSize(model);
        if (calibSize < 0) return -1;
        while ( p && i < calibSize ) {
            long int x = strtol(p, NULL, 0);
            if (is_new){
                new_eeprom.item[i].id = x;
                p = strtok( 0, delimiters );
                if (!p){
                    free(buff);
                    fprintf(stderr, "ERROR: Read calib value failed!\n");
                    return -1;
                }
                long int val = strtol(p, NULL, 0);
                new_eeprom.item[i].value = val;
            }else{
                eeprom.feCalPar[i] = x;
            }
            p = strtok( 0, delimiters );
            i++;
        }
        new_eeprom.count = i;
    }else{
        memcpy(&new_eeprom,buff,size);
        const char *p = strtok( buf, delimiters );
        int i = 0;
        int calibSize = is_new ? MAX_UNIVERSAL_ITEMS_COUNT : getCalibSize(model);
        if (calibSize < 0) return -1;
        while ( p && i < calibSize ) {
            long int x = strtol(p, NULL, 0);
            int idx = -1;
            for(int z = 0; z < new_eeprom.count; z++){
                if (new_eeprom.item[z].id == x){
                    idx = z;
                    break;
                }
            }
            if (idx == -1){
                free(buff);
                fprintf(stderr, "ERROR: Can't find calibration parameter!\n");
                return -1;
            }
            new_eeprom.item[idx].id = x;
            p = strtok( 0, delimiters );
            if (!p){
                free(buff);
                fprintf(stderr, "ERROR: Read calib value failed!\n");
                return -1;
            }
            long int val = strtol(p, NULL, 0);
            new_eeprom.item[idx].value = val;

            p = strtok( 0, delimiters );
            i++;
        }
    }
#ifdef DEBUG
    // Debug output
    if (is_new){
        for(i = 0; i < new_eeprom.count; i++) {
            fprintf(stdout, "(%20d,%20d)", new_eeprom.item[i].id,new_eeprom.item[i].value);
        }
    }else{
        for(i = 0; i < calibSize; i++) {
            fprintf(stdout, "%20d", eeprom.feCalPar[i]);
        }
        fprintf(stdout, "\n");
    }
#endif
    eeprom.dataStructureId = dataStruct;
    new_eeprom.dataStructureId = RP_HW_PACK_ID_V5;
    eeprom.wpCheck = wp + 1;
    new_eeprom.wpCheck = wp + 1;

    rp_calib_params_t calib;
    uint8_t *wbuff = is_new ? (uint8_t*)&new_eeprom : (uint8_t*)&eeprom;
    size = is_new ? sizeof(rp_eepromUniData_t) : sizeof(rp_eepromWpData_t);
    ret =  rp_CalibConvertEEPROM(wbuff,size,&calib);
    if (ret) {
        fprintf(stderr, "ERROR: Convert data failed!\n");
        return ret;
    }
    free(buff);
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
    const char *optstring = "rwfdvhzxiunm";
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

        case 'n':
                want_bits |= WANT_NEW_FORMAT;
            break;

        case 'm':
                want_bits |= WANT_MODIFY;
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
        ret = WriteCalib(model,factory, want_bits & WANT_NEW_FORMAT,want_bits & WANT_MODIFY);
        if (ret) {
            fprintf(stderr, "ERROR: Write failed!\n");
            return ret;
        }
    }

    /* Reset to factory defaults */
    if (want_bits & WANT_DEFAULTS) {
        ret = rp_CalibrationFactoryReset(want_bits & WANT_NEW_FORMAT);
        if (ret) {
            fprintf(stderr, "ERROR: Factory reset failed!\n");
            return ret;
        }
    }

    if (want_bits & WANT_PRINT) {
        uint8_t *buff = NULL;
        uint16_t size = 0;
        int ret = rp_CalibGetEEPROM(&buff,&size,factory);
        if (ret) {
            fprintf(stderr, "ERROR: Read failed!\n");
            return ret;
        }
        rp_calib_params_t calib;
        ret =  rp_CalibConvertEEPROM(buff,size,&calib);
        if (ret) {
            fprintf(stderr, "ERROR: Convert data failed!\n");
            return ret;
        }
        rp_CalibPrint(&calib);
    }

    /* Reset to defaults */
    if (want_bits & WANT_INIT) {
        ret= rp_CalibInit();
        if (ret) {
            fprintf(stderr, "ERROR: Init failed!\n");
            return ret;
        }
        ret = rp_CalibrationReset(factory,want_bits & WANT_NEW_FORMAT);
        if (ret) {
            fprintf(stderr, "ERROR: Reset failed!\n");
            return ret;
        }
    }

    /* Read */
    if (want_bits & WANT_READ) {
        uint8_t *buff = NULL;
        uint16_t size = 0;

        int ret = rp_CalibGetEEPROM(&buff,&size,factory);

        if (ret) {
            fprintf(stderr, "ERROR: Read failed!\n");
            return ret;
        }
        if (buff[0] != RP_HW_PACK_ID_V5){
            print_eeprom(model,(rp_eepromWpData_t*)buff, want_bits);
        }else{
            print_eepromUni((rp_eepromUniData_t*)buff,want_bits);
        }
    }

    return ret;
}
