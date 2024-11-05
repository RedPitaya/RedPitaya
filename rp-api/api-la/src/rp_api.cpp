
#include "rp_api.h"

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "common/common.h"

#include "la_acq.h"
#include "rp.h"

typedef struct {
    int16_t *      buf;
    size_t         buf_size;
    uint32_t       pre_samples;
    uint32_t       post_samples;
    uint32_t       trig_sample;
    uint32_t       last_sample;
} rp_acq_data_t;

/** Maximal digital signal sampling frequency [Hz] */
const double c_max_dig_sampling_rate = 125e6;

/** Maximal digital signal sampling frequency time interval [nS] */
const double c_max_dig_sampling_rate_time_interval_ns = 1e9 / c_max_dig_sampling_rate;

rp_handle_uio_t la_acq_handle;

rp_acq_data_t acq_data;

bool g_acq_running = false;

/**
 * Open device
 */
int rp_OpenUnit() {
    return rp_LaAcqOpen("/dev/uio/la", &la_acq_handle);
}

/**
 * Close device
 */
int rp_CloseUnit() {
    return rp_LaAcqClose(&la_acq_handle);
}

int rp_SetTriggerDigitalPortProperties(RP_DIGITAL_CHANNEL_DIRECTIONS * directions,
                                            uint16_t nDirections){
    // disable triggering by default
    rp_LaAcqGlobalTrigSet(&la_acq_handle, RP_TRG_ALL_MASK);

    rp_la_trg_regset_t trg;
    memset(&trg,0,sizeof(rp_la_trg_regset_t));

    // none of triggers is enabled
    if(directions == NULL){
        return RP_OK;
    }

    uint32_t n;
    uint32_t edge_cnt=0;
    // set trigger pattern settings
    for(n = 0; n < nDirections; n++){
        uint32_t mask = (1<<directions[n].channel);
        if(edge_cnt > 1){
            // more than one pin is set to rising/falling edge
            return RP_EIPV;
        }
        switch(directions[n].direction){
            case RP_DIGITAL_DONT_CARE:
                // default value is don't care
                break;
            case RP_DIGITAL_DIRECTION_LOW:
                trg.cmp_msk|=mask;
                // default val is low
                break;
            case RP_DIGITAL_DIRECTION_HIGH:
                trg.cmp_msk|=mask;
                trg.cmp_val|=mask;
                break;
            case RP_DIGITAL_DIRECTION_RISING:
                trg.edg_pos|=mask;
                edge_cnt++;
                break;
            case RP_DIGITAL_DIRECTION_FALLING:
                trg.edg_neg|=mask;
                edge_cnt++;
                break;
            case RP_DIGITAL_DIRECTION_RISING_OR_FALLING:
                trg.edg_pos|=mask;
                trg.edg_neg|=mask;
                edge_cnt++;
                break;
            default:
                return RP_EIPV;
                break;
        }
    }

    // update settings
    if(edge_cnt==1){
    	// edge - enable pattern & software trigger
        rp_LaAcqSetTrigSettings(&la_acq_handle, trg);
        rp_LaAcqGlobalTrigSet(&la_acq_handle, RP_TRG_LOA_PAT_MASK | RP_TRG_LOA_SWE_MASK);
    }
    else{
    	// else leave only software trigger on
        rp_LaAcqSetTrigSettings(&la_acq_handle, trg);
        rp_LaAcqGlobalTrigSet(&la_acq_handle, RP_TRG_LOA_SWE_MASK);
    }

    return RP_OK;
}

int rp_EnableDigitalPortDataRLE(bool enable){
    return enable ? rp_LaAcqEnableRLE(&la_acq_handle) : rp_LaAcqDisableRLE(&la_acq_handle);
}

int rp_SoftwareTrigger(){
	if(g_acq_running){
		rp_LaAcqTriggerAcq(&la_acq_handle);
	}
    return RP_OK;
}

int rp_SetPolarity(uint32_t reg){
	return rp_LaAcqSetPolarity(&la_acq_handle, reg);
}

