$.fn.fI = function(e) { //Flash Item
    if (!e) { e = {} }
    if (this) { e.e = this }
    switch (e.f) {
        case 0:
            break;
        default:
            switch (e.css) {
                case 0:
                    e.d = 'background-color'
                    break;
                case undefined:
                    e.d = 'border-color'
                    break;
                default:
                    e.d = e.css
                    break;
            }
            if (!e.c1) { e.c1 = '#FF0000' }
            if (!e.c2) { e.c2 = '#A00000' }
            if (!e.p) { e.p = 200 }
            e.e.css(e.d, e.c1)
            setTimeout(function() {
                e.e.css(e.d, e.c2)
                setTimeout(function() {
                    e.e.css(e.d, e.c1)
                    setTimeout(function() {
                        e.e.css(e.d, e.c2)
                        setTimeout(function() {
                            e.e.css(e.d, '')
                        }, e.p)
                    }, e.p)
                }, e.p)
            }, e.p)
            break;
    }
    return this
}


function Validate(x) {
    if (x == '')
        return false;
    if (/^(-{0,1}[0-9]{1,2}\.{0,1}[0-9]*)$/.test(x)) {
        return (true);
    }
    return (false);
}

function ValidateHex(x) {
    if (x == '')
        return false;
    if (/^0[xX][0-9a-fA-F]+$/.test(x)) {
        return (true);
    }
    return (false);
}

function ValidateInt(x) {
    if (x == '')
        return false;
    if (/^(\-{0,1}[0-9]{1,15}?)$/.test(x)) {
        return (true);
    }
    return (false);
}

//Port number changed
var refVoltChange = function(event) {
    if (Validate($("#SS_REF_VOLT").val()) == false) {
        $("#SS_REF_VOLT").val(1);
        $('#SS_REF_VOLT').fI();
        return;
    }

    if ($("#SS_REF_VOLT").val() > 20) {
        $("#SS_REF_VOLT").val(1);
        $('#SS_REF_VOLT').fI();
        return;
    } else if ($("#SS_REF_VOLT").val() < 0.001) {
        $("#SS_REF_VOLT").val(1);
        $('#SS_REF_VOLT').fI();
        return;
    }
}

