
#include "rp_api.h"

#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#include "common.h"

#include "la_acq.h"

/** SIGNAL ACQUISTION  */

/** Maximal digital signal sampling frequency [Hz] */
const double c_max_dig_sampling_rate = 125e6;

/** Maximal digital signal sampling frequency time interval [nS] */
const double c_max_dig_sampling_rate_time_interval_ns = 8;

rp_handle_uio_t la_acq_handle;
rp_handle_uio_t sig_gen_handle;

rp_acq_data_t acq_data;

bool g_acq_running=false;

/**
 * Open device
 */
RP_STATUS rp_OpenUnit(void) {
    int r=RP_API_OK;
    if(rp_LaAcqOpen("/dev/uio/la", &la_acq_handle)!=RP_API_OK){
        r=-1;
    }
    return r;
}

/**
 * Close device
 */
RP_STATUS rp_CloseUnit(void) {
    int r=RP_API_OK;
    if(rp_LaAcqClose(&la_acq_handle)!=RP_API_OK){
        r=-1;
    }
    return r;
}

/**
 * Enable digital port
 *
 * This function will set the individual digital channels' trigger directions. Each trigger
 * direction consists of a channel name and a direction. If the channel is not included in
 * the array of RP_DIGITAL_CHANNEL_DIRECTIONS the driver assumes the
 * digital channel's trigger direction is RP_DIGITAL_DONT_CARE.
 *
 * @param directions      A pointer to an array of structures describing the
 *                         requested properties.
 *                         If directions is NULL, digital triggering is switched off.
 *                         A digital channel that is not included in the array will be set to RP_DIGITAL_DONT_CARE.
 * @param nDirections      The number of digital channel directions being
 *                      passed to the driver.
 *
 */
RP_STATUS rp_SetTriggerDigitalPortProperties(RP_DIGITAL_CHANNEL_DIRECTIONS * directions,
                                            int16_t nDirections)
{
    // disable triggering by default
    rp_LaAcqGlobalTrigSet(&la_acq_handle, RP_TRG_ALL_MASK);

    rp_la_trg_regset_t trg;
    memset(&trg,0,sizeof(rp_la_trg_regset_t));

    // none of triggers is enabled
    if(directions==NULL){
        return RP_API_OK;
    }

    uint32_t n;
    uint32_t edge_cnt=0;
    // set trigger pattern settings
    for(n=0; n < nDirections; n++){
        uint32_t mask = (1<<directions[n].channel);
        if(edge_cnt>1){
            // more than one pin is set to rising/falling edge
            return RP_INVALID_PARAMETER;
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
                return RP_INVALID_PARAMETER;
                break;
        }
    }

    // update settings
    if(edge_cnt==1){
    	// edge - enable pattern & software trigger
        rp_LaAcqSetTrigSettings(&la_acq_handle, trg);
        rp_LaAcqGlobalTrigSet(&la_acq_handle, RP_TRG_LOA_PAT_MASK|RP_TRG_LOA_SWE_MASK);
    }
    else{
    	// else leave only software trigger on
        rp_LaAcqSetTrigSettings(&la_acq_handle, trg);
        rp_LaAcqGlobalTrigSet(&la_acq_handle, RP_TRG_LOA_SWE_MASK);
    }

    //rp_LaAcqFpgaRegDump(&la_acq_handle);

    return RP_API_OK;
}

RP_STATUS rp_EnableDigitalPortDataRLE(bool enable)
{
    if(enable){
        rp_LaAcqEnableRLE(&la_acq_handle);
    }
    else{
        rp_LaAcqDisableRLE(&la_acq_handle);
    }
    return RP_API_OK;
}

RP_STATUS rp_SoftwareTrigger(void){
	if(g_acq_running){
		rp_LaAcqTriggerAcq(&la_acq_handle);
	    return RP_API_OK;
	}
	else{
		return RP_INVALID_STATE;
	}
}

RP_STATUS rp_SetPolarity(uint32_t reg)
{
	return rp_LaAcqSetPolarity(&la_acq_handle, reg);
}