int rp_GetTimebase(uint32_t timebase,
                        double * timeIntervalNanoseconds
                        ){
    *timeIntervalNanoseconds = timebase * c_max_dig_sampling_rate_time_interval_ns;
    return RP_OK;
}


int rp_SetDataBuffer(int16_t * buffer,
                    size_t size
                    // uint32_t segmentIndex,
                                // RP_RATIO_MODE mode
                    ) {
    acq_data.buf = buffer;
    acq_data.buf_size = size;
    return RP_OK;
}





int rp_RunBlock(uint32_t noOfPreTriggerSamples,
                     uint32_t noOfPostTriggerSamples,
                     uint32_t timebase,
                     double * timeIndisposedMs){
    double timeIntervalNanoseconds;
    uint32_t maxSamples = rp_LaAcqBufLenInSamples(&la_acq_handle);
    rp_GetTimebase(timebase,&timeIntervalNanoseconds);

    if(!(inrangeUint32 (noOfPreTriggerSamples + noOfPostTriggerSamples, 10, maxSamples))){
        return RP_EIPV;
    }

    *timeIndisposedMs = (noOfPreTriggerSamples + noOfPostTriggerSamples) * timeIntervalNanoseconds / 10e6;

// configure FPGA to start block mode

   // TODO; sampling rate
    rp_la_decimation_regset_t dec;
    dec.dec=timebase;
    rp_LaAcqSetDecimation(&la_acq_handle, dec);

    rp_la_cfg_regset_t cfg;
    cfg.pre = noOfPreTriggerSamples;
    cfg.pst = noOfPostTriggerSamples;
    if(rp_LaAcqSetCntConfig(&la_acq_handle, cfg)!=RP_OK){
        return RP_EIPV;
    }

    TRACE_SHORT("rp_LaAcqRunAcq");
    // start acq.
    if(rp_LaAcqRunAcq(&la_acq_handle) != RP_OK){
        rp_LaAcqStopAcq(&la_acq_handle);
        return RP_EOP;
    }

    // block till acq. is complete
    TRACE_SHORT("Blocking read");
    g_acq_running = true;
    rp_LaAcqBlockingRead(&la_acq_handle);
    g_acq_running = false;

    // make sure acq. is stopped
    bool isStoped;
    rp_LaAcqAcqIsStopped(&la_acq_handle, &isStoped);
    if(!isStoped){
        TRACE_SHORT("Blocking read not stopped!!");
        rp_LaAcqStopAcq(&la_acq_handle);
        return RP_EOP;
    }

    uint32_t trig_sample;
    uint32_t last_sample;

    bool rle;
    rp_LaAcqIsRLE(&la_acq_handle,&rle);
    if(rle){
    	// in the RLE mode we only check which was the last sample where acq. stopped
        uint32_t current;
        bool buf_ovfl;
        rp_LaAcqGetRLEStatus(&la_acq_handle, &current, &last_sample, &buf_ovfl);
        trig_sample=0;
    }
    else{
        // get trigger position
        uint32_t pst_length;
        bool buf_ovfl;
        if(rp_LaAcqGetCntStatus(&la_acq_handle, &trig_sample, &pst_length, &buf_ovfl) != RP_OK){
            rp_LaAcqStopAcq(&la_acq_handle);
            return RP_EOP;
        }

        TRACE_SHORT("Trig_sample %d, pst_length %d, buf_ovfl %d", trig_sample, pst_length, buf_ovfl);

		// acquired number of post samples must match to req
		if(pst_length != noOfPostTriggerSamples){
			rp_LaAcqStopAcq(&la_acq_handle);
			return RP_EOP;
		}
    }

    // save properties of current acq.
    acq_data.pre_samples=noOfPreTriggerSamples;
    acq_data.post_samples=noOfPostTriggerSamples;
    acq_data.trig_sample=trig_sample;
    acq_data.last_sample=last_sample;

    rp_LaAcqStopAcq(&la_acq_handle);

    return RP_OK;
}


int rp_GetTrigPosition(uint32_t * tigger_pos){
	*tigger_pos = acq_data.trig_sample;
	return RP_OK;
}