var checkGainParameters = function(_id) {
    if (Validate($(_id).val()) == false) {
        CLIENT.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    if ($(_id).val() > 1.5) {
        CLIENT.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    } else if ($(_id).val() < 0.5) {
        CLIENT.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    return -1;
}

var checkIntParameters = function(_id) {
    if (ValidateInt($(_id).val()) == false) {
        CLIENT.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        SM.requestParameters();
        $(_id).fI();
        return 0;
    }
    if ($(_id).val() > 2147483647) {
        CLIENT.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    } else if ($(_id).val() < -2147483647) {
        CLIENT.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    return -1;
}

var checkUIntParameters = function(_id) {
    if (ValidateInt($(_id).val()) == false) {
        CLIENT.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    if ($(_id).val() > 2147483647) {
        CLIENT.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    } else if ($(_id).val() < 0) {
        CLIENT.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    return -1;
}

var checkIntParameters2 = function(_id, _min, _max) {
    if (Validate($(_id).val()) == false) {
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    if ($(_id).val() > _max) {
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    } else if ($(_id).val() < _min) {
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    return -1;
}

var checkFloatParameters2 = function(_id, _min, _max) {
    if (Validate($(_id).val()) == false) {
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    if ($(_id).val() > _max) {
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    } else if ($(_id).val() < _min) {
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    return -1;
}

var checkIntParameters3 = function(_id, _min, _max) {
    var x = undefined;
    if (OBJ.filterHexMode) {
        if (ValidateHex($(_id).val())) {
            x = parseInt($(_id).val(), 16);
        }
    } else {
        if (ValidateInt($(_id).val())) {
            x = parseInt($(_id).val());
        }
    }
    if (x === undefined) {
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    if (x > _max) {
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    } else if (x < _min) {
        CLIENT.requestParameters();
        $(_id).fI();
        return 0;
    }
    return -1;
}

var ch1GainChange = function(event) {
    if (checkGainParameters("#CH1_GAIN") !== 0) {
        CLIENT.parametersCache["ch1_gain_adc_new"] = { value: $("#CH1_GAIN").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var ch2GainChange = function(event) {
    if (checkGainParameters("#CH2_GAIN") !== 0) {
        CLIENT.parametersCache["ch2_gain_adc_new"] = { value: $("#CH2_GAIN").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var ch3GainChange = function(event) {
    if (checkGainParameters("#CH3_GAIN") !== 0) {
        CLIENT.parametersCache["ch3_gain_adc_new"] = { value: $("#CH3_GAIN").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var ch4GainChange = function(event) {
    if (checkGainParameters("#CH4_GAIN") !== 0) {
        CLIENT.parametersCache["ch4_gain_adc_new"] = { value: $("#CH4_GAIN").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var ch1OffChange = function(event) {
    if (checkIntParameters("#CH1_OFFSET") !== 0) {
        CLIENT.parametersCache["ch1_off_adc_new"] = { value: $("#CH1_OFFSET").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var ch2OffChange = function(event) {
    if (checkIntParameters("#CH2_OFFSET") !== 0) {
        CLIENT.parametersCache["ch2_off_adc_new"] = { value: $("#CH2_OFFSET").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var ch3OffChange = function(event) {
    if (checkIntParameters("#CH3_OFFSET") !== 0) {
        CLIENT.parametersCache["ch3_off_adc_new"] = { value: $("#CH3_OFFSET").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var ch4OffChange = function(event) {
    if (checkIntParameters("#CH4_OFFSET") !== 0) {
        CLIENT.parametersCache["ch4_off_adc_new"] = { value: $("#CH4_OFFSET").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var manualDecimationSelector = function(event) {
    CLIENT.parametersCache["manual_decimation"] = { value: $("#B_DEC_SELECTOR").val() };
    CLIENT.sendParameters();
}

var ch1DacGainChange = function(event) {
    if (checkGainParameters("#CH1_DAC_GAIN") !== 0) {
        CLIENT.parametersCache["ch1_gain_dac_new"] = { value: $("#CH1_DAC_GAIN").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var ch2DacGainChange = function(event) {
    if (checkGainParameters("#CH2_DAC_GAIN") !== 0) {
        CLIENT.parametersCache["ch2_gain_dac_new"] = { value: $("#CH2_DAC_GAIN").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var ch1DacOffChange = function(event) {
    if (checkIntParameters("#CH1_DAC_OFFSET") !== 0) {
        CLIENT.parametersCache["ch1_off_dac_new"] = { value: $("#CH1_DAC_OFFSET").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var ch2DacOffChange = function(event) {
    if (checkIntParameters("#CH2_DAC_OFFSET") !== 0) {
        CLIENT.parametersCache["ch2_off_dac_new"] = { value: $("#CH2_DAC_OFFSET").val() };
        CLIENT.sendParameters();
        OBJ.adcCalibChange = true;
    }
}

var gen1TypeChange = function(event) {
    if (checkIntParameters2("#CH1_DAC_TYPE", 0, 8) !== 0) {
        CLIENT.parametersCache["gen1_type"] = { value: $("#CH1_DAC_TYPE").val() };
        CLIENT.sendParameters();
    }
}

var gen1FreqChange = function(event) {
    if (checkIntParameters2("#CH1_DAC_FREQ", 1, OBJ.maxGenFreq) !== 0) {
        CLIENT.parametersCache["gen1_freq"] = { value: $("#CH1_DAC_FREQ").val() };
        CLIENT.sendParameters();
    }
}

var gen1AmpChange = function(event) {
    if (checkFloatParameters2("#CH1_DAC_AMPL", 0.001, 1) !== 0) {
        CLIENT.parametersCache["gen1_amp"] = { value: $("#CH1_DAC_AMPL").val() };
        CLIENT.sendParameters();
    }
}

var gen1OffsetChange = function(event) {
    if (checkFloatParameters2("#CH1_DAC_OFF", -1, 1) !== 0) {
        CLIENT.parametersCache["gen1_offset"] = { value: $("#CH1_DAC_OFF").val() };
        CLIENT.sendParameters();
    }
}


var gen2TypeChange = function(event) {
    if (checkIntParameters2("#CH2_DAC_TYPE", 0, 8) !== 0) {
        CLIENT.parametersCache["gen2_type"] = { value: $("#CH2_DAC_TYPE").val() };
        CLIENT.sendParameters();
    }
}

var gen2FreqChange = function(event) {
    if (checkIntParameters2("#CH2_DAC_FREQ", 1, OBJ.maxGenFreq) !== 0) {
        CLIENT.parametersCache["gen2_freq"] = { value: $("#CH2_DAC_FREQ").val() };
        CLIENT.sendParameters();
    }
}

var gen2AmpChange = function(event) {
    if (checkFloatParameters2("#CH2_DAC_AMPL", 0.001, 1) !== 0) {
        CLIENT.parametersCache["gen2_amp"] = { value: $("#CH2_DAC_AMPL").val() };
        CLIENT.sendParameters();
    }
}

var gen2OffsetChange = function(event) {
    if (checkFloatParameters2("#CH2_DAC_OFF", -1, 1) !== 0) {
        CLIENT.parametersCache["gen2_offset"] = { value: $("#CH2_DAC_OFF").val() };
        CLIENT.sendParameters();
    }
}

var filterDecimationChange = function(event) {
    CLIENT.parametersCache["adc_decimation"] = { value: $("#FILTER_DECIMATION").val() };
    CLIENT.sendParameters();
    CLIENT.parametersCache["zoom_mode"] = { value: false };
    CLIENT.sendParameters();
}



var channelChanged = function(event) {
    CLIENT.parametersCache["adc_channel"] = { value: $("#FILTER_CHANNEL_4CH").val() };
    CLIENT.sendParameters();
}

var filterChangeHyst = function(event) {
    if (checkFloatParameters2("#FILTER_HYST", 0, 1) !== 0) {
        CLIENT.parametersCache["adc_hyst"] = { value: $("#FILTER_HYST").val() };
        CLIENT.sendParameters();
    }
}

var filterFreqChange = function(event) {
    if (checkIntParameters2("#FILTER_DAC_FREQ", 1, OBJ.maxGenFreq) !== 0) {
        CLIENT.parametersCache["filt_gen_freq"] = { value: $("#FILTER_DAC_FREQ").val() };
        CLIENT.sendParameters();
    }
}

var filterAmpChange = function(event) {
    if (checkFloatParameters2("#FILTER_DAC_AMPL", 0.001, 1) !== 0) {
        CLIENT.parametersCache["filt_gen_amp"] = { value: $("#FILTER_DAC_AMPL").val() };
        CLIENT.sendParameters();
    }
}

var filterOffsetChange = function(event) {
    if (checkFloatParameters2("#FILTER_DAC_OFF", -1, 1) !== 0) {
        CLIENT.parametersCache["filt_gen_offset"] = { value: $("#FILTER_DAC_OFF").val() };
        CLIENT.sendParameters();
    }
}

var filterAAChange = function(event) {
    if (checkIntParameters3("#FILTER_AA", 0, 0x3FFFF) !== 0) {
        var x = parseInt($("#FILTER_AA").val());
        if (x !== OBJ.filterAA) {
            CLIENT.parametersCache["filt_aa"] = { value: x };
            CLIENT.sendParameters();
            OBJ.filterCalibChange = true;
        }
    }
}

var filterBBChange = function(event) {
    if (checkIntParameters3("#FILTER_BB", 0, 0x1FFFFFF) !== 0) {
        var x = parseInt($("#FILTER_BB").val());
        if (x !== OBJ.filterBB) {
            CLIENT.parametersCache["filt_bb"] = { value: x };
            CLIENT.sendParameters();
            OBJ.filterCalibChange = true;
        }
    }
}

var filterPPChange = function(event) {
    if (checkIntParameters3("#FILTER_PP", 0, 0x1FFFFFF) !== 0) {
        var x = parseInt($("#FILTER_PP").val());
        if (x !== OBJ.filterPP) {
            CLIENT.parametersCache["filt_pp"] = { value: x };
            CLIENT.sendParameters();
            OBJ.filterCalibChange = true;
        }
    }
}

var filterKKChange = function(event) {
    if (checkIntParameters3("#FILTER_KK", 0, 0x1FFFFFF) !== 0) {
        var x = parseInt($("#FILTER_KK").val());
        if (x !== OBJ.filterKK) {
            CLIENT.parametersCache["filt_kk"] = { value: x };
            CLIENT.sendParameters();
            OBJ.filterCalibChange = true;
        }
    }
}

var filterCalibAmpChange = function(event) {
    if (checkFloatParameters2("#SS_FILT_REF_VOLT", 0.001, 20) !== 0) {
        CLIENT.parametersCache["filt_calib_ref_amp"] = { value: $("#SS_FILT_REF_VOLT").val() };
        CLIENT.sendParameters();
    }
}

var afilterCalibAmpChange = function(event) {
    if (checkFloatParameters2("#SS_A_FILT_REF_VOLT", 0.001, 20) !== 0) {
        CLIENT.parametersCache["f_ref_volt"] = { value: $("#SS_A_FILT_REF_VOLT").val() };
        CLIENT.sendParameters();
    }
}

//Create callback
var changeCallbacks = {}

changeCallbacks["SS_REF_VOLT"] = refVoltChange;
changeCallbacks["CH1_GAIN"] = ch1GainChange;
changeCallbacks["CH2_GAIN"] = ch2GainChange;
changeCallbacks["CH3_GAIN"] = ch3GainChange;
changeCallbacks["CH4_GAIN"] = ch4GainChange;

changeCallbacks["CH1_OFFSET"] = ch1OffChange;
changeCallbacks["CH2_OFFSET"] = ch2OffChange;
changeCallbacks["CH3_OFFSET"] = ch3OffChange;
changeCallbacks["CH4_OFFSET"] = ch4OffChange;

changeCallbacks["B_DEC_SELECTOR"] = manualDecimationSelector;

changeCallbacks["CH1_DAC_GAIN"] = ch1DacGainChange;
changeCallbacks["CH2_DAC_GAIN"] = ch2DacGainChange;
changeCallbacks["CH1_DAC_OFFSET"] = ch1DacOffChange;
changeCallbacks["CH2_DAC_OFFSET"] = ch2DacOffChange;


changeCallbacks["CH1_DAC_TYPE"] = gen1TypeChange;
changeCallbacks["CH1_DAC_FREQ"] = gen1FreqChange;
changeCallbacks["CH1_DAC_AMPL"] = gen1AmpChange;
changeCallbacks["CH1_DAC_OFF"] = gen1OffsetChange;

changeCallbacks["CH2_DAC_TYPE"] = gen2TypeChange;
changeCallbacks["CH2_DAC_FREQ"] = gen2FreqChange;
changeCallbacks["CH2_DAC_AMPL"] = gen2AmpChange;
changeCallbacks["CH2_DAC_OFF"] = gen2OffsetChange;
changeCallbacks["FILTER_DECIMATION"] = filterDecimationChange;
changeCallbacks["FILTER_CHANNEL_4CH"] = channelChanged;
changeCallbacks["FILTER_HYST"] = filterChangeHyst;
changeCallbacks["FILTER_DAC_FREQ"] = filterFreqChange;
changeCallbacks["FILTER_DAC_AMPL"] = filterAmpChange;
changeCallbacks["FILTER_DAC_OFF"] = filterOffsetChange;

changeCallbacks["FILTER_AA"] = filterAAChange;
changeCallbacks["FILTER_BB"] = filterBBChange;
changeCallbacks["FILTER_PP"] = filterPPChange;
changeCallbacks["FILTER_KK"] = filterKKChange;
changeCallbacks["SS_FILT_REF_VOLT"] = filterCalibAmpChange;
changeCallbacks["SS_A_FILT_REF_VOLT"] = afilterCalibAmpChange;

var clickCallbacks = {}

var connectHandlers = function(){
    for (var k in changeCallbacks) {
        $("#" + k).change(changeCallbacks[k]);
    }
    for (var i in clickCallbacks) {
        $("#" + i).click(clickCallbacks[i]);
    }
}
