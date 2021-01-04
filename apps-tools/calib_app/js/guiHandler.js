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

var checkIntParameters = function(_id) {
    if (ValidateInt($(_id).val()) == false) {
        SM.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        SM.sendParameters();
        $(_id).fI();
        return 0;
    }
    if ($(_id).val() > 2147483647) {
        SM.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        SM.sendParameters();
        $(_id).fI();
        return 0;
    } else if ($(_id).val() < -2147483647) {
        SM.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        SM.sendParameters();
        $(_id).fI();
        return 0;
    }
    return -1;
}

var checkUIntParameters = function(_id) {
    if (ValidateInt($(_id).val()) == false) {
        SM.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        SM.sendParameters();
        $(_id).fI();
        return 0;
    }
    if ($(_id).val() > 2147483647) {
        SM.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        SM.sendParameters();
        $(_id).fI();
        return 0;
    } else if ($(_id).val() < 0) {
        SM.parametersCache["calib_sig"] = { value: 2 }; // request old parameters
        SM.sendParameters();
        $(_id).fI();
        return 0;
    }
    return -1;
}

var checkIntParameters2 = function(_id, _min, _max) {
    if (Validate($(_id).val()) == false) {
        SM.sendParameters();
        $(_id).fI();
        return 0;
    }
    if ($(_id).val() > _max) {
        SM.sendParameters();
        $(_id).fI();
        return 0;
    } else if ($(_id).val() < _min) {
        SM.sendParameters();
        $(_id).fI();
        return 0;
    }
    return -1;
}

var checkFloatParameters2 = function(_id, _min, _max) {
    if (Validate($(_id).val()) == false) {
        SM.sendParameters();
        $(_id).fI();
        return 0;
    }
    if ($(_id).val() > _max) {
        SM.sendParameters();
        $(_id).fI();
        return 0;
    } else if ($(_id).val() < _min) {
        SM.sendParameters();
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
        SM.sendParameters();
        $(_id).fI();
        return 0;
    }
    if (x > _max) {
        SM.sendParameters();
        $(_id).fI();
        return 0;
    } else if (x < _min) {
        SM.sendParameters();
        $(_id).fI();
        return 0;
    }
    return -1;
}

var ch1GainChange = function(event) {
    if (checkUIntParameters("#CH1_GAIN") !== 0) {
        SM.parametersCache["ch1_gain_adc_new"] = { value: $("#CH1_GAIN").val() };
        SM.sendParameters2("ch1_gain_adc_new");
        OBJ.adcCalibChange = true;
    }
}

var ch2GainChange = function(event) {
    if (checkUIntParameters("#CH2_GAIN") !== 0) {
        SM.parametersCache["ch2_gain_adc_new"] = { value: $("#CH2_GAIN").val() };
        SM.sendParameters2("ch2_gain_adc_new");
        OBJ.adcCalibChange = true;
    }
}

var ch1OffChange = function(event) {
    if (checkIntParameters("#CH1_OFFSET") !== 0) {
        SM.parametersCache["ch1_off_adc_new"] = { value: $("#CH1_OFFSET").val() };
        SM.sendParameters2("ch1_off_adc_new");
        OBJ.adcCalibChange = true;
    }
}

var ch2OffChange = function(event) {
    if (checkIntParameters("#CH2_OFFSET") !== 0) {
        SM.parametersCache["ch2_off_adc_new"] = { value: $("#CH2_OFFSET").val() };
        SM.sendParameters2("ch2_off_adc_new");
        OBJ.adcCalibChange = true;
    }
}

var ch1DacGainChange = function(event) {
    if (checkUIntParameters("#CH1_DAC_GAIN") !== 0) {
        SM.parametersCache["ch1_gain_dac_new"] = { value: $("#CH1_DAC_GAIN").val() };
        SM.sendParameters2("ch1_gain_dac_new");
        OBJ.adcCalibChange = true;
    }
}