int rp_GetValues(uint32_t * noOfSamples){

    int16_t *map = (int16_t *)mmap(NULL, la_acq_handle.dma_size, PROT_READ | PROT_WRITE, MAP_SHARED, la_acq_handle.dma_fd, 0);
    if (map == NULL) {
        ERROR_LOG("Failed to mmap");
        if (la_acq_handle.dma_fd != -1) {
            close(la_acq_handle.dma_fd);
        }
        return RP_EMMD;
    }

    bool rle;
    rp_LaAcqIsRLE(&la_acq_handle,&rle);
    if(rle){ // RLE mode
        // try to find first sample and trigger sample
        uint32_t first_sample=0;
        uint32_t samples=0;
        acq_data.trig_sample=0;

        uint32_t fist_sample_len_adj;

        uint32_t len=0;
        uint32_t i=0;
        uint32_t index = acq_data.last_sample;
        uint32_t total = acq_data.pre_samples+acq_data.post_samples;

        bool trig_sample_found=false;
        // find first and last sample
        i=0;
        len=0;
        for(;;){

            i++;
            len += ((uint8_t)(map[index]>>8)) + 1;

            // trigger position
            if(!trig_sample_found){
                if(len>=(acq_data.post_samples-1)){
                    acq_data.trig_sample=i;
                    trig_sample_found=true;
                }
            }

            // first sample position
            if(len>=total){
                first_sample=index;
                samples=i;
                // adjust first sample length so that it fits to the
                // length of requested data
                fist_sample_len_adj=len-total;
                break;
            }

            if(index==0){
                index=rp_LaAcqBufLenInSamples(&la_acq_handle)-1;
            }
            else{
                index--;
            }
        }
        // copy data
        index=first_sample;
        for(i=0; i < samples; i++){
            if(index>=rp_LaAcqBufLenInSamples(&la_acq_handle)){
                index=0;
            }
            acq_data.buf[i]=map[index];
            // adjust length of first sample
            if(i==0){
                acq_data.buf[i]-=(int16_t)(fist_sample_len_adj<<8);
            }
            index++;
        }
        acq_data.trig_sample=samples-acq_data.trig_sample;
        *noOfSamples=samples;
    }
    else{

        int32_t first_sample = acq_data.trig_sample - acq_data.pre_samples;
        int32_t last_sample = acq_data.trig_sample + acq_data.post_samples;
        int32_t buf_len = rp_LaAcqBufLenInSamples(&la_acq_handle);

        // printf("\n\r req: pre=%d pos=%d\n\r", acq_data.pre_samples, acq_data.post_samples);
        // printf("\n\r sta: first=%d trig=%d last=%d\n\r", first_sample, acq_data.trig_sample, last_sample);

        int32_t wlen;
        if(first_sample < 0){

//          printf("\n\r first_sample > last_sample\n\r");
            wlen = abs(first_sample);
            first_sample = buf_len + first_sample;
//          printf("\n\r sta: first=%d wlen=%d\n\r", first_sample, wlen);
            for(int i=0; i < wlen; i++){
                acq_data.buf[i]=map[first_sample+i];
            }
            for(int i=0; i < last_sample; i++){
                acq_data.buf[wlen+i]=map[i];
            }

        }
        else if(last_sample >= buf_len){
//          printf("\n\r last_sample > size\n\r");
            wlen=buf_len-first_sample;
            for(int i=0; i < wlen; i++){
                acq_data.buf[i] = map[first_sample+i];
            }
            last_sample-=buf_len;
            for(int i=0; i< last_sample; i++){
                acq_data.buf[wlen+i] = map[i];
            }
        }
        else{
//          printf("\n\r last_sample > first_sample\n\r");
            for(uint32_t i=0; i < (*noOfSamples); i++){
                acq_data.buf[i] = map[first_sample+i];
            }
        }
    }

    if(munmap (map, la_acq_handle.dma_size)==-1){
        ERROR_LOG("Failed to munmap");
        return RP_EUMD;
    }

    return RP_OK;
};


int rp_Stop(void){
	return rp_SoftwareTrigger();
}

