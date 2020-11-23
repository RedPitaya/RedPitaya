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



    OBJ.filterModel = undefined;

    OBJ.filterSignal = [];
    OBJ.filterGraphCache = undefined;
    OBJ.filterCalibChange = false;
    OBJ.zoomMode = false;
    OBJ.cursor_x1_Pos = undefined;
    OBJ.cursor_x2_Pos = undefined;
    OBJ.filterHvLv = false;

    OBJ.filterSetModel = function(_model) {
        if (OBJ.filterModel === undefined) {
            OBJ.filterModel = _model.value;
        }
    }


    OBJ.filterInitData = function() {
        OBJ.filterSignal = [];
    }

    OBJ.filterInitRequest = function() {
        $("#graph_cont").css("visibility", "hidden");

        OBJ.filterCalibChange = false;
        SM.parametersCache["calib_sig"] = { value: 100 };
        SM.sendParameters();
        $('.filter_flipswitch').prop('checked', false);
        OBJ.filterHvLv = false;
        if (OBJ.filterGraphCache !== undefined) {
            delete OBJ.filterGraphCache;
            OBJ.filterGraphCache = undefined;
        }
    }



    OBJ.filterInitPlot = function(update) {
        delete OBJ.filterGraphCache;
        $('#bode_plot_filt').remove();


        OBJ.filterGraphCache = {};
        OBJ.filterGraphCache.elem = $('<div id="bode_plot_filt" class="plot" style="width:100%;height:100%;position: absolute;margin-top: auto;left: 0px;"/>').appendTo('#graph_bode_filt');
        var max_value = 1;
        if (OBJ.filterHvLv) {
            max_value = 10;
        }

        var options = {
            series: {
                shadowSize: 0
            },
            yaxes: [{
                show: true,
                min: -1 * max_value,
                max: max_value,
            }],
            xaxis: {
                show: false,
                color: '#aaaaaa',
                tickColor: '#aaaaaa',
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

        var sig = [];
        if (OBJ.filterSignal.value != undefined) {
            var copySignal = [...OBJ.filterSignal.value];
            for (var i = 0; i < copySignal.length; i++) {
                sig.push([i, copySignal[i]]);
            }
        } else {
            sig.push([0, 0]);
            sig.push([1, 0]);
        }

        var data_points = [{ data: sig, color: '#f3ec1a', label: "wave" }];
        OBJ.filterGraphCache.plot = $.plot(OBJ.filterGraphCache.elem, data_points, options);
        $('.flot-text').css('color', '#aaaaaa');
        OBJ.filterGraphCache.elem.show();
        OBJ.filterGraphCache.plot.resize();
        OBJ.filterGraphCache.plot.setupGrid();
        OBJ.filterGraphCache.plot.draw();

    }



    OBJ.filtDrawSignals = function() {


        if (OBJ.filterGraphCache == undefined) {
            OBJ.filterInitPlot(false);
        }
        var sig = [];
        if (OBJ.filterGraphCache != undefined && OBJ.filterSignal.value != undefined) {
            var copySignal = [...OBJ.filterSignal.value];

            for (var i = 0; i < copySignal.length; i++) {
                sig.push([i, copySignal[i]]);
            }
            $("#graph_cont").css("visibility", "unset");
        } else {
            sig.push([0, 0]);
            sig.push([1, 0]);
        }
        OBJ.filterGraphCache.elem.show();
        OBJ.filterGraphCache.plot.resize();
        OBJ.filterGraphCache.plot.setupGrid();
        var data_points = [{ data: sig, color: '#f3ec1a', label: "wave" }];
        OBJ.filterGraphCache.plot.setData(data_points);
        OBJ.filterGraphCache.plot.draw();

    };

    OBJ.filtSetZoomMode = function(_value) {
        OBJ.zoomMode = _value.value;
        $('#zoom_img').attr("src", OBJ.zoomMode ? "./img/zoom_minus.png" : "./img/zoom_plus.png");
        if (OBJ.zoomMode) {
            $(".cursor_animation").attr("is_hidden", true);
            $('#cur_x1_arrow, #cur_x2_arrow').draggable('disable');
        } else {
            $(".cursor_animation").removeAttr("is_hidden");
            $('#cur_x1_arrow, #cur_x2_arrow').draggable('enable');
        }
    }

    // Updates all elements related to a X cursor
    OBJ.updateXCursorElems = function(ui, save) {
        var x = (ui.helper[0].id == 'cur_x1_arrow' ? 'x1' : 'x2');
        var w = $("#graph_bode_filt").width();
        var ui_x = ui.position.left;
        var x1 = $('#cur_x1');
        var x2 = $('#cur_x2');
        var x1_left = parseInt(x1.css('left'));
        var x2_left = parseInt(x2.css('left'));
        OBJ.cursor_x1_Pos = (x1_left - 32) / (w - 40);
        OBJ.cursor_x2_Pos = (x2_left - 32) / (w - 40);
        SM.parametersCache["cursor_x1"] = { value: OBJ.cursor_x1_Pos };
        SM.parametersCache["cursor_x2"] = { value: OBJ.cursor_x2_Pos };
        SM.sendParameters();
        $('#cur_' + x + ', #cur_' + x + '_info').css('left', ui.position.left);
        OBJ.updateXCursorDiff();

    };

    OBJ.cursorResize = function() {
        var x1 = $('#cur_x1');
        var x2 = $('#cur_x2');
        var x1_left = parseInt(x1.css('left'));
        var x2_left = parseInt(x2.css('left'));
        var w = $("#graph_bode_filt").width() - 40;
        $('#cur_x1 , #cur_x1_arrow').css('left', w * OBJ.cursor_x1_Pos + 32);
        $('#cur_x2 , #cur_x2_arrow').css('left', w * OBJ.cursor_x2_Pos + 32);
        OBJ.updateXCursorDiff();
    }

    // Resizes double-headed arrow showing the difference between X cursors
    OBJ.updateXCursorDiff = function() {
        var x1 = $('#cur_x1');
        var x2 = $('#cur_x2');
        var x1_left = parseInt(x1.css('left'));
        var x2_left = parseInt(x2.css('left'));
        var diff_px = Math.abs(x1_left - x2_left);

        var left = Math.min(x1_left, x2_left);
        var value = $('#cur_x1_info').data('cleanval') - $('#cur_x2_info').data('cleanval');

        $('#cur_x_diff')
            .css('left', left + 1)
            .width(diff_px);

    };

    OBJ.cursorX1 = function(value) {
        if (!SM.state.cursor_dragging && !SM.state.mouseover) {
            if (value) {
                OBJ.cursor_x1_Pos = value.value;
                OBJ.cursorResize();
            }
        }
    }

    OBJ.cursorX2 = function(value) {
        if (!SM.state.cursor_dragging && !SM.state.mouseover) {
            if (value) {
                OBJ.cursor_x2_Pos = value.value;
                OBJ.cursorResize();
            }
        }
    }

    // OBJ.amSetCh1GainADC = function(_value) {
    //     $("#CH1_GAIN").val(_value.value);
    // }

    // OBJ.amSetCh2GainADC = function(_value) {
    //     $("#CH2_GAIN").val(_value.value);
    // }

    // OBJ.amSetCh1OffADC = function(_value) {
    //     $("#CH1_OFFSET").val(_value.value);
    // }

    // OBJ.amSetCh2OffADC = function(_value) {
    //     $("#CH2_OFFSET").val(_value.value);
    // }

    // OBJ.amSetCh1GainDAC = function(_value) {
    //     $("#CH1_DAC_GAIN").val(_value.value);
    // }

    // OBJ.amSetCh2GainDAC = function(_value) {
    //     $("#CH2_DAC_GAIN").val(_value.value);
    // }

    // OBJ.amSetCh1OffDAC = function(_value) {
    //     $("#CH1_DAC_OFFSET").val(_value.value);
    // }

    // OBJ.amSetCh2OffDAC = function(_value) {
    //     $("#CH2_DAC_OFFSET").val(_value.value);
    // }

    OBJ.filterSetMode = function(_mode, _state) {
        if (_mode == "FILTER_CHANNEL") {
            SM.parametersCache["adc_channel"] = { value: _state };
            SM.sendParameters2("adc_channel");
        }
        if (_mode == "FILTER_HV_LV_MODE") {
            SM.parametersCache["filter_hv_lv_mode"] = { value: _state };
            SM.sendParameters2("filter_hv_lv_mode");
            OBJ.filterHvLv = _state;
            if (OBJ.filterGraphCache !== undefined) {
                delete OBJ.filterGraphCache;
                OBJ.filterGraphCache = undefined;
            }
        }
    }

    OBJ.filterSetHyst = function(_value) {
        $("#FILTER_HYST").val(_value.value);
    }

    // OBJ.amSetNewCalib = function(_mode, _new_val) {
    //     if (_mode == "CH1_ADC_OFF") {
    //         SM.parametersCache["ch1_off_adc_new"] = { value: parseInt($("#CH1_OFFSET").val()) + _new_val };
    //         SM.sendParameters2("ch1_off_adc_new");
    //         OBJ.filterCalibChange = true;
    //     }

    //     if (_mode == "CH2_ADC_OFF") {
    //         SM.parametersCache["ch2_off_adc_new"] = { value: parseInt($("#CH2_OFFSET").val()) + _new_val };
    //         SM.sendParameters2("ch2_off_adc_new");
    //         OBJ.filterCalibChange = true;
    //     }

    //     if (_mode == "CH1_ADC_GAIN") {
    //         SM.parametersCache["ch1_gain_adc_new"] = { value: parseInt($("#CH1_GAIN").val()) + _new_val };
    //         SM.sendParameters2("ch1_gain_adc_new");
    //         OBJ.filterCalibChange = true;
    //     }

    //     if (_mode == "CH2_ADC_GAIN") {
    //         SM.parametersCache["ch2_gain_adc_new"] = { value: parseInt($("#CH2_GAIN").val()) + _new_val };
    //         SM.sendParameters2("ch2_gain_adc_new");
    //         OBJ.filterCalibChange = true;
    //     }

    //     if (_mode == "CH1_DAC_OFF") {
    //         SM.parametersCache["ch1_off_dac_new"] = { value: parseInt($("#CH1_DAC_OFFSET").val()) + _new_val };
    //         SM.sendParameters2("ch1_off_dac_new");
    //         OBJ.filterCalibChange = true;
    //     }

    //     if (_mode == "CH2_DAC_OFF") {
    //         SM.parametersCache["ch2_off_dac_new"] = { value: parseInt($("#CH2_DAC_OFFSET").val()) + _new_val };
    //         SM.sendParameters2("ch2_off_dac_new");
    //         OBJ.filterCalibChange = true;
    //     }

    //     if (_mode == "CH1_DAC_GAIN") {
    //         SM.parametersCache["ch1_gain_dac_new"] = { value: parseInt($("#CH1_DAC_GAIN").val()) + _new_val };
    //         SM.sendParameters2("ch1_gain_dac_new");
    //         OBJ.filterCalibChange = true;
    //     }

    //     if (_mode == "CH2_DAC_GAIN") {
    //         SM.parametersCache["ch2_gain_dac_new"] = { value: parseInt($("#CH2_DAC_GAIN").val()) + _new_val };
    //         SM.sendParameters2("ch2_gain_dac_new");
    //         OBJ.filterCalibChange = true;
    //     }
    // }

    // OBJ.amSetCh1GenType = function(_value) {
    //     $("#CH1_DAC_TYPE").val(_value.value);
    // }

    // OBJ.amSetCh2GenType = function(_value) {
    //     $("#CH2_DAC_TYPE").val(_value.value);
    // }

    // OBJ.amSetCh1GenFreq = function(_value) {
    //     $("#CH1_DAC_FREQ").val(_value.value);
    // }

    // OBJ.amSetCh2GenFreq = function(_value) {
    //     $("#CH2_DAC_FREQ").val(_value.value);
    // }

    // OBJ.amSetCh1GenAmp = function(_value) {
    //     $("#CH1_DAC_AMPL").val(_value.value);
    // }

    // OBJ.amSetCh2GenAmp = function(_value) {
    //     $("#CH2_DAC_AMPL").val(_value.value);
    // }

    // OBJ.amSetCh1GenOffset = function(_value) {
    //     $("#CH1_DAC_OFF").val(_value.value);
    // }

    // OBJ.amSetCh2GenOffset = function(_value) {
    //     $("#CH2_DAC_OFF").val(_value.value);
    // }

}(window.OBJ = window.OBJ || {}, jQuery));


