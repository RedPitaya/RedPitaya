
#include "rp_api.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>


/** Maximal digital signal sampling frequency [Hz] */
const double c_max_dig_sampling_rate = 250e6;

/** Maximal digital signal sampling frequency time interval [nS] */
const double c_max_dig_sampling_rate_time_interval_ns = 4;


/**
 * Open device
 */
RP_STATUS rpOpenUnit(void)
{

    return RP_OK;
}

/**
 * Close device
 */
RP_STATUS rpCloseUnit(void)
{

    return RP_OK;
}

/**
 * This function retrieves information about the specified device.
 * If the device fails to open or no device is opened, only the driver version is available.
 *
 * @param string         On exit, the information string selected specified by the info argument.
 *                         If string is NULL, only requiredSize is returned.
 * @param stringLength  On entry, the maximum number of int8_t that may be written to string.
 * @param requiredSize  On exit, the required length of the string array.
 * @param info            A number specifying what information is required. The possible values are listed in the table below.
 */
RP_STATUS rpGetUnitInfo(int8_t * string,
                        int16_t stringLength,
                        int16_t * requiredSize,
                        RP_INFO info)
{


    return RP_OK;
}


/**
 * Enable digital port
 *
 * This function is used to enable the digital port and set the logic level (the voltage at
 * which the state transitions from 0 to 1).
 *
 * @param port          Identifies the port for digital data
 * @param enabled          Whether or not to enable the channel.
 * @param logiclevel     The voltage at which the state transitions between 0
 *                         and 1. Range: –32767 (–5 V) to 32767 (5 V).
 *
 */
RP_STATUS rpSetDigitalPort(RP_DIGITAL_PORT port,
                           int16_t enabled,
                           int16_t logiclevel)
{
    return RP_OK;
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
RP_STATUS rpSetTriggerDigitalPortProperties(RP_DIGITAL_CHANNEL_DIRECTIONS * directions,
                                            int16_t nDirections)
{
    uint32_t n;

    // disable digital triggering
    if(directions==NULL){


        return RP_OK;
    }

    // disable all channels by default
    for(n=RP_DIGITAL_CHANNEL_0; n < RP_MAX_DIGITAL_CHANNELS; n++){


    }

    // enable channels set by
    for(n=0; n < nDirections; n++){
        switch(directions[n].direction){
            case RP_DIGITAL_DONT_CARE:
            case RP_DIGITAL_DIRECTION_LOW:
            case RP_DIGITAL_DIRECTION_HIGH:
            case RP_DIGITAL_DIRECTION_RISING:
            case RP_DIGITAL_DIRECTION_FALLING:
            case RP_DIGITAL_DIRECTION_RISING_OR_FALLING:
                //directions[n].channel
                break;
            default:
                return RP_INVALID_PARAMETER;
                break;
        }
    }

    return RP_OK;
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
RP_STATUS rpGetTimebase(uint32_t timebase,
                        int32_t noSamples,
                        double * timeIntervalNanoseconds,
                        //int16_t oversample,
                        uint32_t * maxSamples
                        //uint32_t segmentIndex
                        )
{
    *timeIntervalNanoseconds=timebase*c_max_dig_sampling_rate_time_interval_ns;
    return RP_OK;
};


/**
 * Set data buffer
 *
 * This function tells the driver where to store the data.
 *
 * @param channel    The channel you want to use with the buffer.
 * @param buffer      The location of the buffer
 * @param bufferLth The size of the buffer array
 *
 */
RP_STATUS rpSetDataBuffer(RP_DIGITAL_PORT channel,
                                 int16_t * buffer,
                                 int32_t bufferLth,
                                // uint32_t segmentIndex,
                                RP_RATIO_MODE mode)
{


    return 0;
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

RP_STATUS rpRunBlock(uint32_t noOfPreTriggerSamples,
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
    uint32_t maxSamples;
    RP_STATUS status = rpGetTimebase(timebase,0,&timeIntervalNanoseconds,&maxSamples);
    if(status!=RP_OK) return status;

    if((noOfPreTriggerSamples+noOfPostTriggerSamples)>maxSamples){
        return RP_INVALID_PARAMETER;
    }

    *timeIndisposedMs=(noOfPreTriggerSamples+noOfPostTriggerSamples)*timeIntervalNanoseconds/10e6;

    // configure FPGA to start block mode

    // block read

    // acquisition is completed -> callback
    (*rpReady)(status,pParameter);

    return RP_OK;
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
 * @param downSampleRatio             See rp3000aGetValues()
 *
 * @param downSampleRatioMode         See rp3000aGetValues()
 *
 * @param overviewBufferSize         The size of the overview buffers. These are temporary buffers used for storing the data
 *                                     before returning it to the application.
 *                                     The size is the same as the bufferLth value passed to rpSetDataBuffer().
 */
RP_STATUS rpRunStreaming(uint32_t * sampleInterval,
                        RP_TIME_UNITS sampleIntervalTimeUnits,
                        uint32_t maxPreTriggerSamples,
                        uint32_t maxPostTriggerSamples,
                        int16_t autoStop,
                        uint32_t downSampleRatio,
                        RP_RATIO_MODE downSampleRatioMode,
                        uint32_t overviewBufferSize)
{

    // configure FPGA to start block mode
    return RP_OK;
};


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
RP_STATUS rpGetValues(uint32_t startIndex,
                      uint32_t * noOfSamples,
                      uint32_t downSampleRatio,
                      RP_RATIO_MODE downSampleRatioMode,
                      //uint32_t segmentIndex,
                      int16_t * overflow){


    return RP_OK;
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
RP_STATUS rpGetValuesAsync(
    uint32_t startIndex,
    uint32_t noOfSamples,
    uint32_t downSampleRatio,
    RP_RATIO_MODE downSampleRatioMode,
    //uint32_t segmentIndex
    void * lpDataReady,
    void * pParameter)
{

    return RP_OK;
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
RP_STATUS rpGetStreamingLatestValues(rpStreamingReady rpReady,
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

    return RP_OK;
}

/**
 * Stops the scope device from sampling data. If this function is called
 * before a trigger event occurs, the oscilloscope may not contain valid data.
 * Always call this function after the end of a capture to ensure that the scope is ready for the next capture.
 */


RP_STATUS rpStop(void){

    return RP_OK;
}
