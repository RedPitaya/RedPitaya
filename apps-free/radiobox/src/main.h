/**
 * @brief Red Pitaya RadioBox main module.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __MAIN_H
#define __MAIN_H


#include <stdint.h>


#ifdef DEBUG
#  define TRACE(args...) fprintf(stderr, args)
#else
#  define TRACE(args...) {}
#endif


/** @defgroup main_h RadioBox main module
 * @{
 */

/** @brief Parameters description structure - must be the same for all RP controllers
 *
 * This structure is used for data transportation between the HTTP-Server and the controlle.so
 **/
typedef struct rp_app_params_s {
    /** @brief name  Name of the parameter */
    char  *name;

    /** @brief value  Value of the parameter */
    float  value;

    /** @brief fpga_update  Do a FPGA register update based on this parameter */
    int    fpga_update;

    /** @brief read_only  The value of this parameter can not be changed */
    int    read_only;

    /** @brief min_val  The lower limit of the value */
    float  min_val;

    /** @brief max_val  The upper limit of the value */
    float  max_val;
} rp_app_params_t;

/** @brief High definition parameters description structure
 *
 * This structure has got expanded data types for higher precision
 * in contrast to  struct rb_app_params_s.
 **/
typedef struct rb_app_params_s {
    /** @brief name  Name of the parameter */
    char  *name;

    /** @brief value  Value of the parameter with high precision */
    double value;

    /** @brief fpga_update  Do a FPGA register update based on this parameter */
    int    fpga_update;

    /** @brief read_only  The value of this parameter can not be changed */
    int    read_only;

    /** @brief min_val  The lower limit of the value high precision */
    double min_val;

    /** @brief max_val  The upper limit of the value high precision */
    double max_val;
} rb_app_params_t;


/* Parameters indexes - these defines should be in the same order as
 * rp_app_params_t structure defined in main.c */

/** @brief RadioBox parameters */
enum rb_params_enum_t {
    RB_RUN                  =  0,
    RB_CALIB,
    RB_TX_MODSRC,
    RB_TX_MODTYP,
    RB_RX_MODTYP,
    RB_LED_CON_SRC_PNT,
    RB_RFOUT1_CON_SRC_PNT,
    RB_RFOUT2_CON_SRC_PNT,
    RB_RX_MUXIN_SRC,

    RB_TX_CAR_OSC_QRG,
    RB_RX_CAR_OSC_QRG,

    RB_TX_MOD_OSC_QRG,
    RB_TX_MUXIN_GAIN,
    RB_RX_MUXIN_GAIN,
    RB_TX_QRG_SEL,
    RB_RX_QRG_SEL,

    RB_TX_AMP_RF_GAIN,
    RB_TX_MOD_OSC_MAG,
    RB_RFOUT1_TERM,
    RB_RFOUT2_TERM,
    RB_QRG_INC,
    RB_OVRDRV,
    RB_AC97_LOL,
    RB_AC97_LOR,

    RB_PARAMS_NUM
} RB_PARAMS_ENUM;


/** @brief RadioBox modulation sources */
enum rb_modsrc_enum_t {
    RB_MODSRC_NONE          =  0,

    RB_MODSRC_RF_IN1,
    RB_MODSRC_RF_IN2,

    RB_MODSRC_EXP_AI0       =  4,
    RB_MODSRC_EXP_AI1,
    RB_MODSRC_EXP_AI2,
    RB_MODSRC_EXP_AI3,

    RB_MODSRC_MOD_OSC       = 15,

    RB_MODSRC_AC97_LINEOUT_L= 48,
    RB_MODSRC_AC97_LINEOUT_R

} RB_MODSRC_ENUM;


/** @brief RadioBox TX modulation types */
enum rb_tx_modtyp_enum_t {
    RB_TX_MODTYP_ALL_ON     =  0,
    RB_TX_MODTYP_OFF,

    RB_TX_MODTYP_USB        =  2,
    RB_TX_MODTYP_LSB,
    RB_TX_MODTYP_AM,
    RB_TX_MODTYP_FM         =  7,
    RB_TX_MODTYP_PM
} RB_TX_MODTYP_ENUM;