// Page onload event handler
$(function() {

    setInterval(OBJ.filtDrawSignals, 100);
    OBJ.filterInitPlot(false);

    $('#zoom_img').click(function() {
        SM.parametersCache["zoom_mode"] = { value: OBJ.zoomMode ? false : true };
        SM.sendParameters2("zoom_mode");
    });

    $('.filter_flipswitch').change(function() {
        $(this).next().text($(this).is(':checked') ? ':checked' : ':not(:checked)');
        OBJ.filterSetMode($(this).attr('id'), $(this).is(':checked'));

    }).trigger('change');


    SM.param_callbacks["zoom_mode"] = OBJ.filtSetZoomMode;
    SM.param_callbacks["cursor_x1"] = OBJ.cursorX1;
    SM.param_callbacks["cursor_x2"] = OBJ.cursorX2;


    $('#cur_x1_arrow, #cur_x2_arrow').mouseenter(function(event) {
        SM.state.mouseover = true;
    });

    $('#cur_x1_arrow, #cur_x2_arrow').mouseleave(function(event) {
        SM.state.mouseover = false;
    });


    $('#cur_x1_arrow, #cur_x2_arrow').mousedown(function(event) {
        SM.state.cursor_dragging = true;
    });

    $('#cur_x1_arrow, #cur_x2_arrow').mouseup(function(event) {
        SM.state.cursor_dragging = false;
    });

    // X cursor arrows dragging
    $('#cur_x1_arrow, #cur_x2_arrow').draggable({
        axis: 'x',
        containment: 'parent',
        start: function(ev, ui) {
            SM.state.cursor_dragging = true;
        },
        drag: function(ev, ui) {
            ui.position.left = Math.max(32, Math.min($("#graph_bode_filt").width() - 8, ui.position.left));
            OBJ.updateXCursorElems(ui, false);
        },
        stop: function(ev, ui) {
            OBJ.updateXCursorElems(ui, true);
            SM.state.cursor_dragging = false;
        }
    });



    SM.param_callbacks["adc_hyst"] = OBJ.filterSetHyst;
    // SM.param_callbacks["gen1_amp"] = OBJ.amSetCh1GenAmp;
    // SM.param_callbacks["gen1_freq"] = OBJ.amSetCh1GenFreq;

    // SM.param_callbacks["gen2_type"] = OBJ.amSetCh2GenType;
    // SM.param_callbacks["gen2_offset"] = OBJ.amSetCh2GenOffset;
    // SM.param_callbacks["gen2_amp"] = OBJ.amSetCh2GenAmp;
    // SM.param_callbacks["gen2_freq"] = OBJ.amSetCh2GenFreq;

    // SM.param_callbacks["ch1_gain_adc"] = OBJ.amSetCh1GainADC;
    // SM.param_callbacks["ch2_gain_adc"] = OBJ.amSetCh2GainADC;
    // SM.param_callbacks["ch1_off_adc"] = OBJ.amSetCh1OffADC;
    // SM.param_callbacks["ch2_off_adc"] = OBJ.amSetCh2OffADC;

    // SM.param_callbacks["ch1_gain_dac"] = OBJ.amSetCh1GainDAC;
    // SM.param_callbacks["ch2_gain_dac"] = OBJ.amSetCh2GainDAC;
    // SM.param_callbacks["ch1_off_dac"] = OBJ.amSetCh1OffDAC;
    // SM.param_callbacks["ch2_off_dac"] = OBJ.amSetCh2OffDAC;

    // $('#B_CH1_SUB_OFF').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH1_ADC_OFF", parseInt($("#B_CH1_VALUE_OFF").val()) * -1);
    // });

    // $('#B_CH1_ADD_OFF').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH1_ADC_OFF", parseInt($("#B_CH1_VALUE_OFF").val()));
    // });

    // $('#B_CH2_SUB_OFF').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH2_ADC_OFF", parseInt($("#B_CH2_VALUE_OFF").val() * -1));
    // });

    // $('#B_CH2_ADD_OFF').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH2_ADC_OFF", parseInt($("#B_CH2_VALUE_OFF").val()));
    // });

    // $('#B_CH1_SUB_GAIN').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH1_ADC_GAIN", parseInt($("#B_CH1_VALUE_GAIN").val()) * -1);
    // });

    // $('#B_CH1_ADD_GAIN').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH1_ADC_GAIN", parseInt($("#B_CH1_VALUE_GAIN").val()));
    // });

    // $('#B_CH2_SUB_GAIN').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH2_ADC_GAIN", parseInt($("#B_CH2_VALUE_GAIN").val() * -1));
    // });

    // $('#B_CH2_ADD_GAIN').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH2_ADC_GAIN", parseInt($("#B_CH2_VALUE_GAIN").val()));
    // });

    // $('#B_CH1_DAC_SUB_OFF').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH1_DAC_OFF", parseInt($("#B_CH1_DAC_VALUE_OFF").val()) * -1);
    // });

    // $('#B_CH1_DAC_ADD_OFF').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH1_DAC_OFF", parseInt($("#B_CH1_DAC_VALUE_OFF").val()));
    // });

    // $('#B_CH2_DAC_SUB_OFF').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH2_DAC_OFF", parseInt($("#B_CH2_DAC_VALUE_OFF").val() * -1));
    // });

    // $('#B_CH2_DAC_ADD_OFF').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH2_DAC_OFF", parseInt($("#B_CH2_DAC_VALUE_OFF").val()));
    // });

    // $('#B_CH1_DAC_SUB_GAIN').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH1_DAC_GAIN", parseInt($("#B_CH1_DAC_VALUE_GAIN").val()) * -1);
    // });

    // $('#B_CH1_DAC_ADD_GAIN').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH1_DAC_GAIN", parseInt($("#B_CH1_DAC_VALUE_GAIN").val()));
    // });

    // $('#B_CH2_DAC_SUB_GAIN').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH2_DAC_GAIN", parseInt($("#B_CH2_DAC_VALUE_GAIN").val() * -1));
    // });

    // $('#B_CH2_DAC_ADD_GAIN').on('click', function(ev) {
    //     OBJ.amSetNewCalib("CH2_DAC_GAIN", parseInt($("#B_CH2_DAC_VALUE_GAIN").val()));
    // });

});