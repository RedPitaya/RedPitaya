/*
 * Red Pitaya Bode analyzer client
 *
 * Author: Dakus <info@eskala.eu>
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
(function(BA, $, undefined) {

    BA.curGraphScale = false;
    BA.param_callbacks = {};

    // App state
    BA.state = {
        socket_opened: false,
        processing: false,
        editing: false,
        trig_dragging: false,
        cursor_dragging: false,
        mouseover: false,
        resized: false,
        graph_grid_height: null,
        graph_grid_width: null,
        demo_label_visible: false,
        cursor_dragging: false
    };

    BA.firstValGot = { 'BA_SIGNAL_1': false, 'BA_SIGNAL_2': false };

    BA.lastSignals = { 'BA_SIGNAL_1': undefined, 'BA_SIGNAL_2': undefined, 'BA_SIGNAL_1_BAD': undefined, 'BA_SIGNAL_2_BAD': undefined };

    // Current step and freq
    BA.currentFreq = 0;
    BA.currentStep = 0;

    //Graph cache
    BA.graphCache = undefined;

    // Other global variables
    BA.ws = null;

    BA.running = false;
    BA.calibrating = false;
    BA.unexpectedClose = false;

    BA.parameterStack = [];
    BA.signalStack = [];

    BA.compressed_data = 0;
    BA.decompressed_data = 0;
    BA.cur_freq = 0;
    BA.input_threshold = 0;

    var g_PacketsRecv = 0;

    BA.cursorsRelative = {
        x1: 0.33,
        x2: 0.66,
        y1: 0.33,
        y2: 0.66,
        z1: 0.33,
        z2: 0.66
    };

    //Write email
    BA.formEmail = function() {
        //var file = new FileReader();
        var mail = "support@redpitaya.com";
        var subject = "Crash report Red Pitaya OS";
        var body = "%0D%0A%0D%0A------------------------------------%0D%0A" + "DEBUG INFO, DO NOT EDIT!%0D%0A" + "------------------------------------%0D%0A%0D%0A";
        body += "Parameters:" + "%0D%0A" + JSON.stringify({ parameters: CLIENT.parametersCache }) + "%0D%0A";
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

    BA.SaveGraphs = function() {
        html2canvas(document.querySelector("body"), {backgroundColor: '#343433'}).then(canvas => {
            var a = document.createElement('a');
            // toDataURL defaults to png, so we need to request a jpeg, then convert for file download.
            a.href = canvas.toDataURL("image/jpeg").replace("image/jpeg", "image/octet-stream");
            a.download = 'graphs.jpg';
            // a.click(); // Not working with firefox
            fireEvent(a, 'click');
        });
    }

    BA.downloadDataAsCSV = function(filename) {
        var header = 'Frequency [Hz], Amplitude [dB], Phase [deg]';
        var row_delim = '\n';
        var strings = header + row_delim;

        var amplitude_signal = BA.lastSignals['BA_SIGNAL_1'];
        var phase_signal = BA.lastSignals['BA_SIGNAL_2'];

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
    BA.setCanvasSize = function() {
        var canvas_width = $('#main').width() - 90;
        var canvas_height = $('#main').height() - 2;// Math.round(canvas_width / 2);

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
    BA.prepareOneSignal = function(signal_name) {
        let signals = Object.assign({}, CLIENT.signalStack[0]);
        for (const property in signals) {
            if (signals[property]['type']){
                if (signals[property]['type'] == 'f'){
                    signals[property]['value'] = CLIENT.base64ToFloatArray(signals[property]['value'] )
                    signals[property]['type'] = ''
                }
                if (signals[property]['type'] == 'i'){
                    signals[property]['value'] = CLIENT.base64ToIntArray(signals[property]['value'] )
                    signals[property]['type'] = ''
                }
            }
        }
        var one_signal = signals[signal_name];
        var bad_signal = signals['BA_BAD_SIGNAL'];
        var signal_param = signals['BA_SIGNAL_PARAMETERS'];

        // Ignore empty signals
        if (one_signal === undefined || one_signal.size == 0)
            return;

        // Create points mas
        var points = [];
        var points_bad = [];
        var startFreq = 0,
            endFreq = 0,
            steps = 0,
            step = 0,
            min_val = undefined,
            max_val = undefined,
            min_all_val = undefined,
            max_all_val = undefined;

        startFreq = signal_param.value[0];
        endFreq = signal_param.value[1];
        steps = signal_param.value[2];
        step = (endFreq - startFreq) / (steps - 1);


        for (var i = 0; i < one_signal.size; i++) {
            var freq = startFreq + i * step;
            var samp_val = one_signal.value[i];
            if (BA.curGraphScale) {
                var a = Math.log10(startFreq);
                var b = Math.log10(endFreq);
                var c = (b - a) / (steps - 1);
                freq = Math.pow(10, c * i + a);
            }
            BA.freq = freq;
            if (i > 0) {
                if (bad_signal.value[i] != bad_signal.value[i - 1]) {
                    var f = (freq + freq_old) / 2.0;
                    var s = (samp_val + one_signal.value[i - 1]) / 2.0;
                    points.push([f, s]);
                    points_bad.push([f, s]);
                    if (bad_signal.value[i] == 0) {
                        points_bad.push([undefined, undefined]);
                    } else {
                        points.push([undefined, undefined]);
                    }
                }
            }

            if ((min_val == undefined || min_val > samp_val) && bad_signal.value[i] == 0) min_val = samp_val;
            if ((max_val == undefined || max_val < samp_val) && bad_signal.value[i] == 0) max_val = samp_val;
            if (min_all_val == undefined || min_all_val > samp_val) min_all_val = samp_val;
            if (max_all_val == undefined || max_all_val < samp_val) max_all_val = samp_val;

            if (bad_signal.value[i] == 0) {
                points.push([freq, samp_val]);
            } else {
                points_bad.push([freq, samp_val]);
            }
            BA.lastSignals[signal_name] = points;
            BA.lastSignals[signal_name + "_BAD"] = points_bad;
            freq_old = freq;
        }



        // AUTOSCALE if switched on
        if ($('#BA_AUTOSCALE_BTN').hasClass('active')) {
            // For autoscale of max Y value
            var min = min_val;
            var max = max_val;
            if ($('#BA_SHOWALL_BTN').hasClass('active')) {
                min = min_all_val;
                max = max_all_val;
            }

            var yaxis;
            var axis_name = ""
            // Choose right yaxis
            if (signal_name == "BA_SIGNAL_1"){
                yaxis = BA.graphCache.plot.getAxes().yaxis;
                axis_name = "BA_GAIN"
            }
            else{
                yaxis = BA.graphCache.plot.getAxes().y2axis;
                axis_name = "BA_PHASE"
            }

            var size = (max - min) * 0.2;
            if (size < 5) size = 5
            $('#' + axis_name + '_MAX').val(max + size);
            yaxis.options.max = max+ size;

            $('#' + axis_name + '_MIN').val(min - size);
            yaxis.options.min = min - size;

            CLIENT.parametersCache["BA_GAIN_MIN"] = { value: $("#BA_GAIN_MIN").val() };
            CLIENT.parametersCache["BA_GAIN_MAX"] = { value: $("#BA_GAIN_MAX").val() };
            CLIENT.parametersCache["BA_PHASE_MIN"] = { value: $("#BA_PHASE_MIN").val() };
            CLIENT.parametersCache["BA_PHASE_MAX"] = { value: $("#BA_PHASE_MAX").val() };
            CLIENT.sendParameters();

        }
    }

    BA.resizeCursorsHolder = function(){
        if (BA.graphCache !== undefined){
            const offset = BA.graphCache.plot.getPlotOffset()
            const w = BA.graphCache.plot.width()
            const h = BA.graphCache.plot.height()
            var ch = $('#cursors_holder');
            ch.css('top', offset.top - 2)
            ch.css('left', offset.left - 2)
            ch.css('width', w + 4)
            ch.css('height', h + 4)   
            ch.show()
        }
    }

    function generateLogarithmicTicks(min, max, minTicks, maxTicks) {
        const ticks = [];
        const startDecade = Math.floor(Math.log10(min));
        const endDecade = Math.floor(Math.log10(max));
        
        // const multipliers = [1, 2, 3, 4 , 5, 6 , 7 , 8 ,9 ];
        const multipliers = [1, 2, 3,  5,  7 , 8.5 ];
        
        for (let decade = startDecade; decade <= endDecade; decade++) {
            for (const mult of multipliers) {
                const freq = mult * Math.pow(10, decade);
                if (freq >= min && freq <= max) {
                    ticks.push(Math.round(freq));
                }
            }
        }
        
        if (!ticks.includes(Math.round((max)))) {
            ticks.push(Math.round(max));
        }
        
        return ticks.sort((a, b) => a - b);
    }

    function generateLogarithmicPower2Ticks(min, max) {
        const result = [];
        
        let current = Math.pow(2, Math.ceil(Math.log2(min)));
        if (current / 2 >= min) {
            current = current / 2;
        }
        
        while (current <= max) {
            if (current >= min) {
                result.push(Math.round(current));
            }
            current *= 2
        }
        
        if (result[0] !== min) result.unshift(Math.round(min));
        if (result[result.length - 1] !== max) result.push(Math.round(max));
        
        return result;
    }

    function generatePowerOf10Scale(min, max) {
        const result = [];
        
        let current = Math.pow(10, Math.ceil(Math.log10(min)));
        if (current / 10 >= min) {
            current = current / 10;
        }
        
        while (current <= max) {
            if (current >= min) {
                result.push(Math.round(current));
            }
            current *= 2.5
        }
        
        if (result[0] !== min) result.unshift(Math.round(min));
        if (result[result.length - 1] !== max) result.push(Math.round(max));
        
        return result;
    }

    function generateRoundedLinearScale(min, max, targetSteps = 10) {
        if (min >= max) throw new Error("The minimum value must be less than the maximum");

        const range = max - min;
        let roughStep = range / targetSteps;
        
        const magnitude = Math.pow(10, Math.floor(Math.log10(roughStep)));
        const normalizedStep = roughStep / magnitude;
        
        let stepMultiplier;
        if (normalizedStep <= 1.5) {
            stepMultiplier = 1;
        } else if (normalizedStep <= 3) {
            stepMultiplier = 2;
        } else {
            stepMultiplier = 5;
        }
        
        const step = stepMultiplier * magnitude;
        
        const start = Math.floor(min / step) * step;
        const end = Math.ceil(max / step) * step;
        
        const result = [];
        for (let value = start; value <= end + Number.EPSILON; value += step) {
            if (value >= min - Number.EPSILON && value <= max + Number.EPSILON) {
                result.push(Math.round(value));
            }
        }
        
        if (result.length === 0 || result[0] > min) result.unshift(Math.round(min));
        if (result[result.length - 1] < max) result.push(Math.round(max));
        
        return result;
    }

    function getXAxisScale() {
        var xMin = CLIENT.getValue("BA_START_FREQ")
        var xMax = CLIENT.getValue("BA_END_FREQ")
        //var steps = CLIENT.getValue("BA_STEPS")
        var t = [];
        const xscale = CLIENT.getValue("BA_X_SCALE")
        if (xscale !== undefined && xMin !== undefined && xMax !== undefined){
            t = generateRoundedLinearScale(xMin,xMax)
            if (xscale === 1){
                t = generateLogarithmicTicks(xMin,xMax)
            }
            if (xscale === 2){
                t = generateLogarithmicPower2Ticks(xMin,xMax)
            }
            if (xscale === 3){
                t = generatePowerOf10Scale(xMin,xMax)
            }
        }
        return t
    }

    BA.initPlot = function(update) {
        delete BA.graphCache;
        $('#bode_plot').remove();

        var yMin1 = CLIENT.getValue("BA_GAIN_MIN")
        var yMax1 = CLIENT.getValue("BA_GAIN_MAX")

        var yMin2 = CLIENT.getValue("BA_PHASE_MIN")
        var yMax2 = CLIENT.getValue("BA_PHASE_MAX")


        BA.graphCache = {};
        BA.graphCache.elem = $('<div id="bode_plot" class="plot" />').css($('#graph_bode_grid').css(['height', 'width'])).appendTo('#graph_bode');
     
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
                },
                {
                    min: yMin2,
                    max: yMax2,
                    labelWidth: 30,
                    alignTicksWithAxis: 2,
                    position: "right"
                }
            ],
            xaxis: {
                color: '#aaaaaa',
                tickColor: '#aaaaaa',
                ticks: getXAxisScale,
                transform: function(v) {
                    if (BA.curGraphScale){
                        return Math.log(v); // move away from zero
                    }
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
            },
            legend: {
                position: "sw",
                backgroundOpacity: 0.15
            }
        };

        var lastsig1 = [];
        var lastsig2 = [];
        var lastsig1_bad = [];
        var lastsig2_bad = [];
        if (update == true) {
            lastsig1 = BA.lastSignals["BA_SIGNAL_1"];
            lastsig2 = BA.lastSignals["BA_SIGNAL_2"];
            lastsig1_bad = BA.lastSignals["BA_SIGNAL_1_BAD"];
            lastsig2_bad = BA.lastSignals["BA_SIGNAL_2_BAD"];
        }else{
            lastsig1.push([$("#BA_START_FREQ").val(),undefined])
            lastsig1.push([$("#BA_END_FREQ").val(),undefined])
        }
        var data_points = [{ data: lastsig1, color: '#f3ec1a', label: "Amplitude" }, { data: lastsig2, color: '#31b44b', label: "Phase", yaxis: 2 }];
        if ($('#BA_SHOWALL_BTN').hasClass('active')) {
            data_points.push({ data: lastsig1_bad, color: '#d26500', label: "Invalid amplitude" });
            data_points.push({ data: lastsig2_bad, color: '#685b00', label: "Invalid phase", yaxis: 2 });
        }
        BA.graphCache.plot = $.plot(BA.graphCache.elem, data_points, options);
        $('.flot-text').css('color', '#aaaaaa');
        BA.graphCache.plot.resize();
        BA.graphCache.plot.setupGrid();
        BA.resizeCursorsHolder()
        BA.updateCursors()
    }

    BA.updateXAxisScale = function(){
        if (BA.graphCache !== undefined) {
            BA.graphCache.plot.resize();
            BA.graphCache.plot.setupGrid();
            BA.graphCache.plot.draw();
            BA.resizeCursorsHolder()
            BA.updateCursors()
            BA.updateLinesAndArrows();
        }
    }

    //Draw signals
    BA.drawSignals = function() {

        if (BA.running == true) {

            var lastsig1 = [];
            var lastsig2 = [];
            var lastsig1_bad = [];
            var lastsig2_bad = [];

            // If there is graph on screen
            if (BA.graphCache == undefined) {
                BA.initPlot(false);
            }

            // Prepare every signal
            BA.prepareOneSignal('BA_SIGNAL_1');
            BA.prepareOneSignal('BA_SIGNAL_2');

            lastsig1 = BA.lastSignals["BA_SIGNAL_1"];
            lastsig2 = BA.lastSignals["BA_SIGNAL_2"];
            lastsig1_bad = BA.lastSignals["BA_SIGNAL_1_BAD"];
            lastsig2_bad = BA.lastSignals["BA_SIGNAL_2_BAD"];

            var data_points = [{ data: lastsig1, color: '#f3ec1a', label: "Amplitude" }, { data: lastsig2, color: '#31b44b', label: "Phase", yaxis: 2 }];
            if ($('#BA_SHOWALL_BTN').hasClass('active')) {
                data_points.push({ data: lastsig1_bad, color: '#d26500', label: "Invalid amplitude" });
                data_points.push({ data: lastsig2_bad, color: '#685b00', label: "Invalid phase", yaxis: 2 });
            }
            BA.graphCache.plot.setData(data_points);
            BA.graphCache.elem.show();
            BA.graphCache.plot.resize();
            BA.graphCache.plot.setupGrid();
            BA.graphCache.plot.draw();            
            BA.updateLinesAndArrows();
            // Reset resize flag
            BA.state.resized = false;
        }else{
            if (BA.graphCache == undefined) {
                BA.initPlot(false);
            }
        }
    };


   
    BA.processStatus = function(new_params) {
        var status = new_params['BA_STATUS'].value
        if (status === 0 || status === 6 || status === 7) {
            $('#BA_STOP').hide();
            $('#BA_RUN').show();
            $('body').addClass('loaded');
            $('#calibration').hide();
            $('#measuring-status').hide();
            BA.calibrating = false;
            BA.running = false;
            const scale = CLIENT.getValue("BA_SCALE")
            const xscale = CLIENT.getValue("BA_X_SCALE")
            if (scale !== undefined && xscale !== undefined){
                if (scale == false && xscale > 0){
                    CLIENT.parametersCache["BA_X_SCALE"] = { value:  0 };
                }
                if (scale == true && xscale === 0){
                    CLIENT.parametersCache["BA_X_SCALE"] = { value:  1 };
                }
            }           	
            CLIENT.parametersCache["BA_STATUS"] = { value: 0 };
            CLIENT.sendParameters();
        }

        if (status === 1 || status === 8) {
            $('#BA_STOP').css('display', 'block');
            $('#BA_RUN').hide();
            BA.running = true;
            $('#measuring-status').show();
        }

        if (status === 5) {
        }
    }

    BA.calib_stop = function(new_params) {
        if ((new_params['BA_CALIBRATE_START'].value === false) && (BA.calibrating == true)) {
            BA.calibrating = false;
            $('body').addClass('loaded');
            $('#calibration').hide();
        }
    }

    BA.calib_enable = function(new_params) {
        var ba_state = $("#BA_CALIB_STATE");
        if ((new_params['BA_CALIBRATE_ENABLE'].value === false)) {
            if (ba_state.attr('src') !== "img/red_led.png")
                ba_state.attr("src", "img/red_led.png");
        } else {
            if (ba_state.attr('src') !== "img/green_led.png")
                ba_state.attr("src", "img/green_led.png");
        }
    }

    BA.change_cur_freq = function(new_params) {

        var val = new_params['BA_CURRENT_FREQ'].value;
        BA.cur_freq = val;
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

    BA.change_cur_step = function(new_params) {
        $('#BA_CUR_STEP').text(new_params['BA_CURRENT_STEP'].value);
    }

    BA.change_input_threshold = function(new_params) {
        var x = parseFloat(BA.input_threshold);
        var y = parseFloat(new_params['BA_INPUT_THRESHOLD'].value);
        if (x !== y) {
            $('#BA_INPUT_THRESHOLD').val(y);
            BA.input_threshold = y;
        }
    }

    BA.setValue = function(param_name,new_params) {
        // var old_params = CLIENT.params.orig;
        if ((!BA.state.editing)) {
            var value = $('#' + param_name).val();
            if (value !== new_params[param_name].value) {
                $('#' + param_name).val(new_params[param_name].value);
            }
        }
    }

    BA.startFreq = function(new_params) {
        var param_name = "BA_START_FREQ"
        BA.setValue(param_name,new_params)
    }

    BA.endFreq = function(new_params) {
        var param_name = "BA_END_FREQ"
        BA.setValue(param_name,new_params)
    }

    BA.setSteps = function(new_params) {
        var param_name = "BA_STEPS"
        BA.setValue(param_name,new_params)
    }

    BA.setScale = function(new_params) {
        var param_name = "BA_SCALE"
        if ((!BA.state.editing)) {
            var radios = $('input[name="' + param_name + '"]');
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
        }
    }

    BA.setLogic = function(new_params) {
        var param_name = "BA_LOGIC_MODE"
        var old_params = CLIENT.params.orig;
        if ((!BA.state.editing)) {
            var radios = $('input[name="' + param_name + '"]');
            radios.closest('.btn-group').children('.btn.active').removeClass('active');
            radios.eq([+new_params[param_name].value]).prop('checked', true).parent().addClass('active');
        }
    }


    BA.setPerNum = function(new_params) {
        var param_name = "BA_PERIODS_NUMBER"
        BA.setValue(param_name,new_params)
    }

    BA.setAverage = function(new_params) {
        var param_name = "BA_AVERAGING"
        BA.setValue(param_name,new_params)
    }

    BA.setOutAmpl = function(new_params) {
        var param_name = "BA_AMPLITUDE"
        BA.setValue(param_name,new_params)
    }

    BA.setDCBias = function(new_params) {
        var param_name = "BA_DC_BIAS"
        BA.setValue(param_name,new_params)
    }

    BA.setInThreshold = function(new_params) {
        var param_name = "BA_INPUT_THRESHOLD"
        BA.setValue(param_name,new_params)
    }

    BA.setGainMin = function(new_params) {
        var param_name = "BA_GAIN_MIN"
        BA.setValue(param_name,new_params)
    }

    BA.setGainMax = function(new_params) {
        var param_name = "BA_GAIN_MAX"
        BA.setValue(param_name,new_params)
    }

    BA.setPhaseMin = function(new_params) {
        var param_name = "BA_PHASE_MIN"
        BA.setValue(param_name,new_params)
    }

    BA.setPhaseMax = function(new_params) {
        var param_name = "BA_PHASE_MAX"
        BA.setValue(param_name,new_params)
    }

    BA.setAutoScale = function(new_params){
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

    BA.setShowAll = function(new_params){
        var param_name = "BA_SHOW_ALL"
        var old_params = CLIENT.params.orig;
        if (((old_params[param_name] !== undefined && old_params[param_name].value !== new_params[param_name].value) ||
            (old_params[param_name] == undefined))
            ) {
                if (new_params[param_name].value){
                    $('#BA_SHOWALL_BTN').addClass('active');
                }else{
                    $('#BA_SHOWALL_BTN').removeClass('active');
                }
        }
    }

   
   
    BA.modelProcess = function(value) {
        var model = value["RP_MODEL_STR"].value
        if (model === "Z20_250_12") {
            $("#CALIB_BODE_IMG").attr("src", "img/bode_calib_250.png");
            $("#BA_IN_GAIN_L").text("1:1");
            $("#BA_IN_GAIN2_L").text("1:20");
            $("#BA_IN_GAIN2_L").text("1:20");
        }
        if (model === "Z20_250_12_120") {
            $("#CALIB_BODE_IMG").attr("src", "img/bode_calib_250.png");
            $("#BA_IN_GAIN_L").text("1:1");
            $("#BA_IN_GAIN2_L").text("1:20");
        }

        if (model != "Z20_250_12" && model != "Z20_250_12_120") {
            var nodes = document.getElementsByClassName("250_12_block");
            [...nodes].forEach((element, index, array) => {
                                    element.parentNode.removeChild(element);
            });
        }
    }

    BA.setGain = function(new_params){
        var param_name = 'BA_IN_GAIN'
        var radios = $('input[name="' + param_name + '"]');
        radios.closest('.btn-group').children('.btn.active').removeClass('active');
        radios.eq([+CLIENT.params.orig[param_name].value]).prop('checked', true).parent().addClass('active');
    }

    BA.setACDC = function(new_params){
        var param_name = 'BA_IN_AC_DC'
        var radios = $('input[name="' + param_name + '"]');
        radios.closest('.btn-group').children('.btn.active').removeClass('active');
        radios.eq([+CLIENT.params.orig[param_name].value]).prop('checked', true).parent().addClass('active');
    }

    BA.setProbe = function(new_params){
        var param_name = 'BA_PROBE'
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(CLIENT.params.orig[param_name].value);
        }
    }

    BA.setXaxisScale = function(new_params){
        var param_name = 'BA_X_SCALE'
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(CLIENT.params.orig[param_name].value);
            BA.updateXAxisScale()
            BA.updateLinesAndArrows()
        }
    }

    BA.setCurEnable = function(new_params, name){
        if (name === "BA_CURSOR_X1"){
            if (new_params[name].value === true){
                BA.enableCursor('x1');
            }else{
                BA.disableCursor('x1');
            }
        }

        if (name === "BA_CURSOR_X2"){
            if (new_params[name].value === true){
                BA.enableCursor('x2');
            }else{
                BA.disableCursor('x2');
            }
        }

        if (name === "BA_CURSOR_Y1"){
            if (new_params[name].value === true){
                BA.enableCursor('y1');
            }else{
                BA.disableCursor('y1');
            }
        }

        if (name === "BA_CURSOR_Y2"){
            if (new_params[name].value === true){
                BA.enableCursor('y2');
            }else{
                BA.disableCursor('y2');
            }
        }

        if (name === "BA_CURSOR_Z1"){
            if (new_params[name].value === true){
                BA.enableCursor('z1');
            }else{
                BA.disableCursor('z1');
            }
        }

        if (name === "BA_CURSOR_Z2"){
            if (new_params[name].value === true){
                BA.enableCursor('z2');
            }else{
                BA.disableCursor('z2');
            }
        }
    }

    BA.setCurPos = function(new_params, name){
        if (name === "BA_CURSOR_X1_POS"){
            BA.cursorsRelative.x1 = new_params[name].value
            BA.updateXLinesPosition()
            BA.updateXLinesAndArrows()
        }

        if (name === "BA_CURSOR_X2_POS"){
            BA.cursorsRelative.x2 = new_params[name].value
            BA.updateXLinesPosition()
            BA.updateXLinesAndArrows()
        }

        if (name === "BA_CURSOR_Y1_POS"){
            BA.cursorsRelative.y1 = new_params[name].value
            BA.updateYLinesPosition()
            BA.updateYLinesAndArrows()
        }

        if (name === "BA_CURSOR_Y2_POS"){
            BA.cursorsRelative.y2 = new_params[name].value
            BA.updateYLinesPosition()
            BA.updateYLinesAndArrows()
        }

        if (name === "BA_CURSOR_Z1_POS"){
            BA.cursorsRelative.z1 = new_params[name].value
            BA.updateZLinesPosition()
            BA.updateZLinesAndArrows()
        }

        if (name === "BA_CURSOR_Z2_POS"){
            BA.cursorsRelative.z2 = new_params[name].value
            BA.updateZLinesPosition()
            BA.updateZLinesAndArrows()
        }
    }

    BA.param_callbacks["BA_STATUS"] = BA.processStatus;

    // BA.param_callbacks["BA_MEASURE_START"] = BA.process_run;
    BA.param_callbacks["BA_CALIBRATE_START"] = BA.calib_stop;
    BA.param_callbacks["BA_CURRENT_FREQ"] = BA.change_cur_freq;
    BA.param_callbacks["BA_CURRENT_STEP"] = BA.change_cur_step;
    BA.param_callbacks["BA_CALIBRATE_ENABLE"] = BA.calib_enable;
    BA.param_callbacks["BA_INPUT_THRESHOLD"] = BA.change_input_threshold;
    BA.param_callbacks["RP_MODEL_STR"] = BA.modelProcess;

    BA.param_callbacks["BA_START_FREQ"] = BA.startFreq;
    BA.param_callbacks["BA_END_FREQ"] = BA.endFreq;
    BA.param_callbacks["BA_STEPS"] = BA.setSteps;
    BA.param_callbacks["BA_SCALE"] = BA.setScale;
    BA.param_callbacks["BA_X_SCALE"] = BA.setXaxisScale;
    BA.param_callbacks["BA_LOGIC_MODE"] = BA.setLogic;

    BA.param_callbacks["BA_PERIODS_NUMBER"] = BA.setPerNum;
    BA.param_callbacks["BA_AVERAGING"] = BA.setAverage;
    BA.param_callbacks["BA_AMPLITUDE"] = BA.setOutAmpl;
    BA.param_callbacks["BA_DC_BIAS"] = BA.setDCBias;
    BA.param_callbacks["BA_INPUT_THRESHOLD"] = BA.setInThreshold;

    BA.param_callbacks["BA_GAIN_MIN"] = BA.setGainMin;
    BA.param_callbacks["BA_GAIN_MAX"] = BA.setGainMax;
    BA.param_callbacks["BA_PHASE_MIN"] = BA.setPhaseMin;
    BA.param_callbacks["BA_PHASE_MAX"] = BA.setPhaseMax;
    BA.param_callbacks["BA_AUTO_SCALE"] = BA.setAutoScale;
    BA.param_callbacks["BA_SHOW_ALL"] = BA.setShowAll;


    BA.param_callbacks["BA_IN_GAIN"] = BA.setGain;
    BA.param_callbacks["BA_IN_AC_DC"] = BA.setACDC;
    BA.param_callbacks["BA_PROBE"] = BA.setProbe;

    BA.param_callbacks["BA_CURSOR_X1"] = BA.setCurEnable;
    BA.param_callbacks["BA_CURSOR_X2"] = BA.setCurEnable;
    BA.param_callbacks["BA_CURSOR_Y1"] = BA.setCurEnable;
    BA.param_callbacks["BA_CURSOR_Y2"] = BA.setCurEnable;
    BA.param_callbacks["BA_CURSOR_Z1"] = BA.setCurEnable;
    BA.param_callbacks["BA_CURSOR_Z2"] = BA.setCurEnable;

    BA.param_callbacks["BA_CURSOR_X1_POS"] = BA.setCurPos;
    BA.param_callbacks["BA_CURSOR_X2_POS"] = BA.setCurPos;
    BA.param_callbacks["BA_CURSOR_Y1_POS"] = BA.setCurPos;
    BA.param_callbacks["BA_CURSOR_Y2_POS"] = BA.setCurPos;
    BA.param_callbacks["BA_CURSOR_Z1_POS"] = BA.setCurPos;
    BA.param_callbacks["BA_CURSOR_Z2_POS"] = BA.setCurPos;


}(window.BA = window.BA || {}, jQuery));




// Page onload event handler
$(function() {

    var reloaded = $.cookie("ba_forced_reload");
    if (reloaded == undefined || reloaded == "false") {
        $.cookie("ba_forced_reload", "true");
        window.location.reload(true);
    }

   
    // Export
    $('#downl_graph').on('click', function() {
        setTimeout(BA.SaveGraphs, 30);
    });

    $('#reset_settings').on('click', function() {
        CLIENT.parametersCache["BA_STATUS"] = { value: 4 };
        CLIENT.sendParameters();
        location.reload();
    });

    $('#downl_csv').on('click', function() {
        BA.downloadDataAsCSV("baData.csv");
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

        CLIENT.parametersCache["BA_START_FREQ"] = { value: $("#BA_START_FREQ").val() };
        CLIENT.parametersCache["BA_END_FREQ"] = { value: $('#BA_END_FREQ').val() };
        CLIENT.parametersCache["BA_STEPS"] = { value: $('#BA_STEPS').val() };
        CLIENT.parametersCache["BA_STATUS"] = { value: 1 };
        CLIENT.sendParameters();
        BA.curGraphScale = CLIENT.getValue("BA_SCALE");

        BA.firstValGot.BA_SIGNAL_1 = false;
        BA.firstValGot.BA_SIGNAL_2 = false;
        delete BA.lastSignals['BA_SIGNAL_1'];
        delete BA.lastSignals['BA_SIGNAL_2'];
        ev.preventDefault();
;
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
        if (BA.calibrating == true) {
            $('body').addClass('loaded');
            $('#calibration').hide();
            CLIENT.parametersCache["BA_STATUS"] = { value: 0 };
            CLIENT.sendParameters();
            BA.calibrating = false;
        }
    });




    // Open changing parameters dialogs
    $('.edit-mode').on('click', function() {
        if (BA.running)
            return;
        BA.state.editing = true;
        $('#right_menu').hide();
        $('#' + $(this).attr('id') + '_dialog').show();
    });

    // Close changing paramerers dialogs
    $('.close-dialog').on('click', function() {
        BA.state.editing = false;
        $('.dialog:visible').hide();
        $('#right_menu').show();
    });


    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {

        if ($('#global_container').length === 0) return
        if ($('#main').length === 0) return

        var window_width = window.innerWidth;
        var window_height = window.innerHeight;

        var global_width = window_width - 30,
            global_height = window_height - 200;


        $('#global_container').css('width', global_width);
        $('#global_container').css('height', global_height);

        $('#main').css('width', (global_width - 170));
        $('#main').css('height', global_height);

        $('#global_container').offset({ left: (window_width - $('#global_container').width()) / 2 });

        // Resize the graph holders
        $('.plot').css($('#graph_bode_grid').css(['height', 'width']));

        // Hide all graphs, they will be shown next time signal data is received
        $('#graph_bode .plot').hide();
        
        BA.setCanvasSize()
        const n_update = (BA.lastSignals['BA_SIGNAL_1'] != undefined) || (BA.lastSignals['BA_SIGNAL_2'] != undefined)
        BA.initPlot(n_update)
        BA.updateLinesAndArrows();
        
        // Set the resized flag
        BA.state.resized = true;


    }).resize();


    //Crash buttons
    $('#send_report_btn').on('click', function() { BA.formEmail() });
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

    $('#BA_SHOWALL_BTN').click(function() {
        if ($(this).hasClass('active')){
            $(this).removeClass('active');
            CLIENT.parametersCache["BA_SHOW_ALL"] = { value: false };
            CLIENT.sendParameters();
        }
        else{
            $(this).addClass('active');
            CLIENT.parametersCache["BA_SHOW_ALL"] = { value: true };
            CLIENT.sendParameters();
        }
    });

   

    // Init help
    Help.init(helpListBA);
    Help.setState("idle");

    // Everything prepared, start application
    BA.previousPageUrl = document.referrer;
    console.log(`Previously visited page URL: ${BA.previousPageUrl}`);
    $("#back_button").attr("href", BA.previousPageUrl)

});