/** @brief RadioBox RX modulation types - using 4 bits */
enum rb_rx_modtyp_enum_t {
    RB_RX_MODTYP_ALL_ON     =  0,
    RB_RX_MODTYP_OFF,

    RB_RX_MODTYP_USB        =  2,
    RB_RX_MODTYP_LSB,
    RB_RX_MODTYP_AMENV,
    RB_RX_MODTYP_AMSYNC_USB,
    RB_RX_MODTYP_AMSYNC_LSB,
    RB_RX_MODTYP_FM,
    RB_RX_MODTYP_PM
} RB_RX_MODTYP_ENUM;

/** @brief RadioBox RX modulation sub types (filter bandwidth) - using 4 bits */
enum rb_rx_amenv_filtvar_enum_t {
	RB_RX_AMENV_FILTVAR_wide = 0,
	RB_RX_AMENV_FILTVAR_mid,
	RB_RX_AMENV_FILTVAR_nar
} RB_RX_AMENV_FILTVAR_ENUM;


/* Output signals */
/** @brief The number of traces beeing hold in the traces buffer */
#define TRACE_NUM   3
/** @brief The number of points that a single trace holds */
#define TRACE_LENGTH (1024) /* Must be 2^n! */


/**
 * @brief Test variable name for single or quad variable names
 *
 * @param[in]   name     C string name to be tested.
 * @retval      1        Confirmative: name is a variable name for quad data transfer.
 * @retval      0        Negative: name is a variable name for single data transfer.
 */
int is_quad(const char* name);


/**
 * @brief Converts from 2x float data to 1x double data by re-interpreting data structure
 *
 * @param[in]   f_se     Single precision data, sign/exponent part.
 * @param[in]   f_hi     Single precision data, MSB part.
 * @param[in]   f_mi     Single precision data, the middle part.
 * @param[in]   f_lo     Single precision data, LSB part.
 * @retval      double   Double precision data.
 */
double cast_4xbf_to_1xdouble(float f_se, float f_hi, float f_mi, float f_lo);

/**
 * @brief Converts from 1x double data to 2x float data by re-interpreting the data structure
 *
 * @param[inout] f_se    Pointer to output float variable for the sign/exponent part.
 * @param[inout] f_hi    Pointer to output float variable for the MSB part.
 * @param[inout] f_mi    Pointer to outpus float variable for the middle part.
 * @param[inout] f_lo    Pointer to output float variable for the LSB part.
 * @param[in]    d       Double precision data to be re-interpreted.
 * @retval       0       Success.
 * @retval       -1      Failed due to argument failure.
 */
int cast_1xdouble_to_4xbf(float* f_se, float* f_hi, float* f_mi, float* f_lo, double d);


/**
 * @brief Returns description cstring for this RadioBox sub-module
 *
 * This function returns a null terminated cstring.
 *
 * @retval      cstring    Description of this RadioBox sub-module
 */
const char* rp_app_desc(void);


/* Internal helper functions */

/**
 * @brief Loads Linux kernel module for AC97 sound interface
 *
 * The FPGA includes an AC97 controller that is compatible to the Xilinx ML403 board IP block.
 * Instead of connecting a TI LM4450 AC97 CODEC, the FPGA ac9ctrl sub-module emulates the CODEC.
 *
 */
void rp_ac97_module_load(void);

/**
 * @brief Unloads Linux kernel module for AC97 sound interface
 *
 * The FPGA includes an AC97 controller that is compatible to the Xilinx ML403 board IP block.
 * Instead of connecting a TI LM4450 AC97 CODEC, the FPGA ac9ctrl sub-module emulates the CODEC.
 *
 */
void rp_ac97_module_unload(void);


/**
 * @brief Prepares buffers for signal traces (not used yet)
 *
 * This function allocates memory for TRACE_NUM traces in the memory and prepares
 * for its usage.
 *
 * @param[inout] a_traces  Pointer to traces buffer
 *
 * @retval       0         Success
 * @retval       -1        Failed to establish the traces buffer
 */
int  rp_create_traces(float** a_traces[TRACE_NUM]);

