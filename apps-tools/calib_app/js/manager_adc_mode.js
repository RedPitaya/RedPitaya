/*
 * Red Pitaya calib_app
 *
 * Author: Danilyuk Nikolay <n.danilyuk@integrasources.eu>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

$(function() {

});

(function(OBJ, $, undefined) {



    OBJ.adcModel = undefined;

    OBJ.adcSig1ArrayAVG = [];
    OBJ.adcSig2ArrayAVG = [];
    OBJ.adcSig1ArrayMIN = [];
    OBJ.adcSig2ArrayMIN = [];
    OBJ.adcSig1ArrayMAX = [];
    OBJ.adcSig2ArrayMAX = [];
    OBJ.adcGraphCacheCh1 = undefined;
    OBJ.adcCalibChange = false;

    OBJ.adcSetModel = function(_model) {
        if (OBJ.adcModel === undefined) {
            OBJ.adcModel = _model.value;
        }
    }

    OBJ.adcPush = function(_obj, _value) {
        if (_obj !== undefined) {
            _obj.push(_value);
            while (_obj.length > 400) {
                _obj.shift();
            }
        }
    }

    OBJ.adcSetCH1Avg = function(_value) {
        $("#CH1_AVG").val(_value.value + " V");
        OBJ.adcPush(OBJ.adcSig1ArrayAVG, _value.value);
    }

    OBJ.adcSetCH2Avg = function(_value) {
        $("#CH2_AVG").val(_value.value + " V");
        OBJ.adcPush(OBJ.adcSig2ArrayAVG, _value.value);
    }

    OBJ.adcSetCH1Max = function(_value) {
        $("#CH1_MAX").val(_value.value + " V");
        OBJ.adcPush(OBJ.adcSig1ArrayMAX, _value.value);
    }

    OBJ.adcSetCH2Max = function(_value) {
        $("#CH2_MAX").val(_value.value + " V");
        OBJ.adcPush(OBJ.adcSig2ArrayMAX, _value.value);
    }

    OBJ.adcSetCH1Min = function(_value) {
        $("#CH1_MIN").val(_value.value + " V");
        OBJ.adcPush(OBJ.adcSig1ArrayMIN, _value.value);
    }

    OBJ.adcSetCH2Min = function(_value) {
        $("#CH2_MIN").val(_value.value + " V");
        OBJ.adcPush(OBJ.adcSig2ArrayMIN, _value.value);
    }

    OBJ.adcInitData = function() {
        OBJ.adcSig1ArrayAVG = [];
        OBJ.adcSig2ArrayAVG = [];
        OBJ.adcSig1ArrayMIN = [];
        OBJ.adcSig2ArrayMIN = [];
        OBJ.adcSig1ArrayMAX = [];
        OBJ.adcSig2ArrayMAX = [];
    }

    OBJ.adcInitRequest = function() {
        OBJ.adcCalibChange = false;
        SM.parametersCache["calib_sig"] = { value: 1 };
        SM.sendParameters();
        $('.flipswitch').prop('checked', false);
    }


    OBJ.prepareDataCH1 = function() {
        var ar_avg = OBJ.adcSig1ArrayAVG.slice();
        var ar_min = OBJ.adcSig1ArrayMIN.slice();
        var ar_max = OBJ.adcSig1ArrayMAX.slice();

        var len = Math.min(ar_avg.length, ar_min.length, ar_max.length);
        if (len === 0) return undefined;
        var s_avg = [];
        var s_min = [];
        var s_max = [];
        var z = 0;
        for (var i = 0; i < 400 - len; i++) {
            s_avg.push([i, undefined]);
            s_max.push([i, undefined]);
            s_min.push([i, undefined]);
            z = i;
        }

        for (var i = 0; i < len; i++) {
            z++;
            s_avg.push([z, ar_avg[i]]);
            s_max.push([z, ar_max[i]]);
            s_min.push([z, ar_min[i]]);
        }
        return { CH_AVG: s_avg, CH_MAX: s_max, CH_MIN: s_min };
    }

    OBJ.prepareDataCH2 = function() {
        var ar_avg = OBJ.adcSig2ArrayAVG.slice();
        var ar_min = OBJ.adcSig2ArrayMIN.slice();
        var ar_max = OBJ.adcSig2ArrayMAX.slice();

        var len = Math.min(ar_avg.length, ar_min.length, ar_max.length);
        if (len === 0) return undefined;
        var s_avg = [];
        var s_min = [];
        var s_max = [];
        var z = 0;
        for (var i = 0; i < 400 - len; i++) {
            s_avg.push([i, undefined]);
            s_max.push([i, undefined]);
            s_min.push([i, undefined]);
            z = i;
        }

        for (var i = 0; i < len; i++) {
            z++;
            s_avg.push([z, ar_avg[i]]);
            s_max.push([z, ar_max[i]]);
            s_min.push([z, ar_min[i]]);
        }
        return { CH_AVG: s_avg, CH_MAX: s_max, CH_MIN: s_min };
    }


    OBJ.adcInitPlotCH1 = function(update) {
        delete OBJ.adcGraphCacheCh1;
        $('#bode_plot_ch1').remove();


        OBJ.adcGraphCacheCh1 = {};
        OBJ.adcGraphCacheCh1.elem = $('<div id="bode_plot_ch1" class="plot" style="width:268px;height:90px;position: absolute;margin-top: auto;left: 0px;"/>').appendTo('#graph_bode_ch1');

        var t = null;
        var options = {
            series: {
                shadowSize: 0
            },
            yaxes: [{
                show: false,
                min: null,
                max: null,
                labelWidth: 5,
                tickDecimals: 1,
                //   alignTicksWithAxis: 1,
                position: "left"
            }],
            xaxis: {
                show: false,
                color: '#aaaaaa',
                tickColor: '#aaaaaa',
                ticks: t,
                // transform: function(v) {
                //     if (BA.scale)
                //         return Math.log(v + 0.0001); // move away from zero
                //     else
                //         return v;

                // },
                tickDecimals: 0,
                reserveSpace: false,
                // tickFormatter: funcxTickFormat,
                min: null,
                max: null,
            },
            grid: {
                show: true,
                color: '#aaaaaa',
                borderColor: '#aaaaaa',
                tickColor: '#aaaaaa',
                tickColor: '#aaaaaa',
                markingsColor: '#aaaaaa'
            },
            legend: {
                show: false,
                position: "sw",
                backgroundOpacity: 0.15
            }
        };

        var sig1 = [];
        var sig2 = [];
        var sig3 = [];

        if (update == true) {
            var x = OBJ.prepareDataCH1();
            if (x !== undefined) {
                sig1 = x.CH_AVG;
                sig2 = x.CH_MAX;
                sig3 = x.CH_MIN;
            }
        }
        var data_points = [{ data: sig1, color: '#f3ec1a', label: "AVG" }, { data: sig2, color: '#ff0000', label: "MAX" }, { data: sig3, color: '#00FF00', label: "MIN" }];
        OBJ.adcGraphCacheCh1.plot = $.plot(OBJ.adcGraphCacheCh1.elem, data_points, options);
        $('.flot-text').css('color', '#aaaaaa');
    }

    OBJ.adcInitPlotCH2 = function(update) {
        delete OBJ.adcGraphCacheCh2;
        $('#bode_plot_ch2').remove();


        OBJ.adcGraphCacheCh2 = {};
        OBJ.adcGraphCacheCh2.elem = $('<div id="bode_plot_ch2" class="plot" style="width:268px;height:90px;position: absolute;margin-top: auto;left: 0px;"/>').appendTo('#graph_bode_ch2');


        var t = null;
        var options = {
            series: {
                shadowSize: 0
            },
            yaxes: [{
                show: false,
                min: null,
                max: null,
                labelWidth: 5,
                alignTicksWithAxis: 1,
                position: "left"
            }],
            xaxis: {
                show: false,
                color: '#aaaaaa',
                tickColor: '#aaaaaa',
                ticks: t,
                // transform: function(v) {
                //     if (BA.scale)
                //         return Math.log(v + 0.0001); // move away from zero
                //     else
                //         return v;

                // },
                tickDecimals: 0,
                reserveSpace: false,
                // tickFormatter: funcxTickFormat,
                min: null,
                max: null,
            },
            grid: {
                show: true,
                color: '#aaaaaa',
                borderColor: '#aaaaaa',
                tickColor: '#aaaaaa',
                tickColor: '#aaaaaa',
                markingsColor: '#aaaaaa'
            },
            legend: {
                show: false,
                position: "sw",
                backgroundOpacity: 0.15
            }
        };

        var sig1 = [];
        var sig2 = [];
        var sig3 = [];

        if (update == true) {
            var x = OBJ.prepareDataCH2();
            if (x !== undefined) {
                sig1 = x.CH_AVG;
                sig2 = x.CH_MAX;
                sig3 = x.CH_MIN;
            }
        }
        var data_points = [{ data: sig1, color: '#f3ec1a', label: "AVG" }, { data: sig2, color: '#ff0000', label: "MAX" }, { data: sig3, color: '#00FF00', label: "MIN" }];
        OBJ.adcGraphCacheCh2.plot = $.plot(OBJ.adcGraphCacheCh2.elem, data_points, options);
        $('.flot-text').css('color', '#aaaaaa');
    }

    OBJ.drawSignalsCH1 = function() {

        var sig1 = [];
        var sig2 = [];
        var sig3 = [];

        if (OBJ.adcGraphCacheCh1 == undefined) {
            OBJ.adcInitPlotCH1(false);
        }

        var x = OBJ.prepareDataCH1();
        if (x !== undefined) {
            sig1 = x.CH_AVG;
            sig2 = x.CH_MAX;
            sig3 = x.CH_MIN;

            OBJ.adcGraphCacheCh1.elem.show();
            OBJ.adcGraphCacheCh1.plot.resize();
            OBJ.adcGraphCacheCh1.plot.setupGrid();
            var data_points = [{ data: sig1, color: '#f3ec1a', label: "AVG" }, { data: sig2, color: '#ff0000', label: "MAX" }, { data: sig3, color: '#00FF00', label: "MIN" }];

            OBJ.adcGraphCacheCh1.plot.setData(data_points);
            OBJ.adcGraphCacheCh1.plot.draw();
        }
    };

    OBJ.drawSignalsCH2 = function() {

        var sig1 = [];
        var sig2 = [];
        var sig3 = [];

        if (OBJ.adcGraphCacheCh2 == undefined) {
            OBJ.adcInitPlotCH2(false);
        }

        var x = OBJ.prepareDataCH2();
        if (x !== undefined) {
            sig1 = x.CH_AVG;
            sig2 = x.CH_MAX;
            sig3 = x.CH_MIN;

            OBJ.adcGraphCacheCh2.elem.show();
            OBJ.adcGraphCacheCh2.plot.resize();
            OBJ.adcGraphCacheCh2.plot.setupGrid();
            var data_points = [{ data: sig1, color: '#f3ec1a', label: "AVG" }, { data: sig2, color: '#ff0000', label: "MAX" }, { data: sig3, color: '#00FF00', label: "MIN" }];

            OBJ.adcGraphCacheCh2.plot.setData(data_points);
            OBJ.adcGraphCacheCh2.plot.draw();
        }
    };

    OBJ.amSetCh1GainADC = function(_value) {
        $("#CH1_GAIN").val(_value.value);
    }

    OBJ.amSetCh2GainADC = function(_value) {
        $("#CH2_GAIN").val(_value.value);
    }

    OBJ.amSetCh1OffADC = function(_value) {
        $("#CH1_OFFSET").val(_value.value);
    }

    OBJ.amSetCh2OffADC = function(_value) {
        $("#CH2_OFFSET").val(_value.value);
    }

    OBJ.amSetCh1GainDAC = function(_value) {
        $("#CH1_DAC_GAIN").val(_value.value);
    }

    OBJ.amSetCh2GainDAC = function(_value) {
        $("#CH2_DAC_GAIN").val(_value.value);
    }

    OBJ.amSetCh1OffDAC = function(_value) {
        $("#CH1_DAC_OFFSET").val(_value.value);
    }

    OBJ.amSetCh2OffDAC = function(_value) {
        $("#CH2_DAC_OFFSET").val(_value.value);
    }

    OBJ.amSetMode = function(_mode, _state) {
        if (_mode == "dac_gain") {
            SM.parametersCache["gen_gain"] = { value: _state };
            SM.sendParameters2("gen_gain");
            setTimeout(OBJ.adcInitData, 1000);
        }
        if (_mode == "ch1_dac_enable") {
            SM.parametersCache["gen1_enable"] = { value: _state };
            SM.sendParameters2("gen1_enable");
            setTimeout(OBJ.adcInitData, 1000);
        }
        if (_mode == "ch2_dac_enable") {
            SM.parametersCache["gen2_enable"] = { value: _state };
            SM.sendParameters2("gen2_enable");
            setTimeout(OBJ.adcInitData, 1000);
        }
        if (_mode == "HV_LV_MODE") {
            SM.parametersCache["hv_lv_mode"] = { value: _state };
            SM.sendParameters2("hv_lv_mode");
            setTimeout(OBJ.adcInitData, 1000);
        }
        if (_mode == "AC_DC_MODE") {
            SM.parametersCache["ac_dc_mode"] = { value: _state };
            SM.sendParameters2("ac_dc_mode");
            setTimeout(OBJ.adcInitData, 1000);
        }
    }

    OBJ.amSetNewCalib = function(_mode, _new_val) {
        if (_mode == "CH1_ADC_OFF") {
            SM.parametersCache["ch1_off_adc_new"] = { value: parseInt($("#CH1_OFFSET").val()) + _new_val };
            SM.sendParameters2("ch1_off_adc_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH2_ADC_OFF") {
            SM.parametersCache["ch2_off_adc_new"] = { value: parseInt($("#CH2_OFFSET").val()) + _new_val };
            SM.sendParameters2("ch2_off_adc_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH1_ADC_GAIN") {
            SM.parametersCache["ch1_gain_adc_new"] = { value: parseInt($("#CH1_GAIN").val()) + _new_val };
            SM.sendParameters2("ch1_gain_adc_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH2_ADC_GAIN") {
            SM.parametersCache["ch2_gain_adc_new"] = { value: parseInt($("#CH2_GAIN").val()) + _new_val };
            SM.sendParameters2("ch2_gain_adc_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH1_DAC_OFF") {
            SM.parametersCache["ch1_off_dac_new"] = { value: parseInt($("#CH1_DAC_OFFSET").val()) + _new_val };
            SM.sendParameters2("ch1_off_dac_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH2_DAC_OFF") {
            SM.parametersCache["ch2_off_dac_new"] = { value: parseInt($("#CH2_DAC_OFFSET").val()) + _new_val };
            SM.sendParameters2("ch2_off_dac_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH1_DAC_GAIN") {
            SM.parametersCache["ch1_gain_dac_new"] = { value: parseInt($("#CH1_DAC_GAIN").val()) + _new_val };
            SM.sendParameters2("ch1_gain_dac_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH2_DAC_GAIN") {
            SM.parametersCache["ch2_gain_dac_new"] = { value: parseInt($("#CH2_DAC_GAIN").val()) + _new_val };
            SM.sendParameters2("ch2_gain_dac_new");
            OBJ.adcCalibChange = true;
        }
    }

    OBJ.amSetCh1GenType = function(_value) {
        $("#CH1_DAC_TYPE").val(_value.value);
    }

    OBJ.amSetCh2GenType = function(_value) {
        $("#CH2_DAC_TYPE").val(_value.value);
    }

    OBJ.amSetCh1GenFreq = function(_value) {
        $("#CH1_DAC_FREQ").val(_value.value);
    }

    OBJ.amSetCh2GenFreq = function(_value) {
        $("#CH2_DAC_FREQ").val(_value.value);
    }

    OBJ.amSetCh1GenAmp = function(_value) {
        $("#CH1_DAC_AMPL").val(_value.value);
    }

    OBJ.amSetCh2GenAmp = function(_value) {
        $("#CH2_DAC_AMPL").val(_value.value);
    }

    OBJ.amSetCh1GenOffset = function(_value) {
        $("#CH1_DAC_OFF").val(_value.value);
    }

    OBJ.amSetCh2GenOffset = function(_value) {
        $("#CH2_DAC_OFF").val(_value.value);
    }

}(window.OBJ = window.OBJ || {}, jQuery));


// Page onload event handler
$(function() {
    // $('#am_ok_btn').on('click', function() { OBJ.amClickOkDialog() });
    setInterval(OBJ.drawSignalsCH1, 100);
    setInterval(OBJ.drawSignalsCH2, 100);

    $('.man_flipswitch').change(function() {
        $(this).next().text($(this).is(':checked') ? ':checked' : ':not(:checked)');
        OBJ.amSetMode($(this).attr('id'), $(this).is(':checked'));

    }).trigger('change');


    SM.param_callbacks["gen1_type"] = OBJ.amSetCh1GenType;
    SM.param_callbacks["gen1_offset"] = OBJ.amSetCh1GenOffset;
    SM.param_callbacks["gen1_amp"] = OBJ.amSetCh1GenAmp;
    SM.param_callbacks["gen1_freq"] = OBJ.amSetCh1GenFreq;

    SM.param_callbacks["gen2_type"] = OBJ.amSetCh2GenType;
    SM.param_callbacks["gen2_offset"] = OBJ.amSetCh2GenOffset;
    SM.param_callbacks["gen2_amp"] = OBJ.amSetCh2GenAmp;
    SM.param_callbacks["gen2_freq"] = OBJ.amSetCh2GenFreq;

    SM.param_callbacks["ch1_gain_adc"] = OBJ.amSetCh1GainADC;
    SM.param_callbacks["ch2_gain_adc"] = OBJ.amSetCh2GainADC;
    SM.param_callbacks["ch1_off_adc"] = OBJ.amSetCh1OffADC;
    SM.param_callbacks["ch2_off_adc"] = OBJ.amSetCh2OffADC;

    SM.param_callbacks["ch1_gain_dac"] = OBJ.amSetCh1GainDAC;
    SM.param_callbacks["ch2_gain_dac"] = OBJ.amSetCh2GainDAC;
    SM.param_callbacks["ch1_off_dac"] = OBJ.amSetCh1OffDAC;
    SM.param_callbacks["ch2_off_dac"] = OBJ.amSetCh2OffDAC;

    $('#B_CH1_SUB_OFF').on('click', function(ev) {
        OBJ.amSetNewCalib("CH1_ADC_OFF", parseInt($("#B_CH1_VALUE_OFF").val()) * -1);
    });

    $('#B_CH1_ADD_OFF').on('click', function(ev) {
        OBJ.amSetNewCalib("CH1_ADC_OFF", parseInt($("#B_CH1_VALUE_OFF").val()));
    });

    $('#B_CH2_SUB_OFF').on('click', function(ev) {
        OBJ.amSetNewCalib("CH2_ADC_OFF", parseInt($("#B_CH2_VALUE_OFF").val() * -1));
    });

    $('#B_CH2_ADD_OFF').on('click', function(ev) {
        OBJ.amSetNewCalib("CH2_ADC_OFF", parseInt($("#B_CH2_VALUE_OFF").val()));
    });

    $('#B_CH1_SUB_GAIN').on('click', function(ev) {
        OBJ.amSetNewCalib("CH1_ADC_GAIN", parseInt($("#B_CH1_VALUE_GAIN").val()) * -1);
    });

    $('#B_CH1_ADD_GAIN').on('click', function(ev) {
        OBJ.amSetNewCalib("CH1_ADC_GAIN", parseInt($("#B_CH1_VALUE_GAIN").val()));
    });

    $('#B_CH2_SUB_GAIN').on('click', function(ev) {
        OBJ.amSetNewCalib("CH2_ADC_GAIN", parseInt($("#B_CH2_VALUE_GAIN").val() * -1));
    });

    $('#B_CH2_ADD_GAIN').on('click', function(ev) {
        OBJ.amSetNewCalib("CH2_ADC_GAIN", parseInt($("#B_CH2_VALUE_GAIN").val()));
    });

    $('#B_CH1_DAC_SUB_OFF').on('click', function(ev) {
        OBJ.amSetNewCalib("CH1_DAC_OFF", parseInt($("#B_CH1_DAC_VALUE_OFF").val()) * -1);
    });

    $('#B_CH1_DAC_ADD_OFF').on('click', function(ev) {
        OBJ.amSetNewCalib("CH1_DAC_OFF", parseInt($("#B_CH1_DAC_VALUE_OFF").val()));
    });

    $('#B_CH2_DAC_SUB_OFF').on('click', function(ev) {
        OBJ.amSetNewCalib("CH2_DAC_OFF", parseInt($("#B_CH2_DAC_VALUE_OFF").val() * -1));
    });

    $('#B_CH2_DAC_ADD_OFF').on('click', function(ev) {
        OBJ.amSetNewCalib("CH2_DAC_OFF", parseInt($("#B_CH2_DAC_VALUE_OFF").val()));
    });

    $('#B_CH1_DAC_SUB_GAIN').on('click', function(ev) {
        OBJ.amSetNewCalib("CH1_DAC_GAIN", parseInt($("#B_CH1_DAC_VALUE_GAIN").val()) * -1);
    });

    $('#B_CH1_DAC_ADD_GAIN').on('click', function(ev) {
        OBJ.amSetNewCalib("CH1_DAC_GAIN", parseInt($("#B_CH1_DAC_VALUE_GAIN").val()));
    });

    $('#B_CH2_DAC_SUB_GAIN').on('click', function(ev) {
        OBJ.amSetNewCalib("CH2_DAC_GAIN", parseInt($("#B_CH2_DAC_VALUE_GAIN").val() * -1));
    });

    $('#B_CH2_DAC_ADD_GAIN').on('click', function(ev) {
        OBJ.amSetNewCalib("CH2_DAC_GAIN", parseInt($("#B_CH2_DAC_VALUE_GAIN").val()));
    });

});