RP_STATUS rp_IsAcquistionComplete(void){
    int i=0;
    while(i<3){
        sleep(1);
        bool status;
        //rp_LaAcqFpgaRegDump(&la_acq_handle);
        rp_LaAcqAcqIsStopped(&la_acq_handle, &status);
        if(status){
            uint32_t trig_addr;
            uint32_t pst_length;
            bool buf_ovfl;
            rp_LaAcqGetCntStatus(&la_acq_handle, &trig_addr, &pst_length, &buf_ovfl);
            printf("\n\r trig_addr=%d pst_length=%d\n\r", trig_addr, pst_length);
            return RP_API_OK;
        }
        else{
            i++;
        }
        //rp_LaAcqTriggerAcq(&la_acq_handle);
    }
   // rp_LaAcqFpgaRegDump(&la_acq_handle);
    return RP_TRIGGER_ERROR;
}

/**
 * Enable digital port
 *
 * This function calculates the sampling rate and maximum number of samples for a
 * given timebase under the specified conditions. The result will depend on the number of
 * channels enabled by the last call to rpSetChannel().
 *
 * @param timebase        Timebase factor
 * @param noSamples      The number of samples required
 * @param timeIntervalNanoseconds     On exit, the time interval between
 *                                      readings at the selected timebase.
 * @param maxSamples    On exit, the maximum number of samples
 *                         available. The result may vary depending on the number of channels
 *                         enabled and the timebase chosen.
 *
 */
RP_STATUS rp_GetTimebase(uint32_t timebase,
                        int32_t noSamples,
                        double * timeIntervalNanoseconds,
                        //int16_t oversample,
                        uint32_t * maxSamples
                        //uint32_t segmentIndex
                        )
{
    *timeIntervalNanoseconds=timebase*c_max_dig_sampling_rate_time_interval_ns;
    return RP_API_OK;
};


/**
 * Set data buffer
 *
 * This function tells the driver where to store the data.
 *
 * @param buffer     The location of the buffer
 * @param bufferLth  The size of the buffer array (notice that one sample is 16 bits)
 *
 */
RP_STATUS rp_SetDataBuffer(      int16_t * buffer,
                                 int32_t bufferLth,
                                // uint32_t segmentIndex,
                                RP_RATIO_MODE mode) {
            acq_data.buf = buffer;
            acq_data.buf_size = bufferLth;
            return RP_API_OK;
}


/**
 *
 * This function tells the driver where to store the data.
 *
 * @param noOfPreTriggerSamples,    The number of samples to return before the trigger event.
 *                                     If no trigger has been set then this argument is ignored and
 *                                     noOfPostTriggerSamples specifies the maximum number of samples to collect.
 * @param noOfPostTriggerSamples      the number of samples to be taken after a trigger event.
 *                                     If no trigger event has been set then this specifies the maximum number of
 *                                     samples to be taken.
 *                                     If a trigger condition has been set, this specifies the number of
 *                                     samples to be taken after a trigger has fired, and the number of samples
 *                                     to be collected is then: noOfPreTriggerSamples + noOfPostTriggerSamples
 * @param timebase                     Timebase
 * @param timeIndisposedS            On exit, the time, in milliseconds, that the scope will spend collecting samples.
 * @param rpReady                     A pointer to the rpBlockReady() callback function that the driver will
 *                                     call when the data has been collected.
 *
 * @param pParameter                A void pointer that is passed to the rpBlockReady() callback function.
 */

#include "rp_dma.h"

