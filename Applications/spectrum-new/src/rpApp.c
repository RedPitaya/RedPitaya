/**
* $Id: $
*
* @brief Red Pitaya lapplication ibrary API interface implementation
*
* @Author Red Pitaya
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "rpApp.h"
#include "worker.h"
#include "spectrometerApp.h"

void SpecIntervalInit();

extern rp_app_params_t rp_main_params[PARAMS_NUM+1];

// SPECTRUM

int rpApp_SpecRun(void)
{
    fprintf(stderr, "SPEC RUN Loading spectrum version %s-%s.\n", "1" ,"1");

    if(rp_spectr_worker_init() < 0) {
	    fprintf(stderr, "rp_spectr_worker_init failed\n");
        return -1;
    }

    rp_set_params(rp_main_params, PARAMS_NUM);

    rp_spectr_worker_change_state(rp_spectr_auto_state);

	SpecIntervalInit();

    return 0;
}

int rpApp_SpecStop(void)
{
    fprintf(stderr, "Unloading spectrum version %s-%s.\n", "1", "1");

    rp_spectr_worker_exit();

    return 0;
}

int rpApp_SpecGetViewData(int source, float *data, size_t size)
{
    return rp_spectr_get_signals_channel(source, data, size);
}

int rpApp_SpecGetJpgIdx(int* jpg)
{
	rp_spectr_worker_res_t res;
	int ret = rp_spectr_get_params(&res);
	if (!ret)
	{
		*jpg = res.jpg_idx;
	}

	return ret;
}

int rpApp_SpecGetPeakPower(int channel, float* power)
{
	rp_spectr_worker_res_t res;
	int ret = rp_spectr_get_params(&res);
	if (!ret)
	{
		*power = channel == 0 ? res.peak_pw_cha : res.peak_pw_chb;
	}

	return ret;
}

int rpApp_SpecGetPeakFreq(int channel, float* freq)
{
	rp_spectr_worker_res_t res;
	int ret = rp_spectr_get_params(&res);
	if (!ret)
	{
		*freq = channel == 0 ? res.peak_pw_freq_cha : res.peak_pw_freq_chb;
	}

	return ret;
}

int rpApp_SpecSetUnit(int freq)
{
	int unit = spectr_fpga_cnv_freq_range_to_unit(freq);

	rp_spectr_worker_update_params_by_idx(unit, FREQ_UNIT_PARAM, 1);
	rp_spectr_worker_update_params_by_idx(unit, PEAK_UNIT_CHA_PARAM, 1);
	rp_spectr_worker_update_params_by_idx(unit, PEAK_UNIT_CHB_PARAM, 1);

	return unit;
}
