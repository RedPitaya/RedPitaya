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
    OBJ.adcSig1ArrayMIN = [];
    OBJ.adcSig1ArrayMAX = [];

    OBJ.adcSig2ArrayAVG = [];
    OBJ.adcSig2ArrayMIN = [];
    OBJ.adcSig2ArrayMAX = [];

    OBJ.adcSig3ArrayAVG = [];
    OBJ.adcSig3ArrayMIN = [];
    OBJ.adcSig3ArrayMAX = [];

    OBJ.adcSig4ArrayAVG = [];
    OBJ.adcSig4ArrayMIN = [];
    OBJ.adcSig4ArrayMAX = [];

    OBJ.adcGraphCacheCh = [];
    OBJ.adcCalibChange = false;
    OBJ.adcPPerBufLastValue = new Map();

    OBJ.adcSetModel = function(_model) {
        if (OBJ.adcModel === undefined) {
            OBJ.adcModel = _model.value;
            setInterval(OBJ.drawSignalsCH1, 100);
            setInterval(OBJ.drawSignalsCH2, 100);
            if(OBJ.adcModel === "Z20_125_4CH"){
                setInterval(OBJ.drawSignalsCH3, 100);
                setInterval(OBJ.drawSignalsCH4, 100);
            }
            OBJ.amConnectCallbacks();

            if(OBJ.adcModel === "Z20"){
                $("#manual_lv_hv_mode").remove();
                $("#dac_type_1").remove();
                $("#dac_type_2").remove();
            }

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
        $("#CH1_AVG").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig1ArrayAVG, _value.value);
    }

    OBJ.adcSetCH1Max = function(_value) {
        $("#CH1_MAX").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig1ArrayMAX, _value.value);
    }

    OBJ.adcSetCH1Min = function(_value) {
        $("#CH1_MIN").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig1ArrayMIN, _value.value);
    }

    OBJ.adcSetCH1PP = function(_value) {
        $("#CH1_P_P").val(_value.value.toFixed(4) + " V");
    }

    OBJ.adcSetCH1IsSin = function(_value) {
        if (_value.value){
            $("#CH1_IS_SINE").attr("src","img/SIN_G.png");
            $("#CH1_PER_COUNT").text('x'+OBJ.adcPPerBufLastValue.get(1));
        }else{
            $("#CH1_IS_SINE").attr("src","img/SIN_R.png");
            $("#CH1_PER_COUNT").text('');
        }
    }

    OBJ.adcSetCH1PPerBuf = function(_value) {
        OBJ.adcPPerBufLastValue.set(1,_value.value);
    }

    OBJ.adcSetCH2Avg = function(_value) {
        $("#CH2_AVG").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig2ArrayAVG, _value.value);
    }

    OBJ.adcSetCH2Max = function(_value) {
        $("#CH2_MAX").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig2ArrayMAX, _value.value);
    }

    OBJ.adcSetCH2Min = function(_value) {
        $("#CH2_MIN").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig2ArrayMIN, _value.value);
    }

    OBJ.adcSetCH2PP = function(_value) {
        $("#CH2_P_P").val(_value.value.toFixed(4) + " V");
    }

    OBJ.adcSetCH2IsSin = function(_value) {
        if (_value.value){
            $("#CH2_IS_SINE").attr("src","img/SIN_G.png");
            $("#CH2_PER_COUNT").text('x'+OBJ.adcPPerBufLastValue.get(2));
        }else{
            $("#CH2_IS_SINE").attr("src","img/SIN_R.png");
            $("#CH2_PER_COUNT").text('');
        }
    }

    OBJ.adcSetCH2PPerBuf = function(_value) {
        OBJ.adcPPerBufLastValue.set(2,_value.value);
    }

    OBJ.adcSetCH3Avg = function(_value) {
        $("#CH3_AVG").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig3ArrayAVG, _value.value);
    }

    OBJ.adcSetCH3Max = function(_value) {
        $("#CH3_MAX").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig3ArrayMAX, _value.value);
    }

    OBJ.adcSetCH3PP = function(_value) {
        $("#CH3_P_P").val(_value.value.toFixed(4) + " V");
    }

    OBJ.adcSetCH3Min = function(_value) {
        $("#CH3_MIN").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig3ArrayMIN, _value.value);
    }

    OBJ.adcSetCH3IsSin = function(_value) {
        if (_value.value){
            $("#CH3_IS_SINE").attr("src","img/SIN_G.png");
            $("#CH3_PER_COUNT").text('x'+ OBJ.adcPPerBufLastValue.get(3));
        }else{
            $("#CH3_IS_SINE").attr("src","img/SIN_R.png");
            $("#CH3_PER_COUNT").text('');
        }
    }

    OBJ.adcSetCH3PPerBuf = function(_value) {
        OBJ.adcPPerBufLastValue.set(3,_value.value);
    }

    OBJ.adcSetCH4Avg = function(_value) {
        $("#CH4_AVG").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig4ArrayAVG, _value.value);
    }

    OBJ.adcSetCH4Max = function(_value) {
        $("#CH4_MAX").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig4ArrayMAX, _value.value);
    }

    OBJ.adcSetCH4Min = function(_value) {
        $("#CH4_MIN").val(_value.value.toFixed(4) + " V");
        OBJ.adcPush(OBJ.adcSig4ArrayMIN, _value.value);
    }

    OBJ.adcSetCH4PP = function(_value) {
        $("#CH4_P_P").val(_value.value.toFixed(4) + " V");
    }

    OBJ.adcSetCH4IsSin = function(_value) {
        if (_value.value){
            $("#CH4_IS_SINE").attr("src","img/SIN_G.png");
            $("#CH4_PER_COUNT").text('x'+OBJ.adcPPerBufLastValue.get(4));
        }else{
            $("#CH4_IS_SINE").attr("src","img/SIN_R.png");
            $("#CH4_PER_COUNT").text('');
        }
    }

    OBJ.adcSetCH4PPerBuf = function(_value) {
        OBJ.adcPPerBufLastValue.set(4,_value.value);
    }

    OBJ.adcInitData = function() {
        OBJ.adcSig1ArrayAVG = [];
        OBJ.adcSig1ArrayMIN = [];
        OBJ.adcSig1ArrayMAX = [];

        OBJ.adcSig2ArrayAVG = [];
        OBJ.adcSig2ArrayMIN = [];
        OBJ.adcSig2ArrayMAX = [];

        OBJ.adcSig3ArrayAVG = [];
        OBJ.adcSig3ArrayMIN = [];
        OBJ.adcSig3ArrayMAX = [];

        OBJ.adcSig4ArrayAVG = [];
        OBJ.adcSig4ArrayMIN = [];
        OBJ.adcSig4ArrayMAX = [];
    }

    OBJ.adcInitRequest = function() {
        OBJ.adcCalibChange = false;
        SM.parametersCache["calib_sig"] = { value: 1 };
        SM.sendParameters();
        $('.flipswitch').prop('checked', false);
    }


    OBJ.prepareDataCH = function(ch) {
        var ar_avg = undefined;
        var ar_min = undefined;
        var ar_max = undefined;
        if (ch === 1){
            ar_avg = OBJ.adcSig1ArrayAVG.slice();
            ar_min = OBJ.adcSig1ArrayMIN.slice();
            ar_max = OBJ.adcSig1ArrayMAX.slice();
        }

        if (ch === 2){
            ar_avg = OBJ.adcSig2ArrayAVG.slice();
            ar_min = OBJ.adcSig2ArrayMIN.slice();
            ar_max = OBJ.adcSig2ArrayMAX.slice();
        }

        if (ch === 3){
            ar_avg = OBJ.adcSig3ArrayAVG.slice();
            ar_min = OBJ.adcSig3ArrayMIN.slice();
            ar_max = OBJ.adcSig3ArrayMAX.slice();
        }

        if (ch === 4){
            ar_avg = OBJ.adcSig4ArrayAVG.slice();
            ar_min = OBJ.adcSig4ArrayMIN.slice();
            ar_max = OBJ.adcSig4ArrayMAX.slice();
        }

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


    OBJ.adcInitPlotCH = function(update,ch) {
        delete OBJ.adcGraphCacheCh[ch];
        $('#bode_plot_ch' + ch).remove();


        OBJ.adcGraphCacheCh[ch] = {};
        OBJ.adcGraphCacheCh[ch].elem = $('<div id="bode_plot_ch'+ch+'" class="plot" style="width:278px;height:78px;position: absolute;margin-top: auto;left: 0px;"/>').appendTo('#graph_bode_ch'+ch);

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
            var x = OBJ.prepareDataCH(ch);
            if (x !== undefined) {
                sig1 = x.CH_AVG;
                sig2 = x.CH_MAX;
                sig3 = x.CH_MIN;
            }
        }
        var data_points = [{ data: sig1, color: '#f3ec1a', label: "AVG" }, { data: sig2, color: '#ff0000', label: "MAX" }, { data: sig3, color: '#00FF00', label: "MIN" }];
        OBJ.adcGraphCacheCh[ch].plot = $.plot(OBJ.adcGraphCacheCh[ch].elem, data_points, options);
        $('.flot-text').css('color', '#aaaaaa');
    }

    OBJ.drawSignalsCH = function(ch) {
        var sig1 = [];
        var sig2 = [];
        var sig3 = [];
        if (OBJ.adcGraphCacheCh[ch] == undefined) {
            OBJ.adcInitPlotCH(false,ch);
        }

        var x = OBJ.prepareDataCH(ch);
        if (x !== undefined) {
            sig1 = x.CH_AVG;
            sig2 = x.CH_MAX;
            sig3 = x.CH_MIN;

            OBJ.adcGraphCacheCh[ch].elem.show();
            OBJ.adcGraphCacheCh[ch].plot.resize();
            OBJ.adcGraphCacheCh[ch].plot.setupGrid();
            var data_points = [{ data: sig1, color: '#f3ec1a', label: "AVG" }, { data: sig2, color: '#ff0000', label: "MAX" }, { data: sig3, color: '#00FF00', label: "MIN" }];

            OBJ.adcGraphCacheCh[ch].plot.setData(data_points);
            OBJ.adcGraphCacheCh[ch].plot.draw();
        }
    }

    OBJ.drawSignalsCH1 = function() {
        OBJ.drawSignalsCH(1)
    };

    OBJ.drawSignalsCH2 = function() {
        OBJ.drawSignalsCH(2)
    };

    OBJ.drawSignalsCH3 = function() {
        OBJ.drawSignalsCH(3)
    };

    OBJ.drawSignalsCH4 = function() {
        OBJ.drawSignalsCH(4)
    };

    OBJ.amSetCh1GainADC = function(_value) {
        $("#CH1_GAIN").val(_value.value);
    }

    OBJ.amSetCh2GainADC = function(_value) {
        $("#CH2_GAIN").val(_value.value);
    }

    OBJ.amSetCh3GainADC = function(_value) {
        $("#CH3_GAIN").val(_value.value);
    }

    OBJ.amSetCh4GainADC = function(_value) {
        $("#CH4_GAIN").val(_value.value);
    }

    OBJ.amSetCh1OffADC = function(_value) {
        $("#CH1_OFFSET").val(_value.value);
    }

    OBJ.amSetCh2OffADC = function(_value) {
        $("#CH2_OFFSET").val(_value.value);
    }

    OBJ.amSetCh3OffADC = function(_value) {
        $("#CH3_OFFSET").val(_value.value);
    }

    OBJ.amSetCh4OffADC = function(_value) {
        $("#CH4_OFFSET").val(_value.value);
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
            SM.parametersCache["request_reset_avg_filter"] = {value: true};
            SM.sendParameters2("gen1_enable");
            setTimeout(OBJ.adcInitData, 1000);
        }
        if (_mode == "ch2_dac_enable") {
            SM.parametersCache["gen2_enable"] = { value: _state };
            SM.parametersCache["request_reset_avg_filter"] = {value: true};
            SM.sendParameters2("gen2_enable");
            setTimeout(OBJ.adcInitData, 1000);
        }
        if (_mode == "HV_LV_MODE") {
            SM.parametersCache["hv_lv_mode"] = { value: _state };
            SM.parametersCache["request_reset_avg_filter"] = {value: true};
            SM.sendParameters2("hv_lv_mode");
            setTimeout(OBJ.adcInitData, 1000);
        }
        if (_mode == "AC_DC_MODE") {
            SM.parametersCache["ac_dc_mode"] = { value: _state };
            SM.parametersCache["request_reset_avg_filter"] = {value: true};
            SM.sendParameters2("ac_dc_mode");
            setTimeout(OBJ.adcInitData, 1000);
        }
        if (_mode == "AVG_LAST_MODE") {
            SM.parametersCache["avg_last_mode"] = { value: _state };
            SM.parametersCache["request_reset_avg_filter"] = {value: true};
            SM.sendParameters2("avg_last_mode");
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

        if (_mode == "CH3_ADC_OFF") {
            SM.parametersCache["ch3_off_adc_new"] = { value: parseInt($("#CH3_OFFSET").val()) + _new_val };
            SM.sendParameters2("ch3_off_adc_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH4_ADC_OFF") {
            SM.parametersCache["ch4_off_adc_new"] = { value: parseInt($("#CH4_OFFSET").val()) + _new_val };
            SM.sendParameters2("ch4_off_adc_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH1_ADC_GAIN") {
            SM.parametersCache["ch1_gain_adc_new"] = { value: parseFloat($("#CH1_GAIN").val()) + _new_val };
            SM.sendParameters2("ch1_gain_adc_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH2_ADC_GAIN") {
            SM.parametersCache["ch2_gain_adc_new"] = { value: parseFloat($("#CH2_GAIN").val()) + _new_val };
            SM.sendParameters2("ch2_gain_adc_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH3_ADC_GAIN") {
            SM.parametersCache["ch3_gain_adc_new"] = { value: parseInt($("#CH3_GAIN").val()) + _new_val };
            SM.sendParameters2("ch3_gain_adc_new");parseFloat
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH4_ADC_GAIN") {
            SM.parametersCache["ch4_gain_adc_new"] = { value: parseFloat($("#CH4_GAIN").val()) + _new_val };
            SM.sendParameters2("ch4_gain_adc_new");
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
            SM.parametersCache["ch1_gain_dac_new"] = { value: parseFloat($("#CH1_DAC_GAIN").val()) + _new_val };
            SM.sendParameters2("ch1_gain_dac_new");
            OBJ.adcCalibChange = true;
        }

        if (_mode == "CH2_DAC_GAIN") {
            SM.parametersCache["ch2_gain_dac_new"] = { value: parseFloat($("#CH2_DAC_GAIN").val()) + _new_val };
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

    OBJ.amSetLastMode = function(_value) {
        $("#AVG_LAST_MODE").val(_value.value);
    }

    OBJ.amSetDecimation = function(_value) {
        $("#B_DEC_SELECTOR").val(_value.value);
    }

    OBJ.amConnectCallbacks = function() {
        $('.man_flipswitch').change(function() {
            $(this).next().text($(this).is(':checked') ? ':checked' : ':not(:checked)');
            OBJ.amSetMode($(this).attr('id'), $(this).is(':checked'));

        }).trigger('change');

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

        $('#B_CH3_SUB_OFF').on('click', function(ev) {
            OBJ.amSetNewCalib("CH3_ADC_OFF", parseInt($("#B_CH3_VALUE_OFF").val() * -1));
        });

        $('#B_CH3_ADD_OFF').on('click', function(ev) {
            OBJ.amSetNewCalib("CH3_ADC_OFF", parseInt($("#B_CH3_VALUE_OFF").val()));
        });

        $('#B_CH4_SUB_OFF').on('click', function(ev) {
            OBJ.amSetNewCalib("CH4_ADC_OFF", parseInt($("#B_CH4_VALUE_OFF").val() * -1));
        });

        $('#B_CH4_ADD_OFF').on('click', function(ev) {
            OBJ.amSetNewCalib("CH4_ADC_OFF", parseInt($("#B_CH4_VALUE_OFF").val()));
        });

        $('#B_CH1_SUB_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH1_ADC_GAIN", parseFloat($("#B_CH1_VALUE_GAIN").val()) * -1);
        });

        $('#B_CH1_ADD_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH1_ADC_GAIN", parseFloat($("#B_CH1_VALUE_GAIN").val()));
        });

        $('#B_CH2_SUB_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH2_ADC_GAIN", parseFloat($("#B_CH2_VALUE_GAIN").val() * -1));
        });

        $('#B_CH2_ADD_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH2_ADC_GAIN", parseFloat($("#B_CH2_VALUE_GAIN").val()));
        });

        $('#B_CH3_SUB_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH3_ADC_GAIN", parseFloat($("#B_CH3_VALUE_GAIN").val() * -1));
        });

        $('#B_CH3_ADD_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH3_ADC_GAIN", parseFloat($("#B_CH3_VALUE_GAIN").val()));
        });

        $('#B_CH4_SUB_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH4_ADC_GAIN", parseFloat($("#B_CH4_VALUE_GAIN").val() * -1));
        });

        $('#B_CH4_ADD_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH4_ADC_GAIN", parseFloat($("#B_CH4_VALUE_GAIN").val()));
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
            OBJ.amSetNewCalib("CH1_DAC_GAIN", parseFloat($("#B_CH1_DAC_VALUE_GAIN").val()) * -1);
        });

        $('#B_CH1_DAC_ADD_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH1_DAC_GAIN", parseFloat($("#B_CH1_DAC_VALUE_GAIN").val()));
        });

        $('#B_CH2_DAC_SUB_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH2_DAC_GAIN", parseFloat($("#B_CH2_DAC_VALUE_GAIN").val() * -1));
        });

        $('#B_CH2_DAC_ADD_GAIN').on('click', function(ev) {
            OBJ.amSetNewCalib("CH2_DAC_GAIN", parseFloat($("#B_CH2_DAC_VALUE_GAIN").val()));
        });
    }

}(window.OBJ = window.OBJ || {}, jQuery));