var ch2DacGainChange = function(event) {
    if (checkUIntParameters("#CH2_DAC_GAIN") !== 0) {
        SM.parametersCache["ch2_gain_dac_new"] = { value: $("#CH2_DAC_GAIN").val() };
        SM.sendParameters2("ch2_gain_dac_new");
        OBJ.adcCalibChange = true;
    }
}

var ch1DacOffChange = function(event) {
    if (checkIntParameters("#CH1_DAC_OFFSET") !== 0) {
        SM.parametersCache["ch1_off_dac_new"] = { value: $("#CH1_DAC_OFFSET").val() };
        SM.sendParameters2("ch1_off_dac_new");
        OBJ.adcCalibChange = true;
    }
}

var ch2DacOffChange = function(event) {
    if (checkIntParameters("#CH2_DAC_OFFSET") !== 0) {
        SM.parametersCache["ch2_off_dac_new"] = { value: $("#CH2_DAC_OFFSET").val() };
        SM.sendParameters2("ch2_off_dac_new");
        OBJ.adcCalibChange = true;
    }
}

var gen1TypeChange = function(event) {
    if (checkIntParameters2("#CH1_DAC_TYPE", 0, 8) !== 0) {
        SM.parametersCache["gen1_type"] = { value: $("#CH1_DAC_TYPE").val() };
        SM.sendParameters2("gen1_type");
    }
}

var gen1FreqChange = function(event) {
    if (checkIntParameters2("#CH1_DAC_FREQ", 1, OBJ.maxGenFreq) !== 0) {
        SM.parametersCache["gen1_freq"] = { value: $("#CH1_DAC_FREQ").val() };
        SM.sendParameters2("gen1_freq");
    }
}

var gen1AmpChange = function(event) {
    if (checkFloatParameters2("#CH1_DAC_AMPL", 0.001, 1) !== 0) {
        SM.parametersCache["gen1_amp"] = { value: $("#CH1_DAC_AMPL").val() };
        SM.sendParameters2("gen1_amp");
    }
}

var gen1OffsetChange = function(event) {
    if (checkFloatParameters2("#CH1_DAC_OFF", -1, 1) !== 0) {
        SM.parametersCache["gen1_offset"] = { value: $("#CH1_DAC_OFF").val() };
        SM.sendParameters2("gen1_offset");
    }
}


var gen2TypeChange = function(event) {
    if (checkIntParameters2("#CH2_DAC_TYPE", 0, 8) !== 0) {
        SM.parametersCache["gen2_type"] = { value: $("#CH2_DAC_TYPE").val() };
        SM.sendParameters2("gen2_type");
    }
}

var gen2FreqChange = function(event) {
    if (checkIntParameters2("#CH2_DAC_FREQ", 1, OBJ.maxGenFreq) !== 0) {
        SM.parametersCache["gen2_freq"] = { value: $("#CH2_DAC_FREQ").val() };
        SM.sendParameters2("gen2_freq");
    }
}

var gen2AmpChange = function(event) {
    if (checkFloatParameters2("#CH2_DAC_AMPL", 0.001, 1) !== 0) {
        SM.parametersCache["gen2_amp"] = { value: $("#CH2_DAC_AMPL").val() };
        SM.sendParameters2("gen2_amp");
    }
}

var gen2OffsetChange = function(event) {
    if (checkFloatParameters2("#CH2_DAC_OFF", -1, 1) !== 0) {
        SM.parametersCache["gen2_offset"] = { value: $("#CH2_DAC_OFF").val() };
        SM.sendParameters2("gen2_offset");
    }
}

var filterDecimationChange = function(event) {
    SM.parametersCache["adc_decimation"] = { value: $("#FILTER_DECIMATION").val() };
    SM.sendParameters2("adc_decimation");
    SM.parametersCache["zoom_mode"] = { value: false };
    SM.sendParameters2("zoom_mode");
}

var filterChangeHyst = function(event) {
    if (checkFloatParameters2("#FILTER_HYST", 0, 1) !== 0) {
        SM.parametersCache["adc_hyst"] = { value: $("#FILTER_HYST").val() };
        SM.sendParameters2("adc_hyst");
    }
}

