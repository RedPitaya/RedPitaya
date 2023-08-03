/**
* $Id: $
*
* @file rpApp.h
* @brief Red Pitaya application library API interface
*
* @Author Red Pitaya
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#ifndef __RP_APP_H
#define __RP_APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "rp.h"
#include "rp_dsp.h"

/** @name Error codes
*  Various error codes returned by the API.
*/
///@{

/** Failed to Start a Thread */
#define RP_APP_EST  100
/** No signal found */
#define RP_APP_ENS  101
/** Failed to allocate array */
#define RP_EAA      102
/** Failed to calculate period */
#define RP_APP_ECP   103

///@}

/**
* Type representing input gain.
*/
typedef enum {
    RPAPP_OSC_IN_GAIN_LV,       //!< Input gain LV
    RPAPP_OSC_IN_GAIN_HV,       //!< Input gain HV
} rpApp_osc_in_gain_t;

/**
* Type representing trigger source.
*/
typedef enum {
    RPAPP_OSC_TRIG_SRC_CH1,     //!< Trigger source channel 1
    RPAPP_OSC_TRIG_SRC_CH2,     //!< Trigger source channel 2
    RPAPP_OSC_TRIG_SRC_CH3,     //!< Trigger source channel 3
    RPAPP_OSC_TRIG_SRC_CH4,     //!< Trigger source channel 4
    RPAPP_OSC_TRIG_SRC_EXTERNAL //!< Trigger source external
} rpApp_osc_trig_source_t;

/**
* Type representing trigger slope.
*/
typedef enum {
    RPAPP_OSC_TRIG_SLOPE_NE,    //!< Trigger source slope negative
    RPAPP_OSC_TRIG_SLOPE_PE     //!< Trigger source slope positive
} rpApp_osc_trig_slope_t;

/**
* Type representing trigger mode.
*/
typedef enum {
    RPAPP_OSC_TRIG_AUTO,        //!< Trigger sweep auto
    RPAPP_OSC_TRIG_NORMAL,      //!< Trigger sweep normal
    RPAPP_OSC_TRIG_SINGLE       //!< Trigger sweep single
} rpApp_osc_trig_sweep_t;

/**
* Type representing oscilloscope math and measure source
*/
typedef enum {
    RPAPP_OSC_SOUR_CH1  =   0,         //!< Trigger source ch1
    RPAPP_OSC_SOUR_CH2  =   1,         //!< Trigger source ch2
    RPAPP_OSC_SOUR_CH3  =   2,         //!< Trigger source channel 3
    RPAPP_OSC_SOUR_CH4  =   3,         //!< Trigger source channel 4
    RPAPP_OSC_SOUR_MATH =   4          //!< Trigger source math
} rpApp_osc_source;

/**
* Type representing math operations.
*/
typedef enum {
    RPAPP_OSC_MATH_NONE,        //!< Math operation add
    RPAPP_OSC_MATH_ADD,         //!< Math operation add
    RPAPP_OSC_MATH_SUB,         //!< Math operation subtract
    RPAPP_OSC_MATH_MUL,         //!< Math operation mltiply
    RPAPP_OSC_MATH_DIV,         //!< Math operation divide
    RPAPP_OSC_MATH_ABS,         //!< Math operation absolute
    RPAPP_OSC_MATH_DER,         //!< Math operation derivative
    RPAPP_OSC_MATH_INT,         //!< Math operation integrate
} rpApp_osc_math_oper_t;


/** @name General
*/
///@{

/**
* Initializes the library. It must be called first, before any other library method.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_Init();

/**
* Releases the library resources. It must be called last, after library is not used anymore. Typically before
* application exits.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_Release();

/**
* Resets all modules. Typically called after rpApp_Init()
* application exits.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_Reset();

/**
* Retrieves the library version number
* @return Library version
*/
const char* rpApp_GetVersion();

/**
* Returns textual representation of error code.
* @param errorCode Error code returned from API.
* @return Textual representation of error given error code.
*/
const char* rpApp_GetError(int errorCode);

///@}

/** @name Oscilloscope
*/
///@{