/**
 * @brief Frees memory used by the traces buffer (not used yet)
 *
 * This function frees memory for TRACE_NUM traces in the memory.
 *
 * @param[inout] a_traces  Pointer to traces buffer
 */
void rp_free_traces(float** a_traces[TRACE_NUM]);


/**
 * @brief Returns the index number of the params vector for which the name attribute matches
 *
 * @param[in]   src      Params vector to be scanned.
 * @param[in]   name     Name to be search for.
 * @retval      -2       Bad attributes.
 * @retval      -1       No matching vector entry found.
 * @retval      int      Value 0..(n-1) as index of the vector.
 */
int rp_find_parms_index(const rp_app_params_t* src, const char* name);

/**
 * @brief Returns the index number of the params vector for which the name attribute matches
 *
 * @param[in]   src      Params vector to be scanned.
 * @param[in]   name     Name to be search for.
 * @retval      -2       Bad attributes.
 * @retval      -1       No matching vector entry found.
 * @retval      int      Value 0..(n-1) as index of the vector.
 */
int rb_find_parms_index(const rb_app_params_t* src, const char* name);


/**
 * @brief Returns the index number of the params vector for which the name attribute matches
 *
 * @param[inout] dst          Destination application parameters, in case of ptr to NULL a new parameter list is generated.
 * @param[in]    param_name   Param name to be updated or created.
 * @param[in]    param_value  Value to assign.
 */
void rb_update_param(rb_app_params_t** dst, const char* param_name, double param_value);

/**
 * @brief Copies RedPitaya standard parameters vector to RadioBox high definition parameters vector
 *
 * @param[inout] dst_line     RadioBox high definition parameters vector.
 * @param[in]    src_line_se  RedPitaya standard parameters vector for data interchange, sign/exponent part.
 * @param[in]    src_line_hi  RedPitaya standard parameters vector for data interchange, MSB part.
 * @param[in]    src_line_mi  RedPitaya standard parameters vector for data interchange, the middle part.
 * @param[in]    src_line_lo  RedPitaya standard parameters vector for data interchange, LSB part.
 */
void rp2rb_params_value_copy(rb_app_params_t* dst_line, const rp_app_params_t src_line_se, const rp_app_params_t src_line_hi, const rp_app_params_t src_line_mi, const rp_app_params_t src_line_lo);

/**
 * @brief Copies RadioBox high definition parameters vector to RedPitaya standard parameters vector
 *
 * @param[inout] dst_line_se  RedPitaya standard parameters vector for data interchange, sign/exponent part.
 * @param[inout] dst_line_hi  RedPitaya standard parameters vector for data interchange, MSB part.
 * @param[inout] dst_line_mi  RedPitaya standard parameters vector for data interchange, the middle part.
 * @param[inout] dst_line_lo  RedPitaya standard parameters vector for data interchange, LSB part.
 * @param[in]    src_line     RadioBox high definition parameters vector.
 */
void rb2rp_params_value_copy(rp_app_params_t* dst_line_se, rp_app_params_t* dst_line_hi, rp_app_params_t* dst_line_mi, rp_app_params_t* dst_line_lo, const rb_app_params_t src_line);


/**
 * @brief Make a copy of Transport parameters
 *
 * Function copies actual Application parameters to the specified destination
 * buffer. This action was intended to prepare two parameter instances, where the first
 * one can be further modified from the user side, while the second one is processed by
 * the worker thread.
 * In case the destination buffer is not allocated yet, it is allocated internally and must
 * be freed outside of the function scope by calling rp_clean_params() function. Note that
 * if function returns failure, the destination buffer could be partially allocated and must
 * be freed in the same way.
 * If the specified destination buffer is already allocated, it is assumed the number of table
 * entries is the same as in the source table. No special check is made internally if this is really
 * the case.
 *
 * @param[out]  dst               Destination application parameters, in case of ptr to NULL a new parameter list is generated.
 * @param[in]   src               Source application parameters. In case of a NULL point the default parameters are take instead.
 * @param[in]   len               The count of parameters in the src vector.
 * @param[in]   do_copy_all_attr  Do a fully copy of all attributes, not just the name, value and fpga_update entries.
 * @retval      count             Successful operation: count of entries in the vector.
 * @retval      -1                Failure, error message is output on standard error
 */
