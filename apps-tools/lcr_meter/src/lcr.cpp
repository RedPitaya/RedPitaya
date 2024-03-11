#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/param.h>

#include "version.h"
#include "rp.h"
#include <sys/syslog.h> //Add custom RP_LCR LOG system

#include "lcrApp.h"

#include "rp-spi.h"
#include "rp-i2c-max7311.h"
#include "rp_hw-calib.h"

const char *g_argv0 = NULL; // Program name

bool checkFreq(int _freq){
    switch(_freq){
        case 10: return true;
        case 100: return true;
        case 1000: return true;
        case 10000: return true;
        case 100000: return true;
        default:
            return false;
    }
}

int checkShunt(int _shunt){
    switch(_shunt){
        case 0: return -1;
        case 10: return 0;
        case 100: return 1;
        case 1000: return 2;
        case 10000: return 3;
        case 100000: return 4;
        case 1000000: return 5;
        default:
            return -2;
    }
}

/** Print usage information */
void usage() {
    const char *format =
            "LCR meter version %s, compiled at %s\n"
            "\n"
            "Usage:\t%s [freq] "
                       "[r_shunt] "
                       "-v "
                       "\n"
            "\n"
            "\tfreq               Signal frequency used for measurement [ 100 , 1000, 10000 , 100000 ] Hz.\n"
            "\tr_shunt            Shunt resistor value in Ohms [ 10, 100, 1000, 10000, 100000, 1000000 ]. If set to 0, Automatic ranging is used.\n"
            "\t-v                 Verbose mode\n"
            "\t                   Automatic ranging demands Extenson module.\n"
            "\n"
            "Output:\tFrequency [Hz], |Z| [Ohm], P [deg], Ls [H], Cs [F], Rs [Ohm], Lp [H], Cp [F], Rp [Ohm], Q, D, Xs [H], Gp [S], Bp [S], |Y| [S], -P [deg]\n";

    fprintf(stderr, format, VERSION_STR, __TIMESTAMP__, g_argv0);
}
char GetPrefix(float value, float *new_value){
    if (fabs(value) > 1e6){
        *new_value = ((float)((int)value / 1000))/1000.0;
        return 'M';
    }
    if (fabs(value) > 1e3){
        *new_value = ((float)((int)value))/1000.0;
        return 'k';
    }
    if (fabs(value) > 1){
        *new_value = ((float)((int)(value * 1000)))/1000.0;
        return 0;
    }
    if (fabs(value) > 1e-3){
        *new_value = ((float)((int)(value * 1e6)))/1000.0;
        return 'm';
    }
    if (fabs(value) > 1e-6){
        *new_value = ((float)((int)(value * 1e9)))/1000.0;
        return 'u';
    }
    if (fabs(value) > 1e-9){
        *new_value = ((float)((int)(value * 1e12)))/1000.0;
        return 'n';
    }
    *new_value = 0;
    return 0;
}

auto getModel() -> rp_HPeModels_t{
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get board model\n");
    }
    return c;
}

auto getModelName() -> std::string{
    auto model = getModel();
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
            return "Z10";
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return "Z20_125";
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return "Z20";
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return "Z20_125_4CH";
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
            return "Z20_250_12";
        case STEM_250_12_120:
            return "Z20_250_12_120";
        default:{
            fprintf(stderr,"[Error:getModelName] Unknown model: %d.\n",model);
            return "";
        }
    }
    return "";
}