/**
* Starts oscilloscope.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscRun();

/**
* Stops oscilloscope.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscStop();

/**
* Resets oscilloscope.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscReset();

/**
* Runs oscilloscope once. Fills the buffer and then stops.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSingle();

/**
* Automatically sets "best" parameters for viewing the signal
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscAutoScale();

/**
 * Gets oscilloscope state. If running is true then oscilloscope is acquiring new data else data is not refreshed.
 * @param running Pointer where oscilloscope state is returned.
 * @return If the function is successful, the return value is RP_OK.
 * If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rpApp_OscIsRunning(bool *running);


int rpApp_OscIsTriggered();


/**
* Sets amplitude offset in volts.
* @param channel Channel 1 or 2 for which we want to set amplitude offset.
* @param offset Amplitude offset in volts.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetAmplitudeOffset(rpApp_osc_source source, double offset);

/**
* Gets amplitude offset in volts.
* @param channel Channel 1 or 2 for which we want to get amplitude offset.
* @param offset Amplitude offset pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetAmplitudeOffset(rpApp_osc_source source, double *offset);

/**
* Sets amplitude scale in volts per division.
* @param channel Channel 1 or 2 for which we want to set amplitude scale.
* @param scale Amplitude scale in volts per division.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetAmplitudeScale(rpApp_osc_source source, double scale);

/**
* Gets amplitude scale in volts per division.
* @param channel Channel 1 or 2 for which we want to get amplitude scale.
* @param scale Amplitude scale pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetAmplitudeScale(rpApp_osc_source source, double *scale);

/**
* Sets probe attenuation ratio.
* @param channel Channel 1 or 2 for which we want to set probe attenuation.
* @param att Probe attenuation.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetProbeAtt(rp_channel_t channel, float att);

/**
* Gets probe attenuation ratio.
* @param channel Channel 1 or 2 for which we want to get probe attenuation.
* @param att Probe attenuation pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetProbeAtt(rp_channel_t channel, float *att);

/**
* Sets input gain. This must be set the same as the jumpers on Red Pitaya board.
* @param channel Channel 1 or 2 for which we want to set input gain.
* @param gain Input gain.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetInputGain(rp_channel_t channel, rpApp_osc_in_gain_t gain);

/**
* Gets input gain.
* @param channel Channel 1 or 2 for which we want to get input gain.
* @param gain Input gain pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetInputGain(rp_channel_t channel, rpApp_osc_in_gain_t *gain);

/**
* Sets time delay in milliseconds.
* @param delay Time delay in milliseconds.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetTimeOffset(float offset);

/**
* Gets time delay in milliseconds.
* @param delay Time delay pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetTimeOffset(float *offset);

/**
* Sets time scale.
* @param scale Scale factor.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetTimeScale(float scale);

/**
* Gets time scale.
* @param scale Scale factor pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetTimeScale(float *scale);

/**
* Sets trigger Mode.
* @param mode Mode determines how the system is triggered
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetTriggerSweep(rpApp_osc_trig_sweep_t mode);

/**
* Gets trigger Mode.
* @param mode Mode pointer
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetTriggerSweep(rpApp_osc_trig_sweep_t *mode);

/**
* Sets trigger source.
* @param triggerSource Determines when the system starts acquiring data.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetTriggerSource(rpApp_osc_trig_source_t triggerSource);

/**
* Gets trigger source.
* @param triggerSource Trigger source pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetTriggerSource(rpApp_osc_trig_source_t *triggerSource);

/**
* Sets trigger slope.
* @param slope Determines positive or negative slope.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetTriggerSlope(rpApp_osc_trig_slope_t slope);

/**
* Gets trigger slope.
* @param slope Slope pointer
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetTriggerSlope(rpApp_osc_trig_slope_t *slope);

/**
* Sets trigger level.
* @param level Level determines value at witch is triggered.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetTriggerLevel(float level);

/**
* Gets trigger level.
* @param level Level pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetTriggerLevel(float *level);

/**
* Sets source signal inverted.
* @param inverted Determines if the signal is to be inverted or not.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetInverted(rpApp_osc_source source, bool inverted);

/**
* Checks if signal is inverted or not.
* @param inverted Returned value.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscIsInverted(rpApp_osc_source source, bool *inverted);

/**
* Gets view size ratio position proportional to ADC buffer size.
* @param ratio Pointer to ratio. Returned value is between 0 and 1
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetViewPart(float *ratio);

/**
* Gets source peak-to-peak voltage.
* @param source Source ch1, ch2 or math on which we want to measure amplitude.
* @param Vpp Peak-to-peak voltage pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscMeasureVpp(rpApp_osc_source source, float *Vpp);

/**
* Gets source mean voltage.
* @param source Source ch1, ch2 or math on which we want to measure mean amplitude.
* @param meanVoltage Mean voltage pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscMeasureMeanVoltage(rpApp_osc_source source, float *meanVoltage);

/**
* Gets source max voltage.
* @param source Source ch1, ch2 or math on which we want to measure max amplitude.
* @param Vmax Max voltage pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscMeasureAmplitudeMax(rpApp_osc_source source, float *Vmax);

/**
* Gets source min voltage.
* @param source Source ch1, ch2 or math on which we want to measure min amplitude.
* @param Vmin Min voltage pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscMeasureAmplitudeMin(rpApp_osc_source source, float *Vmin);

/**
* Gets source frequency.
* @param source Source ch1, ch2 or math on which we want to measure frequency.
* @param frequency Frequency of the signal.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscMeasureFrequency(rpApp_osc_source source, float *frequency);

/**
* Gets source period.
* @param source Source ch1, ch2 or math on which we want to measure period.
* @param period Period of the signal.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscMeasurePeriod(rpApp_osc_source source, float *period);

/**
* Gets source duty cycle. The returned value represents ratio between high time and all time.
* @param source Source ch1, ch2 or math on which we want to measure duty cycle.
* @param dutyCycle Duty cycle pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscMeasureDutyCycle(rpApp_osc_source source, float *dutyCycle);

/**
* Gets source root mean square of the signal.
* @param source Source ch1, ch2 or math on which we want to measure duty cycle.
* @param dutyCycle Duty cycle pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscMeasureRootMeanSquare(rpApp_osc_source source, float *rms);

/**
* Gets voltage at cursor position.
* @param source Source ch1, ch2 or math on which we want to get voltage.
* @param cursor Cursor position at witch we get voltage.
* @param value Amplitude pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetCursorVoltage(rpApp_osc_source source, uint32_t cursor, float *value);

/**
* Gets voltage time in milliseconds at cursor position.
* @param cursor Cursor position at  we get time.
* @param value Time pointer
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetCursorTime(uint32_t cursor, float *value);

/**
* Gets time in milliseconds between cursors.
* @param cursor1 Cursor 1 position.
* @param cursor2 Cursor 2 position.
* @param value Delta time pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetCursorDeltaTime(uint32_t cursor1, uint32_t cursor2, float *value);

/**
* Gets amplitude difference between cursors.
* @param source Source ch1, ch2 or math on which we want to get delta voltage.
* @param cursor1 Cursor 1 position.
* @param cursor2 Cursor 2 position.
* @param value Delta amplitude pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetCursorDeltaAmplitude(rpApp_osc_source source, uint32_t cursor1, uint32_t cursor2, float *value);

/**
* Gets frequency between cursors. This is equal to 1/(time difference).
* @param cursor1 Cursor 1 position.
* @param cursor2 Cursor 2 position.
* @param value Delta frequency pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetCursorDeltaFrequency(uint32_t cursor1, uint32_t cursor2, float *value);

/**
* Sets math operation.
* @param operation Operation of math calculation.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rpApp_OscSetMathOperation(rpApp_osc_math_oper_t operation);

/**
* GGets math operation.
* @param operation Pointer of operation.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rpApp_OscGetMathOperation(rpApp_osc_math_oper_t *operation);

/**
* Sets math sources.
* @param source1 Source 1 of math operation (CH1 or CH2).
* @param source2 Source 2 of math operation (CH1 or CH2).
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rpApp_OscSetMathSources(rp_channel_t source1, rp_channel_t source2);

/**
* Gets math sources.
* @param source1 Pointer of source 1.
* @param source2 Pointer of source 2.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
 */
