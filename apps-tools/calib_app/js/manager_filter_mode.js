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
    OBJ.filterHexMode = false;
    OBJ.filterAA = 0;
    OBJ.filterBB = 0;
    OBJ.filterPP = 0;
    OBJ.filterKK = 0;
    OBJ.filtAutoMode = 0;
    OBJ.filtAutoModeLVRef = 0.9;
    OBJ.filtAutoModeHVRef = 9;

    OBJ.filterSetModel = function(_model) {
        if (OBJ.filterModel === undefined) {
            OBJ.filterModel = _model.value;

            OBJ.filterConnectCallback();
        }
    }


    OBJ.filterInitData = function() {
        OBJ.filterSignal = [];
        OBJ.filtAutoMode = 0;
        OBJ.filtAutoModeLVRef = 0.9;
        OBJ.filtAutoModeHVRef = 9;
    }

    OBJ.filterInitRequest = function() {
        $("#graph_cont").css("visibility", "hidden");

        OBJ.filterCalibChange = false;
        SM.parametersCache["calib_sig"] = { value: 100 };
        SM.sendParameters();
        $('.filter_flipswitch').prop('checked', false);
        OBJ.filterHvLv = false;
        OBJ.filterHexMode = false;
        if (OBJ.filterGraphCache !== undefined) {
            delete OBJ.filterGraphCache;
            OBJ.filterGraphCache = undefined;
        }
    }



    OBJ.filterInitPlot = function(update) {
        if ($('#graph_bode_filt').length === 0) return;

        var value = $('#bode_plot_filt');

        delete OBJ.filterGraphCache;
        $('#bode_plot_filt').remove();


        OBJ.filterGraphCache = {};
        OBJ.filterGraphCache.elem = $('<div id="bode_plot_filt" class="plot" style="width:100%;height:100%;position: absolute;margin-top: auto;left: 0px;top:0px"/>').appendTo('#graph_bode_filt');
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
                ticks: 20,
                tickDecimals: 1
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


        if (OBJ.filterGraphCache == undefined ) {
            OBJ.filterInitPlot(false);
            return;
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

    OBJ.filterSetMode = function(_mode, _state) {
        if (_mode == "FILTER_HV_LV_MODE") {
            SM.parametersCache["filter_hv_lv_mode"] = { value: _state };
            SM.sendParameters2("filter_hv_lv_mode");
            OBJ.filterHvLv = _state;
            if (OBJ.filterGraphCache !== undefined) {
                delete OBJ.filterGraphCache;
                OBJ.filterGraphCache = undefined;
            }
        }
        if (_mode == "FILTER_DEC_HEX") {
            OBJ.filterHexMode = _state;
            OBJ.filterUpdateCoff();
        }
        if (_mode == "FILTER_DAC_CH1") {
            SM.parametersCache["filt_gen1_enable"] = { value: _state };
            SM.parametersCache["filt_gen2_enable"] = { value: _state };
            SM.sendParameters();
        }

        if (_mode == "am_filt_switch") {
            if ($("#FILTER_HV_LV_MODE").is(':checked')) {
                if (_state) {
                    $("#am_filt_dialog_img").attr("src", "./img/125/RP_125_GEN_HV.png");
                    $("#am_filt_dialog_text").text("Please set HV mode and connect OUT1 to IN1 and OUT2 to IN2.");
                    $("#am_filt_dialog_input").hide();
                } else {
                    $("#am_filt_dialog_img").attr("src", "./img/125/RP_125_REF_HV_FILTER.png");
                    $("#am_filt_dialog_text").text("Please set HV mode and connect IN1 and IN2 to reference signal source and set SQUARE SIGNAL 1kHz.");
                    $("#am_filt_dialog_input").show();
                }
            } else {
                if (_state) {
                    $("#am_filt_dialog_img").attr("src", "./img/125/RP_125_GEN.png");
                    $("#am_filt_dialog_text").text("Please set LV mode and connect OUT1 to IN1 and OUT2 to IN2.");
                    $("#am_filt_dialog_input").hide();
                } else {
                    $("#am_filt_dialog_img").attr("src", "./img/125/RP_125_REF_FILTER.png");
                    $("#am_filt_dialog_text").text("Please set LV mode and connect IN1 and IN2 to reference signal source and set SQUARE SIGNAL 1kHz.");
                    $("#am_filt_dialog_input").show();
                }
            }
            OBJ.filtAutoMode = _state;
            SM.parametersCache["filt_calib_auto_mode"] = { value: OBJ.filtAutoMode };
            SM.sendParameters();
        }
    }

    OBJ.filterSetHyst = function(_value) {
        $("#FILTER_HYST").val(_value.value);
    }


    OBJ.filterSetAA = function(_value) {
        OBJ.filterAA = _value.value;
        $("#FILTER_AA").val((OBJ.filterHexMode ? "0x" : "") + OBJ.filterAA.toString(OBJ.filterHexMode ? 16 : 10));

    }

    OBJ.filterSetBB = function(_value) {
        OBJ.filterBB = _value.value;
        $("#FILTER_BB").val((OBJ.filterHexMode ? "0x" : "") + OBJ.filterBB.toString(OBJ.filterHexMode ? 16 : 10));
    }

    OBJ.filterSetPP = function(_value) {
        OBJ.filterPP = _value.value;
        $("#FILTER_PP").val((OBJ.filterHexMode ? "0x" : "") + OBJ.filterPP.toString(OBJ.filterHexMode ? 16 : 10));
    }

    OBJ.filterSetKK = function(_value) {
        OBJ.filterKK = _value.value;
        $("#FILTER_KK").val((OBJ.filterHexMode ? "0x" : "") + OBJ.filterKK.toString(OBJ.filterHexMode ? 16 : 10));
    }

    OBJ.filterUpdateCoff = function() {
        if (OBJ.filterHexMode) {
            $("#FILTER_AA").val("0x" + OBJ.filterAA.toString(16));
            $("#FILTER_BB").val("0x" + OBJ.filterBB.toString(16));
            $("#FILTER_PP").val("0x" + OBJ.filterPP.toString(16));
            $("#FILTER_KK").val("0x" + OBJ.filterKK.toString(16));
        } else {
            $("#FILTER_AA").val(OBJ.filterAA);
            $("#FILTER_BB").val(OBJ.filterBB);
            $("#FILTER_PP").val(OBJ.filterPP);
            $("#FILTER_KK").val(OBJ.filterKK);
        }
    }


    OBJ.filterSetChGenFreq = function(_value) {
        $("#FILTER_DAC_FREQ").val(_value.value);
    }

    OBJ.filterSetChGenAmp = function(_value) {
        $("#FILTER_DAC_AMPL").val(_value.value);
    }

    OBJ.filterSetChGenOffset = function(_value) {
        $("#FILTER_DAC_OFF").val(_value.value);
    }

    OBJ.filtGetCalibSig = function(_value) {
        if (_value.value === 100) {
            $('body').addClass("loaded")
            $('#PROGRESS').hide();
        }
    }

    OBJ.filtSetProgress = function(_value) {
        $('#PROGRESS').attr('value', _value.value);
    }

    OBJ.filterSetDecimation = function(_value) {
        $("#FILTER_DECIMATION").val(_value.value);
    }

    OBJ.setSelectedChannel = function(_value) {
        $("#FILTER_CHANNEL_4CH").val(_value.value);
    }

    OBJ.filterConnectCallback = function() {
        $('#zoom_img').click(function() {
            SM.parametersCache["zoom_mode"] = { value: OBJ.zoomMode ? false : true };
            SM.sendParameters2("zoom_mode");
        });

        $("#B_FILTER_AUTO").click(function() {
            if (OBJ.filterModel === "Z20_125_4CH"){
                OBJ.filtAutoMode = 0;
                if ($("#FILTER_HV_LV_MODE").is(':checked')) {
                    {
                        $("#am_filt_dialog_img").attr("src", "./img/125_4CH/RP_125_REF_HV_FILTER.png");
                        $("#am_filt_dialog_text").text("Please set HV mode and connect IN1, IN2, IN3 and IN4 to reference signal source and set SQUARE SIGNAL 1kHz.");
                        $("#am_filt_dialog_input").show();
                        $("#SS_FILT_REF_VOLT").val(OBJ.filtAutoModeHVRef);
                        SM.parametersCache["filt_calib_ref_amp"] = { value: OBJ.filtAutoModeHVRef };
                    }
                } else {
                    {
                        $("#am_filt_dialog_img").attr("src", "./img/125_4CH/RP_125_REF_FILTER.png");
                        $("#am_filt_dialog_text").text("Please set LV mode and connect IN1, IN2, IN3 and IN4 to reference signal source and set SQUARE SIGNAL 1kHz.");
                        $("#am_filt_dialog_input").show();
                        $("#SS_FILT_REF_VOLT").val(OBJ.filtAutoModeLVRef);
                        SM.parametersCache["filt_calib_ref_amp"] = { value: OBJ.filtAutoModeLVRef };
                    }
                }
            }else{
                if ($("#FILTER_HV_LV_MODE").is(':checked')) {
                    if ($("#am_filt_switch").is(':checked')) {
                        $("#am_filt_dialog_img").attr("src", "./img/125/RP_125_GEN_HV.png");
                        $("#am_filt_dialog_text").text("Please set HV mode and connect OUT1 to IN1 and OUT2 to IN2.");
                        $("#am_filt_dialog_input").hide();
                    } else {
                        $("#am_filt_dialog_img").attr("src", "./img/125/RP_125_REF_HV_FILTER.png");
                        $("#am_filt_dialog_text").text("Please set HV mode and connect IN1 and IN2 to reference signal source and set SQUARE SIGNAL 1kHz.");
                        $("#am_filt_dialog_input").show();
                        $("#SS_FILT_REF_VOLT").val(OBJ.filtAutoModeHVRef);
                        SM.parametersCache["filt_calib_ref_amp"] = { value: OBJ.filtAutoModeHVRef };
                    }
                } else {
                    if ($("#am_filt_switch").is(':checked')) {
                        $("#am_filt_dialog_img").attr("src", "./img/125/RP_125_GEN.png");
                        $("#am_filt_dialog_text").text("Please set LV mode and connect OUT1 to IN1 and OUT2 to IN2.");
                        $("#am_filt_dialog_input").hide();
                    } else {
                        $("#am_filt_dialog_img").attr("src", "./img/125/RP_125_REF_FILTER.png");
                        $("#am_filt_dialog_text").text("Please set LV mode and connect IN1 and IN2 to reference signal source and set SQUARE SIGNAL 1kHz.");
                        $("#am_filt_dialog_input").show();
                        $("#SS_FILT_REF_VOLT").val(OBJ.filtAutoModeLVRef);
                        SM.parametersCache["filt_calib_ref_amp"] = { value: OBJ.filtAutoModeLVRef };
                    }
                }
            }
            SM.parametersCache["filt_calib_auto_mode"] = { value: OBJ.filtAutoMode };
            SM.sendParameters();


            $("#am_filt_external_btn").off('click');
            $('#am_filt_cancel_btn').off('click');
            $('#am_filt_external_btn').on('click', function() {
                $('body').removeClass("loaded")
                $('#PROGRESS').css("display", "block");
                $('#PROGRESS').attr('value', 0);
                SM.parametersCache["filt_calib_step"] = { value: 1 };
                SM.sendParameters();
                $("#am_dialog_filter_calib").modal('hide');
            });
            $('#am_filt_cancel_btn').on('click', function() {});
            $("#am_dialog_filter_calib").modal('show');
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
    }


}(window.OBJ = window.OBJ || {}, jQuery));


// Page onload event handler
$(function() {

    setInterval(OBJ.filtDrawSignals, 100);
    OBJ.filterInitPlot(false);



    SM.param_callbacks["adc_hyst"] = OBJ.filterSetHyst;
    SM.param_callbacks["filt_gen_offset"] = OBJ.filterSetChGenOffset;
    SM.param_callbacks["filt_gen_amp"] = OBJ.filterSetChGenAmp;
    SM.param_callbacks["filt_gen_freq"] = OBJ.filterSetChGenFreq;
    SM.param_callbacks["adc_decimation"] = OBJ.filterSetDecimation;

    SM.param_callbacks["filt_aa"] = OBJ.filterSetAA;
    SM.param_callbacks["filt_bb"] = OBJ.filterSetBB;
    SM.param_callbacks["filt_pp"] = OBJ.filterSetPP;
    SM.param_callbacks["filt_kk"] = OBJ.filterSetKK;
    SM.param_callbacks["filt_calib_step"] = OBJ.filtGetCalibSig;
    SM.param_callbacks["filt_calib_progress"] = OBJ.filtSetProgress;
    SM.param_callbacks["adc_channel"] = OBJ.setSelectedChannel;

});