RP_STATUS rp_RunBlock(uint32_t noOfPreTriggerSamples,
                     uint32_t noOfPostTriggerSamples,
                     uint32_t timebase,
                    // int16_t oversample,
                     double * timeIndisposedMs,
                     //uint32_t segmentIndex,
                     rpBlockReady rpReady,
                     void * pParameter
)
{
    double timeIntervalNanoseconds;
    uint32_t maxSamples=rp_LaAcqBufLenInSamples(&la_acq_handle);

    if(rp_GetTimebase(timebase,0,&timeIntervalNanoseconds,&maxSamples)!=RP_API_OK){
        return RP_INVALID_TIMEBASE;
    }

    printf("\r\n timeIntervalNanoseconds: %f", timeIntervalNanoseconds);

    if(!(inrangeUint32 (noOfPreTriggerSamples+noOfPostTriggerSamples, 10, maxSamples))){
        return RP_INVALID_PARAMETER;
    }

    printf("\r\n max: %d", maxSamples);

    *timeIndisposedMs=(noOfPreTriggerSamples+noOfPostTriggerSamples)*timeIntervalNanoseconds/10e6;

    // configure FPGA to start block mode

   // TODO; sampling rate
    rp_la_decimation_regset_t dec;
    dec.dec=timebase;
    rp_LaAcqSetDecimation(&la_acq_handle, dec);

    rp_la_cfg_regset_t cfg;
    cfg.pre=noOfPreTriggerSamples;
    cfg.pst=noOfPostTriggerSamples;
    if(rp_LaAcqSetCntConfig(&la_acq_handle, cfg)!=RP_OK){
        return RP_INVALID_PARAMETER;
    }

    printf("\r\nrp_LaAcqRunAcq");
    // start acq.
    if(rp_LaAcqRunAcq(&la_acq_handle)!=RP_OK){
        rp_LaAcqStopAcq(&la_acq_handle);
        return RP_BLOCK_MODE_FAILED;
    }

    //rp_LaAcqFpgaRegDump(&la_acq_handle);

    // block till acq. is complete
    printf("\r\nBlocking read");
    g_acq_running=true;
    rp_LaAcqBlockingRead(&la_acq_handle);
    g_acq_running=false;

    // make sure acq. is stopped
    bool isStoped;
    rp_LaAcqAcqIsStopped(&la_acq_handle, &isStoped);
    if(!isStoped){
        printf("\r\n not stopped!!");
        rp_LaAcqStopAcq(&la_acq_handle);
        return RP_BLOCK_MODE_FAILED;
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
        if(rp_LaAcqGetCntStatus(&la_acq_handle, &trig_sample, &pst_length, &buf_ovfl)!=RP_OK){
            rp_LaAcqStopAcq(&la_acq_handle);
            return RP_BLOCK_MODE_FAILED;
        }

        printf("\r\n trig_sample %d, pst_length %d, buf_ovfl %d", trig_sample, pst_length, buf_ovfl);

		// acquired number of post samples must match to req
		if(pst_length!=noOfPostTriggerSamples){
			rp_LaAcqStopAcq(&la_acq_handle);
			return RP_BLOCK_MODE_FAILED;
		}
    }

    // save properties of current acq.
    acq_data.pre_samples=noOfPreTriggerSamples;
    acq_data.post_samples=noOfPostTriggerSamples;
    acq_data.trig_sample=trig_sample;
    acq_data.last_sample=last_sample;

    // acquisition is completed -> callback
    RP_STATUS status=RP_API_OK;
    (*rpReady)(status,pParameter);

    rp_LaAcqStopAcq(&la_acq_handle);

    return RP_API_OK;
}


/**
 *
 * Start collecting data in streaming mode.
 * When data has been collected from the device it is down-sampled if necessary and then
 * delivered to the user. Call rpGetStreamingLatestValues() to retrieve the data.
 *
 * When a trigger is set, the total number of samples stored in the driver is the sum of maxPreTriggerSamples and maxPostTriggerSamples.
 *
 * @param sampleInterval             On entry, the requested time interval between samples;
 *                                     on exit, the actual time interval used.
 * @param sampleIntervalTimeUnits   The unit of time used for sampleInterval.
 *
 * @param maxPreTriggerSamples        The maximum number of raw samples before a trigger event for each enabled channel.
 *                                     If no trigger has been set then this argument is ignored and
 *                                     maxPostTriggerSamples specifies the maximum number of samples to collect.
 *
 * @param maxPostTriggerSamples     The maximum number of raw samples after a trigger event for each enabled channel.
 *                                     If no trigger condition is set, this argument states the maximum number of samples to be stored.
 *
 * @param autoStop                     A flag that specifies if the streaming should stop when all of maxSamples have been captured.
 *
 * @param downSampleRatio             See rpGetValues()
 *
 * @param downSampleRatioMode         See rpGetValues()
 *
 * @param overviewBufferSize         The size of the overview buffers. These are temporary buffers used for storing the data
 *                                     before returning it to the application.
 *                                     The size is the same as the bufferLth value passed to rpSetDataBuffer().
 */
