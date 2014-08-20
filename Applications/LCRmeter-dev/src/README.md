developer: CimeM

Initial goal is to run lcr program with the acquire and generate functions defined in files Applications/LCRmeter-dev  

lcr.c was copied and modified from /Test/lcr
Initial Makefile was renamed from Makefile to InitialMakefile
Makefile from Test/lcr was copied to this folder and modified


Results:
Compiling lcr.c with functions defined in Applications/LCRmeter-dev was a success.
Here are the things that were changed for successful compiling of lcr.c
1. t_params were predefined with a simple array which contains parameters that FPGA uses
	that had to be replaced with the structure that is defined in main.c

	changes:

	commented
	float t_params[PARAMS_NUM] = { 0, 1e6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	added:
	rp_app_params_t  *t_params = NULL;
	//In the for loop where these values are set every loop 
	t_params[EQUAL_FILT_PARAM].value = equal;
	t_params[SHAPE_FILT_PARAM].value = shaping;

2 In main.c 

  Changes: 
  	_First we added two more parameters to the rp_main_params array:
  		_ Equalization filter (eq_filter)
  		_ Shaping filter(shpng_filter)

  		{ /** Equalization filter:
       *    0 - Disabled
       *    1 - Enabled */   
       "eq_filter",     0, 0, 0,         0,         1 },
    { /** Shaping filter:
       *    0 - Disabled
       *    1 - Enabled */   
       "shpng_filter",      0, 0, 0,         0,         1 },
    { /* Must be last! */
        NULL, 0.0, -1, -1, 0.0, 0.0 }    

  		

  	-Subsequently we had to change the main.h file to match the corresponding changed made in main.c:
  	-The change in PARAM_NUM was needed to match the newly added parameters in mainc

  	
  		#define EQUAL_FILT_PARAM  49
		#define SHAPE_FILT_PARAM  50

	function that copies rp_main_parameters to lcr.c allocated buffer

	void rp_fill_params(rp_app_params_t *params){
    rp_copy_params(&rp_main_params[0], (rp_app_params_t **) params);
	}

	in lcr.c function call is set in for loops 
	rp_fill_params(t_params);

	ERROR HANDLING
	we had to comet out and rename some structures/functions in order to fix errors
	lcr.c : 
	( check original lcr.c for changes )
	typedef enum {
	    eSignalSine_lcr,         ///< Sinusoidal waveform.
	    /*eSignalSquare,       ///< Square waveform.
	    eSignalTriangle,     ///< Triangular waveform.
	    eSignalSweep         ///< Sinusoidal frequency sweep.*/
	} signal_e_lcr;

	/** AWG FPGA parameters */
	typedef struct {
	    int32_t  offsgain;   ///< AWG offset & gain.
	    uint32_t wrap;       ///< AWG buffer wrap value.
	    uint32_t step;       ///< AWG step interval.
	} awg_param_t_lcr;

	worker.c comented out:
	at the end of the code 

	/*----------------------------------------------------------------------------------
	inline float rp_osc_meas_cnv_cnt(float data, float adc_max_v)
	{
	    return (data * adc_max_v / (float)(1<<(c_osc_fpga_adc_bits-1)));
	}
	*/

	/*----------------------------------------------------------------------------------*/
	int rp_osc_meas_convert(rp_osc_meas_res_t *ch_meas, float adc_max_v, int32_t cal_dc_offs)
	{
	    ch_meas->min = 0;//rp_osc_meas_cnv_cnt(ch_meas->min+cal_dc_offs, adc_max_v);
	    ch_meas->max = 0;//rp_osc_meas_cnv_cnt(ch_meas->max+cal_dc_offs, adc_max_v);
	    ch_meas->amp = 0;//rp_osc_meas_cnv_cnt(ch_meas->amp, adc_max_v);
	    ch_meas->avg = 0;//rp_osc_meas_cnv_cnt(ch_meas->avg+cal_dc_offs, adc_max_v);

	    return 0;
	}


	code was compiled and returning segmentation fault on run


	ZAKAJ INLINE PRI WORKER.c 

	inline float rp_osc_meas_cnv_cnt(float data, float adc_max_v)



	WORKING METHOD:
	for rp_main_params changing 
	- does not have to copy the params from main.c
	-function rp_set_time_range was made (defined in main.c)
		function sets time parameters in c and it is called in lcr.c