var filterFreqChange = function(event) {
    if (checkIntParameters2("#FILTER_DAC_FREQ", 1, OBJ.maxGenFreq) !== 0) {
        SM.parametersCache["filt_gen_freq"] = { value: $("#FILTER_DAC_FREQ").val() };
        SM.sendParameters2("filt_gen_freq");
    }
}

var filterAmpChange = function(event) {
    if (checkFloatParameters2("#FILTER_DAC_AMPL", 0.001, 1) !== 0) {
        SM.parametersCache["filt_gen_amp"] = { value: $("#FILTER_DAC_AMPL").val() };
        SM.sendParameters2("filt_gen_amp");
    }
}

var filterOffsetChange = function(event) {
    if (checkFloatParameters2("#FILTER_DAC_OFF", -1, 1) !== 0) {
        SM.parametersCache["filt_gen_offset"] = { value: $("#FILTER_DAC_OFF").val() };
        SM.sendParameters2("filt_gen_offset");
    }
}

var filterAAChange = function(event) {
    if (checkIntParameters3("#FILTER_AA", 0, 0x3FFFF) !== 0) {
        var x = parseInt($("#FILTER_AA").val());
        if (x !== OBJ.filterAA) {
            SM.parametersCache["filt_aa"] = { value: x };
            SM.sendParameters2("filt_aa");
            OBJ.filterCalibChange = true;
        }
    }
}

var filterBBChange = function(event) {
    if (checkIntParameters3("#FILTER_BB", 0, 0x1FFFFFF) !== 0) {
        var x = parseInt($("#FILTER_BB").val());
        if (x !== OBJ.filterBB) {
            SM.parametersCache["filt_bb"] = { value: x };
            SM.sendParameters2("filt_bb");
            OBJ.filterCalibChange = true;
        }
    }
}

var filterPPChange = function(event) {
    if (checkIntParameters3("#FILTER_PP", 0, 0x1FFFFFF) !== 0) {
        var x = parseInt($("#FILTER_PP").val());
        if (x !== OBJ.filterPP) {
            SM.parametersCache["filt_pp"] = { value: x };
            SM.sendParameters2("filt_pp");
            OBJ.filterCalibChange = true;
        }
    }
}

var filterKKChange = function(event) {
    if (checkIntParameters3("#FILTER_KK", 0, 0x1FFFFFF) !== 0) {
        var x = parseInt($("#FILTER_KK").val());
        if (x !== OBJ.filterKK) {
            SM.parametersCache["filt_kk"] = { value: x };
            SM.sendParameters2("filt_kk");
            OBJ.filterCalibChange = true;
        }
    }
}

var filterCalibAmpChange = function(event) {
    if (checkFloatParameters2("#SS_FILT_REF_VOLT", 0.001, 20) !== 0) {
        SM.parametersCache["filt_calib_ref_amp"] = { value: $("#SS_FILT_REF_VOLT").val() };
        SM.sendParameters2("filt_calib_ref_amp");
    }
}

var afilterCalibAmpChange = function(event) {
    if (checkFloatParameters2("#SS_A_FILT_REF_VOLT", 0.001, 20) !== 0) {
        SM.parametersCache["f_ref_volt"] = { value: $("#SS_A_FILT_REF_VOLT").val() };
        SM.sendParameters2("f_ref_volt");
    }
}

//Create callback
var changeCallbacks = {}

changeCallbacks["SS_REF_VOLT"] = refVoltChange;
changeCallbacks["CH1_GAIN"] = ch1GainChange;
changeCallbacks["CH2_GAIN"] = ch2GainChange;
changeCallbacks["CH1_OFFSET"] = ch1OffChange;
changeCallbacks["CH2_OFFSET"] = ch2OffChange;

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

//Subscribe changes and clicks
$(document).ready(function() {
    for (var k in changeCallbacks) {
        $("#" + k).change(changeCallbacks[k]);
    }
    for (var i in clickCallbacks) {
        $("#" + i).click(clickCallbacks[i]);
    }
})