int rpApp_OscGetMathSources(rp_channel_t *source1, rp_channel_t *source2);

/**
* Gets source data.
* @param source Source ch1, ch2 or math inticates with data we want toi get.
* @param data View buffer.
* @param size Number of values to be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetViewData(rpApp_osc_source source, float *data, uint32_t size);

/**
* Gets raw data.
* @param source Source ch1, ch2.
* @param data buffer.
* @param size Number of values to be returned.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetRawData(rp_channel_t source, uint16_t *data, uint32_t size);

/**
* Sets view buffer size.
* @param size Buffer size.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscSetViewSize(uint32_t size);

/**
* Gets view buffer size.
* @param size Buffer size pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetViewSize(uint32_t *size);

/**
* Gets start end positions of the valid data in the view buffer.
* @param start Start position pointer.
* @param end Ent position pointer.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscGetViewLimits(uint32_t* start, uint32_t* end);

/**
* Autoscale math channel.
* @return If the function is successful, the return value is RP_OK.
* If the function is unsuccessful, the return value is any of RP_E* values that indicate an error.
*/
int rpApp_OscScaleMath();

///@}

// SPECTRUM

int rpApp_SpecRun();

int rpApp_SpecStop();

int rpApp_SpecRunning();

int rpApp_SpecReset();

int rpApp_SpecGetViewData(float **signals, size_t size);

int rpApp_SpecGetViewSize(size_t* size);

int rpApp_SpecGetPeakPower(rp_channel_t channel, float* power);

int rpApp_SpecGetPeakFreq(rp_channel_t channel, float* freq);

int rpApp_SpecSetFreqRange(float _freq_min, float freq);

int rpApp_SpecSetWindow(rp_dsp_api::window_mode_t mode);

int rpApp_SpecGetWindow(rp_dsp_api::window_mode_t *mode);

int rpApp_SpecSetRemoveDC(int state);

int rpApp_SpecGetRemoveDC();

int rpApp_SpecGetADCFreq();

int rpApp_SpecGetMode(rp_dsp_api::mode_t *mode);

int rpApp_SpecSetMode(rp_dsp_api::mode_t mode);

int rpApp_SpecSetImpedance(double value);

int rpApp_SpecGetImpedance(double *value);

int rpApp_SpecSetADCBufferSize(int size);

int rpApp_SpecGetADCBufferSize();

int rpApp_SpecGetFreqMin(float* freq);

int rpApp_SpecGetFreqMax(float* freq);

int rpApp_SpecSetFreqMin(float freq);

int rpApp_SpecSetFreqMax(float freq);

int rpApp_SpecGetFpgaFreq(float* freq);

int rpApp_OscMeasureMaxValue(rpApp_osc_source source, float *Max);

int rpApp_OscMeasureMinValue(rpApp_osc_source source, float *Min);

#ifdef __cplusplus
}
#endif

#endif //__RP_H