int rp_copy_params(rp_app_params_t** dst, const rp_app_params_t src[], int len, int do_copy_all_attr);

/**
 * @brief Make a copy of Application parameters
 *
 * Function copies actual Application parameters to the specified destination
 * buffer. This action was intended to prepare two parameter instances, where the first
 * one can be further modified from the user side, while the second one is processed by
 * the worker thread.
 * In case the destination buffer is not allocated yet, it is allocated internally and must
 * be freed outside of the function scope by calling rp_clean_params() function. Note that
 * if function returns failure, the destination buffer could be partially allocated and must
 * be freed in the same way.
 * If the specified destination buffer is already allocated, it is assumed the number of table
 * entries is the same as in the source table. No special check is made internally if this is really
 * the case.
 *
 * @param[out]  dst               Destination application parameters, in case of ptr to NULL a new parameter list is generated.
 * @param[in]   src               Source application parameters. In case of a NULL point the default parameters are take instead.
 * @param[in]   len               The count of parameters in the src vector.
 * @param[in]   do_copy_all_attr  Do a fully copy of all attributes, not just the name, value and fpga_update entries.
 * @retval      count             Successful operation: count of entries in the vector.
 * @retval      -1                Failure, error message is output on standard error
 */
int rb_copy_params(rb_app_params_t** dst, const rb_app_params_t src[], int len, int do_copy_all_attr);

/**
 * @brief Copies the RadioBox high definition parameters vector to a Red Pitaya parameters vector
 *
 * In contrast to rp_copy_params() this function copies all attributes.
 *
 * @param[out]  dst               Destination application parameters, in case of ptr to NULL a new parameter list is generated.
 * @param[in]   src               Source application parameters. In case of a NULL point the default parameters are take instead.
 * @retval      count             Successful operation: count of entries in the vector.
 * @retval      -1                Failure, argument dst not valid
 * @retval      -2                Failure, argument src not valid
 * @retval      -3                Failure, out of memory
 */
int rp_copy_params_rb2rp(rp_app_params_t** dst, const rb_app_params_t src[]);

/**
 * @brief Copies a Red Pitaya parameters vector to the RadioBox high definition parameters vector
 *
 * In contrast to rp_copy_params() this function copies all attributes.
 *
 * @param[out]  dst               Destination application parameters, in case of ptr to NULL a new parameter list is generated.
 * @param[in]   src               Source application parameters. In case of a NULL point the default parameters are take instead.
 * @retval      count             Successful operation: count of entries in the vector.
 * @retval      -1                Failure, argument dst not valid
 * @retval      -2                Failure, argument src not valid
 * @retval      -3                Failure, out of memory
 */
int rp_copy_params_rp2rb(rb_app_params_t** dst, const rp_app_params_t src[]);


/**
 * @brief Deallocate the specified buffer of Application parameters
 *
 * Function is used to deallocate the specified buffers, which were previously
 * allocated by calling rp_copy_params() function.
 *
 * @param[in]   params  Application parameters to be showed
 * @retval      0                 Successful operation
 * @retval      -1                Failure, error message is output on standard error
 */
int print_rb_params(rb_app_params_t* params);


/**
 * @brief Deallocate the specified buffer of Application parameters
 *
 * Function is used to deallocate the specified buffers, which were previously
 * allocated by calling rp_copy_params() function.
 *
 * @param[in]   params  Application parameters to be deallocated
 * @retval      0       Success
 * @retval      -1      Failed with non-valid params
 */
int rp_free_params(rp_app_params_t** params);

/**
 * @brief Deallocate the specified buffer of Application parameters
 *
 * Function is used to deallocate the specified buffers, which were previously
 * allocated by calling rb_copy_params() function.
 *
 * @param[in]   params  Application parameters to be deallocated
 * @retval      0       Success
 * @retval      -1      Failed with non-valid params
 */
int rb_free_params(rb_app_params_t** params);

/** @} */


#endif /*  __MAIN_H */