RP_STATUS rp_RunStreaming(uint32_t * sampleInterval,
                        RP_TIME_UNITS sampleIntervalTimeUnits,
                        uint32_t maxPreTriggerSamples,
                        uint32_t maxPostTriggerSamples,
                        int16_t autoStop,
                        uint32_t downSampleRatio,
                        RP_RATIO_MODE downSampleRatioMode,
                        uint32_t overviewBufferSize)
{

    // configure FPGA to start block mode
    return RP_API_OK;
};


RP_STATUS rp_GetTrigPosition(uint32_t * tigger_pos){
	*tigger_pos=acq_data.trig_sample;
	return RP_API_OK;
}

/**
 * This function returns block-mode data, with or without down-sampling, starting at the
 * specified sample number. It is used to get the stored data from the driver after data
 * collection has stopped.
 *
 * This function tells the driver where to store the data.
 *
 * @param startIndex        A zero-based index that indicates the start point for data collection.
 *                             It is measured in sample intervals from the start of the buffer.
 * @param noOfSamples          On entry, the number of samples required. On exit, the actual number retrieved.
 *                          The number of samples retrieved will not be more than the number requested,
 *                          and the data retrieved starts at startIndex.
 * @param downSampleRatio     The down-sampling factor that will be applied to the raw data.
 * @param downSampleRatioMode Which down-sampling mode to use.
 * @param overflow             On exit, a set of flags that indicate whether an over-voltage has occurred
 *                             on any of the channels. It is a bit field with bit 0 denoting Channel A.
 *
 */
