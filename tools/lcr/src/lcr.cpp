#include <complex.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/syslog.h>  //Add custom RP_LCR LOG system
#include <unistd.h>

#include "apiApp/lcrApp.h"
#include "common/version.h"
#include "rp.h"

#include "rp_hw_calib.h"

const char* g_argv0 = NULL;  // Program name

bool checkFreq(int _freq) {
    return _freq >= 1 && _freq <= 1000000;

    // switch(_freq){
    //     case 10: return true;
    //     case 100: return true;
    //     case 1000: return true;
    //     case 10000: return true;
    //     case 100000: return true;
    //     default:
    //         return false;
    // }
}

int checkShunt(int _shunt) {
    switch (_shunt) {
        case 0:
            return RP_LCR_S_NOT_INIT;
        case 10:
            return 0;
        case 100:
            return 1;
        case 1000:
            return 2;
        case 10000:
            return 3;
        case 100000:
            return 4;
        case 1000000:
            return 5;
        default:
            return -2;
    }
}

/** Print usage information */
void usage() {
    const char* format =
        "LCR meter version %s, compiled at %s\n"
        "\n"
        "Usage:\t%s freq r_shunt [-v]\n"
        "\n"
        "\tfreq               Signal frequency used for measurement in Hz.\n"
        "\tr_shunt            Shunt resistor value in Ohms Ω [ 10, 100, 1000, 10000, 100000, 1000000 ]. If set to 0, Automatic ranging is used.\n"
        "\t-v                 Verbose mode\n"
        "\n"
        "Output:\tFrequency [Hz], |Z|, Ohm [Ω], P [deg], Ls [H], Cs [F], Rs [Ω], Lp [H], Cp [F], Rp [Ω], Q, D, Xs [H], Gp [S], Bp [S], |Y| [S], -P [deg]\n";

    fprintf(stderr, format, VERSION_STR, __TIMESTAMP__, g_argv0);
}
char GetPrefix(float value, float* new_value) {
    if (fabs(value) > 1e6) {
        *new_value = ((float)((int)value / 1000)) / 1000.0;
        return 'M';
    }
    if (fabs(value) > 1e3) {
        *new_value = ((float)((int)value)) / 1000.0;
        return 'k';
    }
    if (fabs(value) > 1) {
        *new_value = ((float)((int)(value * 1000))) / 1000.0;
        return 0;
    }
    if (fabs(value) > 1e-3) {
        *new_value = ((float)((int)(value * 1e6))) / 1000.0;
        return 'm';
    }
    if (fabs(value) > 1e-6) {
        *new_value = ((float)((int)(value * 1e9))) / 1000.0;
        return 'u';
    }
    if (fabs(value) > 1e-9) {
        *new_value = ((float)((int)(value * 1e12))) / 1000.0;
        return 'n';
    }
    *new_value = 0;
    return 0;
}

