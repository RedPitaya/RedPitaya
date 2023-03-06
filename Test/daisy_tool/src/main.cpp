/**
 * $Id: acquire.c 1246 2014-02-22 19:05:19Z ales.bardorfer $
 *
 * @brief Red Pitaya daisy chain utility.
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/param.h>
#include <iostream>

#include "rp.h"

/** Program name */
const char *g_argv0 = NULL;

/** Minimal number of command line arguments */
#define MINARGS 2

int get_state(bool *state, const char *str)
{
    if  (strncmp(str, "=Off", 4) == 0)   {
        *state = false;
        return 0;
    }
    if  (strncmp(str, "=On", 3) == 0)  {
        *state = true;
        return 0;
    }

    fprintf(stderr, "Unknown state: %s\n", str);
    return -1;
}

int get_mode(rp_outTiggerMode_t *state, const char *str)
{
    if  (strncmp(str, "=ADC", 4) == 0)   {
        *state = OUT_TR_ADC;
        return 0;
    }
    if  (strncmp(str, "=DAC", 4) == 0)  {
        *state = OUT_TR_DAC;
        return 0;
    }

    fprintf(stderr, "Unknown state: %s\n", str);
    return -1;
}


/** Print usage information */
void usage() {
    const char *format =
            "\n"
            "Usage: %s -e[=State] | -o[=State] | -t[=Mode] | -a[1.0] | -g[1.0] |-d   \n"
            "\n"
            "   -e    Enables clock and trigger sync over SATA daisy chain connectors.\n"
            "   -o    Turns GPION_0 into trigger output for selected source - acquisition or generation.\n"
            "   -t    Sets the trigger source mode. ADC/DAC.\n"
            "   -a    Sets ext. trigger debouncer for acquisition in Us (Value must be positive).\n"
            "   -g    Sets ext. trigger debouncer for generation in Us (Value must be positive).\n"
            "   -d    Register debug mode.\n"
            "\n"
            "Example:\n"
            "\t./daisy_tool -e=On -o=On -t=DAC -a2.2\n"
            "\n"
            "Optional parameter:\n"
            "\tState = [Off | On]  Turns On or Off\n"
            "\tMode = [ADC | DAC]  Set ADC or DAC mode\n"
            "\n"
            "Notice:"
            "\tApplication does not reset register settings when enabling modes.\n"
            "\tIf the flag does not have a parameter, it returns the value from the register.\n"
            "\n";

    fprintf( stderr, format, g_argv0);
}