int main(int argc, char *argv[]) {

    int freq    = 0;
    int r_shunt = 0;
    bool verb_mode = false;
	/** Set program name */
    g_argv0 = argv[0];

    if (argc<3) {
        fprintf(stderr, "Too few arguments!\n\n");
        usage();
        return -1;
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
        if (strcmp(argv[3],"-v") == 0) {
            verb_mode = true;
        }
    }


    lcrApp_lcrInit();
    auto Connected = lcrApp_LcrCheckExtensionModuleConnection() == RP_OK;

    if (rp_HPGetFastADCIsAC_DCOrDefault()){
        rp_AcqSetAC_DC(RP_CH_1,RP_DC);
        rp_AcqSetAC_DC(RP_CH_2,RP_DC);
    }

    auto modelName = getModelName();
    if ( modelName == "Z20_250_12" || modelName == "Z20_250_12_120"){
        // This trick for check LCR module in 250-12. Because two chips are on the same address
        int maxCheck = rp_max7311::rp_check();
        if (maxCheck == 1) Connected = false;
    }

    if (Connected){
        lcr_main_data_t *data = (lcr_main_data_t *)malloc(sizeof(lcr_main_data_t));
        lcrApp_LcrSetShuntIsAuto(false);
        r_shunt == -1 ? lcrApp_LcrSetShuntIsAuto(true) : lcrApp_LcrSetShunt(r_shunt);
        lcrApp_LcrSetFrequency(freq);
        lcrApp_GenRun();
        lcrApp_LcrRun();
        int old_shunt = -2;
        int cur_shunt = 0;
        lcrApp_LcrGetShunt(&cur_shunt);
        while((r_shunt == -1) && (old_shunt !=cur_shunt)) {
            usleep(500000);
            old_shunt = cur_shunt;
            lcrApp_LcrGetShunt(&cur_shunt);
        }
        if (r_shunt >= 0) usleep(500000);
        lcrApp_LcrGetShunt(&cur_shunt);
        lcrApp_LcrCopyParams(data);
                /// Output
        char pref =0;
        float modify_value = 0;
        /*printf(" %.1f    %.3f    %.1f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f    %.10f\n",*/
        if (!verb_mode) {
         printf(" %.1f    %.3e    %.2f    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.3e    %.2f\n",

        /*"Output:\tFrequency [Hz], |Z| [Ohm], P [deg], Ls [H], Cs [F], Rs [Ohm], Lp [H], Cp [F], Rp [Ohm], Q, D, Xs [H], Gp [S], Bp [S], |Y| [S], -P [deg]\n";*/

            (float)freq,
            data->lcr_amplitude,
            data->lcr_phase,
            data->lcr_L_s, // L_s[ i ],
            data->lcr_C_s, // C_s[ i ],
            data->lcr_R_s, // R_s[ i ],
            data->lcr_L_p, // L_p[ i ],
            data->lcr_C_p, // C_p[ i ],
            data->lcr_R_p, // R_p[ i ],
            data->lcr_Q_s, // Q[ i ],
            data->lcr_D_s, // D[ i ],
            data->lcr_X_s, // X_s[ i ],
            data->lcr_G_p, // G_p[ i ],
            data->lcr_B_p, // B_p[ i ],
            data->lcr_Y_abs, // Y_abs[ i ],
            data->lcr_Phase_Y // PhaseY[ i ]
            );
        }else{
            printf("Frequency\t%d Hz\n",freq);
            pref = GetPrefix(data->lcr_amplitude,&modify_value);
            printf("Z\t%lf %cOmh\n",modify_value,pref);

            printf("Phase\t%lf deg\n",data->lcr_phase);

            pref = GetPrefix(data->lcr_L_s,&modify_value);
            printf("L(s)\t%lf %cH\n",modify_value,pref);

            pref = GetPrefix(data->lcr_C_s,&modify_value);
            printf("C(s)\t%lf %cF\n",modify_value,pref);

            pref = GetPrefix(data->lcr_R_s,&modify_value);
            printf("R(s)\t%lf %cOmh\n",modify_value,pref);

            pref = GetPrefix(data->lcr_L_p,&modify_value);
            printf("L(p)\t%lf %cH\n",modify_value,pref);

            pref = GetPrefix(data->lcr_C_p,&modify_value);
            printf("C(p)\t%lf %cF\n",modify_value,pref);

            pref = GetPrefix(data->lcr_R_p,&modify_value);
            printf("R(p)\t%lf %cOmh\n",modify_value,pref);

            printf("Q\t%lf\n",data->lcr_Q_s);
            printf("D\t%lf\n",data->lcr_D_s);
            printf("X_s\t%lf\n",data->lcr_X_s);
            printf("G_p\t%lf\n",data->lcr_G_p);
            printf("B_p\t%lf\n",data->lcr_B_p);
            printf("|Y|\t%lf\n",data->lcr_Y_abs);
            printf("-P_Y\t%lf deg\n",data->lcr_Phase_Y);
        }

        free(data);

    }else{
       fprintf(stderr, "Extension module is not connected!\n\n");
    }

    lcrApp_LcrRelease();

    return 0;
}