int main(int argc, char* argv[]) {

    int freq = 0;
    int r_shunt = 0;
    bool verb_mode = false;
    /** Set program name */
    g_argv0 = argv[0];

    if (argc < 3) {
        fprintf(stderr, "Too few arguments!\n\n");
        usage();
        return -1;
    }

    if (strcmp(argv[1], "-s") == 0) {
        lcr_shunt_t shunt = (lcr_shunt_t)atoi(argv[2]);
        return lcrApp_LcrSetShunt(shunt);
    }

    freq = atoi(argv[1]);
    if (!checkFreq(freq)) {
        fprintf(stderr, "Invalid frequency value!\n\n");
        usage();
        return -1;
    }

    r_shunt = atoi(argv[2]);
    r_shunt = checkShunt(r_shunt);
    if (r_shunt == -2) {
        fprintf(stderr, "Invalid shunt value!\n\n");
        usage();
        return -1;
    }

    if (argc >= 4) {
        if (strcmp(argv[3], "-v") == 0) {
            verb_mode = true;
        }
    }

    lcrApp_lcrInit();
    auto Connected = lcrApp_LcrCheckExtensionModuleConnection(true) == RP_OK;

    if (Connected) {
        if (rp_HPGetFastADCIsAC_DCOrDefault()) {
            rp_AcqSetAC_DC(RP_CH_1, RP_DC);
            rp_AcqSetAC_DC(RP_CH_2, RP_DC);
        }

        lcr_main_data_t* data = (lcr_main_data_t*)malloc(sizeof(lcr_main_data_t));
        lcrApp_LcrSetShuntIsAuto(false);
        r_shunt == RP_LCR_S_NOT_INIT ? lcrApp_LcrSetShuntIsAuto(true) : lcrApp_LcrSetShunt((lcr_shunt_t)r_shunt);
        lcrApp_LcrSetFrequency(freq);
        lcrApp_GenRun();
        lcrApp_LcrRun();
        lcr_shunt_t old_shunt = RP_LCR_S_NOT_INIT;
        lcr_shunt_t cur_shunt = RP_LCR_S_NOT_INIT;
        lcrApp_LcrGetShunt(&cur_shunt);
        while ((r_shunt == RP_LCR_S_NOT_INIT) && (old_shunt != cur_shunt)) {
            usleep(500000);
            old_shunt = cur_shunt;
            lcrApp_LcrGetShunt(&cur_shunt);
        }
        if (r_shunt >= 0)
            usleep(500000);
        lcrApp_LcrGetShunt(&cur_shunt);
        lcrApp_LcrCopyParams(data);
        /// Output
        char pref = 0;
        float modify_value = 0;
        /*printf(" %.1f    %.3f    %.1f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f\n",*/
        if (!verb_mode) {
            printf(" %.1f    %.3e    %.2f    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.2f\n",

                   /*"Output:\tFrequency [Hz], |Z| [Ohm], P [deg], Ls [H], Cs [F], Rs [Ohm], Lp [H], Cp [F], Rp [Ohm], Q, D, Xs [H], Gp [S], Bp [S], |Y| [S], -P [deg]\n";*/

                   (float)freq,
                   data->lcr_amplitude,
                   data->lcr_phase,
                   data->lcr_L_s,     // L_s[ i ],
                   data->lcr_C_s,     // C_s[ i ],
                   data->lcr_R_s,     // R_s[ i ],
                   data->lcr_L_p,     // L_p[ i ],
                   data->lcr_C_p,     // C_p[ i ],
                   data->lcr_R_p,     // R_p[ i ],
                   data->lcr_Q_s,     // Q[ i ],
                   data->lcr_D_s,     // D[ i ],
                   data->lcr_X_s,     // X_s[ i ],
                   data->lcr_G_p,     // G_p[ i ],
                   data->lcr_B_p,     // B_p[ i ],
                   data->lcr_Y_abs,   // Y_abs[ i ],
                   data->lcr_Phase_Y  // PhaseY[ i ]
            );
        } else {
            printf("Frequency\t%d Hz\n", freq);
            pref = GetPrefix(data->lcr_amplitude, &modify_value);
            printf("Z\t%lf %cOhm Ω\n", modify_value, pref);

            printf("Phase\t%lf deg\n", data->lcr_phase);

            pref = GetPrefix(data->lcr_L_s, &modify_value);
            printf("L(s)\t%lf %cH\n", modify_value, pref);

            pref = GetPrefix(data->lcr_C_s, &modify_value);
            printf("C(s)\t%lf %cF\n", modify_value, pref);

            pref = GetPrefix(data->lcr_R_s, &modify_value);
            printf("R(s)\t%lf %cOmh Ω\n", modify_value, pref);

            pref = GetPrefix(data->lcr_L_p, &modify_value);
            printf("L(p)\t%lf %cH\n", modify_value, pref);

            pref = GetPrefix(data->lcr_C_p, &modify_value);
            printf("C(p)\t%lf %cF\n", modify_value, pref);

            pref = GetPrefix(data->lcr_R_p, &modify_value);
            printf("R(p)\t%lf %cOmh Ω\n", modify_value, pref);

            printf("Q\t%lf\n", data->lcr_Q_s);
            printf("D\t%lf\n", data->lcr_D_s);
            printf("X_s\t%lf\n", data->lcr_X_s);
            printf("G_p\t%lf\n", data->lcr_G_p);
            printf("B_p\t%lf\n", data->lcr_B_p);
            printf("|Y|\t%lf\n", data->lcr_Y_abs);
            printf("-P_Y\t%lf deg\n", data->lcr_Phase_Y);
        }

        free(data);

    } else {
        fprintf(stderr, "Extension module is not connected!\n");
    }

    lcrApp_LcrRelease();

    return 0;
}