// Page onload event handler
$(function() {
    // $('#am_ok_btn').on('click', function() { OBJ.amClickOkDialog() });


    SM.param_callbacks["gen1_type"] = OBJ.amSetCh1GenType;
    SM.param_callbacks["gen1_offset"] = OBJ.amSetCh1GenOffset;
    SM.param_callbacks["gen1_amp"] = OBJ.amSetCh1GenAmp;
    SM.param_callbacks["gen1_freq"] = OBJ.amSetCh1GenFreq;
    SM.param_callbacks["manual_decimation"] = OBJ.amSetDecimation;
    SM.param_callbacks["avg_last_mode"] = OBJ.amSetLastMode;


    SM.param_callbacks["gen2_type"] = OBJ.amSetCh2GenType;
    SM.param_callbacks["gen2_offset"] = OBJ.amSetCh2GenOffset;
    SM.param_callbacks["gen2_amp"] = OBJ.amSetCh2GenAmp;
    SM.param_callbacks["gen2_freq"] = OBJ.amSetCh2GenFreq;

    SM.param_callbacks["ch1_gain_adc"] = OBJ.amSetCh1GainADC;
    SM.param_callbacks["ch2_gain_adc"] = OBJ.amSetCh2GainADC;
    SM.param_callbacks["ch3_gain_adc"] = OBJ.amSetCh3GainADC;
    SM.param_callbacks["ch4_gain_adc"] = OBJ.amSetCh4GainADC;
    SM.param_callbacks["ch1_off_adc"] = OBJ.amSetCh1OffADC;
    SM.param_callbacks["ch2_off_adc"] = OBJ.amSetCh2OffADC;
    SM.param_callbacks["ch3_off_adc"] = OBJ.amSetCh3OffADC;
    SM.param_callbacks["ch4_off_adc"] = OBJ.amSetCh4OffADC;

    SM.param_callbacks["ch1_gain_dac"] = OBJ.amSetCh1GainDAC;
    SM.param_callbacks["ch2_gain_dac"] = OBJ.amSetCh2GainDAC;
    SM.param_callbacks["ch1_off_dac"] = OBJ.amSetCh1OffDAC;
    SM.param_callbacks["ch2_off_dac"] = OBJ.amSetCh2OffDAC;
});