RP_STATUS rp_GetValues(uint32_t startIndex,
                      uint32_t * noOfSamples,
                      uint32_t downSampleRatio,
                      RP_RATIO_MODE downSampleRatioMode,
                      //uint32_t segmentIndex,
                      int16_t * overflow){

    // TODO: startIndex & noOfSamples not used yet..

    int16_t * map=NULL;
    map = (int16_t *) mmap(NULL, la_acq_handle.dma_size, PROT_READ | PROT_WRITE, MAP_SHARED, la_acq_handle.dma_fd, 0);
    if (map==NULL) {
        printf("Failed to mmap\n");
        if (la_acq_handle.dma_fd) {
            close(la_acq_handle.dma_fd);
        }
        return -1;
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
       // printf("\n\rtotal: %d", total);
        bool trig_sample_found=false;

			// find first and last sample
			i=0;
			len=0;
			for(;;){

				i++;
				len+=((uint8_t)(map[index]>>8))+1;

				// trigger position
				if(!trig_sample_found){
					if(len>=(acq_data.post_samples-1)){
						//printf("\n\rtrig. sample found: i=%d len=%d", i, len);
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

					//printf("len %d >= total %d\n", len, total);
					break;
				}

			    //printf("\n\r sta: samples=%d trig=%d, ", samples, acq_data.trig_sample);
			    //printf("\r\n %d len: %02x val: %02x ", i,(uint8_t)(map[index]>>8),(uint8_t)map[index]);

				if(index==0){
					index=rp_LaAcqBufLenInSamples(&la_acq_handle)-1;
				}
				else{
					index--;
				}


			}

		//	printf("fist_sample_len_adj %d\n", fist_sample_len_adj);

			// copy data
			index=first_sample;
			for(i=0;i<samples;i++){
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

			printf("\n\r sta: samples=%d trig=%d", samples, acq_data.trig_sample);
			for(i=0;i<samples;i++){
				printf("\r\n %d len: %02x val: %02x ", i,(uint8_t)(acq_data.buf[i]>>8),(uint8_t)acq_data.buf[i]);
			}

			*noOfSamples=samples;
    }
    else{

        int32_t first_sample =   acq_data.trig_sample - acq_data.pre_samples;
        int32_t last_sample =   acq_data.trig_sample + acq_data.post_samples;
        int32_t buf_len =  rp_LaAcqBufLenInSamples(&la_acq_handle);

        printf("\n\r req: pre=%d pos=%d\n\r", acq_data.pre_samples, acq_data.post_samples);
        printf("\n\r sta: first=%d trig=%d last=%d\n\r", first_sample, acq_data.trig_sample, last_sample);

        int32_t wlen;
        if(first_sample<0){

            printf("\n\r first_sample > last_sample\n\r");

            wlen=abs(first_sample);
            first_sample=buf_len+first_sample;
            printf("\n\r sta: first=%d wlen=%d\n\r", first_sample, wlen);
            for(int i=0; i<wlen; i++){
                acq_data.buf[i]=map[first_sample+i];
            }
            for(int i=0; i<last_sample; i++){
                acq_data.buf[wlen+i]=map[i];
            }

        }
        else if(last_sample>=buf_len){

            printf("\n\r last_sample > size\n\r");

            wlen=buf_len-first_sample;
            for(int i=0; i<wlen; i++){
                acq_data.buf[i]=map[first_sample+i];
            }
            last_sample-=buf_len;
            for(int i=0; i<last_sample; i++){
                acq_data.buf[wlen+i]=map[i];
            }

        }
        else{
            printf("\n\r last_sample > first_sample\n\r");
            for(int i=0; i<(*noOfSamples); i++){
                acq_data.buf[i]=map[first_sample+i];
            }
        }
    }

    if(munmap (map, la_acq_handle.dma_size)==-1){
        printf("Failed to munmap\n");
        return -1;
    }

    return RP_API_OK;
};

/**
 * This function returns data either with or without down-sampling, starting at the
 * specified sample number. It is used to get the stored data from the scope after data
 * collection has stopped. It returns the data using a callback.
 *
 * @param startIndex             See rpGetValues()
 * @param noOfSamples             See rpGetValues()
 * @param downSampleRatio         See rpGetValues()
 * @param downSampleRatioMode     See rpGetValues()
 * @param lpDataReady             A pointer to the user-supplied function that will be called when the data is ready.
 *                                 This will be rpDataReady() for block-mode data or ps3000aStreamingReady() for streaming mode data.
 * @param pParameter             A void pointer that will be passed to the callback function.
 *                                 the data type is determined by the application.
 */
RP_STATUS rp_GetValuesAsync(
    uint32_t startIndex,
    uint32_t noOfSamples,
    uint32_t downSampleRatio,
    RP_RATIO_MODE downSampleRatioMode,
    //uint32_t segmentIndex
    void * lpDataReady,
    void * pParameter)
{

    return RP_API_OK;
}

/**
 * Set data buffer
 *
 * This function instructs the driver to return the next block of values to your
 * rpStreamingReady() callback. You must have previously called
 * rpRunStreaming() beforehand to set up streaming.
 *
 * @param rpReady          A pointer to your rpStreamingReady() callback.
 * @param pParameter    A void pointer that will be passed to the rpStreamingReady() callback.
 *                         The callback may optionally use this pointer to return information to the application.
 *
 */
RP_STATUS rp_GetStreamingLatestValues(rpStreamingReady rpReady,
                                     void * pParameter)
{
    // block read
    int32_t noOfSamples;
    uint32_t startIndex;
    int16_t overflow;
    uint32_t triggerAt;
    int16_t triggered;
    int16_t autoStop;

    noOfSamples=0;
    startIndex=0;
    overflow=0;
    triggerAt=0;
    triggered=0;
    autoStop=0;

    // acquisition is completed -> callback
    (*rpReady)(noOfSamples,
               startIndex,
               overflow,
               triggerAt,
               triggered,
               autoStop,
               pParameter);

    return RP_API_OK;
}

/**
 * Stops the scope device from sampling data. If this function is called
 * before a trigger event occurs, the oscilloscope may not contain valid data.
 * Always call this function after the end of a capture to ensure that the scope is ready for the next capture.
 */
RP_STATUS rp_Stop(void){
	return rp_SoftwareTrigger();
	//return rp_LaAcqStopAcq(&la_acq_handle);
}

