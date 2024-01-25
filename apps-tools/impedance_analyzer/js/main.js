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
    MAIN.param_callbacks = {};

    // App state
    MAIN.state = {
        editing: false,
        trig_dragging: false,
        resized: false,
        graph_grid_height: null,
        graph_grid_width: null
    };

    MAIN.lastSignals = { };

    // Current step and freq
    MAIN.currentFreq = 0;
    MAIN.currentStep = 0;
    MAIN.max_adc_rate = 0;
    //Graph cache
    MAIN.graphCache = undefined;

    MAIN.running = true;
    MAIN.calibrating = false;
    MAIN.unexpectedClose = false;

    MAIN.cur_freq = 0;
    MAIN.input_threshold = 0;
    MAIN.currentYAxis = 1;
    MAIN.yAxisSuffix = undefined;


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
        var header = 'Freq [Hz]';
        var row_delim = '\n';
        var delim = ',';
        var coff = {};
        for (var i = 1 ; i <= 15; i++){
            var hs = MAIN.getCurrentSignalSettingsVal(i);
            var sigVal = MAIN.lastSignals[hs.name];
            var min_y = Math.min(...sigVal.value)
            var max_y = Math.max(...sigVal.value)
            if (hs.scale)
                suff = getSuffix(Math.abs((max_y - min_y) / 2.0 + min_y))
            else{
                suff = getSuffix(Math.pow(10,14))
            }
            coff[i] = suff;
        }
        for (var i = 1 ; i <= 15; i++){
            var hs = MAIN.getCurrentSignalSettingsVal(i);
            var suff = coff[i]
            header += delim + hs.label
            header += hs.nominal !== "" ? " [" + suff.name + hs.nominal + "]" : ""
            header += suff.value !== "" ? " " + suff.value : ""
        }
        header += row_delim;

        var strings = header;
        var freq = MAIN.lastSignals["IA_SIGNAL_FREQ"];
        for (var i = 0 ; i < freq.size; i++){
            strings += freq.value[i];
            for (var z = 1 ; z <= 15; z++){
                var hs = MAIN.getCurrentSignalSettingsVal(z);
                var sigVal = MAIN.lastSignals[hs.name];
                var suff = coff[z];
                strings += "," +  (sigVal.value[i] / suff.coff).toFixed(3);
            }
            strings += row_delim;
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

    MAIN.getCurrentSignalSettings = function() {
        return MAIN.getCurrentSignalSettingsVal(MAIN.currentYAxis)
    }

    MAIN.getCurrentSignalSettingsVal = function(X) {
        switch(X){
            case 1:
                return {name: "IA_SIGNAL_Z", label: "|Z|", nominal: "Ohm", scale: true}
            case 2:
                return {name: "IA_SIGNAL_PHASE", label: "P", nominal: "Deg" , scale: false}
            case 3:
                return {name: "IA_SIGNAL_Y", label: "|Y|", nominal: "S" , scale: true}
            case 4:
                return {name: "IA_SIGNAL_NEG_PHASE", label: "-P", nominal: "Deg" , scale: false}
            case 5:
                return {name: "IA_SIGNAL_R_s", label: "Rs", nominal: "Ohm" , scale: true}
            case 6:
                return {name: "IA_SIGNAL_R_p", label: "Rp", nominal: "Ohm" , scale: true}
            case 7:
                return {name: "IA_SIGNAL_X_s", label: "Xs", nominal: "Ohm" , scale: true}
            case 8:
                return {name: "IA_SIGNAL_G_p", label: "Gp", nominal: "S" , scale: true}
            case 9:
                return {name: "IA_SIGNAL_B_p", label: "Bp", nominal: "S" , scale: true}
            case 10:
                return {name: "IA_SIGNAL_C_s", label: "Cs", nominal: "F" , scale: true}
            case 11:
                return {name: "IA_SIGNAL_C_p", label: "Cp", nominal: "F" , scale: true}
            case 12:
                return {name: "IA_SIGNAL_L_s", label: "Ls", nominal: "H" , scale: true}
            case 13:
                return {name: "IA_SIGNAL_L_p", label: "Lp", nominal: "H" , scale: true}
            case 14:
                return {name: "IA_SIGNAL_Q", label: "Q", nominal: "" , scale: true}
            case 15:
                return {name: "IA_SIGNAL_D", label: "D", nominal: "", scale: true}
        }
        return {name: "", label: "undefined", nominal: ""}
    }



    function getSuffix(value) {
        if (value < Math.pow(10,3)) return {name: "p" , value : "10⁻¹²", coff: 1}
        if (value < Math.pow(10,6)) return {name: "n" , value : "10⁻⁹" , coff: Math.pow(10,3)}
        if (value < Math.pow(10,9)) return {name: "µ" , value : "10⁻⁶" , coff: Math.pow(10,6)}
        if (value < Math.pow(10,12)) return {name: "m" , value : "10⁻³" , coff: Math.pow(10,9)}
        if (value < Math.pow(10,15)) return {name: "" , value : "" , coff: Math.pow(10,12)}
        if (value < Math.pow(10,18)) return {name: "k" , value : "10³", coff: Math.pow(10,15)}
        if (value < Math.pow(10,21)) return {name: "M" , value : "10⁶" , coff: Math.pow(10,18)}
        if (value < Math.pow(10,23)) return {name: "G" , value : "10⁹" , coff: Math.pow(10,21)}
        return {name: "" , value : ""}
    }



    function funcxTickFormat(val, axis) {

        if (Math.abs(val) >= 0 && Math.abs(val) < 1000)
            return (val * 1).toFixed(0) + " ";
        else if (Math.abs(val) >= 1000 && Math.abs(val) < 1000000)
            return (val / 1000).toFixed(2) + "k";
        else if (Math.abs(val) >= 1000000)
            return (val / 1000000).toFixed(2) + "M";
    }


    MAIN.getPlot = function() {

        if (MAIN.graphCache && MAIN.graphCache.elem) {
            var plot = MAIN.graphCache.plot;
            return plot;
        }
        return undefined;
    };


    MAIN.initPlot = function(update) {
        delete MAIN.graphCache;
        $('#bode_plot').remove();


        var yMin1 = -5;
        var yMax1 = 5;

        MAIN.graphCache = {};
        MAIN.graphCache.elem = $('<div id="bode_plot" class="plot" />').css($('#graph_bode_grid').css(['height', 'width'])).appendTo('#graph_bode');

        var t = [];
        if (MAIN.scale)
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


        if ($("#IA_SCALE_PLOT0").hasClass("active")) {
            options.xaxis.transform = null;
        }

        var lastsig1 = [];
        if (update !== true) {
            lastsig1.push([$("#IA_START_FREQ").val(),undefined])
            lastsig1.push([$("#IA_END_FREQ").val(),undefined])
        }
        var data_points = [{ data: lastsig1, color: '#f3ec1a'}];
        MAIN.graphCache.plot = $.plot(MAIN.graphCache.elem, data_points, options);
        $('.flot-text').css('color', '#aaaaaa');
    }


    //Draw signals
    MAIN.drawSignals = function(SIG) {
        // console.log(SIG)
        MAIN.lastSignals = SIG;
        MAIN.updatePlot();
    };

    MAIN.updatePlot = function(){

        // If there is graph on screen
        if (MAIN.graphCache == undefined) {
            MAIN.initPlot(false);
        }

        var lastsig1 = [];
        var curSig = MAIN.getCurrentSignalSettings();
        var freqSig = MAIN.lastSignals["IA_SIGNAL_FREQ"];
        var sigVal = MAIN.lastSignals[curSig.name];
        var min_y = 0;
        var max_y = 0;
        var suff = undefined;
        if (freqSig !== undefined && sigVal !== undefined && freqSig.size > 0){
            min_y = Math.min(...sigVal.value)
            max_y = Math.max(...sigVal.value)
            if (curSig.scale)
                suff = getSuffix(Math.abs((max_y - min_y) / 2.0 + min_y))
            else{
                suff = getSuffix(Math.pow(10,14))
            }
            for(var i = 0; i < freqSig.size; i++){
                lastsig1.push([freqSig.value[i], sigVal.value[i] / suff.coff])
            }
            min_y = min_y / suff.coff
            max_y = max_y / suff.coff
            var lab = curSig.label
            lab += curSig.nominal !== "" ? " [" + suff.name + curSig.nominal + "] " + suff.value :  " " + suff.value
            $("#amplitude-info").text(lab)
            MAIN.yAxisSuffix = suff

        }else{
            lastsig1.push([$("#IA_START_FREQ").val(),undefined])
            lastsig1.push([$("#IA_END_FREQ").val(),undefined])
        }

        MAIN.graphCache.elem.show();
        MAIN.graphCache.plot.resize();
        MAIN.graphCache.plot.setupGrid();
        var data_points = [{ data: lastsig1, color: '#f3ec1a' }];
        MAIN.graphCache.plot.setData(data_points);
        MAIN.graphCache.plot.draw();
        var yaxis = MAIN.graphCache.plot.getAxes().yaxis;

        var size = (max_y - min_y) * 0.2;
        // if (size < 5) size = 5
        yaxis.options.max = max_y + size;
        yaxis.options.min = min_y - size;

        MAIN.updateLinesAndArrows();
    };

    MAIN.processSignals = function(SIG){
        MAIN.drawSignals(SIG);
    }

    MAIN.processStatus = function(new_params) {
        var status = new_params['IA_STATUS'].value
        // Stopped
        if (status === 0 || status === 3 || status === 4) {
            $('#IA_STOP').hide();
            $('#IA_RUN').show();
            $('body').addClass('loaded');
            $('#measuring-status').hide();
            MAIN.running = false;
            CLIENT.parametersCache["IA_STATUS"] = { value: 0 };
            CLIENT.sendParameters();
        }

        // Runned
        if (status === 1 || status === 5) {
            $('#IA_STOP').css('display', 'block');
            $('#IA_RUN').hide();
            MAIN.running = true;
            $('#measuring-status').show();
        }

    }


    MAIN.change_cur_freq = function(new_params) {

        var val = new_params['IA_CURRENT_FREQ'].value;
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

        $('#IA_CUR_FREQ').text(result);
    }

    MAIN.change_cur_step = function(new_params) {
        $('#IA_CUR_STEP').text(new_params['IA_CURRENT_STEP'].value);
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
        var param_name = "IA_START_FREQ"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.endFreq = function(new_params) {
        var param_name = "IA_END_FREQ"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setSteps = function(new_params) {
        var param_name = "IA_STEPS"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setScale = function(new_params) {
        var param_name = "IA_SCALE"
        var radios = $('input[name="' + param_name + '"]');
        radios.closest('.btn-group').children('.btn.active').removeClass('active');
        radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
    }

    MAIN.setScalePlot = function(new_params) {
        var param_name = "IA_SCALE_PLOT"
        var radios = $('input[name="' + param_name + '"]');
        radios.closest('.btn-group').children('.btn.active').removeClass('active');
        radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
        MAIN.scale = new_params[param_name].value
        MAIN.initPlot(true);
    }

    MAIN.setAverage = function(new_params) {
        var param_name = "IA_AVERAGING"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setOutAmpl = function(new_params) {
        var param_name = "IA_AMPLITUDE"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setDCBias = function(new_params) {
        var param_name = "IA_DC_BIAS"
        MAIN.setValue(param_name,new_params)
    }

    MAIN.setMaxADC = function(new_params) {
        var param_name = "RP_MAX_ADC"
        MAIN.max_adc_rate  = new_params[param_name].value;
    }

    MAIN.setLcrShunt = function(new_params) {
        var param_name = "IA_LCR_SHUNT"
        $("#"+param_name).val(new_params[param_name].value);
    }

    MAIN.setYAxis = function(new_params) {
        var param_name = "IA_Y_AXIS"
        $("#"+param_name).val(new_params[param_name].value);
        MAIN.currentYAxis = new_params[param_name].value;
        MAIN.updatePlot();
        var c = MAIN.getCurrentSignalSettings()
        $("#amplitude-info").text(c.label)
    }

    MAIN.setShunt = function(new_params) {
        var param_name = "IA_SHUNT"
        $("#"+param_name).val(new_params[param_name].value);
    }


    MAIN.setLCRExt = function(new_params) {
        var lcr_state = $("#IA_LCR_EXT");
        if ((new_params['IA_LCR_EXT'].value === false)) {
            if (lcr_state.attr('src') !== "img/red_led.png")
                lcr_state.attr("src", "img/red_led.png");
            $("#LCR_SHUNT_BOX").hide()
            $("#SHUNT_BOX").show()
        } else {
            if (lcr_state.attr('src') !== "img/green_led.png")
                lcr_state.attr("src", "img/green_led.png");
            $("#LCR_SHUNT_BOX").show()
            $("#SHUNT_BOX").hide()
        }
    }


    MAIN.updateLinesAndArrows = function() {
        CURSORS.updateXLinesAndArrows();
        CURSORS.updateYLinesAndArrows();
    }

    MAIN.modelProcess = function(value) {
        if (value["RP_MODEL_STR"].value === "Z20_250_12") {
            $("#CALIB_BODE_IMG").attr("src", "img/bode_calib_250.png");
        }
        if (value["RP_MODEL_STR"].value === "Z20_250_12_120") {
            $("#CALIB_BODE_IMG").attr("src", "img/bode_calib_250.png");
        }
    }


    MAIN.param_callbacks["IA_STATUS"] = MAIN.processStatus;

    // MAIN.param_callbacks["BA_MEASURE_START"] = MAIN.process_run;
    MAIN.param_callbacks["IA_CURRENT_FREQ"] = MAIN.change_cur_freq;
    MAIN.param_callbacks["IA_CURRENT_STEP"] = MAIN.change_cur_step;
    MAIN.param_callbacks["RP_MODEL_STR"] = MAIN.modelProcess;

    MAIN.param_callbacks["IA_START_FREQ"] = MAIN.startFreq;
    MAIN.param_callbacks["IA_END_FREQ"] = MAIN.endFreq;
    MAIN.param_callbacks["IA_STEPS"] = MAIN.setSteps;
    MAIN.param_callbacks["IA_SCALE"] = MAIN.setScale;
    MAIN.param_callbacks["IA_SCALE_PLOT"] = MAIN.setScalePlot;
    MAIN.param_callbacks["IA_Y_AXIS"] = MAIN.setYAxis;

    MAIN.param_callbacks["IA_LCR_SHUNT"] = MAIN.setLcrShunt;
    MAIN.param_callbacks["IA_SHUNT"] = MAIN.setShunt;

    MAIN.param_callbacks["IA_AVERAGING"] = MAIN.setAverage;
    MAIN.param_callbacks["IA_AMPLITUDE"] = MAIN.setOutAmpl;
    MAIN.param_callbacks["IA_DC_BIAS"] = MAIN.setDCBias;

    MAIN.param_callbacks["IA_CURSOR_X1_ENABLE"] = CURSORS.enableCursorX1Callback;
    MAIN.param_callbacks["IA_CURSOR_X2_ENABLE"] = CURSORS.enableCursorX2Callback;
    MAIN.param_callbacks["IA_CURSOR_Y1_ENABLE"] = CURSORS.enableCursorY1Callback;
    MAIN.param_callbacks["IA_CURSOR_Y2_ENABLE"] = CURSORS.enableCursorY2Callback;

    MAIN.param_callbacks["IA_CURSOR_X1"] = CURSORS.cursorX1Callback;
    MAIN.param_callbacks["IA_CURSOR_X2"] = CURSORS.cursorX2Callback;
    MAIN.param_callbacks["IA_CURSOR_Y1"] = CURSORS.cursorY1Callback;
    MAIN.param_callbacks["IA_CURSOR_Y2"] = CURSORS.cursorY2Callback;

    MAIN.param_callbacks["IA_LCR_EXT"] = MAIN.setLCRExt;
    MAIN.param_callbacks["RP_MAX_ADC"] = MAIN.setMaxADC;



}(window.MAIN = window.MAIN || {}, jQuery));




// Page onload event handler
$(function() {


    // Export
    $('#downl_graph').on('click', function() {
        setTimeout(MAIN.SaveGraphs, 30);
    });

    $('#reset_settings').on('click', function() {
        CLIENT.parametersCache["IA_STATUS"] = { value: 2 };
        CLIENT.sendParameters();
        location.reload();
    });

    $('#downl_csv').on('click', function() {
        MAIN.downloadDataAsCSV("iaData.csv");
    });

    // Process clicks on top menu buttons
    //Run button
    $('#IA_RUN').on('click', function(ev) {

        MAIN.initPlot(false);

        ev.preventDefault();
        //$('#BA_RUN').hide();
        //$('#BA_STOP').css('display', 'block');
        //$('#measuring-status').show();
        CLIENT.parametersCache["IA_STATUS"] = { value: 1 };
        CLIENT.sendParameters();
    });

    //Stop button
    $('#IA_STOP').on('click', function(ev) {
        ev.preventDefault();
        CLIENT.parametersCache["IA_STATUS"] = { value: 0 };
        CLIENT.sendParameters();
    });

    //Loader wrapper
    $('#loader-wrapper').on('click', function(ev) {
        ev.preventDefault();
        if (MAIN.calibrating == true) {
            $('body').addClass('loaded');
            $('#calibration').hide();
            CLIENT.parametersCache["IA_STATUS"] = { value: 0 };
            CLIENT.sendParameters();
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

        // Set the resized flag
        MAIN.state.resized = true;

        MAIN.initPlot(true);
        MAIN.updateLinesAndArrows();

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