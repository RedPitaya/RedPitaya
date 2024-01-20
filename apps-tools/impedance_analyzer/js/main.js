/*
 * Red Pitaya Impdance analyzer
 *
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


(function() {
    var originalAddClassMethod = jQuery.fn.addClass;
    var originalRemoveClassMethod = jQuery.fn.removeClass;
    $.fn.addClass = function(clss) {
        var result = originalAddClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'add');
        return result;
    };
    $.fn.removeClass = function(clss) {
        var result = originalRemoveClassMethod.apply(this, arguments);
        $(this).trigger('activeChanged', 'remove');
        return result;
    }
})();


//Bode analyser
(function(MAIN, $, undefined) {


    MAIN.scale = true;
    MAIN.curGraphScale = true;
    MAIN.param_callbacks = {};

    // App state
    MAIN.state = {
        editing: false,
        trig_dragging: false,
        resized: false,
        graph_grid_height: null,
        graph_grid_width: null
    };

    MAIN.firstValGot = { 'BA_SIGNAL_1': false, 'BA_SIGNAL_2': false };

    MAIN.lastSignals = { 'BA_SIGNAL_1': undefined, 'BA_SIGNAL_2': undefined, 'BA_SIGNAL_1_BAD': undefined, 'BA_SIGNAL_2_BAD': undefined };

    // Current step and freq
    MAIN.currentFreq = 0;
    MAIN.currentStep = 0;

    //Graph cache
    MAIN.graphCache = undefined;

    MAIN.running = true;
    MAIN.calibrating = false;
    MAIN.unexpectedClose = false;

    MAIN.cur_freq = 0;
    MAIN.input_threshold = 0;


    MAIN.graphSize = {
        left: 0,
        right: 0,
        top: 0,
        bottom: 0,
    };



    //Write email
    MAIN.formEmail = function() {
        //var file = new FileReader();
        var mail = "support@redpitaya.com";
        var subject = "Crash report Red Pitaya OS";
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: MAIN.parametersCache }) + "%0D%0A";
        body += "Browser:" + "%0D%0A" + JSON.stringify({ parameters: $.browser }) + "%0D%0A";

        var url = 'info/info.json';
        $.ajax({
            method: "GET",
            url: url
        }).done(function(msg) {
            body += " info.json: " + "%0D%0A" + msg.responseText;
        }).fail(function(msg) {
            var info_json = msg.responseText
            var ver = '';
            try {
                var obj = JSON.parse(msg.responseText);
                ver = " " + obj['version'];
            } catch (e) {};

            body += " info.json: " + "%0D%0A" + msg.responseText;
            document.location.href = "mailto:" + mail + "?subject=" + subject + ver + "&body=" + body;
        });
    }


    // For Firefox
    function fireEvent(obj, evt) {
        var fireOnThis = obj;
        if (document.createEvent) {
            var evObj = document.createEvent('MouseEvents');
            evObj.initEvent(evt, true, false);
            fireOnThis.dispatchEvent(evObj);

        } else if (document.createEventObject) {
            var evObj = document.createEventObject();
            fireOnThis.fireEvent('on' + evt, evObj);
        }
    }

    MAIN.SaveGraphs = function() {
        html2canvas($('body'), {
            background: '#343433', // Like background of BODY
            onrendered: function(canvas) {
                var a = document.createElement('a');
                a.href = canvas.toDataURL('image/jpeg').replace('image/jpeg', 'image/octet-stream');
                a.download = 'graphs.jpg';
                fireEvent(a, 'click');
            }
        });
    }

    MAIN.downloadDataAsCSV = function(filename) {
        var header = 'Frequency [Hz], Amplitude [dB], Phase [deg]';
        var row_delim = '\n';
        var strings = header + row_delim;

        var amplitude_signal = MAIN.lastSignals['BA_SIGNAL_1'];
        var phase_signal = MAIN.lastSignals['BA_SIGNAL_2'];

        if (amplitude_signal.length === phase_signal.length) {
            for (var i = 0; i < amplitude_signal.length; ++i) {
                strings += amplitude_signal[i][0] + ', ' + amplitude_signal[i][1] + ', ' + phase_signal[i][1] + row_delim;
            }
        }

        var a = document.createElement('a');
        a.setAttribute('download', filename);
        a.setAttribute('href', 'data:' + 'text/html' + ';charset=utf-8,' + encodeURIComponent(strings));
        fireEvent(a, 'click');
    }


    // Draws grid on the lowest canvas layer
    MAIN.drawGrid = function() {
        var canvas_width = $('#graph_bode').width() - 2;
        var canvas_height = Math.round(canvas_width / 2);

        //Draw grid
        var ctx = $('#graph_bode_grid')[0].getContext('2d');

        // Set canvas size
        ctx.canvas.width = canvas_width;
        ctx.canvas.height = canvas_height;

        return;
    };






    function funcxTickFormat(val, axis) {

        if (Math.abs(val) >= 0 && Math.abs(val) < 1000)
            return (val * 1).toFixed(0) + " ";
        else if (Math.abs(val) >= 1000 && Math.abs(val) < 1000000)
            return (val / 1000).toFixed(2) + "k";
        else if (Math.abs(val) >= 1000000)
            return (val / 1000000).toFixed(2) + "M";
    }


    //Draw one signal
    MAIN.prepareOneSignal = function(SIGNALS,signal_name) {
        var one_signal = SIGNALS[signal_name];
        var signal_param = SIGNALS['BA_SIGNAL_PARAMETERS'];

        // Ignore empty signals
        if (one_signal.size == 0)
            return;

        // Create points mas
        var points = [];
        var startFreq = 0,
            endFreq = 0,
            steps = 0,
            step = 0,
            min_val = undefined,
            max_val = undefined,

        startFreq = signal_param.value[0];
        endFreq = signal_param.value[1];
        steps = signal_param.value[2];
        step = (endFreq - startFreq) / (steps - 1);


        for (var i = 0; i < one_signal.size; i++) {
            var freq = startFreq + i * step;
            var samp_val = one_signal.value[i];
            if (MAIN.scale) {
                var a = Math.log10(startFreq);
                var b = Math.log10(endFreq);
                var c = (b - a) / (steps - 1);
                freq = Math.pow(10, c * i + a);
            }
            MAIN.freq = freq;

            if ((min_val == undefined || min_val > samp_val)) min_val = samp_val;
            if ((max_val == undefined || max_val < samp_val)) max_val = samp_val;

            points.push([freq, samp_val]);
            MAIN.lastSignals[signal_name] = points;
            freq_old = freq;
        }



        // AUTOSCALE if switched on
        if ($('#BA_AUTOSCALE_BTN').hasClass('active')) {
            // For autoscale of max Y value
            var min = min_val;
            var max = max_val;


            var yaxis;
            var axis_name = ""
            // Choose right yaxis
            if (signal_name == "BA_SIGNAL_1"){
                yaxis = MAIN.graphCache.plot.getAxes().yaxis;
                axis_name = "BA_GAIN"
            }


            var size = (max - min) * 0.2;
            if (size < 5) size = 5
            $('#' + axis_name + '_MAX').val(max + size);
            yaxis.options.max = max+ size;

            $('#' + axis_name + '_MIN').val(min - size);
            yaxis.options.min = min - size;

            CLIENT.parametersCache["BA_GAIN_MIN"] = { value: $("#BA_GAIN_MIN").val() };
            CLIENT.parametersCache["BA_GAIN_MAX"] = { value: $("#BA_GAIN_MAX").val() };
            CLIENT.sendParameters();

        }
    }


    MAIN.initPlot = function(update) {
        delete MAIN.graphCache;
        $('#bode_plot').remove();


        var yMin1 = parseFloat($('#BA_GAIN_MIN').val()) * 1;
        var yMax1 = parseFloat($('#BA_GAIN_MAX').val()) * 1;

        MAIN.graphCache = {};
        MAIN.graphCache.elem = $('<div id="bode_plot" class="plot" />').css($('#graph_bode_grid').css(['height', 'width'])).appendTo('#graph_bode');

        var t = [];
        if ($("#BA_SCALE1").hasClass("active"))
            t = [1, 10, 100, 200, 400, 600, 800, 1000, 2000, 4000, 6000, 8000, 10000, 20000, 40000, 60000, 80000, 100000, 200000, 400000, 600000, 800000, 1000000, 2000000, 4000000, 6000000, 8000000, 10000000, , 20000000, 40000000, 60000000, 80000000, 100000000];
        else
            t = null;

        var options = {
            series: {
                shadowSize: 0
            },
            yaxes: [{
                    min: yMin1,
                    max: yMax1,
                    labelWidth: 30,
                    alignTicksWithAxis: 1,
                    position: "left"
                }
            ],
            xaxis: {
                color: '#aaaaaa',
                tickColor: '#aaaaaa',
                ticks: t,
                transform: function(v) {
                    if (MAIN.scale)
                        return Math.log(v + 0.0001); // move away from zero
                    else
                        return v;

                },
                tickDecimals: 0,
                reserveSpace: false,
                tickFormatter: funcxTickFormat,
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
            }
        };


        if ($("#BA_SCALE0").hasClass("active")) {
            options.xaxis.transform = null;
        }

        var lastsig1 = [];
        if (update == true) {
            lastsig1 = MAIN.lastSignals["BA_SIGNAL_1"];
        }
        var data_points = [{ data: lastsig1, color: '#f3ec1a'}];
        MAIN.graphCache.plot = $.plot(MAIN.graphCache.elem, data_points, options);
        $('.flot-text').css('color', '#aaaaaa');
    }


    //Draw signals
    MAIN.drawSignals = function(SIG) {

        if (MAIN.running == true) {

            var lastsig1 = [];

            // If there is graph on screen
            if (MAIN.graphCache == undefined) {
                MAIN.initPlot(false);
            }


            // Prepare every signal
            MAIN.prepareOneSignal(SIG,'BA_SIGNAL_1');

            lastsig1 = MAIN.lastSignals["BA_SIGNAL_1"];

            MAIN.graphCache.elem.show();
            MAIN.graphCache.plot.resize();
            MAIN.graphCache.plot.setupGrid();
            var data_points = [{ data: lastsig1, color: '#f3ec1a', label: "Amplitude" }];
            MAIN.graphCache.plot.setData(data_points);
            MAIN.graphCache.plot.draw();
            MAIN.updateLinesAndArrows();

            // Reset resize flag
            MAIN.state.resized = false;
        }
    };


    MAIN.enableCursor = function(x) {
        var x2 = (x[1] == '1') ? x[0] + '2' : x[0] + '1';
        var d = (x[1] == '1') ? '1' : '2';
        $('cur_' + x).show();
        $('cur_' + x + '_info').show();
        $('cur_' + x + '_arrow').show();

        if ($('cur_' + x2).is(':visible')) {
            $('cur_' + x[0] + '_diff').show();
            $('cur_' + x[0] + '_diff_info').show();
        }

        CLIENT.params.local['BA_CURSOR_' + x[0].toUpperCase() + '1'] = { value: 1 };
        CLIENT.params.local['BA_CURSOR_' + x[0].toUpperCase() + '2'] = { value: 1 };

        CLIENT.params.local['BA_CUR1_T'] = { value: 1 };
        CLIENT.params.local['BA_CUR2_T'] = { value: 1 };

        if (x[0] == 'x') {
            CURSORS.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, CLIENT.params.local);
            CURSORS.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, CLIENT.params.local);
        }
        if (x[0] == 'y') {
            CURSORS.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, CLIENT.params.local);
            CURSORS.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, CLIENT.params.local);
        }

        CLIENT.params.local = {};
    };

    MAIN.disableCursor = function(x) {
        var d = (x[1] == '1') ? '1' : '2';

        $('cur_' + x).hide();
        $('cur_' + x + '_info').hide();
        $('cur_' + x + '_arrow').hide();
        $('cur_' + x[0] + '_diff').hide();
        $('cur_' + x[0] + '_diff_info').hide();

        CLIENT.params.local['BA_CURSOR_' + x[0].toUpperCase() + '1'] = { value: 0 };
        CLIENT.params.local['BA_CURSOR_' + x[0].toUpperCase() + '2'] = { value: 0 };

        if (x[0] == 'x') {
            CURSORS.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, CLIENT.params.local);
            CURSORS.enableX('BA_CURSOR_' + x[0].toUpperCase() + d, CLIENT.params.local);
        }
        if (x[0] == 'y') {
            CURSORS.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, CLIENT.params.local);
            CURSORS.enableY('BA_CURSOR_' + x[0].toUpperCase() + d, CLIENT.params.local);
        }

        CLIENT.params.local = {};
    };

    MAIN.processSignals = function(SIG){
        MAIN.drawSignals(SIG);
    }

    MAIN.processStatus = function(new_params) {
        var status = new_params['BA_STATUS'].value
        if (status === 0 || status === 6 || status === 7) {
            $('#BA_STOP').hide();
            $('#BA_RUN').show();
            $('body').addClass('loaded');
            $('#calibration').hide();
            $('#measuring-status').hide();
            MAIN.calibrating = false;
            MAIN.running = false;
            CLIENT.parametersCache["BA_STATUS"] = { value: 0 };
            CLIENT.sendParameters();
        }

        if (status === 1 || status === 8) {
            $('#BA_STOP').css('display', 'block');
            $('#BA_RUN').hide();
            MAIN.running = true;
            $('#measuring-status').show();
        }

        if (status === 5) {
        }
    }

    MAIN.calib_stop = function(new_params) {
        if ((new_params['BA_CALIBRATE_START'].value === false) && (MAIN.calibrating == true)) {
            MAIN.calibrating = false;
            $('body').addClass('loaded');
            $('#calibration').hide();
        }
    }

    MAIN.calib_enable = function(new_params) {
        var ba_state = $("#BA_CALIB_STATE");
        if ((new_params['BA_CALIBRATE_ENABLE'].value === false)) {
            if (ba_state.attr('src') !== "img/red_led.png")
                ba_state.attr("src", "img/red_led.png");
        } else {
            if (ba_state.attr('src') !== "img/green_led.png")
                ba_state.attr("src", "img/green_led.png");
        }
    }

    MAIN.change_cur_freq = function(new_params) {

        var val = new_params['BA_CURRENT_FREQ'].value;
        MAIN.cur_freq = val;
        var result = "";

        if (val >= 1000000) {
            val = val / 1000000;
            result = val.toFixed(2) + " MHz";
        } else if (val >= 1000) {
            val = val / 1000;
            result = val.toFixed(2) + " kHz";
        } else {
            result = val + " Hz";
        }

        $('#BA_CUR_FREQ').text(result);
    }

    MAIN.change_cur_step = function(new_params) {
        $('#BA_CUR_STEP').text(new_params['BA_CURRENT_STEP'].value);
    }

    MAIN.updateGraphSize = function() {
        MAIN.graphSize.left = $("#graphs_holder").offset().left + 78;
        MAIN.graphSize.right = MAIN.graphSize.left + $("#graphs_holder").width() - 81;
        MAIN.graphSize.top = $("#graph_bode").offset().top - 1;
        MAIN.graphSize.bottom = MAIN.graphSize.top + $("#graph_bode").height() - 37;
        CURSORS.installCursorsHandlers()
    }

    MAIN.setValue = function(param_name,new_params) {
        var old_params = CLIENT.params.orig;
        if ((!MAIN.state.editing &&
            ((old_params[param_name] !== undefined && old_params[param_name].value !== new_params[param_name].value) ||
            (old_params[param_name] == undefined))
            )) {
            var value = $('#' + param_name).val();
            if (value !== new_params[param_name].value) {
                $('#' + param_name).val(new_params[param_name].value);
            }
        }
    }

    MAIN.startFreq = function(new_params) {
        var param_name = "BA_START_FREQ"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.endFreq = function(new_params) {
        var param_name = "BA_END_FREQ"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setSteps = function(new_params) {
        var param_name = "BA_STEPS"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setScale = function(new_params) {
        var param_name = "BA_SCALE"
        var old_params = CLIENT.params.orig;
        if ((!MAIN.state.editing &&
            ((old_params[param_name] !== undefined && old_params[param_name].value !== new_params[param_name].value) ||
            (old_params[param_name] == undefined))
            )) {
            var radios = $('input[name="' + param_name + '"]');
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
        }
    }

    MAIN.setLogic = function(new_params) {
        var param_name = "BA_LOGIC_MODE"
        var old_params = CLIENT.params.orig;
        if ((!MAIN.state.editing &&
            ((old_params[param_name] !== undefined && old_params[param_name].value !== new_params[param_name].value) ||
            (old_params[param_name] == undefined))
            )) {
            var radios = $('input[name="' + param_name + '"]');
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
        }
    }


    MAIN.setPerNum = function(new_params) {
        var param_name = "BA_PERIODS_NUMBER"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setAverage = function(new_params) {
        var param_name = "BA_AVERAGING"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setOutAmpl = function(new_params) {
        var param_name = "BA_AMPLITUDE"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setDCBias = function(new_params) {
        var param_name = "BA_DC_BIAS"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setGainMin = function(new_params) {
        var param_name = "BA_GAIN_MIN"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setGainMax = function(new_params) {
        var param_name = "BA_GAIN_MAX"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setPhaseMin = function(new_params) {
        var param_name = "BA_PHASE_MIN"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setPhaseMax = function(new_params) {
        var param_name = "BA_PHASE_MAX"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setAutoScale = function(new_params){
        var param_name = "BA_AUTO_SCALE"
        var old_params = CLIENT.params.orig;
        if (((old_params[param_name] !== undefined && old_params[param_name].value !== new_params[param_name].value) ||
            (old_params[param_name] == undefined))
            ) {
                if (new_params[param_name].value){
                    $('#BA_AUTOSCALE_BTN').addClass('active');
                }else{
                    $('#BA_AUTOSCALE_BTN').removeClass('active');
                }
        }
    }




    MAIN.updateLinesAndArrows = function() {
        CURSORS.updateXLinesAndArrows();
        CURSORS.updateYLinesAndArrows();
    }


    MAIN.updateCursors = function() {
        if (MAIN.graphCache === undefined)
            return;
        var plot = MAIN.graphCache.plot;
        var offset = plot.getPlotOffset();
        var left = offset.left + 1 + 'px';
        var right = offset.right + 1 + 'px';
        var top = offset.top + 1 + 'px';
        var bottom = offset.bottom + 9 + 'px';

        //update lines length
        $('.hline').css('left', left);
        $('.hline').css('right', right);
        $('.vline').css('top', top);
        $('.vline').css('bottom', bottom);

        //update arrows positions
        var diff_left = offset.left + 2 + 'px';
        var diff_top = offset.top - 2 + 'px';
        var margin_left = offset.left - 7 - 2 + 'px';
        var margin_top = -7 + offset.top - 2 + 'px';
        var margin_bottom = -2 + offset.bottom + 'px';
        var line_margin_left = offset.left - 2 + 'px';
        var line_margin_top = offset.top - 2 + 'px';
        var line_margin_bottom = offset.bottom - 2 + 'px';

        $('.varrow').css('margin-left', margin_left);
        $('.harrow').css('margin-top', margin_top);
        $('.harrow').css('margin-bottom', margin_bottom);
        $('.vline').css('margin-left', line_margin_left);
        $('.hline').css('margin-top', line_margin_top);
        $('.hline').css('margin-bottom', line_margin_bottom);

        $('#cur_x_diff').css('margin-left', diff_left);
        $('#cur_y_diff').css('margin-top', diff_top);
        $('#cur_z_diff').css('margin-top', diff_top);
        $('#cur_x_diff_info').css('margin-left', diff_left);
        $('#cur_y_diff_info').css('margin-top', diff_top);
        $('#cur_z_diff_info').css('margin-top', diff_top);
    };

    MAIN.modelProcess = function(value) {
        if (value["RP_MODEL_STR"].value === "Z20_250_12") {
            $("#CALIB_BODE_IMG").attr("src", "img/bode_calib_250.png");
        }
        if (value["RP_MODEL_STR"].value === "Z20_250_12_120") {
            $("#CALIB_BODE_IMG").attr("src", "img/bode_calib_250.png");
        }
    }


    MAIN.param_callbacks["BA_STATUS"] = MAIN.processStatus;

    // MAIN.param_callbacks["BA_MEASURE_START"] = MAIN.process_run;
    MAIN.param_callbacks["BA_CALIBRATE_START"] = MAIN.calib_stop;
    MAIN.param_callbacks["BA_CURRENT_FREQ"] = MAIN.change_cur_freq;
    MAIN.param_callbacks["BA_CURRENT_STEP"] = MAIN.change_cur_step;
    MAIN.param_callbacks["BA_CALIBRATE_ENABLE"] = MAIN.calib_enable;
    MAIN.param_callbacks["RP_MODEL_STR"] = MAIN.modelProcess;

    MAIN.param_callbacks["BA_START_FREQ"] = MAIN.startFreq;
    MAIN.param_callbacks["BA_END_FREQ"] = MAIN.endFreq;
    MAIN.param_callbacks["BA_STEPS"] = MAIN.setSteps;
    MAIN.param_callbacks["BA_SCALE"] = MAIN.setScale;
    MAIN.param_callbacks["BA_LOGIC_MODE"] = MAIN.setLogic;

    MAIN.param_callbacks["BA_SCALE"] = MAIN.setPerNum;
    MAIN.param_callbacks["BA_AVERAGING"] = MAIN.setAverage;
    MAIN.param_callbacks["BA_AMPLITUDE"] = MAIN.setOutAmpl;
    MAIN.param_callbacks["BA_DC_BIAS"] = MAIN.setDCBias;

    MAIN.param_callbacks["BA_GAIN_MIN"] = MAIN.setGainMin;
    MAIN.param_callbacks["BA_GAIN_MAX"] = MAIN.setGainMax;
    MAIN.param_callbacks["BA_PHASE_MIN"] = MAIN.setPhaseMin;
    MAIN.param_callbacks["BA_PHASE_MAX"] = MAIN.setPhaseMax;
    MAIN.param_callbacks["BA_AUTO_SCALE"] = MAIN.setAutoScale;


}(window.MAIN = window.MAIN || {}, jQuery));




// Page onload event handler
$(function() {





    // Export
    $('#downl_graph').on('click', function() {
        setTimeout(MAIN.SaveGraphs, 30);
    });

    $('#reset_settings').on('click', function() {
        CLIENT.parametersCache["BA_STATUS"] = { value: 4 };
        CLIENT.sendParameters();
        location.reload();
    });

    $('#downl_csv').on('click', function() {
        MAIN.downloadDataAsCSV("baData.csv");
    });

    // Process clicks on top menu buttons
    //Run button
    $('#BA_RUN').on('click', function(ev) {

        if ($('#BA_AUTOSCALE_BTN').hasClass('active')) {
            $('#BA_GAIN_MIN').val(0);
            $('#BA_GAIN_MAX').val(0);
            $('#BA_PHASE_MAX').val(0);
            $('#BA_PHASE_MIN').val(0);

        }

        MAIN.initPlot(false);
        MAIN.firstValGot.BA_SIGNAL_1 = false;
        MAIN.firstValGot.BA_SIGNAL_2 = false;
        delete MAIN.lastSignals['BA_SIGNAL_1'];
        delete MAIN.lastSignals['BA_SIGNAL_2'];
        ev.preventDefault();
        //$('#BA_RUN').hide();
        //$('#BA_STOP').css('display', 'block');
        //$('#measuring-status').show();
        CLIENT.parametersCache["BA_START_FREQ"] = { value: $("#BA_START_FREQ").val() };
        CLIENT.parametersCache["BA_END_FREQ"] = { value: $('#BA_END_FREQ').val() };
        CLIENT.parametersCache["BA_STEPS"] = { value: $('#BA_STEPS').val() };
        CLIENT.parametersCache["BA_STATUS"] = { value: 1 };
        CLIENT.sendParameters();
        //MAIN.running = true;
        MAIN.curGraphScale = MAIN.scale;
    });

    //Stop button
    $('#BA_STOP').on('click', function(ev) {
        ev.preventDefault();
        CLIENT.parametersCache["BA_STATUS"] = { value: 0 };
        CLIENT.sendParameters();
    });

    //Loader wrapper
    $('#loader-wrapper').on('click', function(ev) {
        ev.preventDefault();
        if (MAIN.calibrating == true) {
            $('body').addClass('loaded');
            $('#calibration').hide();
            CLIENT.parametersCache["BA_STATUS"] = { value: 0 };
            CLIENT.sendParameters();
            MAIN.calibrating = false;
        }
    });




    // Open changing parameters dialogs
    $('.edit-mode').on('click', function() {
        if (MAIN.running)
            return;
        MAIN.state.editing = true;
        $('#right_menu').hide();
        $('#' + $(this).attr('id') + '_dialog').show();
    });

    // Close changing paramerers dialogs
    $('.close-dialog').on('click', function() {
        MAIN.state.editing = false;
        $('.dialog:visible').hide();
        $('#right_menu').show();
    });


    //Draw graph
    MAIN.drawGrid();


    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {
        var divider = 1.6;
        var window_width = window.innerWidth;
        var window_height = window.innerHeight;

        if (window_width > 768 && window_height > 580) {
            var global_width = window_width - 30,
                global_height = global_width / divider;
            if (window_height < global_height) {
                global_height = window_height - 70 * divider;
                global_width = global_height * divider;
            }

            $('#global_container').css('width', global_width);
            $('#global_container').css('height', global_height);


            MAIN.drawGrid();
            var main_width = $('#main').outerWidth(true);
            var main_height = $('#main').outerHeight(true);
            $('#global_container').css('width', main_width);
            $('#global_container').css('height', main_height);

            MAIN.drawGrid();
            main_width = $('#main').outerWidth(true);
            main_height = $('#main').outerHeight(true);
            window_width = window.innerWidth;
            window_height = window.innerHeight;
            if (main_height > (window_height - 80)) {
                $('#global_container').css('height', window_height - 80);
                $('#global_container').css('width', divider * (window_height - 80));
                MAIN.drawGrid();
                $('#global_container').css('width', $('#main').outerWidth(true) - 2);
                $('#global_container').css('height', $('#main').outerHeight(true) - 2);
                MAIN.drawGrid();
            }
        }

        $('#global_container').offset({ left: (window_width - $('#global_container').width()) / 2 });

        // Resize the graph holders
        $('.plot').css($('#graph_bode_grid').css(['height', 'width']));

        // Hide all graphs, they will be shown next time signal data is received
        $('#graph_bode .plot').hide();

        if (MAIN.ws) {
            MAIN.sendParameters();
        }

        MAIN.updateGraphSize();


        // Set the resized flag
        MAIN.state.resized = true;

        if ((MAIN.lastSignals['BA_SIGNAL_1'] != undefined) || (MAIN.lastSignals['BA_SIGNAL_2'] != undefined)) {
            MAIN.initPlot(true);

            setTimeout(function() {
                MAIN.updateLinesAndArrows();
            }, 120);

            MAIN.updateCursors();
        }
    }).resize();




    // Stop the application when page is unloaded
    $(window).on('beforeunload', function() {
        MAIN.ws.onclose = function() {}; // disable onclose handler first
        MAIN.ws.close();
        $.ajax({
            url: MAIN.config.stop_app_url,
            async: false
        });
    });




    //Crash buttons
    $('#send_report_btn').on('click', function() { MAIN.formEmail() });
    $('#restart_app_btn').on('click', function() { location.reload() });

    $('#BA_AUTOSCALE_BTN').click(function() {
        if ($(this).hasClass('active')){
            $(this).removeClass('active');
            CLIENT.parametersCache["BA_AUTO_SCALE"] = { value: false };
            CLIENT.sendParameters();
        }
        else{
            $(this).addClass('active');
            CLIENT.parametersCache["BA_AUTO_SCALE"] = { value: true };
            CLIENT.sendParameters();
        }
    });

    $('#BA_CURSOR_X1').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            MAIN.enableCursor('x1');
        else
            MAIN.disableCursor('x1');
        MAIN.updateLinesAndArrows();
    });

    $('#BA_CURSOR_X2').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            MAIN.enableCursor('x2');
        else
            MAIN.disableCursor('x2');
        MAIN.updateLinesAndArrows();
    });

    $('#BA_CURSOR_Y1').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            MAIN.enableCursor('y1');
        else
            MAIN.disableCursor('y1');
        MAIN.updateLinesAndArrows();
    });

    $('#BA_CURSOR_Y2').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            MAIN.enableCursor('y2');
        else
            MAIN.disableCursor('y2');
        MAIN.updateLinesAndArrows();
    });

    $('#BA_CURSOR_Z1').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            MAIN.enableCursor('z1');
        else
            MAIN.disableCursor('z1');
        MAIN.updateLinesAndArrows();
    });

    $('#BA_CURSOR_Z2').click(function() {
        var btn = $(this);
        if (!btn.hasClass('active'))
            MAIN.enableCursor('z2');
        else
            MAIN.disableCursor('z2');
        MAIN.updateLinesAndArrows();
    });

    $('#BA_CALIBRATE').click(function() {
        $('#calibration_dialog').modal('show');
    });

    // Everything prepared, start application
    CLIENT.startApp();

    MAIN.previousPageUrl = document.referrer;
    console.log(`Previously visited page URL: ${MAIN.previousPageUrl}`);
    const currentUrl = window.location.href;
    if (currentUrl === MAIN.previousPageUrl || MAIN.previousPageUrl === ''){
        MAIN.previousPageUrl = '/'
    }
    $("#back_button").attr("href", MAIN.previousPageUrl)


});