/** Acquire utility main */
int main(int argc, char *argv[])
{
    g_argv0 = argv[0];
    bool e_flag = false;
    bool o_flag = false;
    bool t_flag = false;
    bool a_flag = false;
    bool g_flag = false;
    bool d_flag = false;

    bool set_e_state = false;
    bool e_state  = false;
    bool set_o_state = false;
    bool o_state  = false;
    bool set_t_state = false;
    rp_outTiggerMode_t t_state  = OUT_TR_ADC;

    bool set_a_state = false;
    double a_value = 1;
    bool set_g_state = false;
    double g_value = 1;


    if ( argc < MINARGS ) {
        usage();
        exit ( EXIT_FAILURE );
    }

    const char *optstring = "e::o::t::da::g::";

    int ch = -1;
    while ( (ch = getopt( argc, argv, optstring )) != -1 ) {
        switch ( ch ) {
            case 'd':
                d_flag = true;
                break;

            case 'e':
                e_flag = true;
                if (optarg){
                    set_e_state = true;
                    if (get_state(&e_state,optarg) != 0){
                        usage();
                        exit ( EXIT_FAILURE );
                    }
                }
                break;

            case 'o':
                o_flag = true;
                if (optarg){
                    set_o_state = true;
                    if (get_state(&o_state,optarg) != 0){
                        usage();
                        exit ( EXIT_FAILURE );
                    }
                }
                break;

            case 't':
                t_flag = true;
                if (optarg){
                    set_t_state = true;
                    if (get_mode(&t_state,optarg) != 0){
                        usage();
                        exit ( EXIT_FAILURE );
                    }
                }
                break;

            case 'a':
                a_flag = true;
                if (optarg){
                    set_a_state = true;
                    a_value = strtod(optarg, NULL);
                    if (a_value < 0.0) {
                        fprintf(stderr, "Invalid value: %s\n", optarg);
                        usage();
                        exit ( EXIT_FAILURE );
                    }
                }
                break;

            case 'g':
                g_flag = true;
                if (optarg){
                    set_g_state = true;
                    g_value = strtod(optarg, NULL);
                    if (g_value < 0.0) {
                        fprintf(stderr, "Invalid value: %s\n", optarg);
                        usage();
                        exit ( EXIT_FAILURE );
                    }
                }
                break;

        }
    }

    if (d_flag){
        rp_EnableDebugReg();
    }

    if (rp_InitReset(false) != RP_OK){
        fprintf(stderr,"Error init rp api\n");
        return -1;
    }

    if (e_flag) {
        if (set_e_state){
            if (rp_SetEnableDaisyChainSync(e_state) != RP_OK){
                fprintf(stderr,"[Error] Can't enable daisy chain mode\n");
            }
        }else{
            bool state = false;
            if (rp_GetEnableDaisyChainSync(&state) == RP_OK){
                printf("SATA daisy chain mode = %d\n",state);
            }else{
                fprintf(stderr,"[Error] Can't get daisy chain mode\n");
                return -1;
            }
        }
    }

    if (o_flag) {
        if (set_o_state){
            if (rp_SetDpinEnableTrigOutput(o_state) != RP_OK){
                fprintf(stderr,"[Error] Can't enable trigger output\n");
            }
        }else{
            bool state = false;
            if (rp_GetDpinEnableTrigOutput(&state) == RP_OK){
                printf("GPION_0 into trigger output = %d\n",state);
            }else{
                fprintf(stderr,"[Error] Can't get trigger output\n");
                return -1;
            }
        }
    }

    if (t_flag) {
        if (set_t_state){
            if (rp_SetSourceTrigOutput(t_state) != RP_OK){
                fprintf(stderr,"[Error] Can't set trigger output\n");
            }
        }else{
            rp_outTiggerMode_t mode;
            if (rp_GetSourceTrigOutput(&mode) == RP_OK){
                if (mode == OUT_TR_ADC){
                    printf("Trigger source mode = ADC\n");
                } else if (mode == OUT_TR_DAC){
                    printf("Trigger source mode = DAC\n");
                } else {
                    printf("Trigger source mode = [ERROR]\n");
                }
            }else{
                fprintf(stderr,"[Error] Can't get trigger output\n");
                return -1;
            }
        }
    }

    if (a_flag) {
        if (set_a_state){
            if (rp_AcqSetExtTriggerDebouncerUs(a_value) != RP_OK){
                fprintf(stderr,"[Error] Can't set ext. trigger ADC debouncer\n");
            }
        }else{
            double value = 0;
            if (rp_AcqGetExtTriggerDebouncerUs(&value) == RP_OK){
                printf("Ext. trigger ADC debouncer = %f\n",value);
            }else{
                fprintf(stderr,"[Error] Can't get ext. trigger ADC debouncer\n");
                return -1;
            }
        }
    }

    if (g_flag) {
        if (set_g_state){
            if (rp_GenSetExtTriggerDebouncerUs(g_value) != RP_OK){
                fprintf(stderr,"[Error] Can't set ext. trigger DAC debouncer\n");
            }
        }else{
            double value = 0;
            if (rp_GenGetExtTriggerDebouncerUs(&value) == RP_OK){
                printf("Ext. trigger DAC debouncer = %f\n",value);
            }else{
                fprintf(stderr,"[Error] Can't get ext. trigger DAC debouncer\n");
                return -1;
            }
        }
    }

    if (rp_Release() != RP_OK){
        fprintf(stderr,"Error release rp api\n");
        return -1;
    }

    return 0;
}
