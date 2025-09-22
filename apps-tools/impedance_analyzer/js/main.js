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

    MAIN.running = false;
    MAIN.calibrating = false;
    MAIN.unexpectedClose = false;

    MAIN.cur_freq = 0;
    MAIN.input_threshold = 0;
    MAIN.currentYAxis = 1;
    MAIN.yAxisSuffix = undefined;
    MAIN.plotSize = {
        x_min: undefined,
        x_max: undefined,
        y_min: undefined,
        y_max: undefined,
    }

    MAIN.move_mode = undefined;
    MAIN.rect_mode = undefined;
    MAIN.rect_mode_last = undefined;
    MAIN.x_axis_mode = 0;
    MAIN.y_axis_mode = 0;
    MAIN.x_axis_min = undefined;
    MAIN.x_axis_max = undefined;
    MAIN.x_axis_min_full = undefined;
    MAIN.x_axis_max_full = undefined;
    MAIN.zoom_used_x = false;
    MAIN.zoom_used_y = false;



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
        html2canvas(document.querySelector("body"), {backgroundColor: '#343433'}).then(canvas => {
            var a = document.createElement('a');
            // toDataURL defaults to png, so we need to request a jpeg, then convert for file download.
            a.href = canvas.toDataURL("image/jpeg").replace("image/jpeg", "image/octet-stream");
            a.download = 'graphs.jpg';
            // a.click(); // Not working with firefox
            fireEvent(a, 'click');
        });
    }

    MAIN.downloadDataAsCSV = function(filename) {
        var header = 'Freq [Hz]';
        var row_delim = '\n';
        var delim = ',';
        var coff = {};
        for (var i = 1 ; i <= 16; i++){
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
        for (var i = 1 ; i <= 16; i++){
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
            for (var z = 1 ; z <= 16; z++){
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
            case 16:
                return {name: "IA_SIGNAL_P_p", label: "IN2/p2p", nominal: "V", scale: false}
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

    MAIN.resizeCursorsHolder = function(){
        if (MAIN.graphCache !== undefined){
            const offset = MAIN.graphCache.plot.getPlotOffset()
            const w = MAIN.graphCache.plot.width()
            const h = MAIN.graphCache.plot.height()
            var ch = $('#cursors_holder');
            ch.css('top', offset.top - 2)
            ch.css('left', offset.left - 2)
            ch.css('width', w + 4)
            ch.css('height', h + 4)
            ch.show()
        }
    }

    MAIN.updateXAxisScale = function(){
        if (MAIN.graphCache !== undefined) {
            MAIN.graphCache.plot.resize();
            MAIN.graphCache.plot.setupGrid();
            MAIN.graphCache.plot.draw();
            MAIN.resizeCursorsHolder()
            CURSORS.updateCursors()
            CURSORS.updateLinesAndArrows();
            MAIN.drawInfoValues()
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
        var xMin = CLIENT.getValue("IA_START_FREQ")
        var xMax = CLIENT.getValue("IA_END_FREQ")
        var t = [];
        const xscale = CLIENT.getValue("IA_X_SCALE")
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

    MAIN.getPlot = function() {

        if (MAIN.graphCache && MAIN.graphCache.elem) {
            var plot = MAIN.graphCache.plot;
            return plot;
        }
        return undefined;
    };

    MAIN.updatePlotSize = function() {
        const p = MAIN.getPlot()
        if (p != undefined){
            MAIN.plotSize.x_min = p.getAxes().xaxis.min
            MAIN.plotSize.x_max = p.getAxes().xaxis.max
            MAIN.plotSize.y_min = p.getAxes().yaxis.options.min
            MAIN.plotSize.y_max = p.getAxes().yaxis.options.max
        }
    }

     MAIN.getPoltRect = function(){
        var plot = MAIN.getPlot();
        if (!plot) {
            return {l:0,t:0,w:0,h:0};
        }
        var gPosition = $('#graph_bode_grid').offset();
        var gLeft = gPosition.left;
        var gTop = gPosition.top;
        var gWidth = $('#graph_bode_grid').width();
        var gHeight = $('#graph_bode_grid').height();
        var plotOffset = plot.getPlotOffset();

        gLeft += plotOffset.left
        gTop += plotOffset.top
        gWidth = gWidth - plotOffset.left - plotOffset.right
        gHeight = gHeight - plotOffset.top - plotOffset.bottom
        return {l:gLeft,t:gTop,w:gWidth,h:gHeight}
    }

    MAIN.boundCursor = function(rect,pos){
        if (pos.x < rect.l){
            pos.x = rect.l
        }
        if (pos.x > (rect.l + rect.w)){
            pos.x = (rect.l + rect.w)
        }
        if (pos.y < rect.t){
            pos.y = rect.t
        }
        if (pos.y > (rect.t + rect.h)){
            pos.y = (rect.t + rect.h)
        }
        return pos
    }

    MAIN.convertLog = function(v){
        if (MAIN.x_axis_min === 0) return v;
         var plot = MAIN.getPlot();
        if (!plot) {
            return v;
        }
        var options = plot.getAxes();
        var a = options.xaxis.datamin;
        var b = options.xaxis.datamax;
        v = v > b ? b : v
        var x = Math.log10(b/a)/(b-a);
        var y = b / Math.pow(10,x * b);
        v =  y * Math.pow(10, x * v);
        return v;
    }


    MAIN.convertLogMinMax = function(v,min,max){
        var a = min;
        var b = max;
        v = v > b ? b : v
        var x = Math.log10(b/a)/(b-a);
        var y = b / Math.pow(10,x * b);
        v =  y * Math.pow(10, x * v);
        return v;
    }

    MAIN.getLogValue = function(minVal, maxVal, steps, stepIndex) {
        if (minVal <= 0 || maxVal <= 0) {
            throw new Error("Error values");
        }

        const logMin = Math.log10(minVal);
        const logMax = Math.log10(maxVal);
        const stepSize = (logMax - logMin) / steps;

        const logVal = logMin + stepIndex * stepSize;
        return Math.pow(10, logVal);
    }


    MAIN.setMouseZoom = function(p1,p2,rect) {
        var plot = MAIN.getPlot();
        if (!plot) {
            return;
        }
        var min_x = Math.min(p1.x,p2.x);
        var max_x = Math.max(p1.x,p2.x);
        var min_y = Math.min(p1.y,p2.y);
        var max_y = Math.max(p1.y,p2.y);
        var options = plot.getAxes();
        var range_x   = options.xaxis.max - options.xaxis.min;
        var range_y   = options.yaxis.max - options.yaxis.min;

        var new_y_axis_min = (1 - min_y / rect.h) * range_y + options.yaxis.min;
        var new_y_axis_max = (1 - max_y / rect.h) * range_y + options.yaxis.min;
        var new_x_axis_min = min_x / rect.w * range_x + options.xaxis.min;
        var new_x_axis_max = max_x / rect.w * range_x + options.xaxis.min;
        if (MAIN.scale) {
            new_x_axis_min = MAIN.convertLog(new_x_axis_min);
            new_x_axis_max = MAIN.convertLog(new_x_axis_max);
        }
        console.log("new_y_axis_min " + new_y_axis_min + " new_y_axis_max " +new_y_axis_max )
        console.log("new_x_axis_min " + new_x_axis_min + " new_x_axis_max " +new_x_axis_max )

        var curr_options = plot.getOptions();
        curr_options.xaxes[0].min = new_x_axis_min;
        curr_options.xaxes[0].max = new_x_axis_max;
        curr_options.yaxes[0].min = new_y_axis_max;
        curr_options.yaxes[0].max = new_y_axis_min;

        MAIN.zoom_used_x = true;
        MAIN.zoom_used_y = true;
        plot.setupGrid();
        plot.draw();
        CURSORS.updateLinesAndArrows()
        MAIN.drawInfoValues()
    }

    MAIN.changeY = function(value) {
        var plot = MAIN.getPlot();
        if (!plot) {
            return;
        }

        var options = plot.getOptions();
        var yaxis = plot.getAxes().yaxis;

        var y_val_min = yaxis.options.min - value
        var y_val_max = yaxis.options.max - value

        if (y_val_min < MAIN.plotSize.y_min) {
            y_val_min = MAIN.plotSize.y_min
            y_val_max = yaxis.options.max
        }

        if (y_val_max > MAIN.plotSize.y_max) {
            y_val_min = yaxis.options.min
            y_val_max = MAIN.plotSize.y_max
        }


        options.yaxes[0].min = y_val_min;
        options.yaxes[0].max = y_val_max;

        plot.setupGrid();
        plot.draw();
        CURSORS.updateLinesAndArrows()
        MAIN.drawInfoValues()
    };

    MAIN.changeYZoom = function(direction) {
        var plot = MAIN.getPlot();
        if (!plot) {
            return;
        }

        if (direction == undefined) return;

        var options = plot.getOptions();
        var yaxis = plot.getAxes().yaxis;

        var size = (yaxis.options.max - yaxis.options.min)
        if (isNaN(size)) return

        size = size * (direction == "+" ? 0.8 : 1.2) - size

        var y_val_min = yaxis.options.min - size
        var y_val_max = yaxis.options.max + size

        if (y_val_min < MAIN.plotSize.y_min) {
            y_val_min = MAIN.plotSize.y_min
        }

        if (y_val_max > MAIN.plotSize.y_max) {
            y_val_max = MAIN.plotSize.y_max
        }

        MAIN.zoom_used_y = true
        if (y_val_min == MAIN.plotSize.y_min && y_val_max == MAIN.plotSize.y_max)
            MAIN.zoom_used_y = false

        options.yaxes[0].min = y_val_min;
        options.yaxes[0].max = y_val_max;

        plot.setupGrid();
        plot.draw();
        CURSORS.updateLinesAndArrows()
        MAIN.drawInfoValues()
    };

    MAIN.changeX = function(value,x , steps) {
        var plot = MAIN.getPlot();
        if (!plot) {
            return;
        }
        var xaxis = plot.getAxes().xaxis;
        var curr_options = plot.getOptions();

        var x_val_min = xaxis.options.min + value
        var x_val_max = xaxis.options.max + value

        if (MAIN.scale){
            x_val_min = MAIN.getLogValue (curr_options.xaxes[0].min,curr_options.xaxes[0].max, steps , x)
            x_val_max = MAIN.getLogValue (curr_options.xaxes[0].min,curr_options.xaxes[0].max, steps, x + steps)
        }

        if ( x_val_min < xaxis.datamin) {
            x_val_min = xaxis.datamin
            x_val_max = xaxis.options.max
        }

        if ( x_val_max > xaxis.datamax) {
            x_val_min = xaxis.options.min
            x_val_max = xaxis.datamax
        }

        curr_options.xaxes[0].min = x_val_min;
        curr_options.xaxes[0].max = x_val_max;
        plot.setupGrid();
        plot.draw();
        CURSORS.updateLinesAndArrows()
        MAIN.drawInfoValues()
    };


    MAIN.changeXZoom = function(direction) {
        var plot = MAIN.getPlot();
        if (!plot) {
            return;
        }

        if (direction == undefined) return;

        var options = plot.getOptions();
        var xaxis = plot.getAxes().xaxis;

        var size = (xaxis.options.max - xaxis.options.min)
        if (isNaN(size)) return
        var koff = (direction == "+" ? 0.8 : 1.2)

        var x_val_min = xaxis.options.min -  (size * koff - size)
        var x_val_max = xaxis.options.max +  (size * koff - size)

        if (MAIN.scale){
            x_val_min = MAIN.getLogValue (options.xaxes[0].min,options.xaxes[0].max, size, size - size * koff)
            x_val_max = MAIN.getLogValue (options.xaxes[0].min,options.xaxes[0].max, size, size * koff)
        }

        if (x_val_min < MAIN.plotSize.x_min) {
            x_val_min = MAIN.plotSize.x_min
        }

        if (x_val_max > MAIN.plotSize.x_max) {
            x_val_max = MAIN.plotSize.x_max
        }

        MAIN.zoom_used_x = true
        if (x_val_min == MAIN.plotSize.x_min && x_val_max == MAIN.plotSize.x_max)
            MAIN.zoom_used_x = false

        options.xaxes[0].min = x_val_min;
        options.xaxes[0].max = x_val_max;

        plot.setupGrid();
        plot.draw();
        CURSORS.updateLinesAndArrows()
        MAIN.drawInfoValues()
    };

    MAIN.resetZoom = function() {
        if (MAIN.plotSize.x_min == undefined) return;
        if (MAIN.plotSize.x_max == undefined) return;

        var plot = MAIN.getPlot();
        if (plot === undefined) return;
        var curr_options = plot.getOptions();

        var xaxis = MAIN.graphCache.plot.getAxes().xaxis;
        var yaxis = MAIN.graphCache.plot.getAxes().yaxis;
        var size = (yaxis.datamax - yaxis.datamin) * 0.2;

        curr_options.xaxes[0].min = xaxis.datamin;
        curr_options.xaxes[0].max = xaxis.datamax;
        curr_options.yaxes[0].min = yaxis.datamin - size;
        curr_options.yaxes[0].max = yaxis.datamax + size;

        plot.setupGrid();
        plot.draw();
        MAIN.drawInfoValues()
        MAIN.zoom_used_x = false;
        MAIN.zoom_used_y = false;
    };

    MAIN.initPlot = function(update) {
        delete MAIN.graphCache;
        $('#bode_plot').remove();



        MAIN.graphCache = {};
        MAIN.graphCache.elem = $('<div id="bode_plot" class="plot" />').css($('#graph_bode_grid').css(['height', 'width'])).appendTo('#graph_bode');

        var options = {
            series: {
                shadowSize: 0
            },
            yaxes: [{
                    min: undefined,
                    max: undefined,
                    labelWidth: 30,
                    alignTicksWithAxis: 1,
                    position: "left",
                    tickFormatter: function(num, decimals = 3){
                        const rounded = Math.round(num * 1000) / 1000;
                        const difference = Math.abs(num - rounded);
                        if (difference < 1e-3) {
                            return rounded;
                        }
                        return num;
                    }
                }
            ],
            xaxis: {
                color: '#aaaaaa',
                tickColor: '#aaaaaa',
                ticks: getXAxisScale,
                transform: function(v) {
                    if (MAIN.scale)
                        return Math.log(v);
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

        var lastsig1 = [];
        lastsig1.push([1000,undefined])
        lastsig1.push([1000000,undefined])

        var data_points = [{ data: lastsig1, color: '#f3ec1a'}];
        MAIN.graphCache.plot = $.plot(MAIN.graphCache.elem, data_points, options);
        $('.flot-text').css('color', '#aaaaaa');
        MAIN.graphCache.plot.setData(data_points);
        MAIN.graphCache.elem.show();
        MAIN.graphCache.plot.resize();
        MAIN.graphCache.plot.setupGrid();
        MAIN.graphCache.plot.draw();

        MAIN.resizeCursorsHolder()
        CURSORS.updateCursors()
    }


    //Draw signals
    MAIN.drawSignals = function(SIG) {
        // console.log(SIG)
        MAIN.lastSignals = SIG;
        MAIN.updatePlot(false);
    };

    MAIN.updatePlot = function(force){

        if (MAIN.running || force){
            if (MAIN.graphCache == undefined) {
                MAIN.initPlot(false);
            }

            var lastsig1 = [];
            var curSig = MAIN.getCurrentSignalSettings();
            var freqSig = MAIN.lastSignals["IA_SIGNAL_FREQ"];
            var sigVal = MAIN.lastSignals[curSig.name];
            var min_y = 0;
            var max_y = 0;
            var min_x = 0;
            var max_x = 0;
            var suff = undefined;
            var yRecalc = []

            if (freqSig !== undefined && sigVal !== undefined && freqSig.size > 0){
                min_y = Math.min(...sigVal.value)
                max_y = Math.max(...sigVal.value)
                min_x = Math.min(...freqSig.value)
                max_x = Math.max(...freqSig.value)
                if (curSig.scale)
                    suff = getSuffix(Math.abs((max_y - min_y) / 2.0 + min_y))
                else{
                    suff = getSuffix(Math.pow(10,14))
                }
                for(var i = 0; i < freqSig.size; i++){
                    lastsig1.push([freqSig.value[i], sigVal.value[i] / suff.coff])
                    yRecalc.push(sigVal.value[i] / suff.coff)
                }
                min_y = min_y / suff.coff
                max_y = max_y / suff.coff
                var lab = curSig.label
                lab += curSig.nominal !== "" ? " [" + suff.name + curSig.nominal + "] " + suff.value :  " " + suff.value
                $("#amplitude-info").text(lab)
                MAIN.yAxisSuffix = suff

                var data_points = [{ data: lastsig1, color: '#f3ec1a' }];

                MAIN.graphCache.plot.setData(data_points);

                var xaxis = MAIN.graphCache.plot.getAxes().xaxis;
                var yaxis = MAIN.graphCache.plot.getAxes().yaxis;
                var size = (max_y - min_y) * 0.2;
                yaxis.options.max = max_y + size;
                yaxis.options.min = min_y - size;

                xaxis.options.min = min_x
                xaxis.options.max = max_x

                MAIN.graphCache.elem.show();
                MAIN.graphCache.plot.resize();
                MAIN.graphCache.plot.setupGrid();
                MAIN.graphCache.plot.draw();

                MAIN.resizeCursorsHolder()
                CURSORS.updateCursors()
                CURSORS.updateLinesAndArrows();
                MAIN.updatePlotSize()
                if (force){
                    var ext = RP_MATH.findAllExtremes(freqSig.value, yRecalc)
                    ext.scale = ""
                    RP_PLOT_INFO.setInfo(MAIN.graphCache.plot,MAIN.graphCache.plot.getCanvas().getContext("2d"),ext)
                    MAIN.drawInfoValues()
                }
            }
        }
    };

    MAIN.drawInfoValues = function(){
        if (MAIN.graphCache !== undefined) {
            RP_PLOT_INFO.draw(MAIN.graphCache.plot,MAIN.graphCache.plot.getCanvas().getContext("2d"))
        }
    }

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
            const scale = CLIENT.getValue("IA_SCALE")
            const xscale = CLIENT.getValue("IA_X_SCALE")
            if (scale !== undefined && xscale !== undefined){
                if (scale == false && xscale > 0){
                    CLIENT.parametersCache["IA_X_SCALE"] = { value:  0 };
                }
                if (scale == true && xscale === 0){
                    CLIENT.parametersCache["IA_X_SCALE"] = { value:  1 };
                }
            }

            CLIENT.parametersCache["IA_STATUS"] = { value: 0 };
            CLIENT.sendParameters();
            MAIN.updatePlot(true)
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
        MAIN.updatePlot(true);
        MAIN.zoom_used_x = false;
        MAIN.zoom_used_y = false;
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
            $("#INFO_DIALOG_IMG").attr('src','img/IA_shunt_connection.png')
        } else {
            if (lcr_state.attr('src') !== "img/green_led.png")
                lcr_state.attr("src", "img/green_led.png");
            $("#LCR_SHUNT_BOX").show()
            $("#SHUNT_BOX").hide()
            $("#INFO_DIALOG_IMG").attr('src','img/E_module_connection.png')
        }
    }

    MAIN.setXaxisScale = function(new_params){
        var param_name = 'IA_X_SCALE'
        var field = $('#' + param_name);
        if (field.is('select') || (field.is('input') && !field.is('input:radio')) || field.is('input:text')) {
            field.val(new_params[param_name].value);
            CLIENT.params.orig[param_name] = new_params[param_name];
            MAIN.updateXAxisScale()
            CURSORS.updateLinesAndArrows()
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
    MAIN.param_callbacks["IA_X_SCALE"] = MAIN.setXaxisScale;
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
        CLIENT.parametersCache["IA_START_FREQ"] = { value: $("#IA_START_FREQ").val() };
        CLIENT.parametersCache["IA_END_FREQ"] = { value: $('#IA_END_FREQ').val() };
        CLIENT.parametersCache["IA_STEPS"] = { value: $('#IA_STEPS').val() };
        CLIENT.parametersCache["IA_STATUS"] = { value: 1 };
        MAIN.scale = CLIENT.getValue("IA_SCALE");
        const xscale = CLIENT.getValue("IA_X_SCALE")
        if (MAIN.scale !== undefined && xscale !== undefined){
            if (MAIN.scale == false && xscale > 0){
                CLIENT.parametersCache["IA_X_SCALE"] = { value:  0 };
            }
            if (MAIN.scale == true && xscale === 0){
                CLIENT.parametersCache["IA_X_SCALE"] = { value:  1 };
            }
        }

        CLIENT.sendParameters();
        ev.preventDefault();
        MAIN.resetZoom()
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

    $('#IA_INFO').click(function () {
        $('#info_dialog').modal('show');
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


    // Draws grid on the lowest canvas layer
    MAIN.setCanvasSize = function() {
        var canvas_width = $('#main').width() - 110;
        var canvas_height = $('#main').height() - 22;// Math.round(canvas_width / 2);

        //Draw grid
        var ctx = $('#graph_bode_grid')[0].getContext('2d');

        // Set canvas size
        ctx.canvas.width = canvas_width;
        ctx.canvas.height = canvas_height;

        return;
    };


    // Bind to the window resize event to redraw the graph; trigger that event to do the first drawing
    $(window).resize(function() {

        if ($('#global_container').length === 0) return
        if ($('#main').length === 0) return


        var window_width = $('#root_window').width()
        var window_height = $('#root_window').height()

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

        MAIN.setCanvasSize()
        MAIN.initPlot(true);
        MAIN.updatePlot(true)
        MAIN.updateLinesAndArrows();

        MAIN.state.resized = true;


    }).resize();

    //Crash buttons
    $('#send_report_btn').on('click', function() { MAIN.formEmail() });
    $('#restart_app_btn').on('click', function() { location.reload() });

    MAIN.previousPageUrl = document.referrer;
    console.log(`Previously visited page URL: ${MAIN.previousPageUrl}`);
    const currentUrl = window.location.href;
    if (currentUrl === MAIN.previousPageUrl || MAIN.previousPageUrl === ''){
        MAIN.previousPageUrl = '/'
    }
    $("#back_button").attr("href", MAIN.previousPageUrl)
});